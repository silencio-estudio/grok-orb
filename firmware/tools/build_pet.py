"""Build the standalone pet application without paths containing spaces or private config."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import hashlib
import tempfile
import json
from pet_artifacts import publish, TARGETS

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--idf', type=Path, default=Path(os.environ['IDF_PATH']) if os.environ.get('IDF_PATH') else None)
parser.add_argument('--board', choices=sorted(TARGETS), default='143c',
                    help='143c: Waveshare 1.43C (default); 175c: Waveshare 1.75C')
parser.add_argument('--stage', type=Path)
args = parser.parse_args()
suffix = '' if args.board == '143c' else '-' + args.board
default_stage = Path(tempfile.gettempdir())/('grok-pet-'+hashlib.sha256(str(root).encode()).hexdigest()[:12]+suffix)
stage = (args.stage or default_stage).resolve()
marker = stage / '.grok-pet-stage'
if stage.exists() and not marker.is_file():
    raise SystemExit('UNOWNED_BUILD_DIRECTORY')
if args.idf is None or not (args.idf / 'export.sh').is_file():
    raise SystemExit('ESP_IDF_601_REQUIRED_USE_IDF_OPTION')
stage.mkdir(parents=True, exist_ok=True)
marker.touch()
subprocess.run(['rsync', '-a', '--delete', '--exclude=managed_components', '--exclude=build',
                '--exclude=sdkconfig', '--exclude=sdkconfig.old', '--exclude=.grok-pet-*',
                str(root / 'firmware/pet') + '/', str(stage) + '/'], check=True)
# Board defaults are applied after the shared ones; a change to either, or to
# the board, regenerates sdkconfig.
defaults = ['sdkconfig.defaults'] + ([] if args.board == '143c' else ['sdkconfig.defaults.' + args.board])
defaults_hash = hashlib.sha256(args.board.encode() + b''.join((stage / name).read_bytes() for name in defaults)).hexdigest()
defaults_stamp = stage / '.grok-pet-defaults'
if not defaults_stamp.exists() or defaults_stamp.read_text() != defaults_hash:
    (stage / 'sdkconfig').unlink(missing_ok=True)
    defaults_stamp.write_text(defaults_hash)
env = os.environ.copy()
env['IDF_PATH'] = str(args.idf.resolve())
env['GROK_PET_SDKCONFIG_DEFAULTS'] = ';'.join(defaults)
subprocess.run(['bash', '-c', 'source "$IDF_PATH/export.sh" >/dev/null && exec python "$IDF_PATH/tools/idf.py" -B build -DIDF_TARGET=esp32s3 "-DSDKCONFIG_DEFAULTS=$GROK_PET_SDKCONFIG_DEFAULTS" build'],
               cwd=stage, env=env, check=True)
image = stage / 'build/grok-pet.bin'
if not 1024 <= image.stat().st_size <= TARGETS[args.board]['slot']:
    raise SystemExit('APPLICATION_SLOT_EXCEEDED')
if (stage / 'dependencies.lock').is_file():
    shutil.copy2(stage / 'dependencies.lock', root / 'firmware/pet/dependencies.lock')
boot = {} if args.board == '143c' else {
    'bootloader': stage / 'build/bootloader/bootloader.bin',
    'partition_table': stage / 'build/partition_table/partition-table.bin',
}
publish(root,image,hashlib.sha256((stage/'sdkconfig').read_bytes()).hexdigest(),args.board,**boot)
(root/'.local').mkdir(mode=0o700,exist_ok=True)
(root/'.local/build-info.json').write_text(json.dumps({'stage':str(stage)}))
print('PET_APPLICATION_BUILT', args.board, image.stat().st_size)

print('BUILD_ONLY: use firmware/tools/pet_install.py; do not run the generic idf.py flash command.')
