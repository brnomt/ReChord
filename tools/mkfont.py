#!/usr/bin/env python3
"""
mkfont.py — convert a TTF/OTF into a ReChord theme font atlas (`font.bin`).

This is the PC half of the custom-font prototype described in
docs/CUSTOM_FONTS.md. It rasterises a font at a fixed pixel size and packs a
compact, self-describing glyph atlas that the firmware reads with plain
sequential reads — no font engine, no hinting, no allocation.

    python3 tools/mkfont.py --font /path/to/Inter-Regular.ttf --px 10 \
        --bits 1 --out themes/glifo/assets/font.bin

Format (little-endian), version 1:

    offset  size  field
    0       4     magic "RCHF"
    4       1     version (1)
    5       1     bits per pixel (1, 2 or 4)
    6       1     first codepoint
    7       1     last codepoint
    8       1     cell height in px (max glyph ink height, 1..32)
    9       1     baseline offset from the cell top, in px
    10      1     max cell width in px (1..32)
    11      1     flags (bit0: proportional)
    12      2     glyph count (== last - first + 1)
    14      2     total file size in bytes
    16      8*n   metrics per glyph, in codepoint order:
                    advance  u8   px to move the pen (0 = blank)
                    xoff     i8   bitmap left offset from the pen
                    yoff     i8   bitmap top offset from the cell top
                    width    u8   bitmap width in px
                    height   u8   bitmap height in px
                    offset   u16  byte offset of the bitmap inside the file
                    pad      u8   (aligns each record to 8 bytes)
    ...           bitmaps, MSB-first, row stride = ceil(width * bits / 8),
                  values: bits=1 -> 0/1, 2 -> 0..3, 4 -> 0..15 (alpha),
                  0 = transparent
"""

from __future__ import annotations

import argparse
import struct
import sys

from PIL import Image, ImageDraw, ImageFont

MAGIC = b"RCHF"
VERSION = 1
HEADER_BYTES = 16
METRICS_BYTES = 8
MAX_CELL = 32


def rasterise(font, ch, bits):
    """Render one glyph. Returns (advance, xoff, yoff, w, h, rows)."""
    try:
        advance = int(round(font.getlength(ch)))
    except AttributeError:                      # very old PIL
        advance = font.getmask(ch).size[0]
    if advance <= 0:
        return 0, 0, 0, 0, 0, []

    ascent, descent = font.getmetrics()
    cell_h = ascent + descent
    img = Image.new("L", (max(advance, 1) + 8, cell_h + 12), 0)
    ImageDraw.Draw(img).text((3, 2), ch, fill=255, font=font)
    box = img.getbbox()
    if box is None:
        return advance, 0, 0, 0, 0, []
    left, top, right, bottom = box
    w, h = right - left, bottom - top
    if w <= 0 or h <= 0:
        return advance, 0, 0, 0, 0, []
    if w > MAX_CELL or h > MAX_CELL:
        raise SystemExit(
            f"glyph {ch!r} is {w}x{h}px, over the {MAX_CELL}px per-cell cap")
    crop = img.crop(box)

    levels = (1 << bits) - 1
    stride = (w * bits + 7) // 8
    quant = [[(crop.getpixel((x, y)) * levels + 127) // 255
              for x in range(w)] for y in range(h)]
    # 1-bit thresholding erases genuinely thin glyphs (' at 8 px is one
    # grey column). A visible faint mark beats an empty cell, so keep the
    # brightest pixel when nothing survived the threshold.
    if not any(any(row) for row in quant):
        peak = max((crop.getpixel((x, y)), x, y)
                   for y in range(h) for x in range(w))
        if peak[0] > 0:
            quant[peak[2]][peak[1]] = 1

    rows = []
    for y in range(h):
        row = bytearray(stride)
        for x in range(w):
            q = quant[y][x]
            if not q:
                continue
            bitpos = x * bits
            for b in range(bits):
                if q & (1 << (bits - 1 - b)):
                    row[(bitpos + b) // 8] |= 0x80 >> ((bitpos + b) % 8)
        rows.append(bytes(row))
    return advance, left - 3, top - 2, w, h, rows


def build(path, px, bits, first, last):
    font = ImageFont.truetype(path, px)
    ascent, _descent = font.getmetrics()

    glyphs = [(code, rasterise(font, chr(code), bits))
              for code in range(first, last + 1)]

    blob = bytearray()
    offsets = []
    for _, (_, _, _, _w, _h, rows) in glyphs:
        offsets.append(HEADER_BYTES + METRICS_BYTES * len(glyphs) + len(blob))
        for row in rows:
            blob += row

    cell_h = max([g[1][4] for g in glyphs] + [1])
    max_w = max([g[1][3] for g in glyphs] + [1])
    total = HEADER_BYTES + METRICS_BYTES * len(glyphs) + len(blob)

    out = bytearray()
    out += MAGIC
    out += struct.pack("<8BHH", VERSION, bits, first, last,
                       min(cell_h, MAX_CELL), min(ascent, 255), max_w,
                       0x01, len(glyphs), total)
    for i, (_, (advance, xoff, yoff, w, h, _rows)) in enumerate(glyphs):
        out += struct.pack("<BbbBBHB", min(advance, 255), xoff, yoff,
                           w, h, offsets[i], 0)
    out += blob
    return bytes(out), cell_h, max_w, len(glyphs)


def preview(data, out_path):
    """Contact sheet straight from the packed bytes (proves the packing)."""
    version, bits, first, last, _ch, _base, _mw, _flags, count, _total = \
        struct.unpack_from("<8BHH", data, 4)
    levels = (1 << bits) - 1
    scale = 3
    cells = []
    for i in range(count):
        advance, _xo, _yo, w, h, off, _pad = struct.unpack_from(
            "<BbbBBHB", data, HEADER_BYTES + i * METRICS_BYTES)
        stride = (w * bits + 7) // 8
        glyph = []
        for y in range(h):
            line = []
            for x in range(w):
                bitpos = y * stride * 8 + x * bits
                byte = data[off + (bitpos // 8)]
                shift = 8 - bits - (bitpos % 8)
                line.append((byte >> shift) & levels)
            glyph.append(line)
        cells.append((advance, w, h, glyph))

    cell_h = max([c[2] for c in cells] + [1])
    total_w = sum(max(c[0], 1) for c in cells)
    img = Image.new("L", (total_w * scale, cell_h * scale), 0)
    pen = 0
    for advance, w, h, glyph in cells:
        for y, line in enumerate(glyph):
            for x, v in enumerate(line):
                img.putpixel((pen * scale + x * scale,
                              y * scale), v * 255 // levels)
        pen += max(advance, 1)
    img.save(out_path)
    print(f"  preview -> {out_path} ({total_w}x{cell_h}px @ {scale}x)"
          f"  [version {version}, {count} glyphs, {first}..{last}]")


def main(argv=None):
    ap = argparse.ArgumentParser(
        description="Convert a TTF/OTF into a ReChord theme font atlas.")
    ap.add_argument("--font", required=True, help="TTF/OTF file")
    ap.add_argument("--px", type=int, default=10, help="pixel size (default 10)")
    ap.add_argument("--bits", type=int, default=1, choices=(1, 2, 4),
                    help="1 = 1-bit, 2 = 2-bit AA, 4 = 4-bit AA (default 1)")
    ap.add_argument("--first", type=int, default=32,
                    help="first codepoint (default 32)")
    ap.add_argument("--last", type=int, default=126,
                    help="last codepoint (default 126; 255 adds Latin-1)")
    ap.add_argument("--out", required=True, help="output .bin path")
    ap.add_argument("--preview", help="optional PNG contact sheet")
    args = ap.parse_args(argv)

    if args.px < 6 or args.px > MAX_CELL:
        raise SystemExit(f"--px must be 6..{MAX_CELL}")
    if not (0 <= args.first <= args.last <= 255):
        raise SystemExit("--first/--last must be 0..255 with first <= last")

    data, cell_h, max_w, count = build(args.font, args.px, args.bits,
                                       args.first, args.last)
    with open(args.out, "wb") as f:
        f.write(data)

    print(f"{args.out}: {len(data)} B, {count} glyphs, "
          f"cell {max_w}x{cell_h}px, {args.bits}-bit")
    print(f"  bytes/glyph average: {len(data) / max(count, 1):.1f}")
    if len(data) > 4096:
        print("  WARNING: over the 4 KiB per-asset cap this prototype allows")
    if args.preview:
        preview(data, args.preview)


if __name__ == "__main__":
    sys.exit(main())
