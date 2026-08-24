#!/usr/bin/env python3
"""
pack_fw1.py — Generate a RKnanoFW (fw1/AP) scatter-load image from an ELF.

The Echo Mini's fw1/AP stage is NOT a flat binary: the Mask ROM loads it
through an N-entry memory-map table (see docs/fw1-packing.md). This tool
emits that container from the load segments of a linked ARM ELF:

    [0:8]  "RKnanoFW"
    [8:12] uint32 LE  load base / initial SP
    [12:16] uint32 LE  entry count N
    [16..]  N x 16-byte entries
            Type B load:   {dest_ram, src_xip, size, dest_end}
            Type A bss:    {ram_start, 0, ram_start, zero_len}

IMPORTANT (see docs/fw1-packing.md section 4):
  * Stock fw1 is the RKnanoD A_CORE *module-overlay* build (resident
    SYS_CODE @ 0x03060000 + SYS_DATA @ 0x03000000, codecs/UI/BT/USB as
    overlay modules). A flat ELF linked at those addresses produces a
    structurally-correct container but is NOT the overlay layout the device
    expects.
  * **KNOWN TO BRICK ON HARDWARE (2026-08-25).** The stock fw1 header's
    memory-map table (91 entries) describes the ENTIRE system RAM layout —
    UI framebuffer loaded from flash (0x03024868), audio buffers, FAT cache,
    stacks — which the Mask ROM sets up at boot. The 3-entry table this tool
    emits from a flat ELF replaces all of that with just our segments and
    the device hard-bricks (no USB-storage fallback; maskrom recovery
    required). Until the stock table format is fully reverse-engineered and
    reproduced, images from this tool are for offline analysis ONLY.

Usage:
    python tools/pack_fw1.py build/ap/rechord_ap.elf -o build/ap/fw1_custom.img
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

MAGIC = b"RKnanoFW"
PT_LOAD = 1

# Echo Mini XIP mapping: IMG offset N <-> 0x03000000 + N. The scatter table's
# "src" field is an XIP address; FLASH_BASE is the address the fw1 payload
# occupies when spliced into the IMG (pack_img.py places it at IMG 0x7B8,
# i.e. XIP 0x030007B8). Load-entry src values are FLASH_BASE + payload offset.
FLASH_BASE = 0x030007B8


class Elf32:
    """Minimal little-endian ELF32 reader (program headers only)."""

    def __init__(self, data: bytes):
        if data[:4] != b"\x7fELF":
            raise ValueError("not an ELF file")
        if data[5] != 1:  # EI_DATA: 1 = little-endian
            raise ValueError("only little-endian ELF supported")
        if data[4] != 1:  # EI_CLASS: 1 = 32-bit
            raise ValueError("only 32-bit ELF supported")
        self.data = data
        self.e_entry = self.u32(24)
        self.e_phoff = self.u32(28)
        self.e_phentsize = self.u16(42)
        self.e_phnum = self.u16(44)

    def u16(self, off: int) -> int:
        return struct.unpack_from("<H", self.data, off)[0]

    def u32(self, off: int) -> int:
        return struct.unpack_from("<I", self.data, off)[0]

    def program_headers(self):
        for i in range(self.e_phnum):
            off = self.e_phoff + i * self.e_phentsize
            yield dict(type=self.u32(off), offset=self.u32(off + 4),
                       vaddr=self.u32(off + 8), filesz=self.u32(off + 16),
                       memsz=self.u32(off + 20), flags=self.u32(off + 24))


def build_scatter_table(elf: Elf32):
    """Emit (Type B load, Type A bss) pairs for each PT_LOAD segment.

    Returns (entries, payload) where payload is the concatenated file image
    of the loadable segments, and each load entry's src is FLASH_BASE + the
    segment's offset within that payload (XIP view).
    """
    entries = []
    payload = bytearray()
    for seg in elf.program_headers():
        if seg["type"] != PT_LOAD or seg["memsz"] == 0:
            continue
        vaddr = seg["vaddr"]
        filesz = seg["filesz"]
        memsz = seg["memsz"]
        if filesz:
            src = FLASH_BASE + len(payload)   # XIP address of this chunk
            payload += elf.data[seg["offset"]:seg["offset"] + filesz]
            entries.append((vaddr, src, filesz, vaddr + filesz))
        if memsz > filesz:
            # Type A (zero-fill): {ram_start, size, ram_end, total_size}
            #   — matches the stock table shape {start, span, end, extra=0}.
            zstart = vaddr + filesz
            zero_len = memsz - filesz
            entries.append((zstart, zero_len, zstart + zero_len, 0))
    return entries, bytes(payload)


def pack(elf_path: Path, out_path: Path, load_base: int):
    elf = Elf32(elf_path.read_bytes())
    entries, payload = build_scatter_table(elf)
    if not entries:
        raise ValueError("no loadable segments found")

    blob = bytearray()
    blob += MAGIC
    blob += struct.pack("<I", load_base)
    blob += struct.pack("<I", len(entries))
    for a, b, c, d in entries:
        blob += struct.pack("<IIII", a, b, c, d)
    blob += payload

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(blob)
    return out_path, len(entries), len(payload)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("elf", type=Path, help="linked ARM ELF (e.g. build/ap/rechord_ap.elf)")
    ap.add_argument("-o", "--output", type=Path,
                    default=Path("build/ap/fw1_custom.img"),
                    help="output RKnanoFW image")
    ap.add_argument("--load-base", type=lambda s: int(s, 0), default=0x03050000,
                    help="initial SP / load base (default 0x03050000, stock fw1)")
    args = ap.parse_args()

    if not args.elf.is_file():
        print(f"ERROR: ELF not found: {args.elf}", file=sys.stderr)
        return 1

    out, n_entries, payload_len = pack(args.elf, args.output, args.load_base)
    total = len(args.output.read_bytes())
    print(f"packed fw1 RKnanoFW image: {total:,} bytes -> {out}")
    print(f"  header: {MAGIC!r} + SP 0x{args.load_base:08X}, {n_entries} entries, {payload_len:,} bytes payload")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
