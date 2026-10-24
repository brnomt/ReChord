#!/usr/bin/env python3
"""Host tests for tools/artpack (generated images only, no real covers)."""

import io
import os
import struct
import sys
import tempfile
import unittest
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import artpack as ap                                     # noqa: E402
from PIL import Image                                    # noqa: E402


def png(color, size=(40, 30)):
    im = Image.new("RGB", size, color)
    buf = io.BytesIO()
    im.save(buf, "PNG")
    return buf.getvalue()


def jpg(color, size=(64, 64)):
    im = Image.new("RGB", size, color)
    buf = io.BytesIO()
    im.save(buf, "JPEG", quality=90)
    return buf.getvalue()


def parse_header(pack):
    fields = struct.unpack_from("<4sII I HH IIII IIIIIIII IIII I", pack, 0)
    names = ("magic", "version", "header_bytes", "flags", "large", "small",
             "track_count", "image_count", "image_slots", "free_slots",
             "key_off", "key_bytes", "img_off", "img_bytes",
             "small_off", "small_bytes", "large_off", "large_bytes",
             "key_crc", "img_crc", "small_crc", "large_crc", "header_crc")
    return dict(zip(names, fields))


def make_track(path, size=16):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(b"\x00" * size)


class KeyTests(unittest.TestCase):
    def test_vectors(self):
        # Shared with firmware/services/tests/test_art_pack.c
        vectors = [
            ("Internal/Music/Album/01 Track.flac", 1234567,
             0xECA81DB7A41F0270),
            ("Internal/Music/Album/02 Track.mp3", 42,
             0x12C3628A14C79B82),
            ("SD/Other/song.wav", 0,
             0x7CD2E6CFF42D87CD),
            ("Internal/x.flac", 0xFFFFFFFF,
             0x976542C384FDA54D),
        ]
        for path, size, want in vectors:
            self.assertEqual(ap.track_key(path, size), want, path)

    def test_fnv_offset(self):
        self.assertEqual(ap.fnv1a64(b""), 0xCBF29CE484222325)

    def test_normalize(self):
        self.assertEqual(ap.normalize_path("Internal", "Music/x.flac"),
                         "Internal/Music/x.flac")
        self.assertEqual(ap.normalize_path("Internal", "/Music\\x.flac"),
                         "Internal/Music/x.flac")
        self.assertEqual(ap.normalize_path("SD/", "a/b"), "SD/a/b")


class ImageTests(unittest.TestCase):
    def test_rgb565_byte_order(self):
        im = Image.new("RGB", (2, 1), (255, 0, 0))
        data = ap.to_rgb565_bytes(im, False)
        self.assertEqual(data, b"\x00\xf8\x00\xf8")   # 0xF800 little-endian

    def test_palette_solid(self):
        im = Image.new("RGB", (64, 64), (200, 10, 10))
        p = ap.analyze(im)
        self.assertEqual([int(v) for v in p["dom"]], [200, 10, 10])
        self.assertEqual([int(v) for v in p["mean"]], [200, 10, 10])
        self.assertEqual([int(v) for v in p["sec"]], [200, 10, 10])
        # deep(): hue kept, V forced (0.42 for bg_a, 0.16 for panel)
        self.assertEqual(max(int(v) for v in p["bg_a"]), 107)
        self.assertEqual(max(int(v) for v in p["panel"]), 40)
        # accent: S*1.1 clamped to .9, V*1.15 (0.902 here, below the .98 cap)
        self.assertAlmostEqual(max(p["accent"]) / 255.0,
                               (200.0 / 255.0) * 1.15, places=2)
        # ink colours are fixed by analyze()
        self.assertEqual([int(v) for v in p["ink"]], [34, 30, 28])
        self.assertEqual([int(v) for v in p["ink_dim"]], [112, 104, 96])
        # label = cream mixed 10% with the mean
        self.assertGreater(p["label"][0], 200)
        self.assertLess(p["label"][1], 234)

    def test_palette_secondary_and_accent(self):
        im = Image.new("RGB", (64, 64), (20, 20, 20))     # dominant
        for x in range(32):
            for y in range(32):
                im.putpixel((x, y), (220, 30, 30))        # minority, saturated
        p = ap.analyze(im)
        self.assertLess(max(p["dom"]), 60)
        self.assertGreater(p["sec"][0], 150)
        self.assertGreater(p["accent"][0], 150)
        self.assertGreaterEqual(p["accent"][1], 0)
        # bg_b is the deep secondary: same hue family, V forced to .24
        self.assertEqual(max(int(v) for v in p["bg_b"]), 61)

    def test_dither_changes_output(self):
        im = Image.new("RGB", (8, 8))
        for y in range(8):
            for x in range(8):
                im.putpixel((x, y), (x * 32, y * 32, 128))
        a = ap.to_rgb565_bytes(im, False)
        b = ap.to_rgb565_bytes(im, True)
        self.assertNotEqual(a, b)

    def test_render_cover_sizes(self):
        im = ap.render_cover(png((10, 200, 30)), 24, 12, True)
        self.assertEqual(len(im["large"]), 24 * 24 * 2)
        self.assertEqual(len(im["small"]), 12 * 12 * 2)
        for key in ("dom", "sec", "mean", "accent", "bg_a", "bg_b", "glow",
                    "panel", "label", "ink", "ink_dim"):
            self.assertIn(key, im)
            self.assertEqual(len(im[key]), 3)


class PackTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = os.path.join(self.tmp.name, "Internal")
        self.addCleanup(self.tmp.cleanup)

    def _build(self, extra_root=None, dither=True):
        out = os.path.join(self.tmp.name, "ARTPACK.BIN")
        stats = ap.build(self.root, extra_root, 24, 12, dither, out)
        with open(out, "rb") as f:
            return stats, f.read()

    def test_layout_counts_and_crcs(self):
        music = os.path.join(self.root, "Music", "Album")
        make_track(os.path.join(music, "01.wav"))
        make_track(os.path.join(music, "02.wav"))
        with open(os.path.join(music, "cover.png"), "wb") as f:
            f.write(png((0, 0, 255)))
        make_track(os.path.join(self.root, "Music", "x.wav"))
        stats, pack = self._build()

        h = parse_header(pack)
        self.assertEqual(h["magic"], b"ARTP")
        self.assertEqual(h["version"], 2)
        self.assertEqual(stats["tracks"], 3)
        self.assertEqual(h["track_count"], 2)      # only tracks WITH art
        self.assertEqual(h["image_count"], 1)      # the album cover
        self.assertEqual(stats["missing"], 1)      # x.wav has no art
        self.assertEqual(h["image_slots"], h["image_count"] + 32)
        self.assertEqual(h["free_slots"], 32)
        self.assertEqual(h["large"], 24)
        self.assertEqual(h["small"], 12)

        for key in ("key_off", "img_off", "small_off", "large_off"):
            self.assertEqual(h[key] % 512, 0, key)
        # header CRC covers the 512-byte header with the CRC field zeroed
        zeroed = pack[:84] + b"\x00\x00\x00\x00" + pack[88:512]
        self.assertEqual(h["header_crc"],
                         zlib.crc32(zeroed) & 0xFFFFFFFF)
        key_section = pack[h["key_off"]:h["key_off"] + h["key_bytes"]]
        self.assertEqual(h["key_crc"], zlib.crc32(key_section) & 0xFFFFFFFF)
        self.assertEqual(pack[h["key_off"]:h["key_off"] + 16][8:12],
                         struct.pack("<I", 0))        # image index 0

        keys = [struct.unpack_from("<Q", key_section, i)[0]
                for i in range(0, h["track_count"] * 16, 16)]
        self.assertEqual(keys, sorted(keys))

        # image entry -> pixel blob offsets line up
        img = struct.unpack_from("<IIHHHHHHHHHHHH", pack, h["img_off"])
        self.assertEqual(img[0], 0)
        self.assertEqual(img[1], 0)
        self.assertEqual(img[13], 0)                     # reserved
        self.assertEqual(h["img_bytes"], (h["image_slots"] * 32 + 511) & ~511)
        self.assertEqual(len(pack), h["large_off"] + h["large_bytes"])

    def test_extra_root_keys_use_sd_volume(self):
        make_track(os.path.join(self.root, "Music", "a.wav"))
        sd = os.path.join(self.tmp.name, "SD")
        make_track(os.path.join(sd, "Music", "b.wav"))
        with open(os.path.join(sd, "Music", "cover.png"), "wb") as f:
            f.write(png((90, 90, 90)))
        stats, pack = self._build(extra_root=sd)
        h = parse_header(pack)
        self.assertEqual(stats["tracks"], 2)
        self.assertEqual(h["track_count"], 1)          # SD track has a cover
        key_section = pack[h["key_off"]:h["key_off"] + h["key_bytes"]]
        key = struct.unpack_from("<Q", key_section, 0)[0]
        size = 16                                       # make_track default
        self.assertEqual(key, ap.track_key("SD/Music/b.wav", size))

    def test_id3_embedded_cover(self):
        from mutagen.id3 import ID3, APIC
        path = os.path.join(self.root, "Music", "tagged.mp3")
        make_track(path, 0)
        tags = ID3()
        tags.add(APIC(encoding=3, mime="image/png", type=3, desc="front",
                      data=png((20, 200, 20))))
        tags.save(path)
        self.assertIsNotNone(ap.embedded_art(path))
        stats, _pack = self._build()
        self.assertEqual(stats["id3-apic"], 1)
        self.assertEqual(stats["missing"], 0)

    def test_atomic_write_leaves_no_temp(self):
        make_track(os.path.join(self.root, "Music", "a.wav"))
        out = os.path.join(self.tmp.name, "ARTPACK.BIN")
        ap.build(self.root, None, 16, 8, True, out)
        self.assertTrue(os.path.exists(out))
        self.assertFalse(os.path.exists(out + ".tmp"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
