# Integration record — clean-room firmware tree (2026-10-09)

Six workstreams (drivers, frontend, services, customization, core, modules,
player) built their modules in parallel against the API contracts in
`architecture.md`. This file records the RECONCILIATION decisions taken when
wiring them together and the verification results.

## 1. Reconciliation decisions

| Clash | Decision |
|---|---|
| `theme.h` twice (ui vs theme module) | `firmware/theme/theme.h` is THE theme API; `firmware/ui/theme.h` became a **bridge**: all `THEME_COLOR_*` macros now read `theme_get()` (runtime-themeable UI), geometry constants stay in the UI header. Include is path-relative (`../theme/theme.h`) so the two same-named headers cannot collide. |
| settings: `settings_t` (services, string values) vs `rechord_settings_t` (UI, typed enums) | BOTH kept, different layers: services owns on-disk storage (`settings_t`), UI owns the typed VIEW for the settings screen. Function names split (`settings_view_*` for the view) and `settings_view_from_store()/settings_view_to_store()` adapters live in `ui/settings_schema.c`. Units: `screen_off` is MINUTES on disk (canonical), SECONDS in the view. |
| `fs_list_dir` shape (callback vs pagination) | **Pagination is canonical** (the browser needs pages); the callback form renamed `fs_list_dir_each()`. `fs_dirent_t` + `FS_NAME_MAX` added to the canonical header. |
| `KEY_NONE` etc. redeclared in UI shims | shims now forward to the real headers (`services/input.h` …); enum named `typedef enum input_key {…} input_key_t` (UI expects the type name). |
| `main()` multiply-defined in test suites | test recipes: one binary per `test_*.c` (dsp, player, services). |
| player backends both linked | `ipc_b_mock.c` (host) vs `ipc_b_target.c` (target) are mutually exclusive — same pattern as `fs_host.c`/`fs_target.c` and `panel_io_sim.c`/`panel_io_rknanoc.c`. |
| idle/sleep thresholds | compile-time `RECHORD_IDLE_FRAMES`/`RECHORD_SLEEP_FRAMES` (overridable with `-D`) — tests use IDLE=2/SLEEP=4. |
| player test fixtures | tests write fixtures under `.work/`; the runner must ensure that directory exists. |

## 2. Verification (this session)

- **Host tests: 8/8 modules pass** — drivers (57+358 checks), services, theme,
  modules (loader/registry/builder), player (wav/ringbuf/state/ipc-mock),
  app (50 checks incl. exact bring-up order), ui (250 checks), dsp
  (eq/bass/volume/chain).
- **Cross-compile: 33/33 target sources clean** with
  `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c -Wall`.
- Root Makefile now includes all 9 module fragments
  (`firmware/<module>/<module>.mk`) next to the vendor manifests.

## 3. How to run everything

```
# host tests (per module; see each firmware/<m>/tests/)
cd firmware/drivers && bash tests/run_tests.sh
cd firmware/modules  && bash tests/run_tests.sh
# suites with per-file mains compile one binary per test_*.c (see §1)
```

## 4. Open items (integration-level)

1. Root Makefile: add `host-test` target running every suite (recipes above are
   proven but not yet one-command).
2. `player.mk`/`app.mk` wiring into real image targets (link order with the
   vendor SDK manifests).
3. `panel_init_sequence()` in drivers still a documented TODO (DCS init needs
   extraction from the panel contract work).
4. `fs_target.c` pagination + FAT backend TODO; `ipc_b_target.c` register-level
   TODO; codec vendoring plan in `docs/rewrite/player.md`.
5. `rechord_platform_idle/sleep` hooks: weak symbols, board glue TBD.
