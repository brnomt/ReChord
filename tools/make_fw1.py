#!/usr/bin/env python3
"""
make_fw1.py - one-command AP image pipeline (milestone 1c).

Turn an input ELF (or a flat binary) into a device-flashable AP IMG:

    ELF/binary
      -> fw1 RKnanoFW slot image   (tools/pack_fw1.py conventions)
      -> splice into stock HIFIEC*.IMG (tools/pack_img.py --pack-full logic)
      -> RECOMPUTE the 4-byte trailer CRC
      -> verification report

The fw1 container and the scatter rows are produced with tools/pack_fw1.py's
own reader/table builder (imported here, not modified); the IMG wrap uses
tools/pack_img.py's pack_full(). The trailer CRC is the device checksum:

    trailer = CRC32(poly 0x04C10DB7, non-reflected MSB-first, init 0,
                    xorout 0) over IMG[:-4], stored little-endian.

NOTE THE POLY IS NOT the standard CRC-32 one (0x04C11DB7; bit 0x1000 is
missing). Verified against HIFIEC37.IMG (trailer 0x1EA1C309) and 3 further
stock images (see docs/re/route-b-minimum.md section 4b). The device rejects
wrong-trailer images pre-write with "bad CRC Image" (safe failure).

SAFETY: custom fw1 memory-map tables are KNOWN TO BRICK hardware
(tools/pack_fw1.py header, 2026-08-25). This tool never touches the device
and never writes build/ReChord_BB.IMG (the safe fallback image). Output is
for offline analysis until hardware-UART validation says otherwise.

Usage (run from the repo root):

    # full chain on the real AP ELF -> build/ReChord_AP.IMG
    python3 tools/make_fw1.py build/ap/rechord_ap.elf

    # dry-run proof with a tiny placeholder ELF (vector table + reset loop
    # at the documented base 0x03050000), built with arm-none-eabi-gcc
    python3 tools/make_fw1.py --placeholder -o build/ReChord_AP_dryrun.IMG

    # flat binary input (placed at --load-addr, entry defaults to load-addr)
    python3 tools/make_fw1.py code.bin --load-addr 0x03050000

Outputs:
    <output>          the packed IMG (default build/ReChord_AP.IMG)
    <output>.fw1      the intermediate fw1 slot image (for inspection)
"""

from __future__ import annotations

import argparse
import shutil
import struct
import subprocess
import sys
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import pack_fw1  # noqa: E402  (Elf32, build_scatter_table, MAGIC, FLASH_BASE)
import pack_img  # noqa: E402  (IMG layout constants, pack_full)

# --- trailer CRC (device checksum primitive, see docstring) ---
CRC_POLY = 0x04C10DB7  # NOT 0x04C11DB7 - bit 0x1000 is missing


def _crc_table() -> list:
    tbl = []
    for i in range(256):
        c = i << 24
        for _ in range(8):
            if c & 0x80000000:
                c = ((c << 1) ^ CRC_POLY) & 0xFFFFFFFF
            else:
                c = (c << 1) & 0xFFFFFFFF
        tbl.append(c)
    return tbl


_CRC_TABLE = _crc_table()


def rknano_crc32(data: bytes) -> int:
    """CRC-32(poly 0x04C10DB7, non-reflected, init 0, xorout 0).

    Table form of the known-good MSB-first bit loop; identical results
    (cross-checked bit-loop vs table during dry runs)."""
    crc = 0
    tbl = _CRC_TABLE
    for b in data:
        crc = (tbl[((crc >> 24) ^ b) & 0xFF] ^ (crc << 8)) & 0xFFFFFFFF
    return crc


# --- flat-binary input adapter (same interface pack_fw1.Elf32 exposes) ---

class FlatImage:
    """Present a flat binary as one PT_LOAD segment so pack_fw1's
    build_scatter_table() can consume it unchanged."""

    def __init__(self, data: bytes, load_addr: int, entry: int):
        self.data = data
        self.e_entry = entry
        self._seg = dict(type=pack_fw1.PT_LOAD, offset=0, vaddr=load_addr,
                         filesz=len(data), memsz=len(data), flags=0x5)

    def program_headers(self):
        yield self._seg


# --- placeholder ELF (dry-run proof) ---
# Minimal Cortex-M style vector table + Thumb reset loop at the documented
# fw1 base 0x03050000 (vector table convention, docs/re/route-b-minimum.md
# section 2). Structural proof only - not device-ready code.

PLACEHOLDER_S = """\
/* placeholder_ap.s - minimal vector table + reset loop (dry-run proof only) */
    .syntax unified
    .thumb
    .section .vectors, "ax", %progbits
    .align 2
    .global vector_table
vector_table:
    .word 0x03004000        /* word 0: initial SP (RefCFW convention) */
    .word reset_handler     /* word 1: reset vector (Thumb bit via symbol) */
    .word 0                 /* word 2: NMI placeholder */
    .word 0                 /* word 3: HardFault placeholder */

    .thumb_func
    .global reset_handler
reset_handler:
    b reset_handler         /* placeholder: hang in the reset loop */
"""

PLACEHOLDER_LD = """\
/* placeholder_ap.ld - one RAM span at the fw1 vector-table base */
ENTRY(reset_handler)
MEMORY
{
    RAM (rwx) : ORIGIN = 0x03050000, LENGTH = 64K
}
SECTIONS
{
    .vectors : { KEEP(*(.vectors)) } > RAM
    .text    : { *(.text*) } > RAM
    /DISCARD/ : { *(.ARM.exidx*) *(.comment) *(.note*) *(.ARM.attributes) }
}
"""


def build_placeholder(out_dir: Path) -> Path:
    """Assemble+link the tiny placeholder ELF with arm-none-eabi-gcc."""
    gcc = shutil.which("arm-none-eabi-gcc")
    if not gcc:
        raise RuntimeError(
            "arm-none-eabi-gcc not found in PATH; install the ARM toolchain "
            "or pass an existing ELF instead of --placeholder")
    out_dir.mkdir(parents=True, exist_ok=True)
    src = out_dir / "placeholder_ap.s"
    ld = out_dir / "placeholder_ap.ld"
    obj = out_dir / "placeholder_ap.o"
    elf = out_dir / "placeholder_ap.elf"
    src.write_text(PLACEHOLDER_S)
    ld.write_text(PLACEHOLDER_LD)
    subprocess.run([gcc, "-c", "-mcpu=cortex-m3", "-mthumb",
                    "-o", str(obj), str(src)], check=True)
    subprocess.run([gcc, "-nostdlib", "-mcpu=cortex-m3", "-mthumb",
                    "-T", str(ld), "-Wl,--gc-sections", "-Wl,--build-id=none",
                    "-o", str(elf), str(obj)], check=True)
    return elf


SAFE_MIN = 0x03004000  # ROM-owned RAM below this (2026-08-25 brick)


def fmt_rows(rows) -> None:
    for i, (a, b, c, d) in enumerate(rows):
        if i % 2 == 0:
            # Type B load/copy: {dest, src_xip, size, dest_end}
            print(f"    row {i:3d}: LOAD  dest 0x{a:08X}  src 0x{b:08X}  "
                  f"size 0x{c:06X} ({c:,} B)  end 0x{d:08X}")
        else:
            # Type A data/BSS region: {ram_start, size, ram_end, total_obj}
            print(f"    row {i:3d}: REG   ram  0x{a:08X}  size 0x{b:06X} "
                  f"({b:,} B)  end 0x{c:08X}  obj 0x{d:08X}")



def build_ref_shape_5m(img33: bytes, ref_path: Path, out_path: Path) -> Path:
    """Test #9: a 5MB fw1-ONLY image with the reference's outer-table shape.

    The reference image (5,242,884 B) is the only fw1-only image proven to
    boot on hardware. This builder clones its envelope: outer header from its
    template (brand/date/hash patched to ours), our fw1 container at 0x1F8,
    an empty RKnanoFW marker container at 0x44C1F8, and the CRC trailer in
    the last 4 bytes. Fields describing OUR fw1 extent are patched to ours.
    """
    ref = ref_path.read_bytes()
    IMG_SIZE = 0x500004
    MARKER_BASE = 0x44C000
    MARKER_HDR = 0x44C1F8

    out = bytearray(IMG_SIZE)
    out[:0x1F8] = ref[:0x1F8]                 # outer table template (proven shape)
    struct.pack_into("<I", out, 0x00, 0x0A0B2026)      # our build date
    out[0x10:0x18] = b"ReChord\x00"                    # brand (scrub-clean)
    out[0x30:0x38] = b"cfwtest01"[:8]                   # build tag

    # our fw1 container: header + padded table + payload (copy from the 33MB)
    fw1 = img33[0x1F8:0x7B8]                 # header + table area (padded)
    payload = img33[0x7B8:0x57820]
    fw1_end = 0x7B8
    nz = len(payload.rstrip(b"\x00"))
    if nz:
        fw1_end = 0x7B8 + ((nz + 3) & ~3)
    out[0x1F8:0x7B8] = fw1
    out[0x7B8:fw1_end] = payload[:fw1_end - 0x7B8]

    # Boot entry contract (verdicts #18/#19, reference + stock agree): the ROM
    # copies row 1 ({0x03004000, init_size, ...}) from the bytes right after
    # the row-0 block (IMG row0.dest_end - 0x03050000 + 0x200) and enters via
    # the VT at their start. Verdict #12 / #19: init_size 0 -> no VT -> black.
    dest_end = struct.unpack_from("<I", out, 0x214)[0]
    ram_start, init_size = struct.unpack_from("<II", out, 0x218)
    entry_vt = dest_end - 0x03050000 + 0x200
    sp, reset = struct.unpack_from("<II", out, entry_vt)
    if not (ram_start == 0x03004000 and init_size >= 8
            and sp == 0x03004000 and reset & 1):
        raise SystemExit(f"entry VT: row1 {ram_start:#x}/{init_size:#x}, IMG "
                         f"0x{entry_vt:X} = sp 0x{sp:08X} reset 0x{reset:08X}")
    # leading part-chain field: #12 (updater-accepted) carried our fw1 end here
    struct.pack_into("<I", out, 0x80, fw1_end)

    # empty marker container (the reference's trailing RKnanoFW + zeros)
    out[MARKER_HDR:MARKER_HDR + 16] = ref[MARKER_HDR:MARKER_HDR + 16]

    # self-consistency copy (the -3 check) at fw_end from the template header
    fw_end2 = struct.unpack_from("<I", out, 0x1F4)[0]
    out[fw_end2:fw_end2 + 0x200] = out[0:0x200]

    # trailer CRC over everything except the last 4 bytes
    crc = rknano_crc32(bytes(out[:-4]))
    struct.pack_into("<I", out, IMG_SIZE - 4, crc)
    out_path.write_bytes(out)
    print(f"      ref-shape 5MB image -> {out_path} (fw1 end 0x{fw1_end:X}, CRC 0x{crc:08X})")
    return out_path


def main() -> int:
    ap = argparse.ArgumentParser(
        description="ELF -> fw1 slot -> device IMG + recomputed trailer CRC",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__)
    ap.add_argument("input", nargs="?", type=Path,
                    help="input ARM ELF or flat binary (omit with --placeholder)")
    ap.add_argument("-o", "--output", type=Path, default=Path("build/ReChord_AP.IMG"),
                    help="output IMG path (default build/ReChord_AP.IMG)")
    ap.add_argument("--stock", type=Path, default=Path(pack_img.DEFAULT_STOCK),
                    help=f"stock IMG to splice into (default {pack_img.DEFAULT_STOCK})")
    ap.add_argument("--load-base", type=lambda s: int(s, 0), default=0x03050000,
                    help="fw1 header load base / initial SP (default 0x03050000)")
    ap.add_argument("--load-addr", type=lambda s: int(s, 0), default=0x03050000,
                    help="flat binary only: link address of the blob (default 0x03050000)")
    ap.add_argument("--entry", type=lambda s: int(s, 0), default=None,
                    help="flat binary only: entry address (default = --load-addr)")
    ap.add_argument("--bb", type=Path, default=None,
                    help="optional custom section_3 (BB) bin; default keeps the "
                         "stock section_3 bytes untouched")
    ap.add_argument("--placeholder", action="store_true",
                    help="build a tiny placeholder ELF (vector table + reset "
                         "loop at 0x03050000) and run the chain on it")
    args = ap.parse_args()

    # ---- safety guards: never touch the fallback image or the stock source ----
    out_name = args.output.name
    if out_name == "ReChord_BB.IMG":
        print("ERROR: refusing to write build/ReChord_BB.IMG (safe fallback "
              "image - use build/ReChord_AP.IMG)", file=sys.stderr)
        return 1
    if args.output.resolve() == args.stock.resolve():
        print("ERROR: output must not be the stock IMG itself", file=sys.stderr)
        return 1

    # ---- 1. resolve input ----
    if args.placeholder:
        if args.input is not None:
            print("ERROR: --placeholder takes no input file", file=sys.stderr)
            return 1
        in_path = build_placeholder(Path("build/placeholder"))
        print(f"[1/5] placeholder ELF built: {in_path}")
    elif args.input is not None:
        in_path = args.input
        if not in_path.is_file():
            print(f"ERROR: input not found: {in_path}", file=sys.stderr)
            return 1
        if in_path.resolve() == args.stock.resolve():
            print("ERROR: input must not be the stock IMG", file=sys.stderr)
            return 1
        print(f"[1/5] input: {in_path}")
    else:
        print("ERROR: need an input ELF/binary or --placeholder", file=sys.stderr)
        return 1

    raw = in_path.read_bytes()
    if raw[:4] == b"\x7fELF":
        img_obj = pack_fw1.Elf32(raw)
        entry = img_obj.e_entry
        kind = "ELF32"
    else:
        entry = args.entry if args.entry is not None else args.load_addr
        img_obj = FlatImage(raw, args.load_addr, entry)
        kind = "flat binary"
    print(f"      kind: {kind}, entry 0x{entry:08X}"
          + (" (Thumb)" if entry & 1 else ""))

    # ---- 2. build the fw1 slot image (pack_fw1.py conventions) ----
    print("[2/5] fw1 slot image (tools/pack_fw1.py conventions)")
    rows, payload = pack_fw1.build_scatter_table(img_obj)
    if not rows:
        print("ERROR: no loadable segments in input", file=sys.stderr)
        return 1

    n_rows = len(rows)
    rows_limit = pack_img.FW1_TABLE_MAX        # 91 (header count field)
    # The table area MUST occupy count*16 bytes (zero-padded): pack_img's
    # parse_fw1 splits header|table(count*16)|payload by the count field.
    table_blob = b"".join(struct.pack("<IIII", *r) for r in rows)
    table_blob += bytes(rows_limit * 16 - len(table_blob))
    fw1 = bytearray()
    fw1 += pack_fw1.MAGIC
    # count field = TABLE CAPACITY (91): stock and reference both store
    # 0x5B here, not the live-row count (option-2 structural clone).
    fw1 += struct.pack("<II", args.load_base, rows_limit)
    fw1 += table_blob
    fw1 += payload

    fw1_path = args.output.with_suffix(".fw1")
    fw1_path.parent.mkdir(parents=True, exist_ok=True)
    fw1_path.write_bytes(fw1)

    payload_limit = pack_img.FW1_CODE_END - pack_img.FW1_CODE_OFF  # 0x57068

    print(f"      container: 'RKnanoFW' + SP 0x{args.load_base:08X} + "
          f"{n_rows} rows + {len(payload):,} B payload")
    print(f"      slot image: {len(fw1):,} bytes -> {fw1_path}")
    print("      scatter rows:")
    fmt_rows(rows)
    print("      segments:")
    for i, seg in enumerate(img_obj.program_headers()):
        if seg["type"] != pack_fw1.PT_LOAD or seg["memsz"] == 0:
            continue
        zlen = seg["memsz"] - seg["filesz"]
        print(f"        seg {i}: vaddr 0x{seg['vaddr']:08X}  "
              f"filesz 0x{seg['filesz']:06X} ({seg['filesz']:,} B)  "
              f"memsz 0x{seg['memsz']:06X}  zero-fill 0x{zlen:06X}")

    # slot sizes vs limits
    print("      slot sizes vs limits:")
    ok_rows = n_rows <= rows_limit
    ok_payload = len(payload) <= payload_limit
    print(f"        scatter rows : {n_rows:6d} / {rows_limit:6d} max      "
          f"[{'OK' if ok_rows else 'FAIL'}]")
    print(f"        fw1 payload  : {len(payload):6d} / {payload_limit:6d} "
          f"(0x{payload_limit:X})  [{'OK' if ok_payload else 'FAIL'}]")
    # src window check (CONFIRMED model): rows must source inside the loaded
    # RAM image window = RAM_IMG_BASE + payload span (incident 5667c9c4).
    # the loader loads the WHOLE IMG body (RAM = RAM_IMG_BASE + IMG offset):
    # row srcs may target anywhere in the loaded image (the reference's
    # poke row sources from inside its own code beyond the fw1 window).
    load_lo = pack_fw1.RAM_IMG_BASE
    load_hi = pack_fw1.RAM_IMG_BASE + 0x500004
    def row_ok(i, r):
        a, b, c, d = r
        if a < SAFE_MIN:
            return False
        if i % 2 == 0:
            # Type B {dest, src, size, dest_end}; a poke row (end == 0, the
            # reference carries one at index 2) copies `size` bytes only.
            if d == 0:
                return load_lo <= b and b + c <= load_hi
            return (d == ((a + c) & 0xFFFFFFFF)
                    and load_lo <= b and b + c <= load_hi)
        # Type A {ram_start, size, ram_end, total_obj}: end == start+size
        return c == ((a + b) & 0xFFFFFFFF)
    ok_xip = all(row_ok(i, r) for i, r in enumerate(rows))
    print(f"        src loaded-RAM window: 0x{load_lo:08X}..0x{load_hi:08X}  "
          f"[{'OK' if ok_xip else 'FAIL'}]")
    # Loader-code guard (DFIU 51cc212d): the stock loader code starts at
    # 0x03088000. A row that loads code past it overwrites the loader and
    # bricks the boot even when every size check above passes.
    LOADER_CODE_BASE = 0x03088000
    def row_load_end(i, r):
        a, b, c, d = r
        if i % 2 == 0:                 # Type B: dest_end (or dest+size poke)
            return d if d != 0 else ((a + c) & 0xFFFFFFFF)
        return c                       # Type A: ram_end
    loader_bad = [i for i, r in enumerate(rows)
                  if row_load_end(i, r) > LOADER_CODE_BASE]
    ok_loader = not loader_bad
    print(f"        loader code base: <= 0x{LOADER_CODE_BASE:08X}          "
          f"[{'OK' if ok_loader else 'FAIL'}]")
    if loader_bad:
        print(f"        rows crossing it: {loader_bad}", file=sys.stderr)
    # entry must land inside a loaded segment
    ok_entry = any(s["vaddr"] <= entry < s["vaddr"] + s["memsz"]
                   for s in img_obj.program_headers()
                   if s["type"] == pack_fw1.PT_LOAD)
    print(f"        entry in seg : 0x{entry:08X}                       "
          f"[{'OK' if ok_entry else 'FAIL'}]")
    if not (ok_rows and ok_payload and ok_xip and ok_entry and ok_loader):
        print("ERROR: slot limit / contract check failed", file=sys.stderr)
        return 1

    # ---- 2b. neutralize the sec2 stage (SIGNATURE GATE) ----
    # The stock's second-stage loader at IMG 0x57820 ("RKnanoFW" header)
    # validates fw1's signature ("fw1 Sign error!") and refuses custom fw1
    # images - this is what bricked three builds. The reference image zeroes
    # that header so the loader skips the stage and loads fw1 directly.
    # Minimal delta: zero the 16-byte sec2 header only.
    # ---- 3. wrap into the device IMG (pack_img.py logic) ----
    print(f"[3/5] IMG wrap (tools/pack_img.py pack_full)")
    if not args.stock.is_file():
        print(f"ERROR: stock IMG not found: {args.stock}", file=sys.stderr)
        return 1
    stock = args.stock.read_bytes()
    if len(stock) != pack_img.IMG_SIZE:
        print(f"ERROR: stock size {len(stock):,} != {pack_img.IMG_SIZE:,}",
              file=sys.stderr)
        return 1
    if args.bb is not None:
        section3 = args.bb.read_bytes()
    else:
        section3 = stock[pack_img.SECTION3_OFFSET:pack_img.SECTION3_END]
    if len(section3) > pack_img.SECTION3_SIZE:
        print(f"ERROR: section_3 too large: {len(section3):,} > "
              f"{pack_img.SECTION3_SIZE:,}", file=sys.stderr)
        return 1

    pack_img.pack_full(stock, bytes(fw1), section3, args.output)

    # ---- 4. recompute the trailer CRC ----
    print("[4/5] trailer CRC recompute "
          "(poly 0x04C10DB7, non-reflected, init 0, xorout 0)")
    # self-check the primitive against the known stock trailer first
    stock_crc = rknano_crc32(stock[:-4])
    stock_trailer = pack_img.read_trailer(stock)
    print(f"      self-check vs stock body : 0x{stock_crc:08X}  "
          f"stock trailer 0x{stock_trailer:08X}  "
          f"[{'OK' if stock_crc == stock_trailer else 'FAIL'}]")

    out_bytes = bytearray(args.output.read_bytes())
    # Neutralize the sec2 signature gate (step 2b): zero its 16-byte
    # "RKnanoFW" header so the loader skips the stage that validates fw1
    # ("fw1 Sign error!"). The reference image does exactly this.
    SEC2_HEADER_OFF = 0x57820
    out_bytes[SEC2_HEADER_OFF:SEC2_HEADER_OFF + 16] = bytes(16)
    # ALSO neutralize the section_3 container header: the reference image is
    # fw1-ONLY (5MB, no sec2, no section_3) and installs through the
    # updater's fw1-only path. Carrying a stock section_3 alongside a custom
    # fw1 is a pairing that never occurs naturally (reference=custom+custom,
    # stock=stock+stock) and the A/B slot validation may reject it (verdict
    # #6 concluded the fw1 container structure itself is not the cause).
    SEC3_HEADER_OFF = 0x81A14
    out_bytes[SEC3_HEADER_OFF:SEC3_HEADER_OFF + 16] = bytes(16)
    # SELF-CONSISTENCY COPY (the -3 check, decompiled from the reference's
    # updater FUN_03064474): the image's first 0x200 bytes must appear again
    # at offset fw_end = LE32(header[0x1F4]). No image we ever built had this
    # - it is what the updater rejected every time.
    fw_end = struct.unpack_from("<I", out_bytes, 0x1F4)[0]
    out_bytes[fw_end:fw_end + 0x200] = out_bytes[0:0x200]
    crc_before = pack_img.read_trailer(out_bytes)
    crc_after = rknano_crc32(bytes(out_bytes[:-4]))
    struct.pack_into("<I", out_bytes, pack_img.TRAILER_OFFSET, crc_after)
    args.output.write_bytes(out_bytes)
    print(f"      CRC before (carried from stock) : 0x{crc_before:08X}")
    print(f"      CRC after  (recomputed)        : 0x{crc_after:08X}")

    # ---- 5. verification report ----
    print("[5/5] verification report")
    img = args.output.read_bytes()
    checks = []

    def check(name: str, cond: bool, detail: str = "") -> None:
        checks.append(cond)
        print(f"      [{'OK' if cond else 'FAIL'}] {name}"
              + (f"  {detail}" if detail else ""))

    check("IMG size", len(img) == pack_img.IMG_SIZE,
          f"{len(img):,} bytes (0x{len(img):X})")
    hdr = struct.pack("<8sII", b"RKnanoFW", args.load_base, rows_limit)
    # Boot-contract byte check (R-e9e18de6c692): the vector table must be
    # readable at RAM 0x03050000 = IMG 0xD68 under the contiguous-load model.
    voff = pack_fw1.VECTOR_RAM - pack_fw1.RAM_IMG_BASE
    vt = struct.unpack_from("<II", img, voff)
    check(f"vector table @ IMG 0x{voff:X}",
          vt[0] == 0x03004000 and (vt[1] & 1) == 1,
          f"word0 SP 0x{vt[0]:08X} (want 0x03004000), word1 reset 0x{vt[1]:08X}")
    check("fw1 header @ 0x1F8",
          img[pack_img.FW1_HEADER_OFF:pack_img.FW1_HEADER_OFF + 16] == hdr,
          f"'RKnanoFW' + SP 0x{args.load_base:08X} + capacity {rows_limit} rows")
    check("scatter table @ 0x208",
          img[pack_img.FW1_TABLE_OFF:pack_img.FW1_TABLE_OFF + len(table_blob)]
          == table_blob,
          f"{n_rows} rows x 16 B")
    tbl_pad = img[pack_img.FW1_TABLE_OFF + len(table_blob):pack_img.FW1_CODE_OFF]
    check("table padding zeroed to 0x7B8",
          all(b == 0 for b in tbl_pad),
          f"{len(tbl_pad):,} bytes")
    check("payload @ 0x7B8",
          img[pack_img.FW1_CODE_OFF:pack_img.FW1_CODE_OFF + len(payload)] == payload,
          f"{len(payload):,} bytes")
    tail = img[pack_img.FW1_CODE_OFF + len(payload):pack_img.FW1_CODE_END]
    check("payload tail zeroed to 0x57820",
          all(b == 0 for b in tail),
          f"{len(tail):,} bytes")
    s3 = img[pack_img.SECTION3_OFFSET:pack_img.SECTION3_OFFSET + len(section3)]
    check("section_3 header neutralized",
          img[0x81A14:0x81A24] == bytes(16),
          "fw1-only image like the reference")
    fw_end_chk = struct.unpack_from("<I", img, 0x1F4)[0]
    check("header copy @ fw_end (-3 check)",
          img[fw_end_chk:fw_end_chk + 0x200] == img[0:0x200],
          f"first sector duplicated at 0x{fw_end_chk:X}")
    check("sec2 header neutralized",
          img[0x57820:0x57830] == bytes(16),
          "16 bytes zeroed - loader skips the fw1 signature check")
    # section_3 body is preserved stock-verbatim; only its 16-byte container
    # header is neutralized (fw1-only install path).
    s3_body_ok = (s3[16:] == section3[16:]) and s3[:16] == bytes(16)
    check("section_3 (BB) region", s3_body_ok,
          f"{len(section3):,} bytes"
          + (" (stock preserved)" if args.bb is None else f" (from {args.bb})"))
    stored = pack_img.read_trailer(img)
    check("trailer == recomputed CRC",
          stored == crc_after == rknano_crc32(img[:-4]),
          f"0x{stored:08X} @ 0x{pack_img.TRAILER_OFFSET:08X}")

    print()
    print("      layout summary:")
    print(f"        fw1 header   @ IMG 0x{pack_img.FW1_HEADER_OFF:06X} (16 B)")
    print(f"        scatter rows @ IMG 0x{pack_img.FW1_TABLE_OFF:06X}"
          f" .. 0x{pack_img.FW1_TABLE_OFF + len(table_blob):06X}"
          f" (zero-padded to 0x{pack_img.FW1_CODE_OFF:06X})")
    print(f"        fw1 payload  @ IMG 0x{pack_img.FW1_CODE_OFF:06X}"
          f" .. 0x{pack_img.FW1_CODE_OFF + len(payload):06X}"
          f" (loaded RAM 0x{pack_fw1.RAM_IMG_BASE + pack_fw1.FW1_PAYLOAD_IMG_OFF:08X}"
          f" .. 0x{pack_fw1.RAM_IMG_BASE + pack_fw1.FW1_PAYLOAD_IMG_OFF + len(payload):08X})")
    print(f"        section_3    @ IMG 0x{pack_img.SECTION3_OFFSET:06X}"
          f" .. 0x{pack_img.SECTION3_END:06X}")
    print(f"        trailer      @ IMG 0x{pack_img.TRAILER_OFFSET:06X} "
          f"(CRC 0x{crc_after:08X} LE)")
    print(f"        entry (ELF)  0x{entry:08X}"
          + (" (Thumb)" if entry & 1 else "")
          + f"; container field 0 = load base 0x{args.load_base:08X}")
    print(f"        output       {args.output} ({len(img):,} bytes)")
    print(f"        fw1 slot     {fw1_path} ({len(fw1):,} bytes)")
    print()
    if all(checks):
        print("RESULT: PASS - all verification checks OK")
    # Test #9 companion output: the 5MB ref-shape image
    ref_img = Path("community/refcfw/HIFIEC90.IMG")
    if ref_img.exists():
        build_ref_shape_5m(bytes(img), ref_img, args.output.with_name("ReChord_AP5M.IMG"))
    else:
        print("RESULT: FAIL - see checks above", file=sys.stderr)
        return 1
    print()
    print("  NOTE: flash ReChord_AP5M.IMG (boots on hardware, verdict #20). A")
    print("  standalone fw1 has no USB/updater yet: leaving it needs maskrom")
    print("  (docs/DEBRICK-LINUX.md).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
