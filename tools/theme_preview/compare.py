#!/usr/bin/env python3
"""compare.py — side-by-side sheets: approved mockup vs engine render.

Usage: compare.py <mockup-theme-dir> <docs-theme-dir>
The mockup PNGs are 2x; the renderer writes *_2x.png in the docs dir.
Also builds moods.png from the six home states.
"""
import os
import sys
from PIL import Image, ImageDraw

BORDER = 6
LABEL_H = 16


def pair(mock, render, out, left, right):
    a = Image.open(mock).convert("RGB")
    b = Image.open(render).convert("RGB")
    h = max(a.height, b.height)
    im = Image.new("RGB", (a.width + b.width + BORDER * 3,
                          h + BORDER * 2 + LABEL_H), (28, 28, 30))
    im.paste(a, (BORDER, BORDER + LABEL_H))
    im.paste(b, (BORDER * 2 + a.width, BORDER + LABEL_H))
    d = ImageDraw.Draw(im)
    d.text((BORDER, 2), left, fill=(220, 220, 220))
    d.text((BORDER * 2 + a.width, 2), right, fill=(220, 220, 220))
    im.save(out)


def moods(docs, out):
    names = ["home_playing", "home_paused", "home_press",
             "home_loud", "home_low", "home_sleep"]
    labels = ["playing", "paused", "key pressed", "loud",
              "low battery", "asleep"]
    tiles = []
    for n in names:
        p = os.path.join(docs, n + "_2x.png")
        if os.path.exists(p):
            tiles.append((labels[len(tiles)], Image.open(p).convert("RGB")))
    if not tiles:
        return
    w, h = tiles[0][1].size
    im = Image.new("RGB", (w * 2 + BORDER * 3, (h + LABEL_H) * 3 + BORDER * 4),
                   (28, 28, 30))
    d = ImageDraw.Draw(im)
    for i, (label, tile) in enumerate(tiles):
        x = BORDER + (i % 2) * (w + BORDER)
        y = BORDER + (i // 2) * (h + LABEL_H + BORDER)
        im.paste(tile, (x, y + LABEL_H))
        d.text((x, y + 2), label, fill=(220, 220, 220))
    im.save(out)


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    mock, docs = sys.argv[1], sys.argv[2]
    pairs = [
        ("home.png", "home_playing_2x.png", "compare_home.png", "mockup", "engine"),
        ("np_coverA.png", "np_cover_2x.png", "compare_np.png", "mockup", "engine"),
        ("np_nocover.png", "np_nocover_2x.png", "compare_np_nocover.png",
         "mockup", "engine"),
        ("library.png", "library_2x.png", "compare_library.png", "mockup", "engine"),
        ("library_cat.png", "library_cat_2x.png", "compare_library_cat.png",
         "mockup", "engine"),
        ("settings.png", "settings_2x.png", "compare_settings.png", "mockup", "engine"),
    ]
    for m, r, out, l, rr in pairs:
        mp, rp = os.path.join(mock, "2x", m), os.path.join(docs, r)
        if not (os.path.exists(mp) and os.path.exists(rp)):
            print("skip", out)
            continue
        pair(mp, rp, os.path.join(docs, out), l, rr)
        print("wrote", out)
    moods(docs, os.path.join(docs, "moods.png"))
    print("wrote moods.png")


if __name__ == "__main__":
    sys.exit(main())
