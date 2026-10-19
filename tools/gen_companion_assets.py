#!/usr/bin/env python3
"""Generate the Companion Bot 1-bit eye frames.

The theme is data: these .bin files are the theme's assets, shipped inside
the package and read by the firmware's generic skin engine. Format: raw
rows, MSB-first, stride = (w + 7) // 8.

Look follows intellar/oled_eye_display (MIT): filled rounded rectangles with
no pupils, a squashed/widened blink and a masked lower lid for "happy".
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "themes", "companion-bot", "assets")


def blank(w, h):
    return [[0] * w for _ in range(h)]


def rounded_rect(w, h, r):
    px = [[1] * w for _ in range(h)]
    r = min(r, w // 2, h // 2)
    for i in range(r):
        cut = r - i
        for y in (i, h - 1 - i):
            for x in range(cut):
                px[y][x] = 0
                px[y][w - 1 - x] = 0
    return px


def clear_triangle(px, p0, p1, p2):
    def edge(ax, ay, bx, by, x, y):
        return (x - ax) * (by - ay) - (y - ay) * (bx - ax)

    h = len(px)
    w = len(px[0])
    area = edge(p0[0], p0[1], p1[0], p1[1], p2[0], p2[1])
    if area == 0:
        return
    for y in range(h):
        for x in range(w):
            e0 = edge(p0[0], p0[1], p1[0], p1[1], x, y)
            e1 = edge(p1[0], p1[1], p2[0], p2[1], x, y)
            e2 = edge(p2[0], p2[1], p0[0], p0[1], x, y)
            if (e0 >= 0 and e1 >= 0 and e2 >= 0) or (
                    e0 <= 0 and e1 <= 0 and e2 <= 0):
                px[y][x] = 0


def happy(w, h, r, mirror):
    px = rounded_rect(w, h, r)
    # Reference happy_eye(): a black triangle over the lower lid, mirrored
    # per eye. The third point sits below the box and is clipped.
    if not mirror:
        clear_triangle(px, (0, h // 2), (w - 1, h // 2 + 5),
                       (0, h + h // 2))
    else:
        clear_triangle(px, (w - 1, h // 2), (0, h // 2 + 5),
                       (w - 1, h + h // 2))
    return px


def save(name, px):
    h = len(px)
    w = len(px[0])
    stride = (w + 7) // 8
    data = bytearray()
    for row in px:
        for byte in range(stride):
            value = 0
            for bit in range(8):
                x = byte * 8 + bit
                if x < w and row[x]:
                    value |= 0x80 >> bit
            data.append(value)
    path = os.path.join(OUT, name)
    with open(path, "wb") as f:
        f.write(data)
    print(f"{name:24s} {w}x{h} stride={stride} {len(data)} B")


def main():
    os.makedirs(OUT, exist_ok=True)
    save("eye_open.bin", rounded_rect(64, 64, 16))
    save("eye_half.bin", rounded_rect(64, 32, 10))
    save("eye_closed.bin", rounded_rect(64, 6, 3))
    save("eye_happy_left.bin", happy(64, 64, 16, False))
    save("eye_happy_right.bin", happy(64, 64, 16, True))


if __name__ == "__main__":
    main()
