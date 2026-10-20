#!/usr/bin/env python3
"""Regression check: standalone 5MB builder reproduces the known-good AP5M.

This is a local check, not a CI test: it needs the fw1 container and the
known-good image produced by the current build.  Run it before
`make standalone-ap5m` (that target rewrites build/ReChord_AP5M.IMG), or
pass a saved copy with --golden.

    python3 tools/verify_fw1_5m.py
    python3 tools/verify_fw1_5m.py --fw1 build/ap/ReChord_AP.fw1 \
        --golden /path/to/saved/ReChord_AP5M.IMG
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path

import fw1_5m
from rknano_crc import rknano_crc32

# SHA-256 of the boot-confirmed AP5M image (CRC 0x2264CCBA).
KNOWN_GOOD_SHA256 = (
    "c2238cb388b14d6bfd984da9328ca0005831b90d42cf4b5be77617b48babb93d"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fw1", type=Path, default=Path("build/ReChord_AP.fw1"))
    parser.add_argument("--golden", type=Path, default=Path("build/ReChord_AP5M.IMG"))
    parser.add_argument("-o", "--output", type=Path,
                        default=Path("build/standalone/verify_AP5M.IMG"))
    parser.add_argument("--expect-sha256", default=KNOWN_GOOD_SHA256)
    args = parser.parse_args()

    if not args.fw1.is_file():
        print(f"ERROR: fw1 not found: {args.fw1}", file=sys.stderr)
        return 1

    try:
        fw1_5m.build_standalone_5m(args.fw1.read_bytes(), args.output)
    except (OSError, fw1_5m.LayoutError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    built = args.output.read_bytes()
    digest = hashlib.sha256(built).hexdigest()
    crc = struct.unpack_from("<I", built, len(built) - 4)[0]
    checks = [
        ("size", len(built) == fw1_5m.IMG_SIZE),
        ("trailer CRC", crc == rknano_crc32(built[:-4])),
        ("header copy", built[fw1_5m.MARKER_BASE:fw1_5m.MARKER_BASE + 0x200]
         == built[:0x200]),
        ("marker", built[fw1_5m.MARKER_HDR:fw1_5m.MARKER_HDR + 16]
         == fw1_5m.MARKER_HEADER),
    ]
    for name, ok in checks:
        print(f"  [{'OK' if ok else 'FAIL'}] {name}")
    print(f"  size 0x{len(built):X}, CRC 0x{crc:08X}")
    print(f"  SHA-256 {digest}")

    ok = all(ok for _, ok in checks)
    if args.expect_sha256:
        expected = args.expect_sha256.lower()
        same = digest == expected
        print(f"  [{'OK' if same else 'FAIL'}] expected SHA-256 {expected}")
        ok = ok and same

    if args.golden.is_file():
        golden = args.golden.read_bytes()
        same = built == golden
        print(f"  [{'OK' if same else 'FAIL'}] byte-identical to {args.golden}")
        if not same and len(built) == len(golden):
            first = next((i for i, (a, b) in enumerate(zip(built, golden))
                          if a != b), None)
            if first is not None:
                print(f"        first diff at 0x{first:X}: "
                      f"built {built[first]:02X}, golden {golden[first]:02X}")
        ok = ok and same
    else:
        print(f"  [SKIP] golden not found: {args.golden}")

    print("RESULT:", "PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
