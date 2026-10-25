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

#define THEME_ASSET_MAX      8
#define THEME_ASSET_NAME_MAX 16
#define THEME_ASSET_PATH_MAX 48

/* Home renderer selected by the package. SKIN is the generic interpreter
 * that reads the package's skin file; MENU is the classic built-in list. */
enum {
    THEME_HOME_MENU = 0,
    THEME_HOME_SKIN = 1
};

/* Axis the generic home list uses. */
enum {
    THEME_NAV_VERTICAL = 0,
    THEME_NAV_HORIZONTAL = 1
};

/* Who owns the VOL up/down navigation mapping. `engine` = the global
 * Settings > Interface > Controls choice; `theme` = the package's skin
 * prev/next map wins on screens that use it. */
enum {
    THEME_INPUT_ENGINE = 0,
    THEME_INPUT_THEME = 1
};

typedef struct theme_asset_decl {
    char name[THEME_ASSET_NAME_MAX];
    char path[THEME_ASSET_PATH_MAX];   /* relative to the theme directory */
    int  w, h;
} theme_asset_decl_t;

typedef struct
{
    int format;       /* manifest version (only 1 is accepted)  */
    int home;         /* THEME_HOME_* renderer                  */
    int home_nav;     /* THEME_NAV_* axis for the home items    */
    int input;        /* THEME_INPUT_*: who maps VOL up/down     */

    /* Shared list metrics (menu/library/categories/settings/browser). */
    int pad;          /* left/right margin                      */
    int title_h;      /* header band height                     */
    int item_h;       /* list row height                        */
    int face_h;       /* animated band height (skin refresh hint) */
    int home_bar_y;   /* generic home item band origin            */
    int home_bar_h;   /* generic home item band height            */

    /* Declarative skin files (relative to the theme directory). `skin` is the
     * home/main screen; the rest are optional per-screen skins. The compiled
     * screen is used when a path is empty or the file cannot be loaded. */
    char skin[THEME_ASSET_PATH_MAX];
    char skin_now_playing[THEME_ASSET_PATH_MAX];
    char skin_library[THEME_ASSET_PATH_MAX];
    char skin_library_cat[THEME_ASSET_PATH_MAX];
    char skin_browser[THEME_ASSET_PATH_MAX];
    char skin_settings[THEME_ASSET_PATH_MAX];

    /* 1-bit assets shipped inside the package. */
    int asset_count;
    theme_asset_decl_t assets[THEME_ASSET_MAX];

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

/* 1 when the active package declared `input=theme`: its skin prev/next map
 * owns VOL navigation on screens that use the theme key map. */
int theme_input_owns_keys(void);

/* Commit a fully parsed layout (used by the package runtime after it has
 * validated palette + layout together; nothing is ever half-applied). */
void theme_layout_set(const theme_layout_t *l);

/* Back to the built-in layout. */
void theme_layout_reset(void);

#endif /* RECHORD_THEME_PKG_H */
