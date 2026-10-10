/*
 * theme.h — ReChord theme API (the ONLY theme interface other modules use).
 *
 * A theme is pure data: an RGB565 palette, spacing/radius metrics and a font
 * id. UI code never hardcodes a color — it asks theme_get(). Built-in themes
 * are .c data tables (themes_builtin.c); a user file of `key=value` lines can
 * override any subset of fields at runtime (theme_file.c).
 *
 * WHY RGB565: the display contract (docs/re/route-b-minimum.md) fills and
 * blits RGB565 pixels directly, so the palette stores final pixel values —
 * no per-frame channel swizzling on the Cortex-M3.
 */
#ifndef THEME_H
#define THEME_H

#include <stdint.h>

/* Max theme name length including the NUL terminator (embedded in the struct
 * so a theme_t copy is fully self-contained — no lifetime traps). */
#define THEME_NAME_MAX 24

typedef struct theme
{
    char     name[THEME_NAME_MAX];

    /* Palette — RGB565 (5-6-5) pixel values, exactly as sent to the panel. */
    uint16_t background;  /* screen/clear color                          */
    uint16_t surface;     /* cards, bars, popups sitting on background   */
    uint16_t text;        /* primary text                                */
    uint16_t text_dim;    /* secondary text, hints, disabled items       */
    uint16_t accent;      /* selection, highlights, active toggles       */
    uint16_t danger;      /* destructive actions, errors, low battery    */
    uint16_t success;     /* confirmations, OK states, charging          */

    /* Layout metrics (px). */
    int      spacing;     /* gap between elements                        */
    int      radius;      /* corner radius of rounded rects              */
    int      font_id;     /* driver font id (0 = default 6x11 bitmap)    */
} theme_t;

/*
 * Current theme access.
 *   theme_get()  returns a pointer valid until the next theme_set()/load.
 *   theme_set()  copies *t into the current slot (NULL is ignored).
 * On first use the current theme defaults to built-in theme 0 ("Classic").
 */
const theme_t *theme_get(void);
void           theme_set(const theme_t *t);

/*
 * Built-in theme registry (indices 0 .. theme_count()-1 are stable order).
 *   theme_load_named()  installs a built-in theme by name (ASCII case-
 *                       insensitive); returns 0 on success, <0 if unknown.
 *   theme_name(i)       returns the name of built-in i, NULL if out of range.
 */
int           theme_load_named(const char *name);
int           theme_count(void);
const char   *theme_name(int i);

/* Sanity-check a theme: 0 = usable, <0 = which field failed (see .c). */
int           theme_validate(const theme_t *t);

/*
 * File-based override loader (theme_file.c). Both functions are OVERRIDE
 * style: they write only the keys present in the text and leave every other
 * field of *inout untouched, so a user file may override 1..N fields of a
 * base theme ("a file-based theme can override at runtime").
 *
 *   theme_file_parse()  parses one `key=value` per line text blob.
 *   theme_file_load()   reads the file at `path` and parses it.
 *
 * Tolerant by design (user-edited files must not brick the UI): blank lines,
 * `#`/`;` comments, unknown keys and malformed lines are skipped, values may
 * be decimal or 0x-prefixed hex. Returns the number of keys applied (>= 0),
 * or < 0 on hard errors (NULL arguments, unreadable file).
 */
int           theme_file_parse(const char *text, theme_t *inout);
int           theme_file_load(const char *path, theme_t *inout);

#endif /* THEME_H */
