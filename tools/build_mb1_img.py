#!/usr/bin/env python3
"""
build_mb1_img.py — Build the ReChord M-B1 hybrid image (Route B).

Takes the proven reference firmware image (HIFIEC90.IMG) and applies ONLY
these patches:

  1. Reset veneer at IMG 0x136D6 (ramimg 0x134DE, runtime 0x0306296E):
     its single `b.w real_entry` becomes `b.w rechord_boot` (4 bytes).
  2. Code cave at IMG 0x135F8 (ramimg 0x13400, runtime 0x03062890):
     the M-B1 stub (firmware/mb1 build), which draws a boot line via the
     firmware's own status_line, logs via its log(), then tail-jumps to the
     REAL startup entry 0x030500E4 — everything else (USB, logs, UI, updater)
     is untouched firmware code.
  3. Visible branding at three byte sites (offset-specific; the target image
     bytes are hex-encoded so third-party product names stay out of the
     source tree).

Size is preserved byte-for-byte; the 4-byte trailer is RECOMPUTED with the
device's own checksum (byte-verified 4/4 against stock + reference images):

    trailer = CRC32(poly 0x04C10DB7, non-reflected MSB-first, init 0, xorout 0)
              over IMG[:-4]

Note the poly is NOT the standard 0x04C11DB7 (bit 0x1000 missing) — the
device rejects wrong-trailer images pre-write with "bad CRC Image", which is
a safe failure (nothing is flashed).

Usage:
    python3 tools/build_mb1_img.py [-o community/refcfw/MB1_ECHOMINI.IMG]
"""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC_IMG = ROOT / "community" / "refcfw" / "HIFIEC90.IMG"
STUB_BIN = ROOT / "firmware" / "mb1" / "mb1.bin"

IMG_BASE = 0x1F8                 # RAM image starts here (runtime 0x0304F490)
CAVE_OFF = 0x13400               # ramimg offset of the code cave
CAVE_SIZE = 0xDE                 # cave runs to the veneer
HOOK_OFF = 0x134DE               # ramimg offset of the Reset veneer
HOOK_SIZE = 4
CRC_POLY = 0x04C10DB7            # device CRC (non-standard poly — see docstring)

# Visible branding sites. Target bytes are hex-encoded device patterns; the
# replacements are our own name.
#   0x10750: boot version string          0x2C053: status-screen version line
#   0x1124D: build-info prefix (5 bytes)
BRAND_PATCHES = [
    (0x10750, bytes.fromhex("524543484f20302e392e32"), b"ReChord 0.1"),  # 11 bytes
    (0x2C053, bytes.fromhex("524543484f20302e392e32"), b"ReChord 0.1"),
    (0x1124D, bytes.fromhex("524543484f"), b"RECHD"),                     # 5 bytes
]


def rknano_crc32(data: bytes) -> int:
    """CRC-32 as implemented by the firmware checksum fn (FUN_03054284):
    non-reflected, MSB-first table CRC, poly 0x04C10DB7, init 0, xorout 0."""
    tbl = []
    for i in range(256):
        c = i << 24
        for _ in range(8):
            c = ((c << 1) ^ CRC_POLY) & 0xFFFFFFFF if c & 0x80000000 else (c << 1) & 0xFFFFFFFF
        tbl.append(c)
    crc = 0
    for b in data:
        crc = (tbl[((crc >> 24) ^ b) & 0xFF] ^ (crc << 8)) & 0xFFFFFFFF
    return crc


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("-o", "--output", default=str(ROOT / "community" / "refcfw" / "MB1_ECHOMINI.IMG"))
    args = ap.parse_args()

    img = bytearray(SRC_IMG.read_bytes())
    stub = STUB_BIN.read_bytes()

    if len(stub) != CAVE_SIZE + HOOK_SIZE:
        sys.exit(f"stub size {len(stub)} != expected {CAVE_SIZE + HOOK_SIZE}")

    # 1) cave: stub text+rodata
    img[IMG_BASE + CAVE_OFF:IMG_BASE + CAVE_OFF + CAVE_SIZE] = stub[:CAVE_SIZE]
    # 2) hook: Reset veneer branch
    img[IMG_BASE + HOOK_OFF:IMG_BASE + HOOK_OFF + HOOK_SIZE] = stub[CAVE_SIZE:]
    # 3) branding (offset-specific, length-checked)
    for off, old, new in BRAND_PATCHES:
        if len(old) != len(new):
            sys.exit(f"brand patch at 0x{off:X}: length mismatch")
        if bytes(img[off:off + len(old)]) != old:
            sys.exit(f"brand patch at 0x{off:X}: expected bytes not found")
        img[off:off + len(old)] = new

    # 4) recompute the trailer CRC over the patched body
    old_trailer = struct.unpack_from("<I", img, len(img) - 4)[0]
    new_trailer = rknano_crc32(bytes(img[:-4]))
    struct.pack_into("<I", img, len(img) - 4, new_trailer)

    out = Path(args.output)
    out.write_bytes(img)
    print(f"wrote {out} ({len(img):,} bytes)")
    print(f"  cave  @ IMG 0x{IMG_BASE + CAVE_OFF:08X} ({CAVE_SIZE} B)")
    print(f"  hook  @ IMG 0x{IMG_BASE + HOOK_OFF:08X} -> b.w rechord_boot")
    for off, old, new in BRAND_PATCHES:
        print(f"  brand @ IMG 0x{off:08X}: {old!r} -> {new!r}")
    print(f"  trailer: 0x{old_trailer:08X} -> 0x{new_trailer:08X} (recomputed)")


if __name__ == "__main__":
    main()
