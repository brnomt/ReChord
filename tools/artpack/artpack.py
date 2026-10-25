#!/usr/bin/env python3
"""
artpack.py — build <volume>/.rechord/ARTPACK.BIN for ReChord.

The device does ZERO image decoding and ZERO writes for album art: this PC-side
tool precomputes, for every track, a small/large RGB565 cover plus a palette,
and writes one packed, 512-byte-aligned file that the firmware reads with plain
sector reads.

    python3 tools/artpack/artpack.py --root /media/$USER/Internal \
                                     --extra-root /media/$USER/SD

Format, key normalization and the palette algorithm: docs/ART_PACK.md.
Exit code 0 on success (missing covers are reported, not fatal), 2 on error.
"""

from __future__ import annotations

import argparse
import hashlib
import io
import math
import os
import struct
import sys
import zlib

from PIL import Image, ImageOps

FORMAT_VERSION = 2
MAGIC = b"ARTP"
HEADER_BYTES = 512
KEY_ENTRY_BYTES = 16
IMAGE_ENTRY_BYTES = 32
AUDIO_EXT = (".mp3", ".flac", ".wav")
COVER_NAMES = ("cover", "folder", "front")
COVER_EXT = (".jpg", ".jpeg", ".png")
FREE_IMAGE_SLOTS = 32
DEFAULT_LARGE = 120
DEFAULT_SMALL = 48

FNV_OFFSET = 0xCBF29CE484222325
FNV_PRIME = 0x100000001B3
MASK64 = (1 << 64) - 1

BAYER8 = (
    (0, 32, 8, 40, 2, 34, 10, 42),
    (48, 16, 56, 24, 50, 18, 58, 26),
    (12, 44, 4, 36, 14, 46, 6, 38),
    (60, 28, 52, 20, 62, 30, 54, 22),
    (3, 35, 11, 43, 1, 33, 9, 41),
    (51, 19, 59, 27, 49, 17, 57, 25),
    (15, 47, 7, 39, 13, 45, 5, 37),
    (63, 31, 55, 23, 61, 29, 53, 21),
)


# --------------------------------------------------------------------------
# Keys
# --------------------------------------------------------------------------

def fnv1a64(data: bytes, h: int = FNV_OFFSET) -> int:
    for b in data:
        h = ((h ^ b) * FNV_PRIME) & MASK64
    return h


def normalize_path(volume: str, rel: str) -> str:
    """The firmware's "<Volume>/<relative path>" form (UTF-8, '/'-separated).

    media_scan builds records as "Internal/Music/..." and the player prefixes
    '/', so the key path has NO leading slash; separators are always '/'."""
    rel = rel.replace("\\", "/")
    while rel.startswith("/"):
        rel = rel[1:]
    return volume.rstrip("/") + "/" + rel


def track_key(normalized: str, size: int) -> int:
    """FNV-1a 64 over the normalized path, then over the file size (u32 LE)."""
    h = fnv1a64(normalized.encode("utf-8"))
    return fnv1a64(struct.pack("<I", size & 0xFFFFFFFF), h)


# --------------------------------------------------------------------------
# Art sources
# --------------------------------------------------------------------------

def _pick_picture(pictures):
    """Front cover (type 3) preferred, else the first picture."""
    if not pictures:
        return None
    for p in pictures:
        if getattr(p, "type", 0) == 3:
            return bytes(p.data)
    return bytes(pictures[0].data)


def embedded_art(path: str):
    """Embedded cover: FLAC PICTURE, ID3v2 APIC, then MP4 covr. None if none."""
    try:
        from mutagen.flac import FLAC
        data = _pick_picture(getattr(FLAC(path), "pictures", None))
        if data:
            return data, "flac-picture"
    except Exception:
        pass

    try:
        from mutagen.id3 import ID3, APIC
        tags = ID3(path)
        apics = [v for v in tags.values() if isinstance(v, APIC)]
        if apics:
            front = [a for a in apics
                     if getattr(a, "type", 0) == 3
                     or "front" in str(getattr(a, "desc", "")).lower()]
            pick = (front or apics)[0]
            return bytes(pick.data), "id3-apic"
    except Exception:
        pass

    try:
        from mutagen.mp4 import MP4
        tags = MP4(path).tags
        covr = tags.get("covr") if tags else None
        if covr:
            return bytes(covr[0]), "mp4-covr"
    except Exception:
        pass
    return None


def folder_art(directory: str):
    """cover.jpg / folder.jpg / front.jpg (case-insensitive, jpg/jpeg/png)."""
    try:
        names = os.listdir(directory)
    except OSError:
        return None
    lower = {n.lower(): n for n in names}
    for stem in COVER_NAMES:
        for ext in COVER_EXT:
            name = lower.get(stem + ext)
            if name:
                try:
                    with open(os.path.join(directory, name), "rb") as f:
                        return f.read()
                except OSError:
                    return None
    return None


def sidecar_art(path: str):
    """A cover file named after the track: `01. Song.flac` -> `01. Song.jpg`.

    Some libraries (especially FLAC rips) carry one image per track instead of
    a folder cover; the firmware's key is per track, so the sidecar is the
    natural source. Case-insensitive stem and extension, same COVER_EXT list.
    """
    stem = os.path.splitext(os.path.basename(path))[0]
    directory = os.path.dirname(path)
    try:
        names = os.listdir(directory)
    except OSError:
        return None
    lower = {n.lower(): n for n in names}
    for ext in COVER_EXT:
        name = lower.get((stem + ext).lower())
        if name:
            try:
                with open(os.path.join(directory, name), "rb") as f:
                    return f.read()
            except OSError:
                return None
    return None


# --------------------------------------------------------------------------
# Image processing
# --------------------------------------------------------------------------

def _clamp(v, lo, hi):
    return lo if v < lo else hi if v > hi else v


def rgb_to_565(r: int, g: int, b: int) -> int:
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def _rgb565(c) -> int:
    """Vector -> RGB565, truncating like the mockup's `c[0]>>3` bit ops."""
    return ((int(c[0]) >> 3) << 11) | ((int(c[1]) >> 2) << 5) | (int(c[2]) >> 3)


def _hsv(c):
    r, g, b = c[0] / 255.0, c[1] / 255.0, c[2] / 255.0
    mx = max(r, g, b)
    mn = min(r, g, b)
    d = mx - mn
    h = 0.0
    if d:
        if mx == r:
            h = ((g - b) / d) % 6
        elif mx == g:
            h = (b - r) / d + 2
        else:
            h = (r - g) / d + 4
        h *= 60
        if h < 0:
            h += 360
    return h, (d / mx if mx else 0.0), mx


def _from_hsv(h, s, v):
    c = v * s
    x = c * (1 - abs((h / 60) % 2 - 1))
    m = v - c
    if h < 60:
        r, g, b = c, x, 0
    elif h < 120:
        r, g, b = x, c, 0
    elif h < 180:
        r, g, b = 0, c, x
    elif h < 240:
        r, g, b = 0, x, c
    elif h < 300:
        r, g, b = x, 0, c
    else:
        r, g, b = c, 0, x
    return [(r + m) * 255.0, (g + m) * 255.0, (b + m) * 255.0]


def _mix(a, b, t):
    return [a[i] + (b[i] - a[i]) * t for i in range(3)]


def _lum(c):
    return 0.2126 * c[0] + 0.7152 * c[1] + 0.0722 * c[2]


def analyze(im: Image.Image) -> dict:
    """The mockup's analyze(): k-means (k=4) on a 32x32 downscale, then the
    cassette theme's derived colours. See docs/ART_PACK.md for the exact rules.
    """
    small = im.resize((32, 32), Image.Resampling.LANCZOS)
    px = list(small.getdata())
    n = len(px)
    mean = [sum(p[i] for p in px) / n for i in range(3)]

    # Deterministic init along luminance (stable order, like JS sort).
    order = sorted(range(n), key=lambda i: _lum(px[i]))
    C = [[float(v) for v in px[order[int((k + 0.5) / 4 * n)]]] for k in range(4)]
    cnt = [0] * 4
    for _ in range(10):
        acc = [[0.0, 0.0, 0.0] for _ in range(4)]
        cnt = [0] * 4
        for p in px:
            bi, bd = 0, 1e30
            for k, q in enumerate(C):
                dd = ((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2 +
                      (p[2] - q[2]) ** 2)
                if dd < bd:
                    bd = dd
                    bi = k
            acc[bi][0] += p[0]
            acc[bi][1] += p[1]
            acc[bi][2] += p[2]
            cnt[bi] += 1
        for k in range(4):
            if cnt[k]:
                C[k] = [acc[k][0] / cnt[k], acc[k][1] / cnt[k],
                        acc[k][2] / cnt[k]]

    cl = sorted([(C[k], cnt[k], k) for k in range(4)],
                key=lambda t: (-t[1], t[2]))
    dom = cl[0][0]

    sec = cl[1][0]
    best = -1.0
    for c, ccount, _k in cl[1:]:
        dd = math.sqrt((c[0] - dom[0]) ** 2 + (c[1] - dom[1]) ** 2 +
                       (c[2] - dom[2]) ** 2) * math.sqrt(ccount)
        if dd > best:
            best = dd
            sec = c

    centres = [c for c, _n, _k in cl]
    acc = sorted(centres,
                 key=lambda c: -(_hsv(c)[1] * _hsv(c)[2]))[0]
    h, s, v = _hsv(acc)
    accent = _from_hsv(h, _clamp(s * 1.1, 0.45, 0.90),
                       _clamp(v * 1.15, 0.75, 0.98))

    def deep(c, level):
        hh, ss, _vv = _hsv(c)
        return _from_hsv(hh, _clamp(ss * 1.05, 0.0, 0.85), level)

    return {
        "dom": dom,
        "sec": sec,
        "mean": mean,
        "accent": accent,
        "bg_a": deep(dom, 0.42),
        "bg_b": deep(sec, 0.24),
        "glow": deep(_mix(dom, sec, 0.5), 0.55),
        "panel": deep(mean, 0.16),
        "label": _mix([242.0, 234.0, 218.0], mean, 0.10),
        "ink": [34.0, 30.0, 28.0],
        "ink_dim": [112.0, 104.0, 96.0],
    }


def to_rgb565_bytes(im: Image.Image, dither: bool) -> bytes:
    """RGB565 little-endian, exactly the byte order display_fb uses."""
    px = im.load()
    w, h = im.size
    out = bytearray(w * h * 2)
    i = 0
    for y in range(h):
        row = BAYER8[y & 7]
        for x in range(w):
            r, g, b = px[x, y]
            t = ((row[x & 7] + 0.5) / 64.0 - 0.5) if dither else 0.0
            r5 = _clamp(int(r * 31.0 / 255.0 + t + 0.5), 0, 31)
            g6 = _clamp(int(g * 63.0 / 255.0 + t + 0.5), 0, 63)
            b5 = _clamp(int(b * 31.0 / 255.0 + t + 0.5), 0, 31)
            struct.pack_into("<H", out, i, (r5 << 11) | (g6 << 5) | b5)
            i += 2
    return bytes(out)


def render_cover(data: bytes, large: int, small: int, dither: bool):
    """Square centre-crop + LANCZOS resize, palette + both RGB565 blobs."""
    with Image.open(io.BytesIO(data)) as src:
        src.load()
        im = src.convert("RGB")
    big = ImageOps.fit(im, (large, large), method=Image.Resampling.LANCZOS,
                       centering=(0.5, 0.5))
    tiny = ImageOps.fit(im, (small, small), method=Image.Resampling.LANCZOS,
                        centering=(0.5, 0.5))
    out = analyze(im)
    out["large"] = to_rgb565_bytes(big, dither)
    out["small"] = to_rgb565_bytes(tiny, dither)
    return out


# --------------------------------------------------------------------------
# Pack assembly
# --------------------------------------------------------------------------

def _align512(n: int) -> int:
    return (n + HEADER_BYTES - 1) & ~(HEADER_BYTES - 1)


def _pad(data: bytes) -> bytes:
    n = _align512(len(data))
    if n != len(data):
        data = data + b"\x00" * (n - len(data))
    return data


def build_pack(tracks, images, large: int, small: int, dither: bool) -> bytes:
    """tracks: [(key, image_index)] sorted; images: [rendered dict]."""
    tracks = sorted(tracks, key=lambda t: t[0])
    image_count = len(images)
    image_slots = image_count + FREE_IMAGE_SLOTS

    key_blob = bytearray()
    for key, image in tracks:
        key_blob += struct.pack("<QII", key, image, 0)
    key_blob = _pad(bytes(key_blob))

    img_blob = bytearray()
    small_blob = bytearray()
    large_blob = bytearray()
    for im in images:
        img_blob += struct.pack(
            "<IIHHHHHHHHHHHH",
            len(small_blob),
            len(large_blob),
            _rgb565(im["dom"]),
            _rgb565(im["sec"]),
            _rgb565(im["mean"]),
            _rgb565(im["accent"]),
            _rgb565(im["bg_a"]),
            _rgb565(im["bg_b"]),
            _rgb565(im["glow"]),
            _rgb565(im["panel"]),
            _rgb565(im["label"]),
            _rgb565(im["ink"]),
            _rgb565(im["ink_dim"]),
            0,                      # reserved (pads the entry to 32 B)
        )
        small_blob += im["small"]
        large_blob += im["large"]
    for _ in range(image_slots - image_count):
        img_blob += b"\x00" * IMAGE_ENTRY_BYTES
    img_blob = _pad(bytes(img_blob))
    small_blob = _pad(bytes(small_blob))
    large_blob = _pad(bytes(large_blob))

    off = HEADER_BYTES
    key_off = off
    off += len(key_blob)
    img_off = off
    off += len(img_blob)
    small_off = off
    off += len(small_blob)
    large_off = off
    off += len(large_blob)

    hdr = bytearray(HEADER_BYTES)
    struct.pack_into(
        "<4sII I HH IIII IIIIIIII IIII I",
        hdr, 0,
        MAGIC, FORMAT_VERSION, HEADER_BYTES, 1 if dither else 0,
        large, small,
        len(tracks), image_count, image_slots, image_slots - image_count,
        key_off, len(key_blob), img_off, len(img_blob),
        small_off, len(small_blob), large_off, len(large_blob),
        0, 0, 0, 0,        # section CRCs, patched below
        0,                 # header CRC, patched below
    )
    # CRC-32/ISO-HDLC (zlib): the firmware uses the same convention.
    struct.pack_into("<I", hdr, 68, zlib.crc32(key_blob) & 0xFFFFFFFF)
    struct.pack_into("<I", hdr, 72, zlib.crc32(img_blob) & 0xFFFFFFFF)
    struct.pack_into("<I", hdr, 76, zlib.crc32(small_blob) & 0xFFFFFFFF)
    struct.pack_into("<I", hdr, 80, zlib.crc32(large_blob) & 0xFFFFFFFF)
    struct.pack_into("<I", hdr, 84, zlib.crc32(bytes(hdr)) & 0xFFFFFFFF)
    return bytes(hdr) + key_blob + img_blob + small_blob + large_blob


def write_atomic(path: str, data: bytes) -> None:
    tmp = path + ".tmp"
    with open(tmp, "wb") as f:
        f.write(data)
        f.flush()
        os.fsync(f.fileno())
    os.replace(tmp, path)


# --------------------------------------------------------------------------
# Driver
# --------------------------------------------------------------------------

def _walk_tracks(root: str, volume: str):
    for dirpath, _dirnames, filenames in os.walk(root):
        for name in filenames:
            if not name.lower().endswith(AUDIO_EXT):
                continue
            full = os.path.join(dirpath, name)
            rel = os.path.relpath(full, root)
            try:
                size = os.path.getsize(full)
            except OSError:
                continue
            yield full, normalize_path(volume, rel), size


def build(root: str, extra_root: str | None, large: int, small: int,
          dither: bool, output: str | None, verbose: bool = False):
    roots = [(root, "Internal")]
    if extra_root:
        roots.append((extra_root, "SD"))

    rendered = {}          # source sha256 -> rendered image dict
    images = []            # rendered dicts
    image_index = {}       # processed sha256 -> index
    tracks = []            # (key, image index)
    seen_keys = set()
    stats = {"tracks": 0, "missing": 0, "flac-picture": 0, "id3-apic": 0,
             "mp4-covr": 0, "file": 0, "sidecar": 0}
    missing = []

    for root_dir, volume in roots:
        for full, norm, size in _walk_tracks(root_dir, volume):
            stats["tracks"] += 1
            found = embedded_art(full)
            if found is None:
                data = sidecar_art(full)
                found = (data, "sidecar") if data else None
            if found is None:
                data = folder_art(os.path.dirname(full))
                found = (data, "file") if data else None
            if found is None:
                stats["missing"] += 1
                missing.append(norm)
                continue
            blob, source = found
            stats[source] += 1
            src_hash = hashlib.sha256(blob).hexdigest()
            im = rendered.get(src_hash)
            if im is None:
                try:
                    im = render_cover(blob, large, small, dither)
                except Exception as exc:
                    print(f"artpack: cannot decode cover for {norm}: {exc}",
                          file=sys.stderr)
                    stats["missing"] += 1
                    missing.append(norm)
                    continue
                rendered[src_hash] = im
            proc_hash = hashlib.sha256(
                im["large"] + im["small"] +
                bytes(struct.pack("<HHHHHHHHHHH",
                                  _rgb565(im["dom"]), _rgb565(im["sec"]),
                                  _rgb565(im["mean"]), _rgb565(im["accent"]),
                                  _rgb565(im["bg_a"]), _rgb565(im["bg_b"]),
                                  _rgb565(im["glow"]), _rgb565(im["panel"]),
                                  _rgb565(im["label"]), _rgb565(im["ink"]),
                                  _rgb565(im["ink_dim"])))).hexdigest()
            idx = image_index.get(proc_hash)
            if idx is None:
                idx = len(images)
                image_index[proc_hash] = idx
                images.append(im)
            key = track_key(norm, size)
            if key in seen_keys:
                print(f"artpack: duplicate key for {norm} (skipped)",
                      file=sys.stderr)
                continue
            seen_keys.add(key)
            tracks.append((key, idx))

    pack = build_pack(tracks, images, large, small, dither)
    if output is None:
        output = os.path.join(root, ".rechord", "ARTPACK.BIN")
    os.makedirs(os.path.dirname(output), exist_ok=True)
    write_atomic(output, pack)

    print(f"artpack: {stats['tracks']} tracks, {len(images)} images, "
          f"{stats['missing']} missing art")
    print(f"artpack: sources flac-picture={stats['flac-picture']} "
          f"id3-apic={stats['id3-apic']} mp4-covr={stats['mp4-covr']} "
          f"sidecar={stats['sidecar']} file={stats['file']}")
    print(f"artpack: wrote {output} ({len(pack)} bytes, "
          f"{large}x{large} + {small}x{small}, "
          f"dither={'on' if dither else 'off'})")
    if verbose:
        for norm in missing:
            print(f"artpack: no art: {norm}")
    return stats


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(
        description="Build ARTPACK.BIN (precomputed album art) for ReChord.")
    ap.add_argument("--root", required=True,
                    help="root of the internal volume as mounted on the PC")
    ap.add_argument("--extra-root",
                    help="root of the SD card (keys use the 'SD' volume)")
    ap.add_argument("--large", type=int, default=DEFAULT_LARGE,
                    help=f"large cover size (default {DEFAULT_LARGE})")
    ap.add_argument("--small", type=int, default=DEFAULT_SMALL,
                    help=f"small cover size (default {DEFAULT_SMALL})")
    ap.add_argument("--no-dither", action="store_true",
                    help="disable 8x8 ordered dithering")
    ap.add_argument("--output",
                    help="output path (default <root>/.rechord/ARTPACK.BIN)")
    ap.add_argument("-v", "--verbose", action="store_true",
                    help="list tracks with no art")
    args = ap.parse_args(argv)

    if not os.path.isdir(args.root):
        print(f"artpack: --root is not a directory: {args.root}",
              file=sys.stderr)
        return 2
    if args.extra_root and not os.path.isdir(args.extra_root):
        print(f"artpack: --extra-root is not a directory: {args.extra_root}",
              file=sys.stderr)
        return 2
    if args.large <= 0 or args.small <= 0:
        print("artpack: sizes must be positive", file=sys.stderr)
        return 2

    try:
        build(args.root, args.extra_root, args.large, args.small,
              not args.no_dither, args.output, args.verbose)
    except Exception as exc:                    # noqa: BLE001 — CLI boundary
        print(f"artpack: failed: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
