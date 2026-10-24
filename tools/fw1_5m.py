#!/usr/bin/env python3
"""Build the fw1-only 5 MB AP5M image without reading a stock/reference IMG.

The 5 MB image is the one proven to boot on hardware. Its first 0x1F8 bytes
are the RKnano firmware descriptor; the rest is our fw1 container, the empty
trailing marker container and the CRC trailer.  Historically
tools/make_fw1.py cloned that descriptor from community/refcfw/HIFIEC90.IMG,
which is gitignored and therefore unavailable in a clean clone.

This module generates the descriptor from the documented field table in
docs/fw1-packing.md section 10.  The build patches the fields that describe
our image (date, brand, build tag and fw1 end).  No stock image or reference
binary is read at build time.

Remaining boot-chain dependency (Phase 2, BRNOMT-82): the stock loader/ROM
contract at LOADER_CODE_BASE 0x03088000.  This module only removes the
container/template file dependency; it does not replace the loader.
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

from rknano_crc import rknano_crc32

IMG_SIZE = 0x500004
HEADER_SIZE = 0x1F8
FW1_HEADER_OFF = 0x1F8
FW1_TABLE_COUNT_MAX = 91
FW1_CODE_OFF = 0x7B8
FW1_CODE_END = 0x57820
PAYLOAD_MAX = FW1_CODE_END - FW1_CODE_OFF  # 0x57068

MARKER_BASE = 0x44C000
MARKER_HDR = MARKER_BASE + 0x1F8  # 0x44C1F8

# Current build identity written into the descriptor (same values as the
# known-good AP5M image built by the legacy make_fw1 path).
BUILD_DATE = 0x0A0B2026  # Oct 11 2026 (MM DD 20 YY)
BRAND = b"ReChord\x00"
BUILD_TAG = b"cfwtest01"[:8]
MARKER_HEADER = b"RKnanoFW" + b"\x00" * 8

# docs/fw1-packing.md section 10: non-zero descriptor fields of the reference
# fw1-only image. Fields 0x00, 0x10 and 0x30 are the reference identity and
# are overwritten by BUILD_DATE/BRAND/BUILD_TAG; 0x80 is overwritten with our
# fw1 end.  Everything else is zero.
HEADER_FIELDS = (
    (0x004, 0x00090000),
    (0x008, 0x00000002),
    (0x00C, 0x76543210),
    (0x050, 0x00000004),
    (0x054, 0x00000200),
    (0x05C, 0x00000200),
    (0x064, 0x00000200),
    (0x06C, 0x00000200),
    (0x070, 0x00037670),
    (0x074, 0x00000003),
    (0x078, 0x00037870),
    (0x07C, 0x00201C06),
    (0x080, 0x00239476),
    (0x084, 0x00211C06),
    (0x088, 0x0044B07C),
    (0x08C, 0x0000039A),
    (0x19C, 0x00000001),
    (0x1F4, MARKER_BASE),
)
REFERENCE_BUILD_DATE = 0x10072026
REFERENCE_BRAND = b"RECHO\x00\x00\x00"
REFERENCE_TAG = b"7e287ea\x00"


class LayoutError(ValueError):
    """The fw1 payload does not satisfy the AP5M boot entry contract."""


def header_template() -> bytes:
    """Return the documented 0x1F8-byte descriptor template."""
    header = bytearray(HEADER_SIZE)
    struct.pack_into("<I", header, 0x00, REFERENCE_BUILD_DATE)
    header[0x10:0x18] = REFERENCE_BRAND
    header[0x30:0x38] = REFERENCE_TAG
    for offset, value in HEADER_FIELDS:
        struct.pack_into("<I", header, offset, value)
    return bytes(header)


HEADER_TEMPLATE = header_template()


def _u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def build_standalone_5m(
    fw1: bytes,
    out_path: Path,
    *,
    header: bytes | None = None,
    verbose: bool = False,
) -> Path:
    """Write the fw1-only 5 MB AP5M image and return its path.

    Raises LayoutError if the fw1 payload is malformed or its vector table
    does not satisfy the ROM entry contract (SP/reset at the computed
    entry_vt, row 1 = 0x03004000 with a real init size).
    """
    data = bytes(fw1)
    if header is None:
        header = HEADER_TEMPLATE
    if len(header) != HEADER_SIZE:
        raise LayoutError(f"header template is {len(header)} B, expected {HEADER_SIZE}")

    if data[:8] != b"RKnanoFW":
        raise LayoutError("fw1 image is missing the RKnanoFW magic")
    load_base, count = struct.unpack_from("<II", data, 8)
    if not 1 <= count <= FW1_TABLE_COUNT_MAX:
        raise LayoutError(f"fw1 table count {count} outside 1..{FW1_TABLE_COUNT_MAX}")
    table_len = count * 16
    if len(data) < 16 + table_len:
        raise LayoutError("fw1 image ends before its scatter table")
    payload = data[16 + table_len:]
    if len(payload) > PAYLOAD_MAX:
        raise LayoutError(
            f"fw1 payload {len(payload):,} B exceeds {PAYLOAD_MAX:,} B")
    if len(data) > FW1_CODE_END - FW1_HEADER_OFF:
        raise LayoutError(
            f"fw1 image {len(data):,} B exceeds the "
            f"{FW1_CODE_END - FW1_HEADER_OFF:,} B container area")

    out = bytearray(IMG_SIZE)
    out[:HEADER_SIZE] = header
    struct.pack_into("<I", out, 0x00, BUILD_DATE)
    out[0x10:0x18] = BRAND
    out[0x30:0x38] = BUILD_TAG

    out[FW1_HEADER_OFF:FW1_HEADER_OFF + len(data)] = data

    payload_nz = len(payload.rstrip(b"\x00"))
    fw1_end = FW1_CODE_OFF + ((payload_nz + 3) & ~3) if payload_nz else FW1_CODE_OFF
    struct.pack_into("<I", out, 0x80, fw1_end)

    dest_end = _u32(out, 0x214)
    ram_start = _u32(out, 0x218)
    init_size = _u32(out, 0x21C)
    entry_vt = dest_end - load_base + 0x200
    if not 0 <= entry_vt <= IMG_SIZE - 8:
        raise LayoutError(
            f"entry vector table 0x{entry_vt:X} outside the image "
            f"(row0 end 0x{dest_end:08X})")
    sp, reset = struct.unpack_from("<II", out, entry_vt)
    if not (ram_start == 0x03004000 and init_size >= 8
            and sp == 0x03004000 and reset & 1):
        raise LayoutError(
            f"entry VT: row1 {ram_start:#x}/{init_size:#x}, "
            f"IMG 0x{entry_vt:X} = SP 0x{sp:08X} reset 0x{reset:08X}")

    marker_base = _u32(out, 0x1F4)
    if marker_base != MARKER_BASE:
        raise LayoutError(
            f"descriptor marker base 0x{marker_base:08X}, expected 0x{MARKER_BASE:08X}")
    out[MARKER_BASE:MARKER_BASE + 0x200] = out[:0x200]
    out[MARKER_HDR:MARKER_HDR + len(MARKER_HEADER)] = MARKER_HEADER

    crc = rknano_crc32(bytes(out[:-4]))
    struct.pack_into("<I", out, IMG_SIZE - 4, crc)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(out)
    if verbose:
        print(f"      fw1: SP 0x{load_base:08X}, {count} rows, "
              f"payload {len(payload):,} B, end 0x{fw1_end:X}")
        print(f"      5MB image -> {out_path} (CRC 0x{crc:08X})")
    return out_path


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build the fw1-only 5MB AP5M image (no stock/ref IMG reads)")
    parser.add_argument("fw1", type=Path, help="fw1 container (.fw1)")
    parser.add_argument("-o", "--output", type=Path,
                        default=Path("build/standalone/ReChord_AP5M.IMG"))
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()
    try:
        build_standalone_5m(args.fw1.read_bytes(), args.output,
                            verbose=args.verbose)
    except (OSError, LayoutError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
