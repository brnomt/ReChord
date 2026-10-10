/*
 * ui.h — ReChord UI core: event loop skeleton, screen stack, redraw model.
 *
 * Design (Rockbox-style, clean-room):
 *   - A screen is a const vtable (enter/exit/draw/handle_event) with its own
 *     private static state in its .c file.  Screens are pushed/popped on a
 *     small fixed stack — no dynamic allocation anywhere.
 *   - Redraw model: a screen repaints its full logical content in draw(),
 *     but only DIRTY ROW BANDS are pushed to the panel via
 *     display_flush_rows() — the SPI flush is the expensive part, the
 *     framebuffer fill is cheap.
 *   - Event loop: ui_run() polls input events and dispatches them to the
 *     top screen, then renders.  ui_step-level functions are exposed so host
 *     tests can drive the state machine tick by tick.
 *
 * Part of the frontend workstream.  Pure C99, no libc beyond stdint/stdbool.
 */
#ifndef RECHORD_UI_H
#define RECHORD_UI_H

#include <stdbool.h>

#include "internal/hal.h"   /* input_event_t (services input contract) */

#define RECHORD_UI_VERSION "0.1.0"
#define UI_STACK_MAX       4

/* Screen vtable.  All hooks are optional (NULL-safe). */
typedef struct ui_screen {
    const char *name;                                  /* for logging/tests */
    void (*on_enter)(void);                            /* became top screen */
    void (*on_exit)(void);                             /* popped off stack   */
    void (*draw)(void);                                /* repaint + own dirty*/
    void (*handle_event)(const input_event_t *ev);     /* key event          */
} ui_screen_t;

/* ---- lifecycle / event loop ------------------------------------------ */

void ui_init(void);                 /* reset stack + dirty state            */
void ui_run(void);                  /* run until ui_quit() — skeleton loop  */
void ui_quit(void);                 /* leave ui_run()                       */
void ui_handle_event(const input_event_t *ev); /* dispatch to top screen    */
void ui_render(void);               /* draw top screen, flush dirty rows    */

/* ---- screen stack ----------------------------------------------------- */

bool ui_push_screen(const ui_screen_t *screen); /* false if full/NULL       */
bool ui_pop_screen(void);                       /* false if already empty   */
const ui_screen_t *ui_top_screen(void);
int ui_stack_depth(void);

/* ---- redraw model (dirty row bands, inclusive y) ---------------------- */

void ui_invalidate(int x0, int y0, int x1, int y1); /* mark rows y0..y1    */
void ui_invalidate_all(void);
bool ui_is_dirty(void);

/* Introspection for host tests. */
int  ui_dirty_band_count(void);
bool ui_dirty_band(int idx, int *y0, int *y1);  /* inclusive row range      */

#endif /* RECHORD_UI_H */
