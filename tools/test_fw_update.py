#!/usr/bin/env python3
"""Host check for firmware/services/fw_update.c: real mkfs.fat FAT32 volume
behind an MBR, update image placed contiguously, then find/install/retire.
Also: a corrupted image must be rejected with FWU_ECRC (-5) and leave the
firmware area untouched.   Usage: python3 tools/test_fw_update.py [IMG]"""
import hashlib, os, struct, subprocess, sys, tempfile

IMG = sys.argv[1] if len(sys.argv) > 1 else 'build/ReChord_AP5M.IMG'
FW_BASE, FW = 0x2000, 0x22000          # this unit's IDB system size
VOL = FW_BASE + FW
PART = 2048                            # partition offset inside the volume
FS_SECTORS = 80 * 2048                 # 80 MiB FAT32, 1-sector clusters
T = tempfile.mkdtemp()
EXE = os.path.join(T, 'test_fw_update')

subprocess.check_call(['cc', '-O2', '-Wall', '-Ifirmware', '-o', EXE,
                       'firmware/services/test_fw_update.c', 'firmware/services/fw_update.c'])


def make_disk(payload):
    fs = os.path.join(T, 'fs.img')
    open(fs, 'wb').truncate(FS_SECTORS * 512)
    subprocess.check_call(['/usr/sbin/mkfs.fat', '-F', '32', '-s', '1', '-n', 'ECHO MINI', fs],
                          stdout=subprocess.DEVNULL)
    v = bytearray(open(fs, 'rb').read())
    rsvd, nfats, fatsz, root = (struct.unpack_from('<H', v, 0x0E)[0], v[0x10],
                                struct.unpack_from('<I', v, 0x24)[0],
                                struct.unpack_from('<I', v, 0x2C)[0])
    data = rsvd + nfats * fatsz
    first, n = root + 1, (len(payload) + 511) // 512
    for k in range(nfats):
        fat = (rsvd + k * fatsz) * 512
        for i in range(n):
            struct.pack_into('<I', v, fat + 4 * (first + i), first + i + 1 if i < n - 1 else 0x0FFFFFFF)
    v[(data + first - 2) * 512:(data + first - 2) * 512 + len(payload)] = payload
    d = (data + root - 2) * 512
    while v[d] not in (0, 0xE5):       # skip the volume-label entry
        d += 32
    ent = b'HIFIEC90IMG' + bytes([0x20]) + bytes(8) + struct.pack('<H', first >> 16) + bytes(4) + \
        struct.pack('<HI', first & 0xFFFF, len(payload))
    v[d:d + 32] = ent
    fsinfo = struct.unpack_from('<H', v, 0x30)[0] * 512
    struct.pack_into('<II', v, fsinfo + 0x1E8, 0xFFFFFFFF, 0xFFFFFFFF)   # free count unknown
    mbr = bytearray(512)
    mbr[0x1BE:0x1CE] = bytes([0, 0, 0, 0, 0x0C, 0, 0, 0]) + struct.pack('<II', PART, FS_SECTORS)
    mbr[0x1FE:] = b'\x55\xaa'
    disk = os.path.join(T, 'disk.img')
    with open(disk, 'wb') as f:
        f.truncate((VOL + PART + FS_SECTORS) * 512)
        f.seek(FW_BASE * 512); f.write(b'\x5a' * FW * 512)   # "old firmware"
        f.seek(VOL * 512); f.write(mbr)
        f.seek((VOL + PART) * 512); f.write(v)
    return disk


def run(disk):
    out = subprocess.check_output([EXE, disk, hex(VOL), hex(FW)], text=True)
    print(out, end='')
    return out, open(disk, 'rb').read()


img = open(IMG, 'rb').read()
end = (struct.unpack_from('<I', img, 0x1F4)[0] + 0x104000) & ~0xFFFFF

out, d = run(make_disk(img))
assert f'find 0 HIFIEC90.IMG size {len(img)} fw_end {end:x} sha {hashlib.sha256(img).hexdigest()}' in out
assert 'install 0' in out and 'retire 0' in out and 'again 1' in out
for slot in (0, FW // 2):
    o = (FW_BASE + slot) * 512
    assert d[o:o + end] == img[:end], f'slot at {slot:#x} differs'
assert d[(FW_BASE + end // 512) * 512] == 0x5a                       # slot 1 tail untouched
assert d[(FW_BASE + FW // 2 + end // 512) * 512] == 0x5a             # slot 2 tail untouched
fs = os.path.join(T, 'fs_after.img')
open(fs, 'wb').write(d[(VOL + PART) * 512:])
subprocess.check_call(['/usr/sbin/fsck.fat', '-n', fs])
assert b'HIFIEC90OLD' in d[(VOL + PART) * 512:]

bad = bytearray(img); bad[0x10000] ^= 1
out, d = run(make_disk(bytes(bad)))
assert out.startswith('find -5 HIFIEC90.IMG')
assert d[FW_BASE * 512:(FW_BASE + FW) * 512] == b'\x5a' * FW * 512
print('fw_update: OK')
