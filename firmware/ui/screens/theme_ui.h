/*
 * theme_ui.h — generic per-screen skin bridge.
 *
 * Every compiled screen asks this helper whether the active theme declares a
 * skin for it. If so, the screen draws through the declarative engine and
 * passes its data as a dynamic list source; otherwise it keeps its built-in
 * drawing. No theme code is compiled and the screen's behavior/actions stay in
 * the app.
 */
#ifndef RECHORD_UI_THEME_UI_H
#define RECHORD_UI_THEME_UI_H

#include "../../theme/theme_skin.h"

typedef enum {
    THEME_UI_MAIN = 0,
    THEME_UI_NOW_PLAYING,
    THEME_UI_LIBRARY,
    THEME_UI_LIBRARY_CAT,
    THEME_UI_BROWSER,
    THEME_UI_SETTINGS
} theme_ui_screen_t;

/* Load the skin declared for `id`; returns 1 when the screen should use it,
 * 0 to fall back to the compiled UI (also resets the engine then). */
int theme_ui_load(theme_ui_screen_t id);

/* Declared relative path, or "" when the theme has no skin for this screen. */
const char *theme_ui_path(theme_ui_screen_t id);

/* Clock/idle bookkeeping, fed by the app loop and by each screen's key path. */
void theme_ui_tick(unsigned now_ms);
void theme_ui_note_input(void);

/* Fill the read-only runtime snapshot from the player. `heading` is the
 * screen title (Library, Settings, path, ...). */
void theme_ui_fill_state(theme_skin_state_t *st, const char *heading);

/* Draw the active screen skin. */
void theme_ui_draw(const theme_skin_state_t *st, int sel, int top);

/* Re-render only the package's `dyn=1` primitives (no clear, no static
 * sections) and flush the rects it declares. Returns 0 when the skin has no
 * dynamic rects, so the caller falls back to a band invalidation. */
int theme_ui_draw_dynamic(const theme_skin_state_t *st, int sel, int top);

/* Dynamic list source for the current screen (NULL clears it). */
void theme_ui_list(const theme_skin_list_source_t *src);

/* Map a key through the skin's prev/next bindings. */
int theme_ui_nav(int key);

/* Visible rows and item count of the active screen skin. */
int theme_ui_rows(void);
int theme_ui_rows_hint(void);
int theme_ui_count(void);

/* 1 while the top screen is drawing through a loaded skin; 0 after a
 * compiled screen (or a theme without a skin for it) took over. */
int theme_ui_active(void);
void theme_ui_disarm(void);

#endif /* RECHORD_UI_THEME_UI_H */
