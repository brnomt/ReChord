/*
 * theme_pkg.h — bounded layout DSL for theme packages (BRNOMT-69 B/C/D).
 *
 * The manifest is the SAME key=value `theme.cfg` the palette loader reads:
 * theme_file.c consumes the color keys and ignores these, theme_layout_parse()
 * consumes the layout keys and ignores those. One file, two readers, no
 * recompilation, no scripting VM.
 *
 * Every value is bounded; a layout key outside its range rejects the whole
 * layout (the previous/default one stays), so a malformed package can never
 * draw a half-broken screen. Structures are fixed-size: 0 B of heap.
 */
#ifndef RECHORD_THEME_PKG_H
#define RECHORD_THEME_PKG_H

#define THEME_LAYOUT_MAX_W 320
#define THEME_LAYOUT_MAX_H 170

typedef struct
{
    /* Shared list metrics (menu/library/categories/settings/browser). */
    int pad;          /* left/right margin                      */
    int title_h;      /* header band height                     */
    int item_h;       /* list row height                        */

    /* Now Playing element origins. */
    int np_title_y;
    int np_artist_y;
    int np_state_y;
    int np_info_y;
    int np_bar_y;
    int np_bar_h;
    int np_time_y;
    int np_mode_y;
    int np_vol_y;
    int np_vol_h;
} theme_layout_t;

/* The built-in layout: exactly what the screens used before packages. */
void theme_layout_defaults(theme_layout_t *l);

/* Parse layout keys from `text` into `l` (starting FROM `l`, so a partial file
 * overrides only what it declares). Returns 0 on success, < 0 when any layout
 * value is malformed or out of range (`l` is left as it was). */
int theme_layout_parse(const char *text, theme_layout_t *l);

/* Read `path` through the read-only stdio shim and apply it to the ACTIVE
 * layout. Returns 0 when applied, < 0 when the file is missing/invalid (the
 * active layout is then reset to the defaults). */
int theme_layout_load(const char *path);

/* Active layout — never NULL; defaults when no package is loaded. */
const theme_layout_t *theme_layout(void);

/* Back to the built-in layout. */
void theme_layout_reset(void);

#endif /* RECHORD_THEME_PKG_H */
