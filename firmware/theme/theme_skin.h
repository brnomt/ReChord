/*
 * theme_skin.h — declarative skin interpreter (BRNOMT-69).
 *
 * A theme package ships a skin file (e.g. home.skin) and its assets. The
 * engine in this file parses and executes that file at runtime; NO theme
 * layout or animation is compiled into the firmware. The language is a
 * bounded, Rockbox/Rainmeter-style mix:
 *
 *   [section]
 *   type=fill|roundrect|triangle|bitmap|text|bar|list
 *   key=value        (values may be expressions)
 *
 * Sections may also set `visible=<expr>` to skip drawing. A `[vars]` section
 * defines named expressions reusable from any meter; variables may reference
 * earlier/later vars (bounded recursion).
 *
 * Expressions: integers, strings, variables, arithmetic, comparisons,
 * logical operators, ternary and a small function set:
 *   min max abs clamp mod noise sin seq
 * Variables: t idle playing paused stopped volume battery W H sel n aw ah
 *            title artist album state background surface text text_dim
 *            accent danger success
 *
 * The engine is intentionally bounded: one 4 KiB skin text, 32 sections,
 * 12 properties per section, 64-byte values, recursion depth 12.
 */
#ifndef RECHORD_THEME_SKIN_H
#define RECHORD_THEME_SKIN_H

#include <stdint.h>

#define THEME_SKIN_TEXT_MAX  4096
#define THEME_SKIN_ITEMS_MAX 4

/* Interpreter revision, logged at boot so a device can prove which engine is
 * running (v2 = vector roundrect/triangle + [vars]). */
#define THEME_SKIN_ENGINE 3

/* Key codes match input_key_t (services/input.h) without pulling it in. */
enum {
    THEME_SKIN_KEY_UP = 1,
    THEME_SKIN_KEY_DOWN = 2,
    THEME_SKIN_KEY_SELECT = 3,
    THEME_SKIN_KEY_BACK = 4
};

/* theme_skin_key() results. */
enum {
    THEME_SKIN_NONE = -1,
    THEME_SKIN_REDRAW = 0,
    THEME_SKIN_BACK,
    THEME_SKIN_LIBRARY,
    THEME_SKIN_PLAYER,
    THEME_SKIN_SETTINGS
};

/* Read-only state exposed to skin expressions. The UI adapter fills it; the
 * engine never talks to the player directly. */
typedef struct theme_skin_state {
    int playing;
    int paused;
    int stopped;
    int volume_db;
    int battery_pct;              /* -1 when unknown */
    const char *title;
    const char *artist;
    const char *album;
    const char *state;
} theme_skin_state_t;

/* Read <root>/<id>/<path> into the engine and cache the interactive list.
 * Returns 0 on success, < 0 when the file is missing/too large. */
int theme_skin_load(const char *root, const char *id, const char *path);
void theme_skin_reset(void);

/* The loaded text (tests/diagnostics). NULL when nothing is loaded. */
const char *theme_skin_text(void);

/* Draw the active skin. `now` and `idle` are milliseconds; `sel` is the
 * selected list item. */
void theme_skin_render(unsigned now, unsigned idle, int sel,
                       const theme_skin_state_t *st);

/* Handle one key. Updates *sel on previous/next and returns one of the
 * THEME_SKIN_* results (action for SELECT, REDRAW for movement). */
int theme_skin_key(int key, int *sel);

/* Interactive list metadata (parsed at load). */
int theme_skin_item_count(void);
int theme_skin_selected_default(void);

/* Animation refresh hints from the skin's [skin] section. */
int theme_skin_refresh_ms(void);
int theme_skin_refresh_h(void);

#endif /* RECHORD_THEME_SKIN_H */
