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

# CONFIRMED boot model (audit 2026-10-11, incident 5667c9c4): the loader
# splices the fw1 payload at IMG 0x7B8 and loads it CONTIGUOUSLY into RAM at
# RAM = RAM_IMG_BASE + IMG offset. Scatter "src" fields are addresses in that
# loaded image - NOT XIP. The XIP reading is refuted: under it the entry
# address 0x03050000 reads zeros and 0/46 of the stock's load rows source
# from fw1's own payload window; under contiguous 46/46 do.
RAM_IMG_BASE = 0x0304F298
FW1_PAYLOAD_IMG_OFF = 0x7B8      # where pack_img.py splices the fw1 payload
VECTOR_RAM = 0x03050000          # entry contract: vector-table base (header w2)

# HARD GATE (2026-10-11): rows must never target below this address.
# 0x03000000..0x03004000 is ROM-owned AP fast mem / workspace; the
# 2026-08-25 hard brick was caused by a load row over that zone.
SAFE_MIN_RAM = 0x03004000

# The reference image's 0x5B0-byte payload prefix (its "pointer table"):
# sparse region descriptors pointing into HIGHRAM0. The loader may read it;
# cloning it verbatim removes that variable (option-2 byte-clone test).
REF_PREFIX_LEN = 0x5B0
REF_PREFIX_WORDS = [
    (0x008, bytes.fromhex('e8a80703')),
    (0x00C, bytes.fromhex('00030000')),
    (0x010, bytes.fromhex('60a10703')),
    (0x014, bytes.fromhex('00a00703')),
    (0x018, bytes.fromhex('2c0f0000')),
    (0x028, bytes.fromhex('2caf0703')),
    (0x02C, bytes.fromhex('14100000')),
    (0x030, bytes.fromhex('8cb00703')),
    (0x034, bytes.fromhex('00a00703')),
    (0x038, bytes.fromhex('c4060000')),
    (0x048, bytes.fromhex('c4a60703')),
    (0x050, bytes.fromhex('50b70703')),
    (0x054, bytes.fromhex('00a00703')),
    (0x058, bytes.fromhex('f0080000')),
    (0x068, bytes.fromhex('f0a80703')),
    (0x06C, bytes.fromhex('04000000')),
    (0x090, bytes.fromhex('40c00703')),
    (0x094, bytes.fromhex('00a00703')),
    (0x098, bytes.fromhex('44150000')),
    (0x0A8, bytes.fromhex('44b50703')),
    (0x0AC, bytes.fromhex('10000000')),
    (0x410, bytes.fromhex('84d50703')),
    (0x414, bytes.fromhex('00000001')),
    (0x418, bytes.fromhex('eca00000'))
]


def reference_prefix() -> bytes:
    pref = bytearray(REF_PREFIX_LEN)
    for off, w in REF_PREFIX_WORDS:
        pref[off:off + 4] = w
    return bytes(pref)


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
                       vaddr=self.u32(off + 8), paddr=self.u32(off + 12), filesz=self.u32(off + 16),
                       memsz=self.u32(off + 20), flags=self.u32(off + 24))


def build_scatter_table(elf: Elf32):
    """Build rows + payload for the contiguously loaded fw1 image.

    CONFIRMED model (audit 2026-10-11): the loader copies the payload into RAM
    at RAM_IMG_BASE + IMG offset and then applies the rows. The payload must
    therefore MIRROR the segments' LMA layout (in-place load): each chunk sits
    at payload offset (LMA - load_base), so a chunk with LMA == VMA needs no
    copy and the .data chunk (LMA inside the code, VMA at 0x03004000) is
    copied by the startup from its LMA (=_data_load_start).

    Payload layout: [prefix][chunks...] with prefix = first_LMA - load_base
    (0x5B0 for our link: the vector table at LMA 0x03050000 must land at
    IMG 0xD68 = RAM 0x03050000, the entry contract).

    Row shapes (even = Type B, odd = Type A; strict alternation):
      Type B load/copy:  {dest, src, size, dest_end}  src = loaded-image RAM
      Type A rw region:  {ram_start, init_size, init_end, total_obj}
    """
    all_segs = [s for s in elf.program_headers()
                if s["type"] == PT_LOAD and s["memsz"] > 0]
    # payload placement only from chunks that carry bytes (filesz > 0);
    # file-less segments are pure regions (bss) -> Type A rows only.
    segs = sorted((s for s in all_segs if s["filesz"] > 0),
                  key=lambda s: s["paddr"])
    if not segs:
        return [], b""

    load_base = RAM_IMG_BASE + FW1_PAYLOAD_IMG_OFF   # RAM addr of payload[0]
    prefix = segs[0]["paddr"] - load_base
    assert prefix == REF_PREFIX_LEN, "prefix must be 0x5B0 for the clone"
    if prefix < 0:
        raise SystemExit(
            f"SAFETY: first LMA 0x{segs[0]['paddr']:08X} lies below the loaded "
            f"payload base 0x{load_base:08X}")

    loads, regions = [], []
    payload = bytearray()
    for seg in segs:
        vaddr, paddr = seg["vaddr"], seg["paddr"]
        filesz, memsz = seg["filesz"], seg["memsz"]
        want = paddr - load_base          # payload offset this chunk needs
        if want < len(payload):
            raise SystemExit("SAFETY: overlapping LMA chunks in payload")
        payload += bytes(want - len(payload))   # keep LMA spacing
        if filesz:
            if vaddr < SAFE_MIN_RAM:
                raise SystemExit(
                    f"SAFETY: load row targets 0x{vaddr:08X} < 0x{SAFE_MIN_RAM:08X} "
                    "(stack zone 0x03000000..0x03004000 - brick pattern)")
            payload += elf.data[seg["offset"]:seg["offset"] + filesz]
            if paddr == vaddr:
                # Byte-clone of the reference's row-0 geometry: it copies its
                # linked block up by 0xB68 ({dest=vaddr+0xB68, src=vaddr,
                # size, end}) and enters through the LINKED addresses (its
                # word1 thunk sits at the linked address). The overlapping
                # duplicate is harmless: the loader's copy has memmove
                # semantics (the reference boots with this exact row).
                REF_RELOC_DELTA = 0xB68
                d0 = vaddr + REF_RELOC_DELTA
                loads.append((d0, vaddr, filesz, d0 + filesz))
            # else: LMA != VMA (.data image) - the startup copies it from its
            # LMA; the loader must NOT pre-copy it (would write the VMA too
            # early). The Type A row below describes the region.

    # Type A regions from every PT_LOAD with memsz > filesz (bss/data spans)
    for seg in all_segs:
        vaddr, filesz, memsz = seg["vaddr"], seg["filesz"], seg["memsz"]
        if memsz > filesz:
            regions.append((vaddr, filesz, vaddr + filesz, memsz))
    entries = loads + regions       # Type B first (even), Type A after (odd)
    # Structural clone of the proven images (option-2 test): append the
    # reference's third row verbatim (a 4-byte poke to free RAM at
    # 0x03078CD0; both the reference and stock carry extra rows we do not
    # fully understand - matching them is the point of this test).
    entries.append((0x03078CD0, 0x03079DCC, 0x4, 0))
    for vaddr, *_ in regions:
        if vaddr < SAFE_MIN_RAM:
            raise SystemExit(
                f"SAFETY: region row targets 0x{vaddr:08X} < 0x{SAFE_MIN_RAM:08X}")
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
