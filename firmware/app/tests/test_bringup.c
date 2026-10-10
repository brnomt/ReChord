/*
 * test_bringup.c — bring-up ORDER verification for rechord_main
 * (firmware/app/main.c) with mocked services/drivers/modules.
 *
 * The bring-up order IS the contract (docs/rewrite/core.md): every mock
 * records its call into one ordered log (log_printf records its formatted
 * message = the boot telemetry), and the test asserts the EXACT sequence —
 * first the documented bring-up table, then the main-loop behaviour (input
 * dispatch + idle/sleep policy) for two scripted scenarios.
 */
#include <stdio.h>
#include <string.h>

#include "services/input.h"   /* KEY_UP / KEY_DOWN */

#include "mocks.h"
#include "test_util.h"

void rechord_main(void);   /* firmware/app/main.c */

/* ---- the documented bring-up sequence (docs/rewrite/core.md) ---- */
static const char *const k_bringup[] = {
    "boot: early hw init",                       /* 1. early hw init      */
    "hw_early",
    "boot: params 11111111 22222222 33333333 44444444", /* boot telemetry */
    "boot: display init",                        /* 2. drivers (display)  */
    "display_init",
    "display_clear",
    "display_flush",
    "boot: log init",                            /* 3. services           */
    "log_init",
    "boot: settings init",
    "settings_defaults",
    "settings_load",
    "boot: input init",
    "boot: fs init",
    "boot: theme init",                          /* 4. theme              */
    "theme_load_named",
    "theme_get",
    "boot: theme 'Classic'",
    "boot: modules init",                        /* 5. modules registry   */
    "modules_init",
    "player_init",
    "modules_start_default",
    "boot: ui init",                             /* 6. ui_init            */
    "ui_init",
    "boot: boot screen",                         /* 7. boot screen        */
    "ui_push_screen",
    "log_flush:boot",
    "boot: main loop",                           /* 8. main loop          */
};

/*
 * Scenario 1 loop tail — one key at boot (frame 0), then pure idle:
 * thresholds IDLE=2 / SLEEP=4 frames, budget 6 frames. Expect: input
 * dispatched to the UI, screen-off at 2 idle frames, sleep at 4, and the
 * policy re-arms afterwards.
 */
static const char *const k_loop_idle_sleep[] = {
    "input_poll", "ui_handle_event", "input_poll", "ui_run_frame",   /* f0 */
    "input_poll", "ui_run_frame", "idle: screen off", "platform_idle", /* f1 */
    "input_poll", "ui_run_frame",                                      /* f2 */
    "input_poll", "ui_run_frame", "idle: sleep", "log_flush:idle",
    "platform_sleep",                                                  /* f3 */
    "input_poll", "ui_run_frame",                                      /* f4 */
    "input_poll", "ui_run_frame", "idle: screen off", "platform_idle", /* f5 */
};

/*
 * Scenario 2 loop tail — a key arrives while the screen is off (visible
 * after 3 idle polls, i.e. frame 3): the wake path logs, re-dispatches and
 * the idle policy re-arms. Budget 5 frames.
 */
static const char *const k_loop_wake[] = {
    "input_poll", "ui_run_frame",                                      /* f0 */
    "input_poll", "ui_run_frame", "idle: screen off", "platform_idle", /* f1 */
    "input_poll", "ui_run_frame",                                      /* f2 */
    "input_poll", "idle: wake", "ui_handle_event", "input_poll",
    "ui_run_frame",                                                    /* f3 */
    "input_poll", "ui_run_frame", "idle: screen off", "platform_idle", /* f4 */
};

#define N_BRINGUP ((int)(sizeof k_bringup / sizeof k_bringup[0]))

/* Assert records [offset, offset+nwant) equal `want`, exactly. */
static void check_window(const char *label, const char *const *want,
                         int nwant, int offset)
{
    int i;
    int ok = 1;

    for (i = 0; i < nwant; i++) {
        const char *got = mock_rec_at(offset + i);
        if (got == 0 || strcmp(got, want[i]) != 0)
            ok = 0;
    }

    if (!ok) {
        printf("  sequence '%s' mismatch:\n", label);
        for (i = 0; i < nwant; i++) {
            const char *g = mock_rec_at(offset + i);
            printf("    [%2d] want %-52s got %s\n",
                   offset + i, want[i], g ? g : "(none)");
        }
    }
    CHECK(ok, label);
}

void test_bringup(void)
{
    int n_loop1 = (int)(sizeof k_loop_idle_sleep / sizeof k_loop_idle_sleep[0]);
    int n_loop2 = (int)(sizeof k_loop_wake / sizeof k_loop_wake[0]);

    /* ---- scenario 1: boot with one key event, then idle -> sleep ---- */
    mock_rec_reset();
    rechord_app_max_frames = 6;
    boot_params_capture(0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u);
    mock_input_queue(KEY_UP, -1);
    rechord_main();

    CHECK(mock_rec_count() == N_BRINGUP + n_loop1,
          "scenario 1: exact record count");
    check_window("bring-up order (exact)", k_bringup, N_BRINGUP, 0);
    check_window("main loop: idle -> screen off -> sleep -> re-arm",
                 k_loop_idle_sleep, n_loop1, N_BRINGUP);

    /* ---- scenario 2: key while dimmed -> wake, idle re-arms ---- */
    mock_rec_reset();
    rechord_app_max_frames = 5;
    boot_params_capture(0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u);
    mock_input_delay(3, KEY_DOWN, -1);
    rechord_main();

    CHECK(mock_rec_count() == N_BRINGUP + n_loop2,
          "scenario 2: exact record count");
    check_window("bring-up order stable across boots", k_bringup, N_BRINGUP, 0);
    check_window("main loop: wake on activity, idle re-arms",
                 k_loop_wake, n_loop2, N_BRINGUP);

    rechord_app_max_frames = 0;
}
