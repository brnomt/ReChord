# ReChord Frontend (UI) — Architecture

> Workstream: **frontend**. Scope: `firmware/ui/**` (this doc lives in
> `docs/rewrite/frontend.md`). Status: **skeleton complete, host-tested** —
> UI core + 5 screens compile clean for host and Cortex-M3, 250 host checks
> pass. Clean-room rule: no third-party CFW code or decompiled UI was used;
> only documented facts (geometry, key schema, boot contracts) from
> `docs/re/route-b-minimum.md` and `docs/re/refcfw-analysis.md`.

## 1. What this is

A Rockbox-style modular UI written from scratch in pure C99: a small UI core
(event loop skeleton, screen stack, dirty-region redraw) plus reusable
screens (boot, generic menu, file browser, now-playing, settings). No dynamic
allocation, fixed-size everything, deterministic redraw — suitable for the
RKNanoC's dual Cortex-M3 environment and host-testable with mock backends.

## 2. Architecture diagram

```
                       +---------------------------+
   key ADC / events    |  services (other agents)  |
  ------------------>  |  input_poll(event*)       |
                       |  settings_load/save()     |
                       |  log_printf(fmt,...)      |
                       |  fs_list_dir(path,idx,..) |
                       +-------------+-------------+
                                     ^
                                     |  firmware/ui/internal/hal.h
                                     |  (real headers when they land,
                                     |   temporary shims until then)
+------------------------------------+----------------------------------+
| firmware/ui  (frontend workstream)                                     |
|                                                                        |
|  ui.h/ui.c  — UI core                                                  |
|   +-----------------+   +------------------+   +---------------------+ |
|   | event loop      |   | screen stack     |   | redraw model        | |
|   | ui_run/         |   | push/pop/top     |   | dirty row bands ->  | |
|   | ui_handle_event |   | (depth 4)        |   | display_flush_rows  | |
|   +--------+--------+   +--------+---------+   +----------+----------+ |
|            |                     |                        |            |
|            v                     v                        v            |
|  +-----------------+   +------------------+    +-------------------+  |
|  | screens/*.c     |   | theme.h          |    | display (driver)  |  |
|  | boot_screen     |   | RGB565 palette   |    | fill_rect         |  |
|  | menu_screen     |   | 6x11 font metric |    | draw_text         |  |
|  | browser_screen  |-->| spacing/layout   |--->| flush_rows        |  |
|  | player_screen   |   +------------------+    | clear             |  |
|  | settings_screen |                           +-------------------+  |
|  +--------+--------+                                                     |
|           |                                                              |
|           v                                                              |
|  +---------------------+    +---------------------+                      |
|  | settings_schema.c/h |    | tests/              |   HOST ONLY          |
|  | 12-key schema table |    | mock_display (log)  |                      |
|  | names/values/offsets|    | mock_services       |                      |
|  +---------------------+    | 5 test suites       |                      |
|                             +---------------------+                      |
+--------------------------------------------------------------------------+
```

Event flow: `ui_run()` polls `input_poll()` → `ui_handle_event()` dispatches
to the top screen's handler → the handler mutates its state and calls
`ui_invalidate()` → `ui_render()` runs the top screen's `draw()` once and
pushes only the dirty row bands to the panel via `display_flush_rows()`.

## 3. Module list

| File | Role |
|---|---|
| `ui.h` / `ui.c` | UI core: event loop skeleton (`ui_run/ui_quit`), screen stack (`ui_push_screen/ui_pop_screen`, depth 4, enter/exit/re-enter hooks), redraw model (`ui_invalidate*`, sorted+merged dirty row bands, coalesced flushes) |
| `theme.h` | Colors (RGB565), font metrics (6x11 class: 6 px advance, 11 px glyph, 12 px line pitch, +1 px text offset), spacing/layout constants |
| `settings_schema.h` / `settings_schema.c` | The 12-key config schema: `rechord_settings_t`, per-key table (name, kind, field offset, options/range), defaults (seeded from observed device values), cycle/format helpers |
| `screens/screens.h` | Public API of all screens (control functions + state introspection) |
| `screens/boot_screen.c` | ReChord branding (own wordmark rendering, 1 px weight pass), version, divider, printf-style status lines at 12 px pitch (id × 12 px contract) |
| `screens/menu_screen.c` | Generic list menu (title + scrollable items + selection + right-aligned values) AND the shared navigation/rendering helpers (`menu_view_move`, `menu_draw`, `menu_draw_item`) reused by the other lists |
| `screens/browser_screen.c` | File browser over `fs_list_dir()` (names + counts only): window = one visible page fetched at the view top, cursor clamping, enter-dir/back path handling clamped at a configured root, footer counts |
| `screens/player_screen.c` | Now-playing skeleton: track title, progress bar, times, volume bar (-60..0 dB), play/pause toggle |
| `screens/settings_screen.c` | Binds the 12 schema keys to a menu (label = key name, value = formatted config value), SELECT cycles values, BACK saves via `settings_save()` |
| `internal/hal.h` | The single include door to display + services |
| `internal/shims/*.h` | **TEMPORARY** forward headers (`display.h`, `input.h`, `settings.h`, `log.h`, `fs.h`) — declarations only, delete when the real headers land |
| `tests/` | Host tests with MOCK display backend (records every draw call) and mock services (input queue, log capture, in-memory settings, fake fs tree) |
| `Makefile` | `make test` (host, `cc -Wall -Werror`), `make target` (`arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c`) |
| `ui.mk` | Build fragment for the root Makefile (module wiring per `docs/rewrite/architecture.md` §2) |

## 4. Design decisions (ours)

- **Screen = const vtable + private static state.** No allocation; screens
  are pushed/popped by pointer. A covered screen stays untouched; the
  screen revealed by a pop re-enters (`on_enter` fires again).
- **Redraw model = full logical repaint, partial physical flush.** `draw()`
  repaints the screen (cheap framebuffer fills), but only dirty row bands
  reach the panel (expensive SPI/DCS transfer), merged when overlapping or
  adjacent. Overflow of the 8-band set degrades to bounding bands.
- **Menu navigation clamps at the ends** (no wrap) — predictable with a
  5-key surface; the page scrolls minimally to keep the cursor visible.
- **Browser pagination**: one `fs_list_dir()` call fetches exactly one
  visible page starting at the view top; a stable listing order across
  calls is part of the fs service contract.
- **Settings schema is data, not code**: one table maps config key names to
  struct fields with per-key value kinds (enum options / int range / string),
  so the menu binding, the config serializer and the tests all read the
  same table. Enum option texts are exactly the config-file values.
- **Theme constants are centralized** in `theme.h` (the future
  `firmware/theme/` runtime theme can replace it).

## 5. Host vs target

| Runs on host (`cc`, mock backends) | Runs on target (Cortex-M3) |
|---|---|
| everything under `firmware/ui/` — the UI logic is platform-free C99 | same sources, compiled with `-mcpu=cortex-m3 -mthumb -Os` |
| `tests/`: mock display (records draw calls), mock input/settings/log/fs | real `firmware/drivers/display.h` + `firmware/services/*.h` implementations |
| verifies: menu navigation state machine, screen stack push/pop, dirty-region model, 12-key settings mapping, browser pagination | `ui_run()` binds the loop to real `input_poll()`; app layer (`firmware/app/`) pushes screens |

Verified this change: `cc -std=c99 -Wall -Werror` (all sources incl. tests)
and `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c` (all 7 non-test
sources) are clean; `make test` runs 5 suites / 250 checks / 0 failures.

## 6. Service contracts assumed (for the other workstreams)

These are the exact declarations the UI consumes today (mirrored by the
temporary shims in `firmware/ui/internal/shims/`):

```c
/* display (firmware/drivers/display.h) */
#define DISPLAY_W 320
#define DISPLAY_H 170
void display_init(void);
void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t rgb565);
void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg);
void display_flush_rows(int y0, int y1);
void display_clear(uint16_t rgb565);
/* all coordinates INCLUSIVE; flush_rows sends rows y0..y1 inclusive */

/* input (firmware/services/input.h) */
typedef enum input_key {
    KEY_NONE = 0, KEY_UP, KEY_DOWN, KEY_SELECT, KEY_BACK, KEY_POWER
} input_key_t;
typedef struct input_event { input_key_t type; uint16_t adc_value; } input_event_t;
int input_poll(input_event_t *ev);   /* 1 = event filled, 0 = queue empty */

/* settings (firmware/services/settings.h) */
int settings_load(rechord_settings_t *s);   /* 0 = ok */
int settings_save(const rechord_settings_t *s);

/* log (firmware/services/log.h) */
void log_printf(const char *fmt, ...);

/* fs (firmware/services/fs.h) — names + counts only */
#define FS_NAME_MAX 32
typedef struct fs_dirent { char name[FS_NAME_MAX]; uint8_t is_dir; } fs_dirent_t;
int fs_list_dir(const char *path, uint32_t index, fs_dirent_t *out,
                uint32_t max, uint32_t *out_count, uint32_t *out_total);
```

Once those headers exist: build with `-DRECHORD_UI_TARGET
-Ifirmware/drivers -Ifirmware/services` and drop `internal/shims` from the
include path (see `internal/hal.h`). Any mismatch will then fail at compile
time — that is intentional.

## 7. Host tests (firmware/ui/tests/)

| Suite | Verifies |
|---|---|
| `test_screen_stack.c` | push/pop semantics, covered-screen re-enter, depth cap (4), NULL/full rejection, empty pop |
| `test_ui_redraw.c` | dirty row bands: disjoint flushes, overlap/adjacency merging, clamping, overflow coalescing, flush-only-when-dirty |
| `test_menu_nav.c` | `menu_view_move` clamping/minimal scrolling + `menu_screen` key handling (select fires actions, back pops) + rendering output |
| `test_settings_keys.c` | exact 12 key names/order, per-key field mapping (cycle touches exactly one field), value formatting (`repeat_track`, `8`, `-23`, ...), wrap behavior, settings_screen menu binding + save-on-exit |
| `test_browser_pagination.c` | fetch window == page at view top, refetch offsets, end/start clamping, enter-dir re-rooting, parent navigation, pop at root, rendered names/counts |

Run: `cd firmware/ui && make test` (exits non-zero on any failure).

## 8. Open items

1. **Real service headers** — replace the shims (`-DRECHORD_UI_TARGET`),
   delete `internal/shims/`; resolve any signature drift at compile time.
2. **`settings_load/save` persistence format** — the UI treats
   `rechord_settings_t` as the round-trip unit; the settings service must
   decide the on-disk format (the plain `key=value` file per architecture
   §3) and may need an adapter to its own struct.
3. **Key ADC mapping / chords** — the input service owns ADC channel →
   `input_key_t` mapping (incl. `keys: chord vol+-` behavior); the UI
   currently only sees logical keys.
4. **Boot flow wiring** — `firmware/app/` (or `rechord_app.c`) must push
   `boot_screen`, feed `boot_screen_status()` lines, then switch to the
   main menu; the module system (overlay modules) may host the heavier
   screens later.
5. **Player integration** — `player_screen` is display-only; the player/B-
   core supervisor must feed track/position/volume and receive the local
   key actions (volume, play/pause).
6. **Browser → player handoff** — file-open is a `log_printf` placeholder;
   needs a callback/service to start playback of the selected file.
7. **Fonts** — the built-in 6x11 metrics are assumed; when the driver
   exposes selectable fonts (RefCFW-style font slots), theme.h grows a font
   descriptor and metrics per font.
8. **KEY_POWER policy** — global power/screen-off behavior (screen_off /
   auto_off settings) is not wired; screens currently ignore KEY_POWER.
9. **Root Makefile integration** — `ui.mk` is ready to be included by the
   root Makefile (owned by the build workstream).
10. **i18n / long text** — labels are ASCII, one line, fixed 6 px advance;
    long names truncate at the screen edge (no marquee yet).
