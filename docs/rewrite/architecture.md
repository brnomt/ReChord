# ReChord — Architecture (source-from-.c, modular, Rockbox-style)

> Constitution of the project. Every workstream must follow this document.
> Decision record: language is **C** everywhere (2026-10-09). Layout is
> **Rockbox-style: one subsystem = one folder**, every part rebuildable from
> source, every aspect customizable (themes, DSP, modules, key scripts).

## 1. Build-from-source rule

The firmware image MUST be produced entirely from `.c`/`.S` in this tree plus:

- the **vendor SDK** under `firmware/rockchip/` (Rockchip RKnanoD SDK — the
  platform SDK, compiled from its own source), and
- **vendor codec blobs** only as a temporary fallback (see §4).

No third-party CFW code in any form (source, decompiled, or binary). Hardware
facts and interface contracts (register maps, DCS commands, on-disk formats)
are fair game; implementations are written by us.

## 2. Module map — one subsystem, one folder

```
firmware/
├── startup/        entry, C-runtime init (from the boot contract)
├── drivers/        hardware: display+font, input ADC, storage, power, clock
├── services/       OS-agnostic: settings, logging, fs abstraction, input queue
├── ui/             screens/, widgets/, theme engine, UI event loop
├── theme/          theme data + loader (colors, fonts, spacing) — swappable
├── dsp/            dsp_core + effect modules (EQ, bass, …) — swappable
├── modules/        overlay module loader (loadable feature modules)
├── app/            app glue: main, boot flow, module registry
├── rockchip/       vendor SDK (compiled as-is)
└── tests/          host-side integration tests (per-module tests live inside
                    each module's own folder under tests/)
```

Rules:
- A module talks to other modules ONLY through its public `*.h` API.
- No module includes another module's private headers or `.c`.
- Per-module host tests: `firmware/<module>/tests/*.c`, run with `cc`.
- Build wiring per module: `firmware/<module>/<module>.mk` fragments included
  by the root Makefile (the root Makefile stays thin).
- All code/comments in English. Comments explain WHY.

## 3. Customization (first-class, Rockbox-style)

- **Themes**: `firmware/theme/` — a theme is data (palette, fonts, spacing,
  optional assets). Built-in themes are `.c` data tables; a file-based theme
  can override at runtime. UI never hardcodes colors — it asks the theme.
- **DSP**: `firmware/dsp/` — `dsp_core` (freestanding, host-tested) plus an
  effect-module interface (`init/process/set_param/get_param`) so effects can
  be added/reordered per user config. Same DSP runs in host tests.
- **Modules**: features (player, browser, recorder…) are loadable modules in
  the overlay module format (contract documented in
  `docs/re/route-b-minimum.md`) so the UI and features can grow without
  reflashing everything.
- **Key scripts / config**: user-facing config is a plain `key=value` file;
  every tunable a user cares about goes through settings, not recompiles.

## 4. Codec strategy (trade-off, open)

| Option | Pros | Cons |
|---|---|---|
| Vendor `.lib`/`.bin` codec blobs (22) | Done, HW-proven | Not from source — weakens the build-from-source rule |
| Open-source C codecs (minimp3, dr_flac, stb_vorbis, opus…) | Fully from `.c`, portable, host-testable | Integration + CPU budget on the B core needs proving |

Direction: prefer open-source C codecs behind the `dsp/`/player interface;
vendor blobs only as an explicit, documented fallback during bring-up.

## 5. Verification standard (every workstream)

1. Compiles: host `cc -Wall -Werror` (with mocks) AND
   `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c` (target).
2. Host tests pass for every module that has logic.
3. Docs updated in `docs/rewrite/<workstream>.md` in the same change.
