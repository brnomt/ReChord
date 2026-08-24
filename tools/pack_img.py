#!/usr/bin/env python3
"""
pack_img.py — Splice a custom section_3 (main firmware code) into a stock HIFIEC*.IMG.

IMG layout (HIFIEC37.IMG, 33,554,436 bytes = 32 MiB + 4-byte trailer):

    0x00000000  header + reloc table
    0x00057820  bootloader (stock — preserved)
    0x00081A14  section_3: main firmware code (RKnanoFW image)  ← REPLACEABLE
    0x009BAA0E  ROCK26 resources (UI bitmaps — preserved)
    0x01FC13F6  end of part5 (per header @ 0x14C)
    0x01FC41F8  RKnanoFW end marker + padding
    0x02000000  4-byte EOF trailer (0x1EA1C309 for v3.7.0)

Usage:
    # Extract stock section_3 for analysis or identity test
    python tools/pack_img.py --extract -o build/section3_stock.bin

    # Identity test: extract, repack unchanged, verify byte-identical
    python tools/pack_img.py --identity-test

    # Pack custom section_3 into stock IMG
    python tools/pack_img.py --pack build/section3_custom.bin -o build/custom.IMG

    # Pack with explicit stock source
    python tools/pack_img.py --pack build/section3_custom.bin \
        --stock "stock/3.7.0/ECHO MINI V3.7.0/HIFIEC37.IMG" -o build/custom.IMG

The section_3 region is 0x081A14–0x009BAA0E (9,670,650 bytes = 0x938FFA).
A custom section_3 must be <= this size; if shorter, the remainder is zero-padded.
The EOF trailer is always preserved from stock.

Milestone M2: --identity-test should produce a byte-identical IMG.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path

# --- Layout constants (from docs/memory-map.md + verify_boundaries.py) ---
SECTION3_OFFSET = 0x00081A14
SECTION3_END    = 0x009BAA0E
SECTION3_SIZE   = SECTION3_END - SECTION3_OFFSET  # 0x938FFA = 9,670,650

TRAILER_OFFSET  = 0x02000000
IMG_SIZE        = 0x02000004  # 33,554,436

DEFAULT_STOCK  = "stock/3.7.0/ECHO MINI V3.7.0/HIFIEC37.IMG"


def read_trailer(data: bytes) -> int:
    return struct.unpack_from("<I", data, len(data) - 4)[0]


def extract_section3(img: bytes, out_path: Path) -> int:
    """Extract the section_3 region from an IMG to a .bin file."""
    section = img[SECTION3_OFFSET:SECTION3_END]
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(section)
    print(f"extracted section_3: {len(section):,} bytes (0x{len(section):X})")
    print(f"  SHA-256: {hashlib.sha256(section).hexdigest()[:16]}...")
    print(f"  -> {out_path}")
    return len(section)


def pack_section3(stock_img: bytes, custom_section3: bytes, out_path: Path,
                  keep_stock_tail: bool = False) -> Path:
    """Splice a custom section_3 into a stock IMG, preserving everything else.

    keep_stock_tail=True keeps the STOCK bytes of the section_3 flash region
    beyond the custom image instead of zero-padding. The fw1 (AP) memory-map
    table XIP-copies data from inside this region (e.g. entry 88 ->
    IMG 0x8A5A4, see docs/fw1-packing.md), so zero-padding corrupts load-time
    AP tables (string/font/pointer data) -> blank UI text and crashes.
    With the stock tail intact, a smaller custom BB only replaces the bytes
    it actually occupies."""
    if len(stock_img) != IMG_SIZE:
        raise ValueError(
            f"stock IMG size {len(stock_img)} != expected {IMG_SIZE}"
        )
    if len(custom_section3) > SECTION3_SIZE:
        raise ValueError(
            f"custom section_3 too large: {len(custom_section3):,} > {SECTION3_SIZE:,}"
        )

    # Start from a mutable copy of stock
    out = bytearray(stock_img)

    # Write custom section_3; pad the remainder (zeros or preserved stock tail)
    out[SECTION3_OFFSET : SECTION3_OFFSET + len(custom_section3)] = custom_section3
    if len(custom_section3) < SECTION3_SIZE and not keep_stock_tail:
        pad_start = SECTION3_OFFSET + len(custom_section3)
        out[pad_start:SECTION3_END] = b"\x00" * (SECTION3_SIZE - len(custom_section3))

    # Preserve the original trailer
    trailer = read_trailer(stock_img)
    struct.pack_into("<I", out, TRAILER_OFFSET, trailer)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(out)

    print(f"packed {len(custom_section3):,} bytes of section_3 into {out_path}")
    print(f"  section_3 fill: {len(custom_section3):,}/{SECTION3_SIZE:,} "
          f"({100 * len(custom_section3) / SECTION3_SIZE:.1f}%)")
    print(f"  tail: {'stock bytes preserved' if keep_stock_tail else 'zero-padded'}")
    print(f"  trailer preserved: 0x{trailer:08X}")
    print(f"  output size: {len(out):,} bytes")


# --- fw1 (AP) scatter-load region ---
FW1_HEADER_OFF = 0x000001F8   # RKnanoFW header #1
FW1_TABLE_OFF  = 0x00000208   # memory-map table (entries x 16 B)
FW1_CODE_OFF   = 0x000007B8   # fw1 payload (scatter-load source)
FW1_CODE_END   = 0x00057820   # sec2 header #2
FW1_TABLE_MAX  = (FW1_CODE_OFF - FW1_TABLE_OFF) // 16  # 91 entries


def parse_fw1(fw1: bytes):
    """Split a pack_fw1.py image into (load_base, count, table, payload)."""
    if fw1[:8] != b"RKnanoFW":
        raise ValueError("fw1 image missing RKnanoFW magic")
    load_base, count = struct.unpack_from("<II", fw1, 8)
    table_len = count * 16
    table = fw1[16:16 + table_len]
    payload = fw1[16 + table_len:]
    return load_base, count, table, payload


def pack_full(stock_img: bytes, fw1: bytes, custom_section3: bytes, out_path: Path) -> Path:
    """Splice BOTH a custom fw1 (AP) scatter image and a section_3 (BB) into stock."""
    if len(stock_img) != IMG_SIZE:
        raise ValueError(f"stock IMG size {len(stock_img)} != expected {IMG_SIZE}")
    if len(custom_section3) > SECTION3_SIZE:
        raise ValueError(
            f"custom section_3 too large: {len(custom_section3):,} > {SECTION3_SIZE:,}")

    load_base, count, table, payload = parse_fw1(fw1)
    if count > FW1_TABLE_MAX:
        raise ValueError(f"fw1 has {count} entries; max {FW1_TABLE_MAX} before payload")
    if len(payload) > (FW1_CODE_END - FW1_CODE_OFF):
        raise ValueError(
            f"fw1 payload {len(payload):,} bytes exceeds region "
            f"{FW1_CODE_END - FW1_CODE_OFF:,} (flat AP build is oversized — see docs/fw1-packing.md)")

    out = bytearray(stock_img)

    # fw1 header + memory-map table (zero-pad the table to the payload offset)
    struct.pack_into("<8sII", out, FW1_HEADER_OFF, b"RKnanoFW", load_base, count)
    out[FW1_TABLE_OFF:FW1_CODE_OFF] = b"\x00" * (FW1_CODE_OFF - FW1_TABLE_OFF)
    out[FW1_TABLE_OFF:FW1_TABLE_OFF + len(table)] = table

    # fw1 payload (scatter-load source), zero-pad the remainder
    out[FW1_CODE_OFF:FW1_CODE_OFF + len(payload)] = payload
    if len(payload) < (FW1_CODE_END - FW1_CODE_OFF):
        out[FW1_CODE_OFF + len(payload):FW1_CODE_END] = b"\x00" * (
            FW1_CODE_END - FW1_CODE_OFF - len(payload))

    # section_3 (BB) — same logic as pack_section3
    out[SECTION3_OFFSET:SECTION3_OFFSET + len(custom_section3)] = custom_section3
    if len(custom_section3) < SECTION3_SIZE:
        pad = SECTION3_OFFSET + len(custom_section3)
        out[pad:SECTION3_END] = b"\x00" * (SECTION3_SIZE - len(custom_section3))

    trailer = read_trailer(stock_img)
    struct.pack_into("<I", out, TRAILER_OFFSET, trailer)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(out)

    print(f"packed full AP+BB IMG -> {out_path}")
    print(f"  fw1: SP 0x{load_base:08X}, {count} entries, {len(payload):,} bytes payload")
    print(f"  section_3: {len(custom_section3):,} bytes")
    print(f"  trailer preserved: 0x{trailer:08X}")
    print()
    print("  !!! WARNING: custom fw1 memory-map table is NOT validated —")
    print("  !!! this combination BRICKED a device on 2026-08-25 (the stock")
    print("  !!! 91-entry table also lays out the UI framebuffer, audio buffers,")
    print("  !!! FAT cache and stacks; a custom table must reproduce all of it).")
    print("  !!! Flash only with maskrom recovery ready. Safe default:")
    print("  !!!   make release  ->  build/ReChord_BB.IMG (stock AP + custom BB)")
    return out_path



def identity_test(stock_path: Path) -> bool:
    """Extract section_3, repack unchanged, verify byte-identical output."""
    print(f"=== Identity Test (M2 milestone) ===")
    """Extract section_3, repack unchanged, verify byte-identical output."""
    print(f"=== Identity Test (M2 milestone) ===")
    print(f"stock: {stock_path}")
    print()

    stock_img = stock_path.read_bytes()
    if len(stock_img) != IMG_SIZE:
        print(f"ERROR: stock size {len(stock_img)} != {IMG_SIZE}", file=sys.stderr)
        return False

    # Extract
    print("[1/3] Extracting section_3 from stock...")
    section3 = stock_img[SECTION3_OFFSET:SECTION3_END]
    print(f"      {len(section3):,} bytes, SHA-256 {hashlib.sha256(section3).hexdigest()[:16]}...")
    print()

    # Repack
    print("[2/3] Repacking section_3 into a new IMG...")
    out = bytearray(stock_img)
    out[SECTION3_OFFSET:SECTION3_END] = section3  # identity
    trailer = read_trailer(stock_img)
    struct.pack_into("<I", out, TRAILER_OFFSET, trailer)
    print(f"      output: {len(out):,} bytes, trailer 0x{trailer:08X}")
    print()

    # Verify
    print("[3/3] Verifying byte-identical...")
    if bytes(out) == stock_img:
        print("      PASS — output is byte-identical to stock")
        print()
        print("M2 PASSED: pack_img.py can reproduce stock IMG from its own section_3.")
        print("Next: flash the repacked IMG to confirm the device boots identically.")
        return True
    else:
        # Find first diff
        diffs = [i for i in range(min(len(out), len(stock_img))) if out[i] != stock_img[i]]
        if diffs:
            first = diffs[0]
            print(f"      FAIL — first diff at 0x{first:08X}")
            print(f"        stock: {stock_img[first:first+16].hex()}")
            print(f"        out:   {bytes(out[first:first+16]).hex()}")
            print(f"        total diff bytes: {len(diffs)}")
        else:
            print("      FAIL — size or content mismatch")
        return False


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Pack/extract section_3 in Echo Mini HIFIEC*.IMG"
    )
    ap.add_argument(
        "--stock",
        type=Path,
        default=Path(DEFAULT_STOCK),
        help=f"stock IMG path (default: {DEFAULT_STOCK})",
    )
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument(
        "--extract",
        action="store_true",
        help="extract section_3 from stock IMG to -o",
    )
    mode.add_argument(
        "--pack",
        type=Path,
        metavar="BIN",
        help="pack a custom section_3 .bin into stock IMG",
    )
    ap.add_argument(
        "--keep-stock-tail",
        action="store_true",
        help="keep stock bytes beyond the custom section_3 instead of "
             "zero-padding (protects fw1 scatter-table data that XIPs "
             "from the section_3 flash region — recommended when the AP "
             "stays stock)",
    )
    mode.add_argument(
        "--identity-test",
        action="store_true",
        help="extract + repack unchanged + verify byte-identical (M2 milestone)",
    )
    mode.add_argument(
        "--pack-full",
        action="store_true",
        help="pack BOTH fw1 (--fw1) and section_3 (--bb) into stock IMG",
    )
    ap.add_argument(
        "--fw1",
        type=Path,
        metavar="IMG",
        help="fw1 (AP) RKnanoFW image from tools/pack_fw1.py (required for --pack-full)",
    )
    ap.add_argument(
        "--bb",
        type=Path,
        metavar="BIN",
        help="section_3 (BB) bin (required for --pack-full)",
    )
    ap.add_argument(
        "-o", "--output",
        type=Path,
        default=Path("build/section3_stock.bin"),
        help="output path for --extract, --pack, or --pack-full",
    )
    args = ap.parse_args()

    if not args.stock.is_file():
        print(f"ERROR: stock IMG not found: {args.stock}", file=sys.stderr)
        print(f"  Expected: {DEFAULT_STOCK}", file=sys.stderr)
        print("  Update --stock or place HIFIEC37.IMG there.", file=sys.stderr)
        return 1

    if args.identity_test:
        ok = identity_test(args.stock)
        return 0 if ok else 1

    stock_img = args.stock.read_bytes()

    if args.extract:
        extract_section3(stock_img, args.output)
        return 0

    if args.pack:
        if not args.pack.is_file():
            print(f"ERROR: section_3 bin not found: {args.pack}", file=sys.stderr)
            return 1
        custom = args.pack.read_bytes()
        pack_section3(stock_img, custom, args.output,
                      keep_stock_tail=args.keep_stock_tail)
        return 0

    if args.pack_full:
        if not (args.fw1 and args.fw1.is_file()):
            print(f"ERROR: --pack-full needs --fw1 <image> (from tools/pack_fw1.py)", file=sys.stderr)
            return 1
        if not (args.bb and args.bb.is_file()):
            print(f"ERROR: --pack-full needs --bb <section_3.bin>", file=sys.stderr)
            return 1
        fw1 = args.fw1.read_bytes()
        custom = args.bb.read_bytes()
        pack_full(stock_img, fw1, custom, args.output)
        return 0

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
