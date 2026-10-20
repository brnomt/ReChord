# Modular theme system (BRNOMT-69)

> Status: **implemented through milestone F** (package discovery, live switch,
> persistence, and a declarative skin interpreter with package assets). The
> firmware embeds no theme content or theme code.

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
- The **screens are interpreted, not compiled**: a package can ship a skin for
  the main/home, Now Playing, Library, category list, browser and Settings.
  Each screen loads its own skin on entry; the compiled screen remains the
  fallback and keeps all behavior (cursor, actions, persistence). The engine
  (`firmware/theme/theme_skin.c`) and the `theme_ui.c` bridge have no
  theme-specific code. `home=menu|skin` still selects the classic list or the
  interpreted home.
- Layout DSL (`firmware/theme/theme_pkg.{h,c}`): shared list metrics, Now
  Playing origins, home selector, per-screen skin paths and asset declarations.
  All-or-nothing, bounded, no heap.
- The firmware seeds **no theme content**. `/.rechord/themes/<id>/` is created
  empty; packages (theme.cfg + skins + assets) are installed from storage.
- 1-bit assets: `display_bitmap1()` + a 1 KiB bounded cache
  (`theme_assets.c`). A declared-but-unreadable asset rejects the package.
  Assets are optional; the Companion theme is vector.

## 2. Package layout

The repo ships authoring copies under `themes/<id>/`. The firmware seeds **no**
theme content; a package exists on the device only once its directory is
copied to storage. The built-in palettes (`themes_builtin.c`) remain only as
the safe fallback when no package is usable.

```
/<volume>/.rechord/themes/<theme_id>/
  theme.cfg        # palette + layout + home selector + skin paths + assets
  home.skin        # optional declarative main/home
  now_playing.skin # optional Now Playing
  library.skin     # optional Library root
  library_cat.skin # optional category (names/songs)
  browser.skin     # optional file browser
  settings.skin    # optional Settings
  assets/          # 1-bit bitmaps referenced by any skin
```

`theme.cfg` is the same `key=value` dialect as `rechord.cfg` (sections are
ignored by the flat parser). A skin package adds:

```
home=skin
skin=home.skin
skin_now_playing=now_playing.skin
skin_library=library.skin
skin_library_cat=library_cat.skin
skin_browser=browser.skin
skin_settings=settings.skin
# optional: asset_icon=assets/icon.bin,32,32
```

`home.skin` is the theme's main logic: `[section]` meters (`fill`, `roundrect`,
`triangle`, `bitmap`, `text`, `bar`, `list`) whose properties are expressions
over runtime state and time. A `[vars]` section defines reusable expressions.
The per-screen skins use the same engine; their `list` meter names a dynamic
`source` supplied by the compiled screen and a `rows` window. Full syntax and
limits: `themes/README.md` and `docs/THEMING.md`.

## 3. Renderer API (theme-independent)

The theme describes intent; the renderer owns the panel. The engine uses the
small `display_*` surface (no allocations, explicit clipping):

```c
void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t color);
void display_bitmap1(int x, int y, int w, int h, int stride,
                     const uint8_t *bits, uint16_t fg, uint16_t bg);
void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg);
```

The skin interpreter adds the layout/state layer: `fill`, `roundrect`,
`triangle`, `bitmap`, `text`, `bar` and an interactive `list` meter (static
`item0..` or a dynamic `source` window), plus a `[vars]` section for reusable
expressions. Coordinates, geometry and colors come from the skin; the engine
only executes them.

Existing `display_*` primitives map 1:1 (`fill_rect`, `draw_text`); `bitmap1`
is the only new one (row-major MSB-first, stride from the manifest, clipped).
Text is 8x8 opaque cells today; `max_w` truncates at a cell boundary.

## 4. Bounds (hard limits, validated at load)

| Thing | Limit |
|---|---|
| themes enumerable | 16 |
| manifest size read | 4 KiB |
| skin text | 4 KiB, 32 sections |
| keys per section | 12 |
| string value | 64 bytes (`SKIN_VAL_MAX`); property 144 bytes |
| list rows | browser <= 12 (its window); others fit the screen |
| assets per theme | 8 |
| single asset bitmap | 64 x 64 px, stride <= 8 B |
| asset bytes cached | 1 KiB total |
| evaluation steps per frame | 2048, depth 12 |

Anything out of range **rejects the theme** (not a clamp): a malformed package
must never produce a half-drawn screen.

## 5. Measured budgets (m9g, `build/cfw/rechord_ap.elf`)

- `.bss` ends at `0x03049CA8`; the hard AP gate is `0x0304A000` → **~856 B
  headroom**. The theme runtime is part of that total; no per-theme heap.
- `.text` 224,920 B; fw1 payload 226,648 B. The engine and all six screen
  adapters are already linked; adding a skin is data only.
- Display 320x170 (54,400 px). A full-screen redraw is already the norm on key
  events; a static skin adds no per-frame work. An animated skin opts in with
  `[skin] refresh_ms` and is re-flushed only while its screen is on top.
- Storage: manifest reads go through the existing read-only `fs.h` path; a
  missing/slow volume must not block the audio pump (read once per switch, not
  per frame).

## 6. Theme-visible state (read-only snapshots)

```c
typedef struct theme_skin_state {
    int playing, paused, stopped;
    int volume_db, battery_pct;           /* battery -1 when unknown      */
    const char *title, *artist, *album, *path, *heading, *state;
    long position_ms, duration_ms;
    int  bits; long sample_rate;          /* 0 when unknown                */
    int  playlist_pos, playlist_count;    /* 1-based / 0 when no list      */
    int  shuffle, repeat;                 /* 0/1 and off/all/one           */
} theme_skin_state_t;
```

`heading` is the screen title/path (Library, Settings, category, browser
path). Skin variable names: `playing paused stopped volume battery title artist
album path heading state pos dur pos_ms dur_ms pos_fmt dur_fmt bits rate pl_i
pl_n shuffle repeat`, plus engine state `t idle sel n top W H aw ah` and the
palette names. `pos`/`dur` are integer seconds; `pos_fmt`/`dur_fmt` are `M:SS`
strings. Missing values render as empty strings / 0 (never a placeholder
glyph). Strings concatenate with `+`; integers are converted in a string
expression.

Sources: `player_get_state()`, `player_get_track()`, `player_get_position_ms()`,
`player_get_volume()`, `player_get_shuffle()`, `player_get_repeat()`,
`battery_percent()`, `ap_ms()` and the UI selection. The engine never talks to
the player directly; `firmware/ui/screens/theme_ui.c` fills the snapshot and
registers the screen's dynamic list source, while `skin_screen.c` remains the
home adapter. Full guide: `docs/THEMING.md`.

## 7. Hot reload + fallback

1. Settings → Theme: `theme_pkg_scan()` enumerates `/.rechord/themes/*` (each
   readable `theme.cfg` is a package) and SELECT cycles them by id.
2. On selection: `theme_pkg_apply()` reads the manifest once, parses palette
   (over the built-in Mono base) + layout into scratch structs and validates
   both. Any error keeps the current theme and the picker tries the next
   package.
3. On success: palette + layout are committed together, the main skin text and
   assets are loaded from the package, the id is written as `theme=<id>` in
   `/.rechord/rechord.cfg` (and mirrored to the legacy
   `/.rechord/active_theme`), and the app inserts/removes the skin home so
   `home=` takes effect without leaving Settings. The Settings screen reloads
   its own skin for the new package. `ui_invalidate_all()` repaints; audio is
   never touched. Other screen skins are loaded by their screens on entry, so
   a theme change is fully live.
4. Boot: the cfg `theme=` id is applied; configs without it fall back to
   `active_theme`, then to `mono`, then to the built-in palette/default layout.
5. Any failure anywhere → **built-in `Mono`**, never a blank screen.

## 8. Milestones

- **A (small). DONE (2026-10-18).** Runtime theme switch from Settings: a
  "Theme" row (the schema and its on-disk key layout are untouched; the
  layout now has 15 keys after BRNOMT-67) whose
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
- **E. DONE (2026-10-19)** — live package switching:
  - `theme_runtime.{h,c}`: `theme_pkg_scan()` (fixed 16-entry table, sorted by
    id), `theme_pkg_apply()` (one manifest read; palette + layout validated
    together and committed atomically), `theme_pkg_boot()` (cfg `theme=` id →
    legacy `active_theme` → `mono` → built-in defaults).
  - Settings → Theme cycles **installed packages** (not compiled palettes),
    persists the selection and syncs the home screen under the open Settings
    screen (`ui_insert_screen`/`ui_remove_screen`).
- **F. DONE (2026-10-19)** — no theme code in the firmware:
  - `theme_skin.{h,c}`: bounded declarative interpreter. Sections + meters
    (`fill`, `roundrect`, `triangle`, `bitmap`, `text`, `bar`, `list`) whose
    properties are expressions over `t idle playing volume W H sel n aw ah
    title ...`, with `min max abs clamp mod noise sin seq if`. A `[vars]`
    section defines reusable expressions; the interactive list carries its
    own labels/actions and `prev`/`next` key bindings.
  - `theme_assets.{h,c}`: 1-bit assets loaded from the package into a 1 KiB
    cache; declared-but-unreadable assets reject the package.
  - `display_bitmap1()` blits raw MSB-first rows; `skin_screen.c` is a generic
    UI adapter (no theme symbols).
  - `themes/companion-bot/` now contains `theme.cfg` + `home.skin`; the
    firmware seeds no theme content. The Companion is vector exactly like the
    reference (rounded rects + masking triangle), with independent saccades,
    gaze at the selected item, blink/wake/sleep/happy as data.
  - Tests: `test_skin_home` loads the real package, verifies expressions,
    vector drawing and `prev=down`/`next=up` mapping; `test_theme_pkg` covers
    skin/asset validation; `test_theme_runtime` covers atomic apply + skin
    loading.
  - Measured `.bss_end 0x03049910` (gate `0x0304A000`, ~1.7 KiB free).

Acceptance for every milestone: host tests (format/limits/fallback), all suites
green, packer `RESULT: PASS`, and a measured `.bss_end` recorded here.
