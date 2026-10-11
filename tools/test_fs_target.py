#!/usr/bin/env python3
"""Host check for firmware/services/fs_target.c: FAT16, FAT32 (behind an
MBR) and exFAT volumes filled by the kernel through a udisks loop mount (no
root), then every directory / file listed and read back must match os.walk.
Usage: python3 tools/test_fs_target.py"""
import os, re, struct, subprocess, tempfile, time

T = tempfile.mkdtemp()
EXE = os.path.join(T, 'test_fs_target')
subprocess.check_call(['cc', '-O2', '-Wall', '-Ifirmware', '-o', EXE,
                       'firmware/services/test_fs_target.c', 'firmware/services/fs_target.c'])


def fnv(b):
    h = 2166136261
    for x in b:
        h = ((h ^ x) * 16777619) & 0xFFFFFFFF
    return h


def fill(root):
    os.makedirs(f'{root}/Music/Artist Name With A Long Directory/Album (2024)')
    for i in range(300):                                  # dir spanning clusters
        open(f'{root}/Music/track number {i:03d} with a long name.flac', 'wb').write(os.urandom(i * 37))
    for i in range(0, 300, 2):                            # holes -> fragmented file
        os.remove(f'{root}/Music/track number {i:03d} with a long name.flac')
    open(f'{root}/Music/Artist Name With A Long Directory/Album (2024)/01 – Ünïcödé 日本.mp3', 'wb').write(os.urandom(3 * 1024 * 1024 + 5))
    open(f'{root}/README.TXT', 'wb').write(b'hello\n')
    open(f'{root}/lower.txt', 'wb').write(b'x' * 513)
    open(f'{root}/empty', 'wb').close()
    os.makedirs(f'{root}/EmptyDir')


def expect(root):
    out = []
    for d, dirs, files in os.walk(root):
        rel = '/SD' + d[len(root):]
        out += [f'D {rel}/{x}' for x in dirs]
        for x in files:
            b = open(os.path.join(d, x), 'rb').read()
            out.append(f'F {rel}/{x} {len(b)} {fnv(b):08x}')
    return sorted(out)


def build(mkfs, size_mb, mbr):
    fs = os.path.join(T, 'fs.img')
    open(fs, 'wb').truncate(size_mb << 20)
    subprocess.check_call(mkfs + [fs], stdout=subprocess.DEVNULL)
    dev = re.search(r'/dev/loop\d+', subprocess.check_output(
        ['udisksctl', 'loop-setup', '-f', fs], text=True)).group(0)
    try:
        mnt = ''
        for _ in range(20):
            mnt = subprocess.run(['findmnt', '-S', dev, '-no', 'TARGET'], capture_output=True, text=True).stdout.strip()
            if mnt:
                break
            time.sleep(0.25)
        if not mnt:
            mnt = re.search(r' at (\S+)', subprocess.check_output(['udisksctl', 'mount', '-b', dev], text=True)).group(1)
        fill(mnt)
        want = expect(mnt)
        subprocess.check_call(['udisksctl', 'unmount', '-b', dev], stdout=subprocess.DEVNULL)
    finally:
        subprocess.call(['udisksctl', 'loop-delete', '-b', dev],    # may be auto-cleared already
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if not mbr:
        return fs, want
    disk = os.path.join(T, 'disk.img')
    m = bytearray(512)
    m[0x1BE:0x1CE] = bytes([0, 0, 0, 0, 0x0C, 0, 0, 0]) + struct.pack('<II', 2048, (size_mb << 20) // 512)
    m[0x1FE:] = b'\x55\xaa'
    with open(disk, 'wb') as f:
        f.write(m)
        f.seek(2048 * 512)
        f.write(open(fs, 'rb').read())
    return disk, want


for label, mkfs, size, mbr in (('FAT16', ['/usr/sbin/mkfs.fat', '-F', '16', '-s', '1'], 32, False),
                               ('FAT32', ['/usr/sbin/mkfs.fat', '-F', '32', '-s', '1'], 64, True),
                               ('exFAT', ['/usr/sbin/mkfs.exfat'], 64, False)):
    img, want = build(mkfs, size, mbr)
    out = subprocess.check_output([EXE, img], text=True).splitlines()
    assert out[0] == f'mount 0 {label}', out[0]
    assert out[1] == 'root 1 SD', out[1]
    got = sorted(out[2:])
    assert got == want, f'{label}: ' + repr(sorted(set(got) ^ set(want))[:6])
    print(f'{label}: {len(got)} entries OK')
print('fs_target: OK')
