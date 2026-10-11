#!/usr/bin/env python3
"""HOOK TEST image: verbatim reference fw1 + `bl ref_hook` (docs/HANDOVER.md).

Patches ONLY the payload: the `bl 0x03051DD8` at RAM 0x03050B3C and the hook
blob in the zero run. Header and the -3 self-copy stay byte-identical; only
the trailer CRC is recomputed. Usage:
    make_hook_img.py hook.bin hook.elf -o build/ReChord_HOOK.IMG
"""
import argparse
import hashlib
import subprocess
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from make_fw1 import rknano_crc32  # noqa: E402

REF = Path("community/refcfw/HIFIEC90.IMG")
REF_MD5 = "4c333e193739512bb4bb706495523955"
LOAD = 0x0304F298                 # RAM = LOAD + IMG offset (PHASE A audit)
CALL_SITE = 0x03050B3C
OLD_TARGET = 0x03051DD8
ZERO_RUN = (0x03064280, 0x03074B70)
# reset-vector words in the reference image (RAM addr -> original bytes):
# VT @fw1 base, VT heading the .data image, VT right after the .data image.
VECTORS = {0x03050004: struct.pack("<I", 0x0306296F),
           0x03074B74: struct.pack("<I", 0x030500E5),
           0x03076670: struct.pack("<I", 0x030500E5)}


def bl(src: int, dst: int) -> bytes:
    off = dst - (src + 4)
    assert -(1 << 24) <= off < (1 << 24) and not off & 1
    s = (off >> 24) & 1
    i1, i2 = (off >> 23) & 1, (off >> 22) & 1
    j1, j2 = (~(i1 ^ s)) & 1, (~(i2 ^ s)) & 1
    hw1 = 0xF000 | (s << 10) | ((off >> 12) & 0x3FF)
    hw2 = 0xD000 | (j1 << 13) | (j2 << 11) | ((off >> 1) & 0x7FF)
    return struct.pack("<HH", hw1, hw2)


def sym(elf: str, name: str) -> int:
    out = subprocess.run(["arm-none-eabi-nm", elf], capture_output=True,
                         text=True, check=True).stdout
    for line in out.splitlines():
        a, _, n = line.split()
        if n == name:
            return int(a, 16)
    raise SystemExit(f"{name} not in {elf}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("bin")
    ap.add_argument("elf")
    ap.add_argument("-o", required=True)
    ap.add_argument("--early", action="store_true",
                    help="patch bl 0x030514C0 at 0x03050B12 (main entry)")
    ap.add_argument("--vector", type=lambda s: int(s, 0), action="append",
                    help="RAM address of a reset-vector word to point at ChainEntry "
                         "(repeatable; see VECTORS)")
    a = ap.parse_args()
    site_ram, old = (0x03050B12, 0x030514C0) if a.early else (CALL_SITE, OLD_TARGET)

    ref = REF.read_bytes()
    if hashlib.md5(ref).hexdigest() != REF_MD5:
        raise SystemExit("reference image md5 mismatch")
    blob = Path(a.bin).read_bytes()
    if a.vector:
        base, entry = sym(a.elf, "ap_vectors"), sym(a.elf, "ChainEntry")
        patches = [(v, struct.pack("<I", entry | 1), VECTORS[v]) for v in a.vector]
    else:
        entry = base = sym(a.elf, "ref_hook")  # .text.entry heads the blob
        patches = [(site_ram, bl(site_ram, entry), bl(site_ram, old))]
    img = lambda ram: ram - LOAD  # noqa: E731

    sites = [img(v) for v, _, _ in patches]
    for (v, _, expect), site in zip(patches, sites):
        assert ref[site:site + 4] == expect, f"patch site 0x{v:08X} moved"
    assert ZERO_RUN[0] <= base and base + len(blob) <= ZERO_RUN[1], "hook too big"
    assert not any(ref[img(ZERO_RUN[0]):img(ZERO_RUN[1])]), "zero run not zero"

    out = bytearray(ref)
    for (_, patch, _), site in zip(patches, sites):
        out[site:site + 4] = patch
    out[img(base):img(base) + len(blob)] = blob
    struct.pack_into("<I", out, len(out) - 4, rknano_crc32(bytes(out[:-4])))

    diff = [i for i in range(len(ref) - 4) if ref[i] != out[i]]
    assert diff and min(diff) >= min(sites + [img(base)]) >= 0xD68, "header/prefix changed"
    assert all(any(s <= i < s + 4 for s in sites) or img(base) <= i < img(base) + len(blob)
               for i in diff), "stray byte change"
    assert rknano_crc32(bytes(out[:-4])) == struct.unpack_from("<I", out, len(out) - 4)[0]
    assert rknano_crc32(ref[:-4]) == struct.unpack_from("<I", ref, len(ref) - 4)[0]

    Path(a.o).write_bytes(out)
    print(f"hook @0x{entry:08X} ({len(blob)} B), patched IMG "
          f"{', '.join(f'0x{s:X}={out[s:s+4].hex()}' for s in sites)}, {len(diff)} bytes changed, "
          f"md5 {hashlib.md5(out).hexdigest()} -> {a.o}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
