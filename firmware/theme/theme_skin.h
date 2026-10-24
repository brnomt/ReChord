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
 *            accent danger success art_ok art_c1 art_c2 art_mean art_accent
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
#define THEME_SKIN_ENGINE 4

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
    const char *state;            /* "Playing" / "Paused" / "Stopped" */
    const char *path;             /* current track path ("" when none) */
    const char *heading;          /* screen title/path (Library, Settings, ...) */
    long position_ms;
    long duration_ms;
    int  bits;                    /* source depth (0 unknown) */
    long sample_rate;             /* Hz (0 unknown) */
    int  playlist_pos;            /* 1-based, 0 when no list */
    int  playlist_count;
    int  shuffle;
    int  repeat;                  /* PLAYLIST_REPEAT_* */
} theme_skin_state_t;

/* Dynamic list source for a compiled screen (Library, Settings, Browser,
 * ...). The callbacks fill `buf` and return it (or NULL for an empty row).
 * `value` is optional (right-aligned column). The skin's `list` meter opts
 * into this by declaring `source=<anything>`. */
typedef struct theme_skin_list_source {
    int count;
    const char *(*label)(int index, char *buf, int cap);
    const char *(*value)(int index, char *buf, int cap);
} theme_skin_list_source_t;

void theme_skin_set_list(const theme_skin_list_source_t *src);

/* Album-art source used by the `art` meter and the art_* variables. The UI
 * adapter registers it (services/art_pack.c backs it on the device); the
 * engine only reads the palette and calls draw(), so it never touches storage.
 * NULL = no art support: art_ok is 0 and `art` draws its placeholder. */
typedef struct theme_skin_art {
    int ok;                    /* 1 = the current track has a cover        */
    int c1, c2, mean, accent;  /* RGB565 palette                           */
    int large_px, small_px;    /* stored cover sizes (0 = unknown)         */
    /* Stream a cover into the framebuffer at (x,y), clipped. 0 = drawn,
     * <0 = unavailable (the engine draws the placeholder). */
    int (*draw)(int x, int y, int large);
} theme_skin_art_t;

void theme_skin_set_art(const theme_skin_art_t *art);

/* Navigation results for a compiled screen using the skin's prev/next map.
 * Only vol up/down are mapped; SELECT and BACK are fixed. */
enum {
    THEME_SKIN_PREV = 10,
    THEME_SKIN_NEXT,
    THEME_SKIN_SELECT
};
int theme_skin_nav(int key);

/* Read <root>/<id>/<path> into the engine and cache the interactive list.
 * Returns 0 on success, < 0 when the file is missing/too large. */
int theme_skin_load(const char *root, const char *id, const char *path);
void theme_skin_reset(void);

/* The loaded text (tests/diagnostics). NULL when nothing is loaded. */
const char *theme_skin_text(void);

/* Draw the active skin. `now` and `idle` are milliseconds; `sel` is the
 * selected list item and `top` the first visible item. */
void theme_skin_render(unsigned now, unsigned idle, int sel, int top,
                       const theme_skin_state_t *st);

/* Handle one key for the classic home list. Updates *sel on previous/next and
 * returns one of the THEME_SKIN_* results (action for SELECT, REDRAW for
 * movement). Compiled screens use theme_skin_nav() instead. */
int theme_skin_key(int key, int *sel);

/* Interactive list metadata (parsed at load / set by the screen). */
int theme_skin_item_count(void);   /* active list: dynamic source or static  */
int theme_skin_selected_default(void);
int theme_skin_list_rows(void);    /* visible rows from the skin, 0 = all    */
/* The skin's declared `rows` before a dynamic source is registered (used by
 * a screen to size its fetch window on entry). */
int theme_skin_list_rows_hint(void);
/* Bounds of the drawn list (for a partial redraw). Returns 0 when a list
 * exists, < 0 otherwise. */
int theme_skin_list_bounds(int *y, int *h);

/* Animation refresh hints from the skin's [skin] section. `animated` is 1
 * only when the skin explicitly declares refresh_ms > 0, so static per-screen
 * skins do not wake the CPU. */
int theme_skin_animated(void);
int theme_skin_refresh_ms(void);
int theme_skin_refresh_h(void);

#endif /* RECHORD_THEME_SKIN_H */
