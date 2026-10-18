# Modular theme system (BRNOMT-69)

> Status: **design + budgets** (implementation in milestones, see §8). The issue
> requires explicit memory/time budgets *before* implementation, so this file is
> the contract the milestones are built against.

## 1. Today

- `firmware/theme/`: `theme_t` (7 RGB565 colors + 3 unused metrics), 4 built-in
  palettes (`Mono` default, `Classic`, `Paper`, `Night`), a `theme.cfg` file
  loader (`theme_file.c`, key=value, partial overrides) and `theme_boot_load()`.
- Screens never hardcode colors: they ask `theme_get()` through the
  `THEME_COLOR_*` macros (`firmware/ui/theme.h`), so a palette swap restyles
  every screen live.
- Layout is currently **code** (coordinates in each screen), not data.

So "modular" today = palettes only. This design adds packages + layouts + assets
without giving up the host-testable, no-heap model.

## 2. Package layout

```
/<volume>/.rechord/themes/<theme_id>/
  theme.cfg        # manifest + layout declarations (text, bounded)
  assets/          # 1-bit bitmaps/sprites/fonts, declared in the manifest
```

`theme.cfg` is the same `key=value` dialect as `rechord.cfg` (no new parser
engine), with two extensions: `[section]` lines and `asset`/`layout` records.
Example (also shipped as the reference package):

```
format=1
id=mono-nothing
name=Nothing Mono
author=ReChord
colors=#000000,#FFFFFF,#808080,#C6C6C6,#FFFFFF,#FFFFFF

[now_playing]
title.x=4   title.y=24  title.font=builtin-8x8  title.tag=title
artist.x=4  artist.y=36 artist.tag=artist       artist.style=dim
meta.x=4    meta.y=60   meta.tag=fmt+i/N        meta.align=left,right
bar.x=4 bar.y=74 bar.w=312 bar.h=2
time.x=4 time.y=84 time.tag=elapsed+total
bottom.x=4 bottom.y=148 bottom.tag=modes,volume
```

Rules: one `key=value` per line, `#` comments, unknown keys are ignored (forward
compatibility), and **every value is bounded** (§4).

## 3. Renderer API (theme-independent)

The theme describes intent; the renderer owns the panel. Small C surface, no
allocations, explicit clipping:

```c
void r_fill(int x, int y, int w, int h, uint16_t color);
void r_bitmap1(int x, int y, const uint8_t *bits, int w, int h, int stride,
               uint16_t fg, uint16_t bg);           /* 1-bit, MSB-first     */
void r_text(int x, int y, const char *s, int max_w, uint16_t fg, uint16_t bg);
void r_bar(int x, int y, int w, int h, int fill_px, uint16_t fg, uint16_t bg);
void r_rule(int x0, int y, int x1, uint16_t color);
void r_text_width(const char *s);                   /* for right/centre align */
```

Existing `display_*` primitives map 1:1 (`fill_rect`, `draw_text`); `bitmap1`
is the only new one (row-major MSB-first, stride from the manifest, clipped).
Text is 8x8 opaque cells today; `max_w` truncates at a cell boundary.

## 4. Bounds (hard limits, validated at load)

| Thing | Limit |
|---|---|
| themes enumerable | 16 |
| manifest size read | 4 KiB |
| sections per theme | 8 |
| keys per section | 24 |
| string value | 64 bytes |
| assets per theme | 16 |
| single asset bitmap | 64 x 64 px, stride <= 8 B |
| asset bytes cached | 8 KiB total |
| layout coordinates | 0..319 / 0..169, w/h >= 1 |
| text length drawn | 40 chars (title 40, artist 40, meta 24) |
| evaluation steps per frame | 512 |

Anything out of range **rejects the theme** (not a clamp): a malformed package
must never produce a half-drawn screen.

## 5. Measured budgets (2026-10-18, `build/cfw/rechord_ap.elf`)

- `.bss` ends at `0x03046030`; the hard AP gate is `0x0304A000` → **16,336 B
  (≈16 KiB) of .bss headroom**. The theme runtime must fit in **≤ 10 KiB**,
  leaving ~6 KiB for future services. Target inside that: parser scratch
  (stack, ≤ 1.5 KiB), the active layout struct (≤ 2 KiB), the asset cache
  (≤ 8 KiB, only when a theme declares assets).
- `.text` 203,688 B; fw1 payload 207,132 B of a 356,456 B region → **~149 KiB
  flash headroom**. The runtime + one reference package as C data must stay
  under ~24 KiB.
- Display 320x170 (54,400 px). A full-screen redraw is already the norm on key
  events; the theme path must not add per-frame work beyond the declared
  elements (≤ 512 evaluation steps, one pass, no allocation).
- Storage: manifest reads go through the existing read-only `fs.h` path; a
  missing/slow volume must not block the audio pump (read once per switch, not
  per frame).

## 6. Theme-visible state (read-only snapshots)

```c
typedef struct {
    const char *title, *artist, *album;   /* "" when absent                */
    int bits, sample_rate;                /* 0 when unknown                */
    int queue_pos, queue_count;           /* 0 when no list                */
    int state;                            /* STOPPED/PLAYING/PAUSED/ERROR  */
    unsigned long position_ms, duration_ms;
    int volume_db;
    int battery_pct;                      /* -1 while unavailable (83)     */
    int shuffle, repeat;                  /* 0/1 and off/all/one           */
    const char *mode_tags;                /* pre-rendered "SHUF REP1"      */
} theme_state_t;
```

Tags resolved by the renderer: `title artist album fmt i/N elapsed total modes
volume battery state`. Missing values render as an empty string (never a
placeholder glyph).

## 7. Hot reload + fallback

1. Settings → Theme: enumerate `/.rechord/themes/*` + the built-ins, show name
   and a preview line.
2. On selection: read + validate the manifest into a *scratch* struct; on any
   error keep the current theme and show a one-line reason.
3. On success: copy the scratch into the active struct (atomic pointer swap),
   invalidate all rows, redraw. No reboot, no audio touch (the theme runtime
   never runs inside the audio path).
4. Boot: `theme_boot_load()` keeps its current behavior (cfg override →
   built-in default), then the package layer overlays layouts/assets if present.
5. Any failure anywhere → **built-in `Mono`**, never a blank screen.

## 8. Milestones

- **A (small). DONE (2026-10-18).** Runtime theme switch from Settings: a 15th
  "Theme" row (the schema and its 14-key on-disk layout are untouched) whose
  value is `theme_get()->name`; SELECT cycles the built-ins with
  `theme_load_named()` + `ui_invalidate_all()`, so every screen restyles
  immediately — no reboot, no audio interruption. Measured `.bss_end
  0x03046070`. Persisting the choice is **not** in A: `stdio_fs` is read-only,
  so writing `theme.cfg` needs the in-place path (pre-created file +
  `fs_target_file_rewrite`) that milestone B introduces; today persistence is by
  editing `theme.cfg`, which boot already reads.
- **B.** Package enumeration + manifest parse/validate (`/.rechord/themes/`),
  host tests for the parser/limits, and the reference `mono-nothing` package as
  data in the repo.
- **C.** Renderer API + the layout DSL applied to **Now Playing** first (the
  screen with the most value), still bounded by §4.
- **D.** Layouts for Main Menu, Library and Settings; per-screen previews.

Acceptance for every milestone: host tests (format/limits/fallback), all suites
green, packer `RESULT: PASS`, and a measured `.bss_end` recorded here.
