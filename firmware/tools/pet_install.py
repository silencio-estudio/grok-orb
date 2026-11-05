"""Verified private backup and guarded installation for the supported boards.

1.43C: application-only install into an existing Muse-compatible layout.
1.75C: the factory layout is not compatible, so the first install replaces
the boot area (bootloader, partition table, NVS/otadata/phy sectors) and the
application slot, only on the same device whose full flash was backed up and
only while its boot area still matches that backup. Later installs write the
application slot only. `restore` writes the complete verified backup back.
"""
from pathlib import Path
import argparse
import binascii
import contextlib
import hashlib
import json
import os
import struct
import sys
from datetime import datetime, timezone
sys.path.insert(0,str(Path(__file__).resolve().parent))
from pet_artifacts import verify_application, verify_boot_images, commit_backup, recover_backup

ROOT = Path(__file__).resolve().parents[2]
BOARD = 'Waveshare ESP32-S3-Touch-AMOLED-1.43C'
BOARD_175C = 'Waveshare ESP32-S3-Touch-AMOLED-1.75C'
FLASH_BYTES = 0x800000
TABLE_OFFSET = 0x10000
OTADATA_OFFSET, OTADATA_BYTES = 0x1d000, 0x2000
APP_OFFSET, APP_END = 0x20000, 0x400000
PARTITIONS = [('nvs',1,2,0x11000,0xc000),('otadata',1,0,0x1d000,0x2000),
              ('phy_init',1,1,0x1f000,0x1000),('ota_0',0,0x10,APP_OFFSET,0x3e0000),
              ('ota_1',0,0x11,0x400000,0x3e0000),('prod_data',1,0x40,0x7e0000,0x1000),
              ('prod_bak',1,0x41,0x7e1000,0x1000)]
# firmware/pet/partitions_175c.csv
PARTITIONS_175C = [('nvs',1,2,0x11000,0xc000),('otadata',1,0,0x1d000,0x2000),
                   ('phy_init',1,1,0x1f000,0x1000),('ota_0',0,0x10,APP_OFFSET,0x600000),
                   ('ota_1',0,0x11,0x620000,0x600000)]
PROFILES = {
    BOARD: {'flash':(FLASH_BYTES,),'partitions':PARTITIONS,'app_end':APP_END,
            'record':'device-backup.json','backup':'before-pet-full.bin',
            'manifest':'docs/pet-firmware-manifest.json','full_install':False},
    # Current units ship with 32 MiB; some listings and examples say 16 MiB.
    BOARD_175C: {'flash':(0x1000000,0x2000000),'partitions':PARTITIONS_175C,'app_end':0x620000,
                 'record':'device-backup-175c.json','backup':'before-pet-full-175c.bin',
                 'manifest':'docs/pet-firmware-manifest-175c.json','full_install':True},
}
BOOT_AREA = APP_OFFSET  # bootloader, partition table, nvs, otadata, phy_init
FACTORY_BOOT_END = 0x9000


def ensure_new(path):
    if path.exists():raise ValueError('BACKUP_EXISTS_DO_NOT_OVERWRITE')


def require_board(status,board=BOARD):
    if not status or status.get('board') != board:raise ValueError('WRONG_BOARD')


def require_table(table,partitions):
    found=[]
    for index in range(0,0x1000,32):
        magic,kind,sub,offset,size,label,flags=struct.unpack('<HBBII16sI',table[index:index+32])
        if magic in (0xffff,0xebeb):break
        if magic!=0x50aa or flags:raise ValueError('PARTITION_TABLE_UNSUPPORTED')
        found.append((label.split(b'\0',1)[0].decode('ascii'),kind,sub,offset,size))
    if found!=partitions:raise ValueError('PARTITION_LAYOUT_UNSUPPORTED')


def require_confirmed_ota0(metadata):
    if metadata==b'\xff'*OTADATA_BYTES:raise ValueError('OTA_RECORD_NOT_CONFIRMED')
    # Follow ESP-IDF 6.0.1's confirmed-record CRC and maximum-sequence rule.
    # Pending, aborted/corrupt records and active ota_1 remain unsupported.
    sequences=[]
    for offset in (0,0x1000):
        record=metadata[offset:offset+32]
        if record==b'\xff'*32:continue
        seq=struct.unpack_from('<I',record,0)[0]
        state,crc=struct.unpack_from('<II',record,24)
        expected=binascii.crc32(record[:4],0xffffffff)&0xffffffff
        if not 1<=seq<0xffffffff or state!=2 or crc!=expected:
            raise ValueError('OTA_RECORD_NOT_CONFIRMED')
        sequences.append(seq)
    if not sequences or (max(sequences)-1)%2!=0:
        raise ValueError('OTA_SELECTION_UNSUPPORTED')


def require_safe_layout(data,board=BOARD):
    if len(data) not in PROFILES[board]['flash']:raise ValueError('PARTITION_FLASH_SIZE_MISMATCH')
    require_table(data[TABLE_OFFSET:TABLE_OFFSET+0x1000],PROFILES[board]['partitions'])
    require_confirmed_ota0(data[OTADATA_OFFSET:OTADATA_OFFSET+OTADATA_BYTES])


def confirmed_otadata():
    """A valid, confirmed ESP-IDF otadata record selecting ota_0 (sequence 1)."""
    data=bytearray(b'\xff'*OTADATA_BYTES)
    struct.pack_into('<I',data,0,1)
    struct.pack_into('<II',data,24,2,binascii.crc32(struct.pack('<I',1),0xffffffff)&0xffffffff)
    return bytes(data)


def boot_area(bootloader,table):
    """Bootloader at 0x0 and the table at 0x10000; NVS, otadata and phy_init are
    reset so no data from the factory firmware is read as this app's settings."""
    if len(bootloader)>TABLE_OFFSET or len(table)>0x1000:raise ValueError('PET_IMAGE_NOT_VERIFIED')
    data=bytearray(b'\xff'*BOOT_AREA)
    data[0:len(bootloader)]=bootloader
    data[TABLE_OFFSET:TABLE_OFFSET+len(table)]=table
    data[OTADATA_OFFSET:OTADATA_OFFSET+OTADATA_BYTES]=confirmed_otadata()
    require_table(bytes(data[TABLE_OFFSET:TABLE_OFFSET+0x1000]),PARTITIONS_175C)
    return bytes(data)


def install_plan(current,backup,board):
    """'full' while the device's boot area is still the backed-up one, 'app'
    once it already carries this project's layout; anything else is refused."""
    if not PROFILES[board]['full_install']:raise ValueError('WRONG_BOARD')
    # Bootloader and the factory table at 0x8000; the factory app may still
    # write its own NVS data above them, which the backup already holds.
    if current[:FACTORY_BOOT_END]==backup[:FACTORY_BOOT_END]:return 'full'
    try:
        require_table(current[TABLE_OFFSET:TABLE_OFFSET+0x1000],PROFILES[board]['partitions'])
        require_confirmed_ota0(current[OTADATA_OFFSET:OTADATA_OFFSET+OTADATA_BYTES])
    except ValueError:raise ValueError('BOOT_METADATA_CHANGED') from None
    return 'app'


def verified_backup(root,manifest,device):
    path=(root/manifest['path']).resolve()
    sizes=PROFILES.get(manifest.get('board'),PROFILES[BOARD])['flash']
    if not path.is_relative_to((root/'.local').resolve()):raise ValueError('BACKUP_PATH_REFUSED')
    if (manifest.get('verified') is not True or manifest.get('bytes') not in sizes
            or manifest.get('deviceIdHash')!=device or path.stat().st_size!=manifest.get('bytes')
            or hashlib.sha256(path.read_bytes()).hexdigest()!=manifest.get('sha256')):
        raise ValueError('BACKUP_NOT_VERIFIED')
    return path


def main():
    parser=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('action',choices=['backup','install','restore'])
    parser.add_argument('--port',help='USB serial port; auto-selection requires exactly one board')
    parser.add_argument('--board',choices=list(PROFILES),
                        help='Physical board; required for backup. install/restore default to the 1.43C')
    args=parser.parse_args()
    if args.action=='backup' and args.board not in PROFILES:raise ValueError('BOARD_CONFIRMATION_REQUIRED')
    board=args.board or BOARD;profile=PROFILES[board]
    import esptool.cmds as cmds
    from serial.tools import list_ports
    sys.path.insert(0,str(ROOT/'firmware/tools/vendor'))
    import chat
    port=args.port or chat.pick_port()
    usb=next((p for p in list_ports.comports() if p.device==port),None)
    if not usb or not usb.serial_number:raise ValueError('DEVICE_ID_REQUIRED')
    device=hashlib.sha256(usb.serial_number.encode()).hexdigest()
    private=ROOT/'.local'
    if private.is_symlink():raise ValueError('PRIVATE_DIRECTORY_SYMLINK_REFUSED')
    private.mkdir(mode=0o700,exist_ok=True);os.chmod(private,0o700)
    record=private/profile['record']
    if args.action=='backup':
        if recover_backup(ROOT,record,device,profile['flash']):print('PET_BACKUP_COMMIT_RECOVERED');return
        ensure_new(record)
        path=private/profile['backup'];partial=path.with_suffix('.partial')
        ensure_new(path);ensure_new(partial)
    else:
        manifest=json.loads(record.read_text())
        require_board(manifest,board)
        path=verified_backup(ROOT,manifest,device)
        backup=path.read_bytes()
        if not profile['full_install']:require_safe_layout(backup,board)
        if args.action=='restore':
            if profile['full_install']:image=path
            else:
                image=private/'before-pet-application-slot.bin'
                image.write_bytes(backup[APP_OFFSET:APP_END]);os.chmod(image,0o600)
        else:
            built=json.loads((ROOT/profile['manifest']).read_text())
            image=verify_application(ROOT,built,board)
            if profile['full_install']:
                boot=private/'pet-boot-area-175c.bin'
                boot.write_bytes(boot_area(*verify_boot_images(ROOT,built)));os.chmod(boot,0o600)
    with (private/('pet-'+args.action+'.log')).open('w') as log,contextlib.redirect_stdout(log),contextlib.redirect_stderr(log):
        esp=cmds.detect_chip(port,baud=460800,connect_mode='default-reset')
        try:
            if esp.CHIP_NAME!='ESP32-S3':raise ValueError('WRONG_CHIP')
            esp=esp.run_stub()
            size=1<<((esp.flash_id()>>16)&0xff)
            if size not in profile['flash']:raise ValueError('WRONG_FLASH_SIZE')
            if args.action=='backup':
                cmds.read_flash(esp,0,size,str(partial),no_progress=True)
                if partial.stat().st_size!=size:raise ValueError('BACKUP_INCOMPLETE')
                cmds.verify_flash(esp,[(0,str(partial))])
                os.chmod(partial,0o600);data=partial.read_bytes()
                result={'board':board,'path':'.local/'+path.name,'bytes':len(data),'verified':True,
                        'sha256':hashlib.sha256(data).hexdigest(),'deviceIdHash':device,
                        'verifiedAt':datetime.now(timezone.utc).isoformat()}
                commit_backup(ROOT,partial,path,result,record);os.chmod(record,0o600)
            elif profile['full_install']:
                if size!=len(backup):raise ValueError('WRONG_FLASH_SIZE')
                if args.action=='restore':
                    cmds.write_flash(esp,[(0,str(image))]);cmds.verify_flash(esp,[(0,str(image))])
                else:
                    plan=install_plan(cmds.read_flash(esp,0,BOOT_AREA,no_progress=True),backup,board)
                    blocks=[(0,str(boot))] if plan=='full' else []
                    blocks.append((APP_OFFSET,str(image)))
                    cmds.write_flash(esp,blocks);cmds.verify_flash(esp,blocks)
                    print('PET_INSTALL_PLAN',plan)
            else:
                # Refuse changed boot metadata. Only the validated app slot is ever written.
                for offset,length in [(TABLE_OFFSET,0x1000),(OTADATA_OFFSET,OTADATA_BYTES)]:
                    if cmds.read_flash(esp,offset,length,no_progress=True)!=backup[offset:offset+length]:
                        raise ValueError('BOOT_METADATA_CHANGED')
                cmds.write_flash(esp,[(APP_OFFSET,str(image))]);cmds.verify_flash(esp,[(APP_OFFSET,str(image))])
        finally:
            try:esp.hard_reset()
            finally:esp._port.close()
    print('PET_'+args.action.upper()+'_VERIFIED')


if __name__=='__main__':
    try:main()
    except Exception as error:
        # Print only known validation codes; third-party errors may contain private data.
        code=str(error) if isinstance(error,ValueError) and str(error).isupper() else type(error).__name__
        print('PET_OPERATION_FAILED: '+code+'; see .local/pet-<action>.log',file=sys.stderr)
        raise SystemExit(1)
