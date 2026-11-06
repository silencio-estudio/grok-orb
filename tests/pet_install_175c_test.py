"""Waveshare 1.75C installer guards: layout, first install from the factory
layout, later app-only installs and full restore. No hardware is touched."""
import hashlib
import json
import pathlib
import re
import struct
import sys
import tempfile
import types
import unittest
from unittest.mock import patch

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'firmware/tools'))
import pet_install as m
from pet_artifacts import TARGETS, publish

MIB = 0x100000


def table(entries):
    data = bytearray(b'\xff' * 0xc00)
    for i, (name, kind, sub, offset, size) in enumerate(entries):
        data[i * 32:(i + 1) * 32] = struct.pack('<HBBII16sI', 0x50aa, kind, sub, offset, size, name.encode(), 0)
    return bytes(data)


def factory_image(size=32 * MIB):
    """Factory-like flash: bootloader at 0 and the stock table at 0x8000."""
    data = bytearray(b'\xff' * size)
    data[0:4] = b'\xe9\x03\x02\x5f'
    stock = [('nvsfactory', 1, 2, 0x9000, 0x32000), ('nvs', 1, 2, 0x3b000, 0xd2000), ('otadata', 1, 0, 0x10d000, 0x2000),
             ('phy_init', 1, 1, 0x10f000, 0x1000), ('factory', 0, 0, 0x110000, 0x900000)]
    data[0x8000:0x8000 + 0xc00] = table(stock)
    data[0x9000:0x9010] = b'factory-nvs-data'
    return data


BOOTLOADER = b'\xe9\x03\x02\x4f' + b'b' * 20000
TABLE = table(m.PARTITIONS_175C)


class LayoutTests(unittest.TestCase):
    def test_partition_csv_and_sdkconfig_match_the_installer(self):
        rows = []
        for line in (ROOT / 'firmware/pet/partitions_175c.csv').read_text().splitlines():
            if not line.strip() or line.startswith('#'):
                continue
            name, kind, sub, offset, size = [x.strip() for x in line.split(',')[:5]]
            kinds = {'app': 0, 'data': 1}
            subs = {'nvs': 2, 'ota': 0, 'phy': 1, 'ota_0': 0x10, 'ota_1': 0x11}
            rows.append((name, kinds[kind], subs[sub] if sub in subs else int(sub, 16), int(offset, 16), int(size, 16)))
        self.assertEqual(rows, m.PARTITIONS_175C)
        ota0 = next(r for r in rows if r[0] == 'ota_0')
        self.assertEqual(ota0[3], m.APP_OFFSET)
        self.assertEqual(ota0[4], TARGETS['175c']['slot'])
        self.assertEqual(ota0[3] + ota0[4], m.PROFILES[m.BOARD_175C]['app_end'])
        self.assertLessEqual(max(r[3] + r[4] for r in rows), min(m.PROFILES[m.BOARD_175C]['flash']))
        config = (ROOT / 'firmware/pet/sdkconfig.defaults.175c').read_text()
        self.assertIn('CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y', config)
        self.assertIn('CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions_175c.csv"', config)
        base = (ROOT / 'firmware/pet/sdkconfig.defaults').read_text()
        self.assertEqual(int(re.search(r'CONFIG_PARTITION_TABLE_OFFSET=(0x[0-9a-fA-F]+)', base)[1], 16), m.TABLE_OFFSET)

    def test_boot_area_resets_settings_and_selects_ota_zero(self):
        area = m.boot_area(BOOTLOADER, TABLE)
        self.assertEqual(len(area), m.APP_OFFSET)
        self.assertEqual(area[:len(BOOTLOADER)], BOOTLOADER)
        self.assertEqual(area[len(BOOTLOADER):m.TABLE_OFFSET], b'\xff' * (m.TABLE_OFFSET - len(BOOTLOADER)))
        self.assertEqual(area[0x11000:0x1d000], b'\xff' * 0xc000)
        m.require_confirmed_ota0(area[0x1d000:0x1f000])
        self.assertEqual(struct.unpack_from('<I', area, 0x1d000 + 28)[0], 1195612314)
        with self.assertRaises(ValueError):
            m.boot_area(b'\xe9' * 0x10001, TABLE)
        with self.assertRaises(ValueError):
            m.boot_area(BOOTLOADER, table(m.PARTITIONS))

    def test_install_plan_only_from_backed_up_or_own_layout(self):
        backup = bytes(factory_image())
        self.assertEqual(m.install_plan(backup[:m.BOOT_AREA], backup, m.BOARD_175C), 'full')
        # The factory app may have written its NVS since the backup.
        current = bytearray(backup[:m.BOOT_AREA]); current[0x9000] ^= 1
        self.assertEqual(m.install_plan(bytes(current), backup, m.BOARD_175C), 'full')
        self.assertEqual(m.install_plan(m.boot_area(BOOTLOADER, TABLE), backup, m.BOARD_175C), 'app')
        changed = bytearray(backup[:m.BOOT_AREA]); changed[0x8010] ^= 1
        with self.assertRaisesRegex(ValueError, 'BOOT_METADATA_CHANGED'):
            m.install_plan(bytes(changed), backup, m.BOARD_175C)
        ours = bytearray(m.boot_area(BOOTLOADER, TABLE)); ours[0x1d000] = 2
        with self.assertRaisesRegex(ValueError, 'BOOT_METADATA_CHANGED'):
            m.install_plan(bytes(ours), backup, m.BOARD_175C)
        with self.assertRaisesRegex(ValueError, 'WRONG_BOARD'):
            m.install_plan(backup[:m.BOOT_AREA], backup, m.BOARD)

    def test_backup_sizes_follow_the_recorded_board(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d); (root / '.local').mkdir()
            for size, ok in [(32 * MIB, True), (16 * MIB, True), (8 * MIB, False)]:
                blob = b'\xff' * size; path = root / '.local/flash.bin'; path.write_bytes(blob)
                manifest = {'board': m.BOARD_175C, 'path': '.local/flash.bin', 'bytes': size, 'verified': True,
                            'sha256': hashlib.sha256(blob).hexdigest(), 'deviceIdHash': 'device'}
                if ok:
                    self.assertEqual(m.verified_backup(root, manifest, 'device'), path.resolve())
                else:
                    with self.assertRaises(ValueError):
                        m.verified_backup(root, manifest, 'device')


class FlowTests(unittest.TestCase):
    def run_tool(self, root, action, current, flash_id=0x190000):
        serial = 'test-1.75c'; calls = []; flash = bytearray(current)

        class ESP:
            CHIP_NAME = 'ESP32-S3'
            _port = types.SimpleNamespace(close=lambda: None)
            def run_stub(self): return self
            def flash_id(self): return flash_id
            def hard_reset(self): calls.append('reset')

        def write_flash(esp, blocks):
            for offset, path in blocks:
                data = pathlib.Path(path).read_bytes(); flash[offset:offset + len(data)] = data
                calls.append(('write', offset, len(data)))
        cmds = types.ModuleType('esptool.cmds')
        cmds.detect_chip = lambda *a, **kw: ESP()
        cmds.write_flash = write_flash
        cmds.verify_flash = lambda esp, blocks: calls.append(('verify', [b[0] for b in blocks]))
        def read_flash(esp, offset, size, output=None, **kw):
            data = bytes(flash[offset:offset + size])
            if output is None:
                return data
            pathlib.Path(output).write_bytes(data)
        cmds.read_flash = read_flash
        esptool = types.ModuleType('esptool'); esptool.cmds = cmds
        chat = types.ModuleType('chat'); chat.pick_port = lambda: 'fake'
        serial_module = types.ModuleType('serial'); tools = types.ModuleType('serial.tools'); ports = types.ModuleType('serial.tools.list_ports')
        ports.comports = lambda: [types.SimpleNamespace(device='fake', serial_number=serial)]
        tools.list_ports = ports; serial_module.tools = tools
        argv = ['pet_install', action, '--board', m.BOARD_175C]
        with patch.object(m, 'ROOT', root), patch.object(sys, 'argv', argv), patch.dict(sys.modules, {
                'esptool': esptool, 'esptool.cmds': cmds, 'chat': chat, 'serial': serial_module,
                'serial.tools': tools, 'serial.tools.list_ports': ports}):
            m.main()
        return calls, bytes(flash), hashlib.sha256(serial.encode()).hexdigest()

    def project(self, d):
        root = pathlib.Path(d)
        for path in ['.local', 'docs', 'firmware/pet', 'firmware/build']:
            (root / path).mkdir(parents=True, exist_ok=True)
        (root / 'firmware/pet/main.c').write_text('source')
        (root / 'firmware/pet/dependencies.lock').write_text('locked')
        (root / 'app.bin').write_bytes(b'\xe9' + b'a' * 4095)
        (root / 'bootloader.bin').write_bytes(BOOTLOADER)
        (root / 'table.bin').write_bytes(TABLE)
        publish(root, root / 'app.bin', 'sdk', '175c', bootloader=root / 'bootloader.bin', partition_table=root / 'table.bin')
        return root

    def test_backup_first_install_upgrade_and_restore(self):
        with tempfile.TemporaryDirectory() as d:
            root = self.project(d); factory = factory_image()
            calls, flash, device = self.run_tool(root, 'backup', factory)
            record = json.loads((root / '.local/device-backup-175c.json').read_text())
            self.assertEqual((record['board'], record['bytes'], record['deviceIdHash']), (m.BOARD_175C, 32 * MIB, device))
            self.assertFalse((root / '.local/device-backup.json').exists())
            self.assertEqual(calls, [('verify', [0]), 'reset'])

            calls, flash, _ = self.run_tool(root, 'install', factory)
            self.assertEqual(calls, [('write', 0, m.BOOT_AREA), ('write', m.APP_OFFSET, 4096), ('verify', [0, m.APP_OFFSET]), 'reset'])
            self.assertEqual(flash[m.APP_OFFSET:m.APP_OFFSET + 4096], b'\xe9' + b'a' * 4095)
            m.require_table(flash[m.TABLE_OFFSET:m.TABLE_OFFSET + 0x1000], m.PARTITIONS_175C)
            self.assertEqual(flash[0x9000:0x9010], b'\xff' * 16)

            calls, flash, _ = self.run_tool(root, 'install', flash)
            self.assertEqual(calls, [('write', m.APP_OFFSET, 4096), ('verify', [m.APP_OFFSET]), 'reset'])

            calls, flash, _ = self.run_tool(root, 'restore', flash)
            self.assertEqual(calls, [('write', 0, 32 * MIB), ('verify', [0]), 'reset'])
            self.assertEqual(flash, bytes(factory))

    def test_refuses_changed_device_and_wrong_flash_size(self):
        with tempfile.TemporaryDirectory() as d:
            root = self.project(d); factory = factory_image()
            self.run_tool(root, 'backup', factory)
            changed = bytearray(factory); changed[0x100] ^= 1
            with self.assertRaisesRegex(ValueError, 'BOOT_METADATA_CHANGED'):
                self.run_tool(root, 'install', changed)
            with self.assertRaisesRegex(ValueError, 'WRONG_FLASH_SIZE'):
                self.run_tool(root, 'install', factory, flash_id=0x170000)
            with self.assertRaisesRegex(ValueError, 'WRONG_FLASH_SIZE'):
                self.run_tool(root, 'restore', factory, flash_id=0x180000)

    def test_rebuilt_source_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            root = self.project(d); factory = factory_image()
            self.run_tool(root, 'backup', factory)
            (root / 'firmware/pet/main.c').write_text('changed')
            with self.assertRaisesRegex(ValueError, 'PET_IMAGE_NOT_VERIFIED'):
                self.run_tool(root, 'install', factory)


if __name__ == '__main__':
    unittest.main()
