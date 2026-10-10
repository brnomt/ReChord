# ReChord Customization — theme system & DSP effect-module system

> Workstream: CUSTOMIZATION (Rockbox-style "every visual and sonic aspect is
> user-customizable"). Owns `firmware/theme/**`, `firmware/dsp/**` and this
> document. Follows `docs/rewrite/architecture.md`: one subsystem = one
> folder, per-module host tests, `.mk` build fragments, everything from `.c`.

## 1. Theme system (`firmware/theme/`)

A theme is pure data: an RGB565 palette, layout metrics and a font id. UI
code never hardcodes a color — it asks `theme_get()` (see `theme.h`, the ONLY
theme interface other modules consume).

| File | Role |
|---|---|
| `theme.h` | public API: `theme_t`, get/set, named load, count/name, validate, file loader |
| `theme_priv.h` | internal: built-in table access (other modules must not include) |
| `theme.c` | registry + current-theme slot (one plain struct copy, no heap) |
| `themes_builtin.c` | built-in themes as `.c` data — the ONLY place color values exist |
| `theme_file.c` | tolerant `key=value` file parser + file loader |
| `theme.mk` | build fragment (`THEME_SRCS`, `THEME_TEST_SRCS`) |

### 1.1 Built-in themes

- **Classic** — dark slate, full-contrast (default on first use).
- **Paper** — warm light/sepia, eye-strain-friendly (no pure white/black).
- **Night** — very dim dark palette for bedside use.

### 1.2 Theme file format

One `key=value` per line; `#` and `;` start comments; blank lines ignored.
Colors are `0x`-hex or decimal RGB565; metrics are plain integers; `name` is
free text. Unknown keys and malformed lines are SKIPPED (a user typo must
never brick the UI); the last occurrence of a key wins.

```
# my.theme — override any subset of fields
name = My Theme
background = 0x1083
accent = 0x44FF
spacing = 8
```

Loading is **override-style** (`theme_file_parse`/`theme_file_load` write only
the keys present into an existing `theme_t`):

```c
theme_t t = *theme_get();            /* or a built-in via theme_load_named() */
theme_file_parse(text, &t);          /* apply user overrides */
theme_set(&t);
```

`theme_validate()` enforces the minimum usability rules (non-empty name,
text/accent contrast against background, distinct danger/success, sane
metrics) and is used by tests and (later) the settings UI.

## 2. DSP effect-module system (`firmware/dsp/`)

| File | Role |
|---|---|
| `dsp_module.h` | effect interface (`dsp_module_t`), param enums, `DSP_STATE_STORAGE` |
| `dsp_chain.h/.c` | ordered chain (up to 8 modules), per-module bypass, set-by-name |
| `mod_eq.c` | 3-band EQ (low shelf 60 Hz / mid peaking 1 kHz / high shelf 8 kHz) |
| `mod_bass.c` | bass boost (low shelf at the core's 80 Hz, default +10 dB) |
| `mod_volume.c` | gain with soft ramp (no clicks), int16 saturation |
| `dsp_sat.h` | internal int16 saturation / 24-bit conversion helpers |
| `dsp.mk` | build fragment (`DSP_SRCS`, `DSP_TEST_SRCS`, `DSP_HOST_LIBS`) |

### 2.1 Interface contract (`dsp_module.h`)

```c
typedef struct dsp_module {
    const char *name;
    int (*init)(void *state, int sample_rate);
    int (*process)(void *state, int16_t *samples, int n);
    int (*set_param)(void *state, int param, int value);
    int (*get_param)(void *state, int param);
    int state_size;
} dsp_module_t;
```

- Stream: interleaved **stereo int16** (L,R,L,R...), `n` counts values;
  processing is in place and state flows across calls (click-free blocks).
- Gains are **tenths of dB** (`+60` = +6.0 dB) so params stay plain ints for
  `key=value` config files.
- `set_param`: 0 ok, -1 unknown param, -2 out of range.
- `get_param`: value, or **INT_MIN** on error (every legal value is bounded
  far from INT_MIN, so negative gains like -600 stay unambiguous).
- State lives in **caller-owned buffers** (no heap): declare with
  `DSP_STATE_STORAGE(name, size)` and pass to `dsp_chain_add()` which
  validates size (>= `state_size`) and 8-byte alignment.

### 2.2 Chain (`dsp_chain.h/.c`)

`dsp_chain_init(chain, sample_rate)` → `dsp_chain_add(mod, state, bytes)`* →
`dsp_chain_process(samples, n)`. Per-module bypass flag (byte-exact skip),
parameter access by name (config files, ASCII case-insensitive) and by index
(UI, duplicate module types). Capacity 8 modules.

### 2.3 How the effect modules build on `rechord_dsp_core`

The DSP core is a **stateful singleton**: its filter bank lives in file
globals behind `rch_dsp_configure/process/reset`, and the core files must not
be modified (the root Makefile uses them). The module interface needs
**per-instance** state so several effects coexist in one chain (eq + bass +
volume).

Resolution: each core-backed module (`mod_eq.c`, `mod_bass.c`) compiles the
core's own biquad engine into its translation unit —
`#include "../rechord_dsp_core.c"` with the three entry points renamed per
TU — and keeps its filter bank inside the module state. Consequences:

- ALL filter math (Q2.30 Direct Form I, RBJ cookbook, band frequencies,
  headroom pre-scale `0.9/max-gain`) comes verbatim from
  `rechord_dsp_core.c`: single source of truth, no duplicated formulas;
- `test_mod_eq.c` proves **bit-level parity** (≤1 LSB rounding) between
  mod_eq and the core's public API on pseudo-random input;
- the root build is unaffected (effect objects carry renamed symbols, so
  `rechord_dsp_core.o` links alongside them);
- flash cost: one core copy per core-backed module (~1 KB each at -Os).

Known deviations from the deliverable sketch (documented, tested):

- "saturate helpers from the core" don't exist (the core's fixed-point
  helpers are file-static and unexported) — `mod_volume.c` uses the module
  layer's own `dsp_sat.h` (int16 clamp + 24-bit conversion).
- The core's dedicated bass plugin hardcodes +10 dB; `mod_bass` takes the
  adjustable low-shelf path instead (same math, user-dialable boost), default
  kept at the core's +10 dB / 80 Hz tuning.

### 2.4 Parameter summary

| Module | Param | Range | Default |
|---|---|---|---|
| `eq` | `MOD_EQ_LOW/MID/HIGH` (tenths dB) | -200..+200 | 0 |
| `bass` | `MOD_BASS_BOOST` (tenths dB) | 0..+150 | +100 (+10 dB) |
| `volume` | `MOD_VOLUME_GAIN` (tenths dB) | -600..+60 | 0 |
| `volume` | `MOD_VOLUME_RAMP_MS` | 0..1000 | 20 |

`mod_volume` ramps the gain sample-by-sample to the target (monotonic, no
overshoot, max ~8 LSB step at default settings) and saturates at ±full scale
instead of wrapping. `MOD_VOLUME_RAMP_MS = 0` gives instant gain (tests,
seek).

## 3. Build fragments

Root Makefile wiring (future — the root Makefile is owned elsewhere):

```make
include firmware/theme/theme.mk
include firmware/dsp/dsp.mk
```

`theme.mk` exposes `THEME_SRCS`/`THEME_TEST_SRCS`; `dsp.mk` exposes
`DSP_SRCS`/`DSP_TEST_SRCS`/`DSP_HOST_LIBS` (`-lm`). `rechord_dsp_core.c` is
deliberately NOT in `DSP_SRCS` (see §2.3).

## 4. Verification (all executed 2026-09-10, host `cc -Wall -Werror`)

| Test binary (compile lines below) | Result |
|---|---|
| `firmware/theme/tests/test_theme.c` | PASSED (0 failures) — 43 checks: built-in palettes valid, case-insensitive named load, failed load leaves current untouched, file round-trip (12 keys), partial override |
| `firmware/dsp/tests/test_mod_eq.c` | PASSED (0 failures) — per-band relative boosts, 0.9x headroom passthrough, param contract, core parity ≤1 LSB |
| `firmware/dsp/tests/test_mod_bass.c` | PASSED (0 failures) — 50 Hz/1 kHz ratio 2.66 at +10 dB and rising with boost, 0 dB identity, param contract |
| `firmware/dsp/tests/test_mod_volume.c` | PASSED (0 failures) — bit-exact 0 dB, ramp monotonic/no-overshoot/no-click (max step 9 LSB), full-scale saturation without wrap, -60 dB silence |
| `firmware/dsp/tests/test_dsp_chain.c` | PASSED (0 failures) — order matters (+6/-6 → 16422 vs -6/+6 → 32766), bypass byte-exact, set-by-name, add-time validation |

Target compilation — `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -Wall -c`
clean for all 7 module sources (`theme.c`, `themes_builtin.c`,
`theme_file.c`, `dsp_chain.c`, `mod_eq.c`, `mod_bass.c`, `mod_volume.c`).

Host compile lines (from the repo root):

```
cc -Wall -Werror -O2 -o test_theme \
   firmware/theme/tests/test_theme.c firmware/theme/theme.c \
   firmware/theme/themes_builtin.c firmware/theme/theme_file.c

cc -Wall -Werror -O2 -o test_mod_eq \
   firmware/dsp/tests/test_mod_eq.c firmware/dsp/mod_eq.c \
   firmware/rechord_dsp_core.c -lm

cc -Wall -Werror -O2 -o test_mod_bass \
   firmware/dsp/tests/test_mod_bass.c firmware/dsp/mod_bass.c \
   firmware/rechord_dsp_core.c -lm

cc -Wall -Werror -O2 -o test_mod_volume \
   firmware/dsp/tests/test_mod_volume.c firmware/dsp/mod_volume.c -lm

cc -Wall -Werror -O2 -o test_dsp_chain \
   firmware/dsp/tests/test_dsp_chain.c firmware/dsp/dsp_chain.c \
   firmware/dsp/mod_eq.c firmware/dsp/mod_bass.c firmware/dsp/mod_volume.c \
   firmware/rechord_dsp_core.c -lm
```

## 5. Open items

- `theme_file_load()` uses stdio (`fopen`/`fgets`); once `services/` lands
  its fs abstraction, route the reader through it (the parser itself is
  freestanding and unchanged).
- Per-instance core instantiation gives each core-backed module its own
  filter bank, but not N instances of the SAME core-backed module type per
  chain (a second `mod_eq` in one chain would share the TU bank). If that is
  ever needed, either compile extra instantiations (rename sets) or promote
  the core to an explicit instance API (requires touching the core files).
- Theme file assets (per-theme fonts/bitmaps) are out of scope so far — the
  `font_id` field reserves the hook for the drivers/ font set.
- Settings UI glue (theme picker, EQ sliders, module ordering screen) belongs
  to `firmware/ui/`; the APIs above are designed to be consumed from there
  and from `key=value` user config.
