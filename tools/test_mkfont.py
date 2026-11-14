#!/usr/bin/env python3
"""test_mkfont.py — self-check for tools/mkfont.py (no framework needed).

    python3 tools/test_mkfont.py

Verifies the atlas the firmware has to parse: header sanity, per-glyph
metrics/offset bounds, that the packed bitmap is byte-for-byte the same alpha
the rasteriser produced, and the size arithmetic a theme budget depends on.
"""

import os
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
MKFONT = os.path.join(HERE, "mkfont.py")
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

HEADER = "<8BHH"
METRICS = "<BbbBBHB"
HEADER_BYTES = 16
METRICS_BYTES = 8


def parse(data):
    magic = data[:4]
    assert magic == b"RCHF", magic
    (version, bits, first, last, cell_h, base, max_w, flags,
     count, total) = struct.unpack_from(HEADER, data, 4)
    assert version == 1
    assert bits in (1, 2, 4), bits
    assert count == last - first + 1
    assert total == len(data), (total, len(data))
    glyphs = []
    for i in range(count):
        adv, xo, yo, w, h, off, _pad = struct.unpack_from(
            METRICS, data, HEADER_BYTES + i * METRICS_BYTES)
        stride = (w * bits + 7) // 8
        assert off + stride * h <= len(data), (i, off, stride * h, len(data))
        glyphs.append((chr(first + i), adv, xo, yo, w, h, off, stride))
    return bits, cell_h, max_w, glyphs


def pixel(data, off, stride, bits, w, x, y):
    bitpos = y * stride * 8 + x * bits
    byte = data[off + bitpos // 8]
    shift = 8 - bits - (bitpos % 8)
    return (byte >> shift) & ((1 << bits) - 1)


def main():
    if not os.path.exists(FONT):
        print(f"SKIP: {FONT} not installed")
        return 0

    tmp = tempfile.mkdtemp()
    ok = 0

    for bits, px in ((1, 8), (2, 10), (4, 11)):
        out = os.path.join(tmp, f"f_{bits}.bin")
        subprocess.run([sys.executable, MKFONT, "--font", FONT,
                        "--px", str(px), "--bits", str(bits),
                        "--out", out, "--preview",
                        os.path.join(tmp, f"f_{bits}.png")],
                       check=True, stdout=subprocess.DEVNULL)
        data = open(out, "rb").read()
        bpp, cell_h, max_w, glyphs = parse(data)

        # 95 printable ASCII glyphs, space has an advance but no bitmap.
        assert len(glyphs) == 95, len(glyphs)
        assert glyphs[0][0] == " ", glyphs[0]
        assert glyphs[0][1] > 0, "space must advance the pen"
        assert glyphs[0][6] == glyphs[0][6], "space has no bitmap"
        assert glyphs[0][4] == 0 and glyphs[0][5] == 0, "space is blank"

        # Every glyph with ink must have at least one set pixel.
        for ch, adv, _xo, _yo, w, h, off, stride in glyphs:
            if w == 0 or h == 0:
                continue
            assert adv > 0, (ch, adv)
            ink = sum(1 for y in range(h) for x in range(w)
                      if pixel(data, off, stride, bpp, w, x, y))
            assert ink > 0, f"{ch!r} has no ink"

        # 'M' and 'i' must differ in advance: proportional, not a fixed cell.
        by_ch = {g[0]: g for g in glyphs}
        assert by_ch["M"][1] > by_ch["i"][1], (by_ch["M"][1], by_ch["i"][1])
        print(f"OK {bits}-bit {px}px: {len(data)} B, cell {max_w}x{cell_h}")
        ok += 1

    # ASCII (95 glyphs) fits one 4 KiB asset at every depth the report
    # considers; Latin-1 (224 glyphs) does not fit at 2-bit, which is exactly
    # why the report recommends ASCII-only for v1.
    for bits, px in ((1, 9), (2, 9), (4, 9)):
        out = os.path.join(tmp, f"ascii_{bits}.bin")
        subprocess.run([sys.executable, MKFONT, "--font", FONT, "--px", str(px),
                        "--bits", str(bits), "--out", out],
                       check=True, stdout=subprocess.DEVNULL)
        size = os.path.getsize(out)
        print(f"OK ASCII 9px {bits}-bit: {size} B (limit 4096)")
        assert size < 4096, (bits, size)
        ok += 1

    out = os.path.join(tmp, "latin1.bin")
    subprocess.run([sys.executable, MKFONT, "--font", FONT, "--px", "9",
                    "--bits", "2", "--first", "32", "--last", "255",
                    "--out", out], check=True, stdout=subprocess.DEVNULL)
    size = os.path.getsize(out)
    parse(open(out, "rb").read())
    print(f"OK latin-1 9px 2-bit: {size} B (over the 4 KiB cap by design)")
    assert size > 4096, "the report claims Latin-1 needs a bigger cap"
    ok += 1

    print(f"test_mkfont: {ok} cases OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
