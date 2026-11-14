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

/* Per-theme options (theme.cfg `opt_<id>=<Label>,<choice>|<choice>...`).
 * The selected index is exposed to every skin expression as `opt_<id>`, so a
 * package can ship variants (clock vs cover, 20 vs 60 fps) as pure data. */
#define THEME_OPT_MAX         4
#define THEME_OPT_NAME_MAX    16
#define THEME_OPT_LABEL_MAX   24
#define THEME_OPT_CHOICES_MAX 4
#define THEME_OPT_CHOICE_MAX  16

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

/* One declared option: the skin reads `opt_<id>` (the selected index). */
typedef struct theme_opt_decl {
    char id[THEME_OPT_NAME_MAX];               /* lowercase identifier     */
    char label[THEME_OPT_LABEL_MAX];           /* English UI label         */
    char choice[THEME_OPT_CHOICES_MAX][THEME_OPT_CHOICE_MAX];
    int  count;                                /* 2..THEME_OPT_CHOICES_MAX */
    int  value;                                /* selected index (runtime) */
} theme_opt_decl_t;

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

    /* Per-theme options. `value` is filled by the caller from the persisted
     * selection (default 0) AFTER parsing: the manifest only declares. */
    int opt_count;
    theme_opt_decl_t opts[THEME_OPT_MAX];

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
 * value is malformed or out of range (`l` is left as it was).
 *
 * A malformed `opt_<id>` line is different: it is DROPPED (counted by
 * theme_opt_rejected()) instead of rejecting the package, because an
 * unreadable option must never cost the user the theme. Everything else keeps
 * the atomic policy. */
int theme_layout_parse(const char *text, theme_layout_t *l);

/* Number of `opt_` lines the last parse dropped as malformed. The package
 * runtime logs it, so a theme author can see a typo in `/.rechord/LOG`. */
int theme_opt_rejected(void);

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

/* Selected index of option `id`, or -1 when the active package does not
 * declare it. The skin engine exposes it as `opt_<id>`. */
int theme_opt_value(const char *id);

/* Set option `id` to `value` (clamped to the declared choice count).
 * Returns the stored index, or -1 when the option is unknown. */
int theme_opt_set(const char *id, int value);

/* ---- option persistence text (pure, host-tested) ------------------------
 * The cfg keeps every theme's choices in one value:
 *   theme_opts=<theme-id>:<digit per option>;<theme-id>:<digits>;
 * `theme_opts_decode` fills `out[0..max-1]` with the digits stored for `id`
 * (zeros when the theme has no entry yet) and returns how many digits the
 * entry actually held (0 = no entry). Ids containing ':' or ';' are rejected
 * by both functions: they could not round-trip.
 * `theme_opts_encode_entry` rewrites `text` as a NUL-terminated buffer,
 * dropping any stale entry for `id` and appending the new one. Returns the
 * new length, < 0 when the buffer cannot hold the result. */
int theme_opts_decode(const char *text, const char *id, int *out, int max);
int theme_opts_encode_entry(char *text, int cap, const char *id,
                           const int *vals, int n);

/* Apply the digits for `id` (as decoded above) to the declared options of the
 * ACTIVE layout. A digit at or past a declared choice count is clamped, so a
 * theme that drops a choice never renders an out-of-range variant. */
void theme_opts_apply(const int *vals, int n);

/* The active layout's current selections, in declaration order. */
void theme_opts_values(int *out, int max);

#endif /* RECHORD_THEME_PKG_H */
