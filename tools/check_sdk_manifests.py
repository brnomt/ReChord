#!/usr/bin/env python3
"""Validate the checked-in AP and BB source manifests.

The manifests are the reproducible boundary between the two RKnanoD builds.
They intentionally do not depend on the ignored community SDK directory.
"""

from __future__ import annotations

import argparse
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
AP_MANIFEST = ROOT / "firmware" / "rockchip" / "manifests" / "ap.mk"
BB_MANIFEST = ROOT / "firmware" / "rockchip" / "manifests" / "bb.mk"


def read_make_list(path: Path, variable: str) -> list[str]:
    prefix = f"{variable} :="
    collecting = False
    values: list[str] = []

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not collecting:
            if not line.startswith(prefix):
                continue
            collecting = True
            line = line[len(prefix) :].strip()

        continued = line.endswith("\\")
        if continued:
            line = line[:-1].strip()
        if line and not line.startswith("#"):
            values.extend(line.split())
        if not continued:
            return values

    raise ValueError(f"{variable} not found or unterminated in {path}")


def duplicates(values: list[str]) -> list[str]:
    seen: set[str] = set()
    repeated: set[str] = set()
    for value in values:
        if value in seen:
            repeated.add(value)
        seen.add(value)
    return sorted(repeated)


def validate_paths(
    label: str, values: list[str], *, directories: bool = False
) -> list[str]:
    errors: list[str] = []
    repeated = duplicates(values)
    if repeated:
        errors.append(f"{label} contains duplicates: {', '.join(repeated)}")

    for value in values:
        path = ROOT / value
        exists = path.is_dir() if directories else path.is_file()
        if not exists:
            kind = "directory" if directories else "file"
            errors.append(f"{label} missing {kind}: {value}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--require-complete-ap",
        action="store_true",
        help="fail while the Keil AP manifest still has missing imported sources",
    )
    args = parser.parse_args()

    ap_sources = read_make_list(AP_MANIFEST, "AP_SRCS")
    ap_missing = read_make_list(AP_MANIFEST, "AP_MISSING_SRCS")
    ap_includes = read_make_list(AP_MANIFEST, "AP_INCLUDE_DIRS")
    bb_sources = read_make_list(BB_MANIFEST, "BB_SRCS")
    bb_includes = read_make_list(BB_MANIFEST, "BB_INCLUDE_DIRS")

    errors: list[str] = []
    errors.extend(validate_paths("AP_SRCS", ap_sources))
    errors.extend(validate_paths("AP_INCLUDE_DIRS", ap_includes, directories=True))
    errors.extend(validate_paths("BB_SRCS", bb_sources))
    errors.extend(validate_paths("BB_INCLUDE_DIRS", bb_includes, directories=True))

    if duplicates(ap_missing):
        errors.append("AP_MISSING_SRCS contains duplicates")
    if args.require_complete_ap and ap_missing:
        errors.append(
            f"AP import is incomplete: {len(ap_missing)} effective Keil sources missing"
        )

    overlap = sorted(set(ap_sources) & set(bb_sources))
    print(f"AP sources present: {len(ap_sources)}")
    print(f"AP sources missing from import: {len(ap_missing)}")
    print(f"AP include directories: {len(ap_includes)}")
    print(f"BB sources: {len(bb_sources)}")
    print(f"BB include directories: {len(bb_includes)}")
    print(f"Shared AP/BB sources: {len(overlap)}")

    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        return 1

    print("SDK manifests OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
