"""Durable image/manifest publication and recoverable backup commit; no hardware I/O."""
from pathlib import Path
import hashlib
import json
import os
import uuid

BOARD='Waveshare ESP32-S3-Touch-AMOLED-1.43C'
BOARD_175C='Waveshare ESP32-S3-Touch-AMOLED-1.75C'
# Per-board build outputs. The 1.43C keeps its original names and slot size.
TARGETS={
    '143c':{'board':BOARD,'manifest':'docs/pet-firmware-manifest.json','prefix':'grok-pet','slot':0x3e0000},
    '175c':{'board':BOARD_175C,'manifest':'docs/pet-firmware-manifest-175c.json','prefix':'grok-pet-175c','slot':0x600000},
}
BOARD_TARGETS={target['board']:key for key,target in TARGETS.items()}
EXCLUDED={'build','managed_components','sdkconfig','sdkconfig.old','__pycache__'}


def sync_dir(path):
    fd=os.open(path,os.O_RDONLY)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


def atomic_bytes(path,data,mode=0o644):
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    temporary=path.with_name('.'+path.name+'.'+uuid.uuid4().hex+'.tmp')
    try:
        with temporary.open('xb') as out:
            os.chmod(temporary,mode);out.write(data);out.flush();os.fsync(out.fileno())
        os.replace(temporary,path);sync_dir(path.parent)
    finally:
        temporary.unlink(missing_ok=True)


def atomic_json(path,value):
    atomic_bytes(path,(json.dumps(value,indent=2)+'\n').encode())


def source_digest(root):
    base=Path(root)/'firmware/pet';digest=hashlib.sha256()
    for path in sorted(base.rglob('*')):
        relative=path.relative_to(base)
        if not path.is_file() or any(part in EXCLUDED for part in relative.parts):
            continue
        name=relative.as_posix().encode();data=path.read_bytes()
        digest.update(len(name).to_bytes(4,'big'));digest.update(name)
        digest.update(len(data).to_bytes(8,'big'));digest.update(data)
    return digest.hexdigest()


def publish_part(root,prefix,name,data):
    sha=hashlib.sha256(data).hexdigest();relative='firmware/build/'+prefix+'-'+name+'-'+sha[:16]+'.bin'
    atomic_bytes(Path(root)/relative,data)
    return {'path':relative,'bytes':len(data),'sha256':sha}


def publish(root,image,sdkconfig_sha,target='143c',bootloader=None,partition_table=None):
    root=Path(root);data=Path(image).read_bytes();spec=TARGETS[target]
    if not 1024<=len(data)<=spec['slot'] or data[0]!=0xe9:
        raise ValueError('INVALID_APPLICATION')
    sha=hashlib.sha256(data).hexdigest();relative='firmware/build/'+spec['prefix']+'-'+sha[:16]+'.bin'
    atomic_bytes(root/relative,data)
    manifest={'board':spec['board'],'offset':0x20000,'bytes':len(data),'path':relative,'sha256':sha,
              'sourceSha256':source_digest(root),
              'dependenciesSha256':hashlib.sha256((root/'firmware/pet/dependencies.lock').read_bytes()).hexdigest(),
              'sdkconfigSha256':sdkconfig_sha}
    if bootloader is not None:
        # Boards installed from their factory layout also need these two images.
        boot=Path(bootloader).read_bytes();table=Path(partition_table).read_bytes()
        if not 1024<=len(boot)<0x10000 or boot[0]!=0xe9 or not 32<=len(table)<=0x1000 or table[:2]!=b'\xaa\x50':
            raise ValueError('INVALID_BOOT_IMAGES')
        manifest['bootloader']=publish_part(root,spec['prefix'],'bootloader',boot)
        manifest['partitionTable']=publish_part(root,spec['prefix'],'partition-table',table)
    atomic_json(root/spec['manifest'],manifest)
    atomic_bytes(root/('firmware/build/'+spec['prefix']+'.bin'),data)
    return manifest


def verify_application(root,manifest,board=BOARD):
    root=Path(root);path=(root/manifest['path']).resolve()
    if not path.is_relative_to((root/'firmware/build').resolve()):
        raise ValueError('APPLICATION_PATH_REFUSED')
    data=path.read_bytes();slot=TARGETS[BOARD_TARGETS[board]]['slot']
    if (manifest.get('board')!=board or manifest.get('offset')!=0x20000
            or not 1024<=len(data)<=slot or len(data)!=manifest.get('bytes') or data[0]!=0xe9
            or hashlib.sha256(data).hexdigest()!=manifest.get('sha256')
            or source_digest(root)!=manifest.get('sourceSha256')
            or hashlib.sha256((root/'firmware/pet/dependencies.lock').read_bytes()).hexdigest()!=manifest.get('dependenciesSha256')):
        raise ValueError('PET_IMAGE_NOT_VERIFIED')
    return path


def verify_boot_images(root,manifest):
    """Bootloader and partition table bytes recorded by publish(), re-hashed."""
    root=Path(root);parts=[]
    for key in ('bootloader','partitionTable'):
        part=manifest.get(key)
        if not isinstance(part,dict):raise ValueError('PET_IMAGE_NOT_VERIFIED')
        path=(root/part['path']).resolve()
        if not path.is_relative_to((root/'firmware/build').resolve()):raise ValueError('APPLICATION_PATH_REFUSED')
        data=path.read_bytes()
        if len(data)!=part.get('bytes') or hashlib.sha256(data).hexdigest()!=part.get('sha256'):
            raise ValueError('PET_IMAGE_NOT_VERIFIED')
        parts.append(data)
    return tuple(parts)


def commit_backup(root,partial,path,manifest,record):
    root=Path(root);partial=Path(partial);path=Path(path);record=Path(record)
    if record.exists() or path.exists():
        raise ValueError('BACKUP_EXISTS_DO_NOT_OVERWRITE')
    with partial.open('rb') as stream:
        os.fsync(stream.fileno())
    # The journal contains the verified digest, never flash/credential contents.
    atomic_json(record.with_suffix('.pending.json'),manifest)
    partial.rename(path);sync_dir(path.parent)
    atomic_json(record,manifest)
    record.with_suffix('.pending.json').unlink();sync_dir(record.parent)


def recover_backup(root,record,device,sizes=(0x800000,)):
    root=Path(root);record=Path(record);journal=record.with_suffix('.pending.json')
    if record.exists() or not journal.exists():
        return False
    manifest=json.loads(journal.read_text());path=(root/manifest['path']).resolve()
    if not path.is_relative_to((root/'.local').resolve()):
        raise ValueError('BACKUP_PATH_REFUSED')
    partial=path.with_suffix('.partial');candidate=path if path.exists() else partial
    data=candidate.read_bytes()
    if (manifest.get('verified') is not True or manifest.get('deviceIdHash')!=device
            or manifest.get('bytes') not in sizes or len(data)!=manifest.get('bytes')
            or hashlib.sha256(data).hexdigest()!=manifest.get('sha256')):
        raise ValueError('BACKUP_RECOVERY_REFUSED')
    if candidate==partial:
        partial.rename(path);sync_dir(path.parent)
    atomic_json(record,manifest);journal.unlink();sync_dir(record.parent)
    return True
