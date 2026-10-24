# Cassette — theme reference & screenshots

`themes/cassette/` is a data-only theme that recreates the cassette-deck
mockup on the 320x170 panel. This directory holds the visual source of truth
and the simulator-vs-mockup sheets.

```
mockup.html                  interactive mockup (open in a browser; ?view=screen
                             renders a single 320x170 frame)
home_*.png / np_*.png        mockup | firmware side-by-side (2x, RGB565)
noart_fallback.png           no cover: theme.cfg palette + cassette glyph
```

The firmware sheets are produced by the real interpreter and the real display
driver (headless host build using `panel_io_sim`), with a cover pack built by
`tools/artpack` from generated covers. The mockup halves come from headless
Chromium with the matching `view=screen` parameters.

## Palette

The pack carries the mockup's `analyze()` colours (see
[`docs/ART_PACK.md`](../../ART_PACK.md) §2): `art_c1/c2/mean/accent`,
`art_bg_a`, `art_bg_b`, `art_glow`, `art_panel`, `art_label`, `art_ink`,
`art_ink_dim`. The theme uses the deep pairs for the gradient, the panel for
cards, the cream label for the cassette paper and the fixed inks for text.
When the track has no cover, every `art_*` is 0 and the skins fall back to the
`theme.cfg` palette (warm grey / cream / amber).

## Engine pieces this theme needed

* `gradient` meter (two RGB565 colours, vertical/diagonal, 4x4 ordered
  dithering) — the full-screen background.
* `sqrt()` in expressions — the area-true tape-pack radius
  `r = sqrt(13^2 + (30^2-13^2)*f)`.
* text `maxc` (truncate with `~`) and `align=right` (counters, i/N, volume).
* section cap 32 → 52 (sections are parsed, not stored: **0 B RAM**).

## How it is built

* **Home**: gradient > shell (shadow, body, 1-px highlight) > window > reels
  (pack + lighter core + hub bitmap + rotating accent dot) > label pieces that
  cover the reel spill > badge/title/artist/stripes > lower shell (trapezoid,
  capstan holes, screws from an 8x8 bitmap, tape line) > status/counter >
  engine-drawn home list. The reels derive their radius and angle from
  `pos_ms` (never `t`), so they freeze exactly when playback pauses.
* **Now Playing**: gradient > title bar > artwork (shadow + 1-px border) >
  info card (title/artist/album, mini cassette, state/format, modes, volume,
  progress strip). `refresh_ms=100`: the art is re-streamed on every redraw,
  so 10 Hz keeps the hubs/progress alive without doubling the read cost.
* **Library/settings/browser**: solid palette surfaces (the engine's list rows
  use the theme palette), accent heading, static.

## Known differences from the mockup

* No radial "glow" behind the cassette (the engine has no radial gradient).
* Tape packs are flat dark brown with a lighter core instead of the mockup's
  radial sheen + tape rings; the small window scale ticks are omitted.
* The shell shadow is a solid offset (no alpha), the label is flat (no subtle
  gradient), and text is opaque on solid surfaces by design.
* Reel rotation is derived from the playback position (physical tape model),
  not from the mockup's wall clock; paused frames therefore differ in spoke
  phase.

## Budget

| item | value |
|---|---|
| home.skin | 3,947 B / 50 sections |
| now_playing.skin | 3,692 B / 41 sections |
| library/library_cat/browser/settings | 496–508 B / 6 sections each |
| assets | screw 8 B, hub 32 B |
| AP `.text` | +1,096 B (baseline `bab4831`) |
| AP `.bss` | +48 B; `_bss_end 0x03049E5C` (420 B below the 0x0304A000 cap) |
| code end | `0x03087F44` (188 B below the 0x03088000 loader base) |
