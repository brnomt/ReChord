#!/usr/bin/env python3
"""
compile_check.py — Compile SDK .c files with arm-none-eabi-gcc and extract
function names + sizes from the resulting object files.

This builds the foundation for code-pattern matching:
  1. Compile each SDK .c file to an .o file
  2. Parse the symbol table to get function names and sizes
  3. Match binary functions by size + code hash against compiled SDK functions
  4. Name matching functions in Ghidra

Output: build/sdk_compiled_functions.json
  {
    "function_name": {
      "file": "relative/path.c",
      "size": 1234,
      "object_file": "path/to/file.o"
    }
  }
"""
import subprocess
import json
import re
import sys
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SDK_ROOTS = [
    ROOT / "firmware" / "rockchip",        # RKnanoD SDK (Echo Mini)
]
OUTPUT_DIR = ROOT / "build" / "sdk_objects"
OUTPUT = ROOT / "build" / "sdk_compiled_functions.json"

# ARM compiler flags for Cortex-M3 Thumb-2
CFLAGS = [
    "-mcpu=cortex-m3",
    "-mthumb",
    "-Os",  # optimize for size (firmware likely uses -Os)
    "-g0",  # no debug info
    "-c",   # compile only
    "-ffunction-sections",
    "-fdata-sections",
    "-fno-common",
    "-fno-exceptions",
    "-MMD",
    "-MP",
    "-nostdlib",
    "-nostartfiles",
    # The SDK is Keil/armcc-era C90: implicit declarations and armcc keywords
    # are everywhere. We only need compilable objects for symbol extraction,
    # so: inject the armcc keyword compat shim and keep old-C leniency.
    "-include", str(ROOT / "firmware" / "rockchip" / "include" / "armcc_compat.h"),
    "-std=gnu89",
    "-Wno-implicit-function-declaration",
    "-Wno-implicit-int",
    "-Wno-builtin-declaration-mismatch",
    "-I", str(ROOT / "firmware"),
    "-I", str(ROOT / "firmware" / "rockchip"),
]

# Include paths from SDK: every directory holding a header. SDK headers include
# each other by bare name across subsystem dirs (e.g. "MemDev.h"), so each header
# directory must be on the include path. Collected dynamically — no hardcoded
# list to rot when the tree changes.
def collect_include_dirs():
    dirs = {str(ROOT / "firmware"), str(ROOT / "firmware" / "rockchip")}
    for f in (ROOT / "firmware").rglob("*.h"):
        dirs.add(str(f.parent))
    return [d for pair in (("-I", d) for d in sorted(dirs)) for d in pair]

SDK_INCLUDES = collect_include_dirs()


def find_c_files():
    """Find all .c files in SDK directories."""
    files = []
    for sdk_root in SDK_ROOTS:
        if sdk_root.is_dir():
            for f in sorted(sdk_root.rglob("*.c")):
                files.append(f)
    return files


def compile_file(c_file: Path, output_dir: Path) -> Path | None:
    """Compile a .c file to .o. Returns path to .o or None on failure."""
    rel = c_file.relative_to(ROOT)
    obj_path = output_dir / (str(rel).replace("/", "_").replace("\\", "_") + ".o")
    obj_path.parent.mkdir(parents=True, exist_ok=True)

    cmd = ["arm-none-eabi-gcc"] + CFLAGS + SDK_INCLUDES + ["-o", str(obj_path), str(c_file)]
    try:
        result = subprocess.run(cmd, capture_output=True, timeout=30, text=True)
        if result.returncode == 0:
            return obj_path
        return None
    except Exception:
        return None


def parse_symbol_table(obj_file: Path) -> list[dict]:
    """Parse the symbol table of an object file to get function names and sizes.

    Uses arm-none-eabi-nm to get symbols and arm-none-eabi-size for sizes.
    """
    functions = []

    # Get symbols with sizes
    try:
        result = subprocess.run(
            ["arm-none-eabi-nm", "-S", "--size-sort", str(obj_file)],
            capture_output=True, text=True, timeout=10
        )
        if result.returncode != 0:
            return []

        for line in result.stdout.strip().split("\n"):
            if not line:
                continue
            # Format: addr size type name
            parts = line.split()
            if len(parts) >= 4:
                try:
                    size = int(parts[1], 16) if parts[1] else 0
                    sym_type = parts[2]
                    name = parts[3]
                    # t/T = text (function), local/global
                    if sym_type in ("t", "T") and size > 0:
                        functions.append({
                            "name": name,
                            "size": size,
                            "type": sym_type,
                        })
                except (ValueError, IndexError):
                    pass
    except Exception:
        pass

    return functions


def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    c_files = find_c_files()
    print(f"Found {len(c_files)} .c files")

    all_functions = {}
    compiled = 0
    failed = 0

    for i, c_file in enumerate(c_files):
        if i % 50 == 0:
            print(f"  Compiling {i}/{len(c_files)}...")

        obj_path = compile_file(c_file, OUTPUT_DIR)
        if obj_path is None:
            failed += 1
            continue

        compiled += 1
        funcs = parse_symbol_table(obj_path)
        rel_path = str(c_file.relative_to(ROOT))

        for func in funcs:
            name = func["name"]
            if name in all_functions:
                # Keep the one with larger size
                if func["size"] > all_functions[name]["size"]:
                    all_functions[name] = {
                        "size": func["size"],
                        "file": rel_path,
                        "object": str(obj_path.relative_to(ROOT)),
                    }
            else:
                all_functions[name] = {
                    "size": func["size"],
                    "file": rel_path,
                    "object": str(obj_path.relative_to(ROOT)),
                }

    print(f"\nCompiled: {compiled}, Failed: {failed}")
    print(f"Functions extracted: {len(all_functions)}")

    # Size distribution
    sizes = [f["size"] for f in all_functions.values()]
    if sizes:
        print(f"  Min size: {min(sizes)}")
        print(f"  Max size: {max(sizes)}")
        print(f"  Avg size: {sum(sizes) // len(sizes)}")

    OUTPUT.write_text(json.dumps(all_functions, indent=2), encoding="utf-8")
    print(f"Saved to: {OUTPUT}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
