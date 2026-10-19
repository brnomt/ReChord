# Modular theme system (BRNOMT-69)

> Status: **implemented through milestone E** (discovery, live package switch,
> package-selected home renderer, persistence). Assets/bitmap blit remain the
> next step (see §8). The issue requires explicit memory/time budgets *before*
> implementation, so this file is the contract the milestones are built
> against.

## 1. Today

- `firmware/theme/`: `theme_t` (7 RGB565 colors + 3 unused metrics), 4 built-in
  palettes (`Mono` default, `Classic`, `Paper`, `Night`), a `theme.cfg` file
  loader (`theme_file.c`, key=value, partial overrides).
- **Packages are live** (`theme_runtime.c`): `/.rechord/themes/<id>/theme.cfg`
  is enumerated, applied atomically (palette + layout + home renderer) and the
  selection is persisted as `theme=<id>` in `/.rechord/rechord.cfg` (with the
  pre-created `/.rechord/active_theme` kept as the fallback for configs that
  predate the key). Settings → Theme cycles
  the installed packages at runtime; no reboot, no recompilation.
- Screens never hardcode colors: they ask `theme_get()` through the
  `THEME_COLOR_*` macros (`firmware/ui/theme.h`), so a palette swap restyles
  every screen live. Geometry comes from `theme_layout()`.
- The **home renderer is data**: `home=menu|companion` plus `home_nav=
  vertical|horizontal`. `companion` is a bounded engine renderer selected by
  the package; a menu-only package never runs it. `face_h`/`home_bar_*` are
  package geometry.
- Layout DSL (`firmware/theme/theme_pkg.{h,c}`): shared list metrics, Now
  Playing origins, home selection/axis and the companion band. All-or-nothing,
  bounded, no heap.
- Still pending: bitmap assets (`r_bitmap1` + cache) and broader per-screen
  layout records. Nothing in the current UI falls back to hardcoded styling.

## 2. Package layout

> The repo ships copies of the four built-in themes under `themes/<id>/theme.cfg`
> (`mono`, `classic`, `paper`, `night`). The built-ins themselves are COMPILED IN
> (`themes_builtin.c`), so boot creates **no** device folder: a package exists on
> the device only once it is copied there. Install notes: `themes/README.md`.

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

1. Settings → Theme: `theme_pkg_scan()` enumerates `/.rechord/themes/*` (each
   readable `theme.cfg` is a package) and SELECT cycles them by id.
2. On selection: `theme_pkg_apply()` reads the manifest once, parses palette
   (over the built-in Mono base) + layout into scratch structs and validates
   both. Any error keeps the current theme and the picker tries the next
   package.
3. On success: palette + layout are committed together, the id is written as
   `theme=<id>` in `/.rechord/rechord.cfg` (and mirrored to the legacy
   `/.rechord/active_theme`), and the app inserts/removes the companion home
   so `home=` takes effect without leaving Settings. `ui_invalidate_all()`
   repaints; audio is never touched.
4. Boot: the cfg `theme=` id is applied; configs without it fall back to
   `active_theme`, then to `mono`, then to the built-in palette/default layout.
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
- **B/C/D. DONE (2026-10-18)** — first complete pass:
  - `firmware/theme/theme_pkg.{h,c}`: the **layout DSL** (`pad`, `title_h`,
    `item_h` + the Now Playing origins) read from the same `theme.cfg`,
    **all-or-nothing** (any bad/out-of-range value rejects the whole layout and
    the previous/defaults stay), bounded, no heap.
  - The UI reads it **live**: `THEME_PAD/TITLE_H/ITEM_H/MENU_TOP` and the
    `PLAYER_*` origins are `theme_layout()` expressions, so one package moves
    every list (menu/library/categories/settings/browser) **and** Now Playing
    with no per-screen code.
  - Boot applies the package: `theme_boot_load()` (palette) +
    `theme_layout_load(THEME_FILE_PATH)` (layout). The Settings Theme row still
    cycles the built-in palettes live.
  - Reference package: `docs/rewrite/examples/mono-nothing/theme.cfg`.
  - Host tests: `test_theme_pkg` (defaults, partial override, comments/sections,
    out-of-range and malformed rejection, missing file) — 22 checks.
  - Measured `.bss_end 0x030460A8` (gate `0x0304A000`, ~15.8 KiB free).
  - Deferred: multi-package enumeration/preview (`/.rechord/themes/*`), the
    bitmap blit (`r_bitmap1`) + asset cache, and persisting the Settings choice.
- **E. DONE (2026-10-19)** — real package-level theming:
  - `theme_runtime.{h,c}`: `theme_pkg_scan()` (fixed 16-entry table, sorted by
    id), `theme_pkg_apply()` (one manifest read; palette + layout validated
    together and committed atomically), `theme_pkg_boot()` (cfg `theme=` id →
    legacy `active_theme` → `mono` → built-in defaults).
  - Package manifest now carries **behavior**, not only values:
    `home=menu|companion` selects the home renderer and
    `home_nav=vertical|horizontal` selects the item axis. The companion is
    only inserted into the stack when the active package declares it, so a
    simple list theme never runs the animated renderer. `face_h` and
    `home_bar_y`/`home_bar_h` are package geometry; `face_h` >= 20 is required
    for `home=companion`. Legacy manifests without `home=` infer the renderer
    from `face_h`.
  - Settings → Theme now cycles **installed packages** (not the compiled
    palettes), persists the selection and syncs the home renderer under the
    open Settings screen (`ui_insert_screen`/`ui_remove_screen`).
  - `menu_draw()` no longer paints the companion; the face belongs to the
    companion home only. Library/category/browser/settings/Now Playing keep
    reading the live layout.
  - Tests: `test_theme_pkg` (home/nav/renderer validation, legacy inference,
    format rejection) and `test_theme_runtime` (scan, atomic apply, invalid
    package keeps the active theme, persistence, boot fallback); UI
    `test_home_theme` covers horizontal/vertical companion items.
  - Measured `.bss_end 0x030474B0` (gate `0x0304A000`, ~10.8 KiB free).

Acceptance for every milestone: host tests (format/limits/fallback), all suites
green, packer `RESULT: PASS`, and a measured `.bss_end` recorded here.
