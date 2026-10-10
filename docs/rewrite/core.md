# ReChord Core — boot/startup + app glue (workstream doc)

> Workstream: **core** (boot contract + app glue). Owners: `firmware/startup/`,
> `firmware/app/`. Constitution: `docs/rewrite/architecture.md`.
> Boot facts source: `docs/re/route-b-minimum.md` §2 (byte-verified).

This document covers the clean-room C-runtime startup, the vector table, and
`rechord_main` — the deterministic bring-up that turns the other workstreams'
modules into ONE bootable firmware.

## 1. Boot flow (ASCII)

```
 Mask ROM / loader
       |
       |  loads RAM image @ 0x0304F490 (route-b-minimum.md §2)
       v
 +---------------------------------------------------------------+
 |  vector table @ 0x03050000        (firmware/startup/vectors.c) |
 |    [0] initial SP = 0x03004000                                |
 |    [1] Reset      = rechord_startup_entry (Thumb, odd)         |
 +---------------------------------------------------------------+
       |
       v  (hardware loads SP, branches to Reset)
 +---------------------------------------------------------------+
 |  rechord_startup_entry   (startup.c, naked: r0..r3 pass-thru)  |
 |    4 ROM boot words arrive in r0..r3  [ABI TODO]               |
       |
       v
 |  rechord_startup(w0,w1,w2,w3)   (startup.c)                   |
 |    1. IRQs off            (cpsid i)                            |
 |    2. capture boot params (boot_params_capture -> 4 words)     |
 |    3. .data copy          (LMA -> VMA, if link provides LMA)   |
 |    4. .bss zero           (_bss_start .. _bss_end)             |
 |       re-store boot params (storage lives in .bss, see note)   |
 |    5. DSB / ISB                                                |
 |    6. call rechord_main()                                      |
 |    7. hang if main ever returns                                |
 +---------------------------------------------------------------+
       |
       v
 |  rechord_main()  (firmware/app/main.c) — see table in §3       |
 |    ... bring-up ... -> boot screen -> main loop                |
 |    main loop: input_poll -> ui_handle_event -> ui_run_frame    |
 |                -> idle/sleep policy                            |
 +---------------------------------------------------------------+
```

Note on the double capture (step 2 + re-store): the boot contract captures
the handoff words *before* C-runtime init, but our storage is a .bss global
and step 4 would erase it. The words therefore survive in callee-saved
registers/stack between the two stores (C argument semantics). Both stores
are intentional in `startup.c`.

## 2. Memory model (verified facts, route-b-minimum.md §2)

| Item | Value | Where in our code |
|---|---|---|
| RAM image base | `0x0304F490` | link scripts (image layout) |
| Vector table | `0x03050000` | `startup/vectors.c` (`.vectors`, 256-aligned) |
| Initial SP | `0x03004000` | `vectors[0]` |
| Reset word | ours = `rechord_startup_entry` (reference chains a veneer at `0x0306296E` -> real entry `0x030500E4`) | `vectors[1]` |
| Boot params | 4 words, ROM handoff, reference stores via the `DAT_0305015c` block | `app/boot_params.h` + `startup.c` |
| C-runtime startup | IRQs off -> capture -> .data -> .bss -> DSB/ISB -> main -> hang | `startup.c` |

Linker contract used by `startup.c`:
- `_data_start`, `_data_end`, `_bss_start`, `_bss_end` (required),
- `_data_load_start` (optional, **weak**): load image of `.data`; when the
  link does not provide it (RAM image where `.data` already runs in place,
  e.g. the legacy `firmware/firmware.ld`), the copy is skipped.

## 3. Bring-up order (`rechord_main`, host-verified)

The order is a CONTRACT: each layer may only depend on layers above it, and
every step emits a `log_printf` boot-telemetry line (the ring-buffer logger
works from the first step with a frozen clock). `firmware/app/tests/`
asserts this exact call sequence.

| # | Step | Calls | Log line |
|---|------|-------|----------|
| 0 | boot telemetry | `boot_params_get()` | `boot: params <w0> <w1> <w2> <w3>` |
| 1 | early hw init | `rechord_hw_early_init()` (weak board hook, TODO: clocks/power) | `boot: early hw init` |
| 2 | drivers | `display_init`, `display_clear`, `display_flush_rows` | `boot: display init` |
| 3a | services: log | `log_init(0)` (frozen clock until board tick TODO) | `boot: log init` |
| 3b | services: settings | `settings_defaults`, `settings_load` (defaults kept on error) | `boot: settings init` |
| 3c | services: input | (static queue — nothing to start) | `boot: input init` |
| 3d | services: fs | (backend chosen at link time) | `boot: fs init` |
| 4 | theme | `theme_load_named("Classic")`, `theme_get()` (keep default on error) | `boot: theme init`, `boot: theme '…'` |
| 5 | modules registry | `module_registry_reset`, `add(overlay, player)`, `init_all()` (= `modules_init`, `player_init`), `start_all()` (= `modules_start_default`) | `boot: modules init` |
| 6 | ui | `ui_init()` | `boot: ui init` |
| 7 | boot screen | `ui_push_screen(&boot_screen)`, then `log_flush("boot")` | `boot: boot screen` |
| 8 | main loop | `input_poll` -> `ui_handle_event`, `ui_run_frame`, idle/sleep policy | `boot: main loop` |

Main loop policy (frame-counted; thresholds are compile knobs
`RECHORD_IDLE_FRAMES` / `RECHORD_SLEEP_FRAMES`):

- any input event resets the idle counter and, if the screen-off hook is
  active, logs `idle: wake`;
- `idle >= RECHORD_IDLE_FRAMES`: `idle: screen off` + `rechord_platform_idle()`
  (once until next activity);
- `idle >= RECHORD_SLEEP_FRAMES`: `idle: sleep` + `log_flush("idle")` +
  `rechord_platform_sleep()`, then the policy re-arms;
- `rechord_app_max_frames` (global, default 0 = forever) bounds the loop for
  host tests.

## 4. Files

| File | Role |
|---|---|
| `firmware/startup/startup.c` | RKnanoFW image header + C-runtime startup + boot-param capture + Reset entry trampoline |
| `firmware/startup/vectors.c` | core vector table (SP `0x03004000`, Reset -> `rechord_startup_entry`), default handler |
| `firmware/startup/startup.mk` | build fragment (sources + target-only note) |
| `firmware/app/main.c` | `rechord_main`: bring-up order + main loop + weak board hooks |
| `firmware/app/module_registry.h/.c` | static feature-module registry (name/init/start/stop) + iteration API |
| `firmware/app/boot_params.h` | documented 4-word boot-param struct + accessors (header-only by design) |
| `firmware/app/internal/shims.h` | TEMPORARY forward decls for not-yet-published APIs (`ui_run_frame`, `boot_screen`, `modules_*`, `player_init`) + board-hook decls |
| `firmware/app/app.mk` | build fragment (sources + exact host-test and target compile commands) |
| `firmware/app/tests/` | host tests: bring-up order (mock modules record the sequence), registry add/iterate, boot-param parsing |

Registry contract (module_registry.h): descriptors are pointers to static
const tables (no allocation); `init_all()` runs in add order and **halts** at
the first failing init, `start_all()` runs only after all inits succeeded,
`stop_all()` runs in **reverse** order (dependents first). Query API for
UI/settings: `module_registry_count/get/find`.

Boot params (`boot_params.h`): `rechord_boot_params_t { uint32_t raw[4]; }`
kept bit-exact (identity TODO), plus `boot_params_capture/get/word/
captured`. Header-only so the pre-C-runtime capture has zero link
dependencies and the same code is host-tested and booted.

## 5. Build wiring

- `firmware/startup/startup.mk` and `firmware/app/app.mk` are included by the
  root Makefile when the thin-Makefile wiring happens (not in this
  workstream's ownership); until then the exact compile lines live in the
  fragments' comments.
- App code compiles with **`-DRECHORD_UI_TARGET`** plus
  `-Ifirmware -Ifirmware/services -Ifirmware/drivers`: `ui/ui.h` reaches
  services/drivers through `ui/internal/hal.h`, which otherwise serves UI's
  own forward shims; the app consumes the REAL headers.
- Startup code is ARM-only glue (naked entry, barrier asm) — target compile
  only, no host tests by contract.

## 6. Verification (2026-10-09, this change)

Host tests (`cc -Wall -Werror`, command in `app.mk`):

```
boot_params                  PASS   (14 checks)
module_registry              PASS   (30 checks)
bringup_order                PASS   ( 6 checks)
----
50 checks, 0 failures
```

Cross compile (`arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -Wall -Werror -c`):
`startup.c`, `vectors.c`, `app/main.c`, `app/module_registry.c` — all clean.
Sanity link of the vector table confirmed the linked words at `0x03050000`:
SP `0x03004000`, Reset odd (`rechord_startup_entry` | 1), handlers odd.
`startup.c` also compiles under the legacy `make` rule's exact BB flags.

Legacy `make bb` note: the build is currently red in untouched vendor SDK
sources (`firmware/rockchip/.../AudioControl.c`, implicit declarations) —
pre-existing and out of this workstream's scope.

## 7. Open items

1. **Boot param field identification (primary TODO).** The identity of the
   4 words captured at Reset (ROM handoff ABI) is unknown; the reference
   stores them via the `DAT_0305015c` block. Our struct preserves them
   bit-exact and boot telemetry prints them (`boot: params ...`) so hardware
   logs can be correlated. When identified: union named fields over `raw[4]`
   in `boot_params.h` and document each word here.
2. **Boot-param transport ABI.** We assume the 4 words arrive in `r0..r3`
   (AAPCS) at the Reset entry (TODO confirm; if the ROM passes a memory
   block instead, change only `rechord_startup_entry`/`rechord_startup` in
   `startup.c`).
3. **`ui_run_frame` semantics** — shim in `app/internal/shims.h` assumes
   "one UI frame: render + housekeeping", with input dispatch owned by the
   app loop (`input_poll` -> `ui_handle_event`). Replace when `ui/ui.h`
   publishes the real prototype.
4. **modules/ and player/ headers** — `modules_init`, `modules_start_default`,
   `player_init` signatures are shims (assume `int f(void)`, 0 = success).
   Fold the real headers in and delete those shim blocks when the
   workstreams land.
5. **Idle thresholds** — `RECHORD_IDLE_FRAMES`/`RECHORD_SLEEP_FRAMES` are
   frame-counted constants; bind to `settings.screen_off_min` /
   `settings.auto_off_min` once the frame rate is pinned.
6. **Peripheral IRQ slots** in `vectors.c` — TODO once the RKNanoC IRQ map
   is integrated (core exceptions are wired to `rechord_default_handler`).
7. **Route-B link script** — the image link that places the vector table at
   `0x03050000` and provides `_data_load_start` is future work (root
   Makefile / packaging workstream); `startup.c`'s linker contract is
   documented in §2.
8. **Legacy BB path** — `firmware/rechord_app.c` still defines its own
   `rechord_main` for the SDK BB image; when the app glue is wired into an
   image, exactly one `rechord_main` must be linked (app's supersedes it).
