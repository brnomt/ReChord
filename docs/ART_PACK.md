# ARTPACK.BIN — precomputed album art

ReChord shows album art with **zero image decoding and zero writes on the
device**. A PC-side tool reads each track's cover, crops/resizes it to two
RGB565 sizes, computes a palette, and writes one packed file:

```
/Internal/.rechord/ARTPACK.BIN          (device path; ARTPACK_PATH)
```

The firmware only reads it (`firmware/services/art_pack.c`). If the file is
missing or invalid, or the track has no art, nothing else breaks: `art_ok` is
0 and themes draw their `art` placeholder.

---

## 1. Building the pack

```sh
python3 -m pip install -r tools/artpack/requirements.txt
python3 tools/artpack/artpack.py --root /media/$USER/Internal \
                                 [--extra-root /media/$USER/SD] [-v]
```

Mount the device's **Internal** volume over USB (the same volume the firmware
calls `/Internal`). The output is `<root>/.rechord/ARTPACK.BIN`, written
atomically (temp file + `os.replace`). Tracks on the SD card are included with
`--extra-root`, keyed with the `SD` volume; the pack itself always lives on
Internal.

| flag | default | meaning |
|---|---|---|
| `--root` | — | internal volume root as mounted on the PC |
| `--extra-root` | — | SD card root (keys use the `SD` volume) |
| `--large N` | 120 | large cover size (px, square) |
| `--small N` | 48 | small cover size (px, square) |
| `--no-dither` | off | disable 8x8 ordered dithering |
| `--output PATH` | `<root>/.rechord/ARTPACK.BIN` | pack path |
| `-v` | off | list tracks with no art |

The summary prints tracks, unique images, missing art, per-source counts and
the file size. Exit code 0 even with missing covers; 2 on error.

A full rebuild is done every run (deterministic; no incremental state).

### Art source per track, in order

1. Embedded **FLAC PICTURE** (type 3 front cover preferred, else the first).
2. **ID3v2 APIC** (type 3 / `desc` containing "front", else the first).
3. **MP4 `covr`** (first image).
4. A folder file named `cover`, `folder` or `front` with `.jpg`, `.jpeg` or
   `.png` (case-insensitive) next to the track.

Any format Pillow can open (progressive JPEG, PNG, …) is accepted. Tracks
that end up without a cover are simply absent from the key table and counted
in the summary.

### Image processing

* Centre **square crop** then resize (`PIL.ImageOps.fit`, LANCZOS), both
  sizes from the same source.
* RGB565: `((r>>3)<<11) | ((g>>2)<<5) | (b>>3)`.
* **Byte order**: little-endian `uint16_t`, exactly `display_fb`'s in-RAM
  layout (the flush path sends the high byte first on the wire).
* **Dither** (default on): 8x8 Bayer, ±0.5 LSB of the target channel, applied
  per channel before quantization; recorded in the header flags.
* **Dedupe**: covers are hashed (SHA-256 of the processed large+small pixels +
  palette) so tracks sharing a cover share one image entry. Source images are
  also cached by content hash, so each unique file is decoded once.

---

## 2. Palette (the cassette mockup's `analyze()`, deterministic)

The tool reproduces the theme mockup's `analyze()` exactly, on the PC:

1. Downscale the source cover to **32x32** (LANCZOS) and read its pixels.
2. k-means, **k=4**, 10 iterations, Euclidean RGB, deterministic init: sort
   the pixels by luminance (stable) and seed the four centres at the 12.5/37.5/
   62.5/87.5 percentiles. Clusters are sorted by count (ties keep the seed
   order).
3. `dominant` = the most populous cluster centre.
4. `secondary` = among the other clusters, the one maximizing
   `distance(centre, dominant) * sqrt(count)` (the most prominent distinct
   colour).
5. `accent` = the cluster centre with the highest `S*V`; its colour is
   `fromHsv(h, clamp(s*1.1, .45, .90), clamp(v*1.15, .75, .98))`.
6. Derived shades (all from those centres):
   * `bg_a` = dominant at HSV **V=0.42** (S clamped to 0.85),
   * `bg_b` = secondary at **V=0.24**,
   * `glow` = mix(dominant, secondary, 0.5) at **V=0.55**,
   * `panel` = mean at **V=0.16**,
   * `label` = cream `#F2EADA` mixed **10 %** with the mean,
   * `ink` = `#221E1C`, `ink_dim` = `#706860` (fixed).
   `deep(c, V)` keeps the hue and sets `S = clamp(s*1.05, 0, .85)`, `V`.
7. `mean` = the average RGB of the 32x32 pixels.

All eleven colours are converted to RGB565 (channels truncated) and stored in
the image entry; themes read them as `art_c1`, `art_c2`, `art_mean`,
`art_accent`, `art_bg_a`, `art_bg_b`, `art_glow`, `art_panel`, `art_label`,
`art_ink`, `art_ink_dim`. No HSV maths runs on the device.

## 3. Key normalization

The firmware's track path is `"/" + media_rec.path`, where the media index
stores `"<Volume>/<relative path>"` (`Internal/Music/…`, `SD/…`). The pack key
therefore hashes:

* the path **without the leading `/`**, UTF-8 bytes, `/` separators (never
  `\`), exactly the names the index/FAT reports (no case folding);
* then the **file size** as 4 little-endian bytes.

Hash = **FNV-1a 64** (offset basis `0xCBF29CE484222325`, prime
`0x100000001B3`) over the path, continued over the size bytes.

Shared test vectors (also in `firmware/services/tests/test_art_pack.c`):

| normalized path | size | key |
|---|---|---|
| `Internal/Music/Album/01 Track.flac` | 1234567 | `0xECA81DB7A41F0270` |
| `Internal/Music/Album/02 Track.mp3` | 42 | `0x12C3628A14C79B82` |
| `SD/Other/song.wav` | 0 | `0x7CD2E6CFF42D87CD` |
| `Internal/x.flac` | 4294967295 | `0x976542C384FDA54D` |

Because the size is part of the key, replacing a file with a different size
misses (the tool must be re-run); editing a cover only requires re-running the
tool.

---

## 4. File format (version 2)

All integers little-endian. **Every section starts on a 512-byte boundary**
(the device reads whole sectors); unused tail bytes are zero. Offsets in the
header are absolute file offsets, sizes are section byte lengths (including
padding).

```
offset  type        field
0       u32         magic 'ARTP' (0x50545241 LE)
4       u32         version = 2
8       u32         header_bytes = 512
12      u32         flags: bit0 = ordered dithering used
16      u16         large_px
18      u16         small_px
20      u32         track_count          key-table entries (tracks WITH art)
24      u32         image_count          images in the table
28      u32         image_slots          reserved table entries (>= count)
32      u32         free_slots           image_slots - image_count
36      u32         key_off
40      u32         key_bytes
44      u32         img_off
48      u32         img_bytes
52      u32         small_off
56      u32         small_bytes
60      u32         large_off
64      u32         large_bytes
68      u32         key_crc              CRC-32 of the key section
72      u32         img_crc
76      u32         small_crc
80      u32         large_crc
84      u32         header_crc           CRC-32 of bytes 0..511 with this = 0
88..511                     zero (reserved)
```

CRC is **CRC-32/ISO-HDLC** (poly `0xEDB88320`, init/final `0xFFFFFFFF`), i.e.
`zlib.crc32` in Python.

**Key entry** (16 B each, sorted ascending by key, unique):

```
u64 key; u32 image_index; u32 reserved (0)
```

**Image entry** (32 B each, `image_slots` entries; unused ones zero):

```
u32 small_off   (relative to small_off)
u32 large_off   (relative to large_off)
u16 c1 (dominant), u16 c2 (secondary), u16 mean, u16 accent   (RGB565)
u16 bg_a, bg_b, glow, panel, label, ink, ink_dim              (RGB565)
u16 reserved (0)
```

**Pixel blobs**: `image_count` covers, tightly packed, row-major,
`px * px * 2` bytes each, little-endian RGB565. Small comes before large.

Order: header → key table → image table → small blob → large blob.

`image_slots`/`free_slots` reserve room for a future on-device fill (see §8).

### Device read path

* **Track change** (`art_pack_set_track`): stat the track (one open/close for
  the size), then read the header (1 sector), verify magic/version/header CRC,
  read+CRC the key section, binary-search the key (one key entry per probe),
  read the image entry (1 sector), validate offsets, store the palette.
* **Draw** (`art_pack_draw`): re-read the header (1 sector, detects a pack
  replaced between lookup and draw), seek to the image, then stream the pixels
  through **one 512-byte buffer** into `display_blit_span()` row segments,
  clipped by the display driver. No full-image buffer, no allocation.
* The pack is opened/closed per use: holding a handle would take one of
  `fs_target`'s four read slots, shared with the codec, the Core-B file
  service and the gapless pre-open.

---

## 5. Costs (measured)

| operation | sectors read | notes |
|---|---|---|
| track-change lookup | `1 + key_bytes/512 + ~log2(tracks) + 1` | e.g. 300 tracks → 21 sectors (~10 KB) |
| large cover draw (120²) | `1 + 57` | 28.8 KB |
| small cover draw (48²) | `1 + 10` | 4.6 KB |

Host wall time (tmpfs, 300-track/10-image pack): lookup 0.056 ms, draw
0.016 ms. On the device the per-sector cost dominates; the lookup is deferred
by the app loop until **after** the PCM ring is refilled (a gapless boundary
has ~85 ms of cover), so it never sits inside the boundary hand-off. A
static skin draws the cover once; an animated skin whose refresh band covers
the cover pays the stream on each refresh — keep covers on static screens or
below the animated band.

---

## 6. Failure modes

| condition | firmware result |
|---|---|
| pack missing | `art_ok = 0`, `art` draws the placeholder |
| bad magic / version / header CRC | lookup fails, art cleared |
| key-table CRC mismatch | lookup fails, art cleared |
| key not found (no art / file size changed) | `art_ok = 0` |
| track file unreadable (no size) | `art_ok = 0` |
| pack replaced after lookup | draw returns -1 and clears the state |
| truncated/corrupt pixel blob | draw stops, returns -1; no crash (bounds checked) |
| no `art` element in the skin | zero cost: nothing is read |

The firmware never writes to storage for this feature.

---

## 7. Theme API

```
[cover]
type=art
x=8
y=20
size=large        # small | large
```

Variables (RGB565 ints, usable in `color=` and expressions):
`art_ok`, `art_c1`, `art_c2`, `art_mean`, `art_accent`.
See `docs/THEMING.md` §4/§5.

---

## 8. Out of scope / future work

* **On-device fill of missing covers** (decode once on the device and write
  into the pack): the format reserves `image_slots` (image table) and stores
  `free_slots`, so a future writer can claim a slot; pixel blobs would grow by
  appending. Not implemented — this version is read-only.
* **CoverFlow**: small covers exist for many visible tiles and the large one
  for the selected item; a future screen can binary-search once per visible
  track and draw small covers. No format change needed.
* On-device JPEG/PNG decoding: deliberately excluded.


## Operational check after generating

`artpack.py` writes atomically (temp + rename) and prints the byte count.
Still verify the file on the mounted volume afterwards:

    ls -l <root>/.rechord/ARTPACK.BIN

It must be non-zero (a full library is a few MB). One field unit came back
from the device with the directory entry at size 0 while the data clusters
stayed allocated, which makes every lookup miss; regenerating fixed it. If
the size is 0, delete the file and re-run the tool.
