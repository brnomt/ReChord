/*
 * hal.h — the only door through which the UI reaches display + services.
 *
 * The real headers live in other workstreams' trees:
 *   - firmware/drivers/display.h   (display primitives)
 *   - firmware/services/input.h    (input event queue)
 *   - firmware/services/settings.h (settings persistence)
 *   - firmware/services/log.h      (logging)
 *   - firmware/services/fs.h       (fs_list_dir file-listing service)
 *
 * Until those land, the UI builds against TEMPORARY FORWARD SHIMS in
 * firmware/ui/internal/shims/ (declarations only — no code).  Build with
 * -DRECHORD_UI_TARGET plus the real include dirs to compile against the real
 * headers instead; the shims must then not be on the include path.
 *
 * Contract assumptions made here (documented in docs/rewrite/frontend.md):
 *   - all rectangle coordinates are INCLUSIVE (x0..x1, y0..y1);
 *   - display_flush_rows(y0, y1) flushes inclusive row range y0..y1;
 *   - input_poll() returns non-zero when it filled *ev with a real event;
 *   - settings_load()/settings_save() operate on rechord_settings_t and
 *     return 0 on success.
 */
#ifndef RECHORD_UI_HAL_H
#define RECHORD_UI_HAL_H

#ifdef RECHORD_UI_TARGET
#include "display.h"
#include "input.h"
#include "settings.h"
#include "log.h"
#include "fs.h"
#else
#include "shims/display.h"
#include "shims/input.h"
#include "shims/settings.h"
#include "shims/log.h"
#include "shims/fs.h"
#endif

#endif /* RECHORD_UI_HAL_H */
