# Glifo

`themes/glifo/` is the data-only Nothing-style screen: pure black, one red,
a ring of 24 lamps and a dot-matrix clock. User-facing strings are Spanish
and ASCII (`REPRODUCIENDO`, `EN PAUSA`, `DETENIDO`, `BIBLIOTECA`, `AHORA`,
`AJUSTES`). The built-in font has no accents.

Install by copying the directory to `/.rechord/themes/glifo/` and choosing
**Glifo** in Settings. Nothing in the package is compiled.

## Now Playing

Left: 24 lamps on a circle (`sin()` at 15° steps, same function as the red
playhead, so the head sits on its lamp). Lamps already passed are white while
playing and drop back to the dim grey when paused. The head is a larger red
disc; while paused it blinks (`mod(t, 800) < 400`).

The elapsed time is a 5×7 dot matrix, each cell scaled to a 3 px block
(19×27). Ten digits do not fit in eight assets, so they are stacked in pairs
(`d01` is 0 over 1, through `d89`). The skin shifts the bitmap by one glyph
and paints a black strip over the other half. The colon is its own 8×27
bitmap. Remaining time, the header and the metadata use the 8×8 font.
`lvl_l`/`lvl_r` drive a row of ten lamps; the peak lamp is red. Battery under
15% turns the percent red.

`refresh_ms=100` redraws the whole skin. There is no cover to re-stream.

## Home and lists

The home shows the `GLIFO` wordmark (1-bit, pitch-2 dot matrix), the current
title while a track is loaded, and a vertical list. Library, category,
browser and Settings keep the compiled behaviour and draw a grey header with
a red rule.

## Budget

| item | value |
|---|---|
| now_playing.skin | 4082 / 4096 B, 52 / 52 sections |
| assets | 7 files, 949 / 1024 B (`d01`…`d89`, `colon`, `logo`) |

The digit bitmaps are original 5×7 drawings, raw MSB-first rows. The repo
has no theme-bitmap generator, so the rasteriser was not committed.

## Differences from the mockup

* No theme font. The huge title/artist of the mockup is the 8×8 font,
  truncated with `~`. The clock is the bitmap hero.
* Ring lamps are 4×4 squares. The playhead is the round red disc.
* `24/96` and the `03 / 14` playlist counter are not on this screen. The
  skin is full: 52 sections and 14 bytes under the 4 KiB cap. Remaining time
  is the line under the clock.
* The header reads `REPRODUCIENDO` rather than the mockup's English
  `NOW PLAYING`.

Screenshots (engine, `tools/theme_preview/run.sh`):

![playing and paused](pair_2x.png)
