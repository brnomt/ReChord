/*
 * screens.h — public API of the ReChord screens (frontend workstream).
 *
 * Every screen is a const ui_screen_t that can be pushed on the UI core's
 * screen stack; control functions let the app layer (and host tests) feed
 * data in and inspect view state.  Screen-private state stays in each .c.
 */
#ifndef RECHORD_UI_SCREENS_H
#define RECHORD_UI_SCREENS_H

#include <stdbool.h>
#include <stdint.h>

#include "../ui.h"
#include "../settings_schema.h"

/* ======================================================================
 * boot_screen — ReChord branding + status lines
 * ====================================================================== */

#define BOOT_STATUS_MAX_LINES 4
#define BOOT_STATUS_LEN       40

extern const ui_screen_t boot_screen;

void boot_screen_reset(void);   /* clear all status lines                  */
/* Set one status line (0..BOOT_STATUS_MAX_LINES-1) printf-style.  Lines are
 * laid out at 12 px pitch below the branding block (cf. the id*12px status
 * line contract in docs/re/route-b-minimum.md). */
void boot_screen_status(int line, const char *fmt, ...);

/* ======================================================================
 * menu_screen — generic list menu (title + scrollable items + selection)
 * ====================================================================== */

typedef struct menu_item {
    const char *label;
    const char *(*value_text)(void); /* optional right-aligned value, or NULL */
    void (*on_select)(void);         /* optional action, or NULL              */
} menu_item_t;

typedef struct menu_model {
    const char        *title;
    const menu_item_t *items;
    int                item_count;
} menu_model_t;

/* Scrollable view state — the navigation state machine (host-tested). */
typedef struct menu_view {
    int cursor;  /* selected item index, 0..item_count-1 */
    int top;     /* first visible item index             */
} menu_view_t;

extern const ui_screen_t menu_screen;

void menu_screen_bind(const menu_model_t *model); /* model must outlive use */
const menu_model_t *menu_screen_model(void);
const menu_view_t  *menu_screen_view(void);

/* Pure navigation helpers (shared with settings_screen, host-tested). */
void menu_view_init(menu_view_t *v);
/* Move cursor by delta, clamp at both ends, scroll minimally to keep the
 * cursor inside a window of `page` visible rows. */
void menu_view_move(menu_view_t *v, int delta, int count, int page);
int  menu_page_size(void);   /* visible rows below the title bar            */

/* Rendering helpers (shared with settings_screen / browser_screen). */
void menu_draw(const menu_model_t *model, const menu_view_t *view);
void menu_draw_item(int y, const char *label, const char *value, bool selected);

/* ======================================================================
 * browser_screen — file browser over fs_list_dir() (names + counts only)
 * ====================================================================== */

#define BROWSER_PATH_MAX 128
#define BROWSER_WINDOW   12   /* entries fetched per fs_list_dir() call ==
                                 visible page rows                          */

typedef struct browser_state {
    char        path[BROWSER_PATH_MAX];
    fs_dirent_t window[BROWSER_WINDOW];
    uint32_t    win_start;  /* index of window[0] inside the dir listing    */
    uint32_t    win_count;  /* entries currently in window                  */
    uint32_t    total;      /* total entries in path                        */
    int         cursor;     /* absolute selected index                      */
    int         top;        /* absolute index of first visible row          */
    bool        mounted;    /* a listing was loaded at least once           */
} browser_state_t;

extern const ui_screen_t browser_screen;

/* Set the root directory the browser starts in (call before pushing). */
void browser_screen_set_root(const char *path);
const browser_state_t *browser_screen_state(void);

/* ======================================================================
 * player_screen — now-playing skeleton
 * ====================================================================== */

#define PLAYER_TITLE_MAX 64

extern const ui_screen_t player_screen;

void player_screen_set_track(const char *title, uint32_t duration_s);
void player_screen_set_position(uint32_t position_s);
void player_screen_set_volume(int volume_db);   /* -60..0 dB */
void player_screen_set_playing(bool playing);

/* Pure geometry helper (host-tested): filled width in px of a bar `width`
 * px wide at position/duration.  dur==0 or overflow-safe: returns 0. */
int player_bar_width(int width, uint32_t pos, uint32_t dur);

/* ======================================================================
 * settings_screen — the 12-key schema bound to a menu
 * ====================================================================== */

extern const ui_screen_t settings_screen;

void settings_screen_reload(void);   /* settings_load() into the live copy  */
void settings_screen_store(void);    /* settings_save() the live copy       */
const rechord_settings_t *settings_screen_get(void);
const menu_model_t       *settings_screen_model(void);
const menu_view_t        *settings_screen_view(void);

#endif /* RECHORD_UI_SCREENS_H */
