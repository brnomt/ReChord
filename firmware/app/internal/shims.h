/*
 * shims.h — TEMPORARY forward declarations for APIs that other workstreams
 * have not published as headers yet.
 *
 * Every declaration below is a SHIM (declaration only, no code) standing in
 * for a future public header. When the real header lands, delete the shim
 * block and include the real header instead — one edit per block, marked
 * TODO(workstream). Until then this file is the single, clearly-marked place
 * where those contracts live, so nothing is invented across the tree twice.
 *
 * Do NOT add logic here. Do NOT include this file from tests of other
 * modules — it exists for firmware/app and firmware/startup only.
 */
#ifndef RECHORD_APP_INTERNAL_SHIMS_H
#define RECHORD_APP_INTERNAL_SHIMS_H

#include "ui/ui.h"   /* ui_screen_t (real header) */

/* ---- board hooks (OURS, extension points for the board layer) ----------
 * Weak defaults live in firmware/app/main.c so the firmware links before a
 * board layer exists; host tests provide strong overrides to record calls. */

/* Early hardware init, first step of bring-up: PLLs/clocks/power TODO when
 * the drivers workstream exposes them. */
void rechord_hw_early_init(void);

/* Idle policy hooks (see the main loop in main.c):
 *   platform_idle  — user idle reached: dim/backlight-off TODO
 *   platform_sleep — deep idle reached: WFI / power-off TODO            */
void rechord_platform_idle(void);
void rechord_platform_sleep(void);

/* ---- ui (TODO(ui): fold into firmware/ui/ui.h / screens.h) ------------- */

/*
 * Run ONE UI frame (render + internal housekeeping). ASSUMED SEMANTICS
 * (documented in docs/rewrite/core.md): input dispatch is owned by the app
 * main loop (input_poll -> ui_handle_event), ui_run_frame only renders and
 * does UI-internal per-frame work. Replace this shim when ui.h publishes
 * the real prototype.
 */
void ui_run_frame(void);

/* The boot screen (branding + status lines) lives in ui/screens today;
 * declared here until ui publishes a stable screens header. */
extern const ui_screen_t boot_screen;

/* ---- modules (TODO(modules): firmware/modules/modules.h) --------------- */

/* Init the overlay module subsystem; 0 = success. */
int modules_init(void);

/* Start the default set of overlay modules; 0 = success. */
int modules_start_default(void);

/* ---- player (TODO(player): firmware/player/player.h) ------------------- */

/* Init the player service; 0 = success. */
int player_init(void);

#endif /* RECHORD_APP_INTERNAL_SHIMS_H */
