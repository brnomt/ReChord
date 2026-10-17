#!/usr/bin/env python3
"""Host check for firmware/services/fs_target.c: FAT16, FAT32 (behind an
MBR) and exFAT volumes filled by the kernel through a udisks loop mount (no
root), then every directory / file listed and read back must match os.walk.
Usage: python3 tools/test_fs_target.py"""
import os, re, struct, subprocess, tempfile, time

T = tempfile.mkdtemp()
EXE = os.path.join(T, 'test_fs_target')
subprocess.check_call(['cc', '-O2', '-Wall', '-Ifirmware', '-o', EXE,
                       'firmware/services/test_fs_target.c',
                       'firmware/services/fs_target.c',
                       'firmware/services/settings.c'])


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
    print('  build: make image', flush=True)
    fs = os.path.join(T, 'fs.img')
    open(fs, 'wb').truncate(size_mb << 20)
    subprocess.check_call(mkfs + [fs], stdout=subprocess.DEVNULL)
    dev = re.search(r'/dev/loop\d+', subprocess.check_output(
        ['udisksctl', 'loop-setup', '-f', fs], text=True, timeout=60)).group(0)
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
        subprocess.check_call(['udisksctl', 'unmount', '-b', dev], stdout=subprocess.DEVNULL,
                              timeout=60)
    finally:
        subprocess.call(['udisksctl', 'loop-delete', '-b', dev],    # may be auto-cleared already
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=30)
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
    print('phase', label, flush=True)
    img, want = build(mkfs, size, mbr)
    print('  build done; running', EXE, flush=True)
    lines = subprocess.check_output([EXE, img], text=True).splitlines()
    print('  test binary done', flush=True)
    assert lines[0] == f'mount 0 {label}', lines[0]
    assert lines[1] == 'root 1 SD', lines[1]
    got = sorted(l for l in lines[2:] if l[:2] in ('D ', 'F ', 'E '))
    assert got == want, f'{label}: ' + repr(sorted(set(got) ^ set(want))[:6])

    # In-place rewrite lines ("W <tag> <payload...>"): lower.txt is 513 B
    # (2 clusters of 512 B on the FAT images), so 1000 B fits and 1 MiB must
    # be refused. exFAT refuses the whole path (entry-set checksum).
    w = {}
    for l in lines:
        if l.startswith('W '):
            p = l.split()
            w[p[1]] = p[2:]
    payload = bytes(((i * 31 + 7) & 0xFF) for i in range(1000))
    assert w['set'] == ['0'], w
    assert w['miss'][0] != '0' and w['dir'][0] != '0', w
    if label == 'exFAT':
        for tag in ('big', 'put', 'cut', 'put2'):
            assert w[tag] == [str(-4)], w         # refused, not corrupted
        assert w['back'] == w['back3'] and w['back'][0] == '513', w
    else:
        assert w['big'] == [str(-4)], w          # FS_ERR_UNSUPPORTED
        assert w['put'] == ['0'], w
        assert w['back'] == ['1000', f'{fnv(payload):08x}', '1000'], w
        assert w['cut'] == ['0'], w
        assert w['back2'] == ['100', f'{fnv(payload[:100]):08x}', '100'], w
        assert w['put2'] == ['0'], w
        assert w['back3'] == ['1000', f'{fnv(payload):08x}', '1000'], w

    # The image must stay filesystem-consistent (no FAT/layout writes).
    fsck = '/usr/sbin/fsck.exfat' if label == 'exFAT' else '/usr/sbin/fsck.fat'
    if os.path.exists(fsck) and not mbr:
        subprocess.check_call([fsck, '-n', img], stdout=subprocess.DEVNULL)
        print(f'{label}: rewrite OK, fsck clean')
    elif os.path.exists(fsck) and mbr:
        part = os.path.join(T, 'part.img')
        with open(img, 'rb') as f:
            f.seek(2048 * 512)
            open(part, 'wb').write(f.read())
        subprocess.check_call([fsck, '-n', part], stdout=subprocess.DEVNULL)
        print(f'{label}: rewrite OK, fsck clean')
    else:
        print(f'{label}: rewrite OK (no fsck tool)')

# ---- creation (allocation) phase -------------------------------------------
# Fresh images only: fs_target_mkdir + fs_target_file_create write FAT metadata
# (free-cluster search, both FAT copies, directory entries), so the kernel has
# to agree with the result: mount it, look at the files, then fsck.
mkfs_fat = '/usr/sbin/mkfs.fat'
if os.path.exists(EXE) and os.path.exists(mkfs_fat):
    for label, args, size, mbr in (('FAT16', ['-F', '16', '-s', '1'], 32, False),
                                   ('FAT32', ['-F', '32', '-s', '1'], 64, True)):
        fresh = os.path.join(T, f'fresh-{label}.img')
        open(fresh, 'wb').truncate(size << 20)
        subprocess.check_call([mkfs_fat] + args + [fresh], stdout=subprocess.DEVNULL)
        if mbr:
            disk = os.path.join(T, f'fresh-{label}-disk.img')
            m = bytearray(512)
            m[0x1BE:0x1CE] = bytes([0, 0, 0, 0, 0x0C, 0, 0, 0]) + \
                struct.pack('<II', 2048, (size << 20) // 512)
            m[0x1FE:] = b'\x55\xaa'
            with open(disk, 'wb') as f:
                f.write(m)
                f.seek(2048 * 512)
                f.write(open(fresh, 'rb').read())
            fresh = disk
        lines = subprocess.check_output([EXE, fresh, 'create'], text=True).splitlines()
        assert lines[0] == f'mount 0 {label}', lines
        c = {}
        for l in lines:
            if l.startswith('C '):
                p = l.split()
                c[p[1]] = p[2:]
        assert c['mkdir'] == ['0'], c
        assert c['cfg'] == ['0'] and c['state'] == ['0'], c
        assert c['put'] == ['0'], c
        assert c['again'] == ['0', '0', '0'], c                 # idempotent
        assert c['clash'] != ['0'], c                           # dir isn't a file
        assert c['utf8'] == [str(-4)], c                        # ASCII-only names
        assert c['fill'] == ['20'], c
        assert c['read'] == ['1'], c
        # Append: grow the chain, keep the bytes before the append point.
        pay = bytes(((i * 17 + 3) & 0xFF) for i in range(600))
        want_app = b'hello' + pay + pay
        assert c['mkapp'] == ['0'], c
        assert c['ap1'] == ['0'] and c['ap2'] == ['0'] and c['ap3'] == ['0'], c
        assert c['ap0'] == ['0'], c                 # len 0 is a no-op
        assert c['apmiss'] != ['0'] and c['apdir'] != ['0'], c
        assert c['rapp'][0] == str(len(want_app)), c['rapp']
        assert c['rapp'][1] == f'{fnv(want_app):08x}', c['rapp']
        assert c['rapp'][2] == str(len(want_app)), c['rapp']
        # Patch: 'helXYZ' + payload*2, same length, out-of-range refused.
        want_app = b'helXYZ' + pay[1:] + pay   # patch overwrote 'llo'
        assert c['pt1'] == ['0'], c
        assert c['ptbig'] != ['0'], c
        assert c['rapp'][1] == f'{fnv(want_app):08x}', c['rapp']
        assert c['rapp'][0] == str(len(want_app)), c['rapp']
        # The cfg now holds exactly the default text; state is 4096 zeros.
        size_cfg = int(c['rcfg'][0])
        assert 100 < size_cfg < 512, c['rcfg']
        assert c['rcfg'][2] == str(size_cfg), c['rcfg']
        assert c['rstate'][0] == '4096' and c['rstate'][2] == '4096', c['rstate']

        # The kernel's view: the directory, its entries and the cfg's text.
        dev = re.search(r'/dev/loop\d+', subprocess.check_output(
            ['udisksctl', 'loop-setup', '-f', fresh], text=True)).group(0)
        try:
            mnt = ''
            for _ in range(20):
                mnt = subprocess.run(['findmnt', '-S', dev, '-no', 'TARGET'],
                                     capture_output=True, text=True).stdout.strip()
                if mnt:
                    break
                time.sleep(0.25)
            if not mnt:
                mnt = re.search(r' at (\S+)', subprocess.check_output(
                    ['udisksctl', 'mount', '-b', dev], text=True)).group(1)
            names = sorted(os.listdir(f'{mnt}/.rechord'))
            assert names == sorted(['rechord.cfg', 'state', 'LOG', 'app'] +
                                   [f'f{i:02d}' for i in range(20)]), names
            assert open(f'{mnt}/.rechord/app', 'rb').read() == want_app
            body = open(f'{mnt}/.rechord/rechord.cfg', 'rb').read()
            assert len(body) == size_cfg and b'play=' in body and b'\x00' not in body, body[:40]
            assert open(f'{mnt}/.rechord/state', 'rb').read() == bytes(4096)
            subprocess.check_call(['udisksctl', 'unmount', '-b', dev],
                                  stdout=subprocess.DEVNULL, timeout=60)
        finally:
            subprocess.call(['udisksctl', 'loop-delete', '-b', dev],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=30)

        # fsck: structural errors fail, the deliberate cfg slack is expected
        # (a preallocated file whose size is smaller than its chain; the
        # device never runs a repair, see docs/HANDOVER.md).
        if os.path.exists('/usr/sbin/fsck.fat'):
            part = fresh
            if mbr:
                part = os.path.join(T, f'fresh-{label}-part.img')
                with open(fresh, 'rb') as f:
                    f.seek(2048 * 512)
                    open(part, 'wb').write(f.read())
            out = subprocess.run(['/usr/sbin/fsck.fat', '-n', part],
                                 capture_output=True, text=True)
            # Allow only: fsck's own chatter, the summary, and the ONE expected
            # finding -- the cfg is preallocated (64 KiB) so its chain is longer
            # than its size. Anything else (cross-links, FAT mismatch, bad
            # clusters, ...) fails the test.
            allow_prefix = ('fsck.fat', 'Checking', 'Free cluster', 'Currently, ',
                            'FAT32', f'{part}:')
            unexpected = []
            for l in (out.stdout + out.stderr).splitlines():
                if not l.strip() or l == 'Leaving filesystem unchanged.':
                    continue
                if l.startswith(allow_prefix):
                    continue
                if '.rechord' in l and ('RECHORD.CFG' in l or 'rechord.cfg' in l):
                    continue                    # the file the note is about
                if 'File size is' in l or 'Truncating' in l:
                    continue                    # the slack note itself
                unexpected.append(l)
            assert not unexpected, unexpected
            assert 'File size is' in out.stdout, 'expected the cfg slack note'
            print(f'{label}: create OK, fsck clean (cfg slack noted)')
        else:
            print(f'{label}: create OK (no fsck tool)')
print('fs_target: OK')
