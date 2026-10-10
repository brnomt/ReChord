/*
 * main.c — ReChord app glue: the deterministic bring-up and the main loop.
 *
 * This is `rechord_main`, the single entry the startup sequence calls once
 * the C runtime is up (firmware/startup/startup.c, boot contract in
 * docs/re/route-b-minimum.md §2). Everything here is ORDERED ON PURPOSE:
 * each layer may only depend on layers above it in the bring-up table
 * (docs/rewrite/core.md), and every step is logged (boot telemetry style)
 * so a boot log alone shows how far the device got.
 *
 * Bring-up order (the contract; host-verified in tests/test_bringup.c):
 *   1. early hw init        board hooks (clocks/power TODO)
 *   2. drivers              display_init (+ clean screen)
 *   3. services             log, settings, input, fs
 *   4. theme                built-in theme install
 *   5. modules registry     register feature modules, init_all, start_all
 *   6. ui                   ui_init
 *   7. boot screen          push the boot screen
 *   8. main loop            ui_run_frame + input_poll + idle/sleep policy
 *
 * WHY this order: the display must show failures of everything after it;
 * the logger is a service but its ring buffer works from step 1 (frozen
 * clock) so even display failures can be logged later; settings drive the
 * theme and the idle policy; modules and UI must see a fully initialized
 * world, so they run last.
 */
#include <stdint.h>

#include "app/boot_params.h"
#include "app/internal/shims.h"
#include "app/module_registry.h"
#include "drivers/display.h"
#include "services/input.h"
#include "services/log.h"
#include "services/settings.h"
#include "theme/theme.h"
#include "ui/ui.h"

/* First screen clear color: black. The theme service comes up AFTER the
 * display (bring-up order), so this clear cannot use theme colors yet; the
 * boot screen repaints with theme colors once ui_init() has run. */
#define BOOT_BG_COLOR 0x0000u

/* Built-in theme installed at boot. On failure the theme service keeps its
 * default ("Classic"), which is why the fallback is just a log line. */
#define BOOT_THEME_NAME "Classic"

/* Idle policy thresholds in frames. TODO: bind these to
 * settings.screen_off_min / settings.auto_off_min once the frame rate is
 * pinned down; until then they are conservative compile-time knobs. */
#ifndef RECHORD_IDLE_FRAMES
#define RECHORD_IDLE_FRAMES   300u   /* ~10 s at 30 fps: dim / screen off */
#endif
#ifndef RECHORD_SLEEP_FRAMES
#define RECHORD_SLEEP_FRAMES 1800u   /* ~60 s at 30 fps: deep sleep       */
#endif

/* Main-loop frame budget: 0 = run forever (the firmware's normal mode).
 * Host tests set this global to run deterministic short loops. */
#ifdef RECHORD_APP_MAX_FRAMES
unsigned rechord_app_max_frames = RECHORD_APP_MAX_FRAMES;
#else
unsigned rechord_app_max_frames = 0;
#endif

/* ---- board hooks (weak defaults; board layer or host tests override) ----
 * Weak so the firmware links and boots safely before a board layer exists:
 * an unimplemented hook is a no-op, never a crash. See internal/shims.h. */
__attribute__((weak)) void rechord_hw_early_init(void) { }
__attribute__((weak)) void rechord_platform_idle(void) { }
__attribute__((weak)) void rechord_platform_sleep(void) { }

/* Live settings copy for bring-up decisions (the settings screen keeps its
 * own copy — see ui/screens; this one is read-only after load). */
static settings_t g_settings;

/* ---- built-in feature modules registered at boot ------------------------
 * Wrappers adapt the not-yet-published module APIs (internal/shims.h) to
 * the registry's uniform hook signature. Order = init/start order:
 * the overlay registry first (it owns the module loader), player after. */
static int builtin_overlay_init(void)  { return modules_init(); }
static int builtin_overlay_start(void) { return modules_start_default(); }
static int builtin_player_init(void)   { return player_init(); }

static const module_descriptor_t k_builtin_modules[] = {
    { "overlay", builtin_overlay_init, builtin_overlay_start, 0 },
    { "player",  builtin_player_init,  0,                    0 },
};

/* Boot telemetry: dump the 4 ROM handoff words so field identification
 * (open item) can correlate logs with hardware state. */
static void log_boot_params(void)
{
    const rechord_boot_params_t *bp = boot_params_get();

    if (bp == 0) {
        log_printf("boot: params none");
        return;
    }
    log_printf("boot: params %08lx %08lx %08lx %08lx",
               (unsigned long)boot_params_word(bp, 0),
               (unsigned long)boot_params_word(bp, 1),
               (unsigned long)boot_params_word(bp, 2),
               (unsigned long)boot_params_word(bp, 3));
}

/* ---- main loop ---------------------------------------------------------
 * One frame = drain input into the UI, run one UI frame, apply the idle
 * policy. The app OWNS input dispatch (input_poll -> ui_handle_event);
 * ui_run_frame renders and does UI-internal per-frame work (assumed
 * semantics of the ui_run_frame shim, see internal/shims.h).
 */
static void app_main_loop(void)
{
    unsigned idle = 0;   /* frames since the last input event             */
    unsigned frame = 0;
    int dimmed = 0;      /* screen-off hook currently active             */

    for (;;) {
        input_event_t ev;

        while (input_poll(&ev) > 0) {
            if (dimmed) {
                log_printf("idle: wake");   /* visible activity ends idle */
                dimmed = 0;
            }
            idle = 0;
            ui_handle_event(&ev);
        }

        ui_run_frame();

        idle++;
        if (idle >= RECHORD_SLEEP_FRAMES) {
            log_printf("idle: sleep");
            log_flush("idle");   /* persist telemetry before power-down   */
            rechord_platform_sleep();
            idle = 0;
            dimmed = 0;
        } else if (idle >= RECHORD_IDLE_FRAMES && !dimmed) {
            log_printf("idle: screen off");
            rechord_platform_idle();
            dimmed = 1;
        }

        if (rechord_app_max_frames != 0 && frame + 1 >= rechord_app_max_frames)
            break;
        frame++;
    }
}

/* ---- bring-up + main -------------------------------------------------- */
void rechord_main(void)
{
    unsigned i;

    /* 1. early hardware init */
    log_printf("boot: early hw init");
    rechord_hw_early_init();
    log_boot_params();

    /* 2. drivers: display first so later failures are visible */
    log_printf("boot: display init");
    display_init();
    display_clear(BOOT_BG_COLOR);
    display_flush_rows(0, DISPLAY_H - 1);

    /* 3. services: log, settings, input, fs */
    log_printf("boot: log init");
    log_init(0);   /* frozen clock until the board tick hook lands (TODO) */
    log_printf("boot: settings init");
    settings_defaults(&g_settings);
    if (settings_load(&g_settings) != SETTINGS_OK)
        log_printf("boot: settings missing, defaults kept");
    log_printf("boot: input init");   /* static queue, nothing to start    */
    log_printf("boot: fs init");      /* backend is selected at link time  */

    /* 4. theme */
    log_printf("boot: theme init");
    if (theme_load_named(BOOT_THEME_NAME) != 0)
        log_printf("boot: theme '%s' missing, keeping default",
                   BOOT_THEME_NAME);
    log_printf("boot: theme '%s'", theme_get()->name);

    /* 5. modules registry: register, then init ALL before ANY start */
    log_printf("boot: modules init");
    module_registry_reset();
    for (i = 0; i < sizeof k_builtin_modules / sizeof k_builtin_modules[0]; i++) {
        if (module_registry_add(&k_builtin_modules[i]) != MODULE_REG_OK)
            log_printf("boot: module '%s' not registered",
                       k_builtin_modules[i].name);
    }
    if (module_registry_init_all() != MODULE_REG_OK)
        log_printf("boot: module init failed");
    if (module_registry_start_all() != MODULE_REG_OK)
        log_printf("boot: module start failed");

    /* 6. ui */
    log_printf("boot: ui init");
    ui_init();

    /* 7. boot screen */
    log_printf("boot: boot screen");
    ui_push_screen(&boot_screen);

    log_flush("boot");   /* the boot telemetry is complete — persist it   */

    /* 8. main loop */
    log_printf("boot: main loop");
    app_main_loop();
}
