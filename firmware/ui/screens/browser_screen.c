/*
 * browser_screen.c — file browser skeleton over the abstract fs_list_dir()
 * service (names + counts only — no sizes, no dates).
 *
 * Pagination model (host-tested in tests/test_browser_pagination.c):
 *   - the listing is a flat, stable-ordered sequence of `total` entries;
 *   - the visible page is [top, top + page), page = browser_page_rows();
 *   - the fetch window always starts at `top` and is exactly that page:
 *     whenever the view scrolls, fs_list_dir(path, top, window, page, ...)
 *     refetches (never during draw);
 *   - cursor movement clamps at both ends of the listing.
 *
 * Navigation:
 *   KEY_UP/KEY_DOWN  move selection (scrolls the page),
 *   KEY_SELECT       enter a directory / open a file (placeholder hook),
 *   KEY_BACK         parent directory (clamped at the configured root);
 *                    pops the screen stack when already at the root.
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "theme_ui.h"
#include "../theme.h"
#include "../internal/hal.h"
#include "../../services/media_db.h"       /* BRNOMT-90: folder -> list */
#include "../../services/media_query.h"

static browser_state_t s_state;
static char            s_root[BROWSER_PATH_MAX];
static int             s_use_skin;

static const char *browser_label(int index, char *buf, int cap)
{
    const fs_dirent_t *ent;
    uint32_t widx;

    if (index < 0 || index >= (int)s_state.total)
        return "";
    widx = (uint32_t)index - s_state.win_start;
    if (widx >= s_state.win_count)
        return "";
    ent = &s_state.window[widx];
    if (ent->is_dir)
        snprintf(buf, (size_t)cap, "%s/", ent->name);
    else
        snprintf(buf, (size_t)cap, "%s", ent->name);
    return buf;
}

static theme_skin_list_source_t s_source = { 0, browser_label, 0 };

/* One saved cursor/scroll per descent level, so BACK from a folder returns to
 * the folder you came from instead of the top (BRNOMT-93). Depth is small
 * (the scanner caps at 8); 10 leaves headroom. */
#define BROWSER_NAV_DEPTH 10
static struct { int cursor, top; } s_nav[BROWSER_NAV_DEPTH];
static int             s_nav_depth;

/* ---- listing / window -------------------------------------------------- */

/* Footer-safe page: the i/N row is drawn at DISPLAY_H - item_h, so the list
 * must stop short of it. The fetch window is exactly this page (BRNOMT-72). */
int browser_page_rows(void)
{
    int rows = (DISPLAY_H - THEME_ITEM_H - THEME_MENU_TOP) / THEME_ITEM_H;

    if (rows > BROWSER_WINDOW)
        rows = BROWSER_WINDOW;
    if (rows < 1)
        rows = 1;
    return rows;
}

static int browser_page(void)
{
    if (s_use_skin) {
        int rows = theme_ui_rows_hint();

        if (rows > 0) {
            if (rows > BROWSER_WINDOW)
                rows = BROWSER_WINDOW;
            return rows;
        }
    }
    return browser_page_rows();
}

static void browser_clamp_view(void)
{
    if (s_state.mounted) {
        if (s_state.cursor > (int)s_state.total - 1)
            s_state.cursor = (int)s_state.total - 1;
        if (s_state.cursor < 0)
            s_state.cursor = 0;
    }
    if (s_state.top > s_state.cursor)
        s_state.top = s_state.cursor;
    if (s_state.top < 0)
        s_state.top = 0;
}

static void browser_fetch_window(void)
{
    uint32_t got = 0;
    uint32_t total = 0;
    int rc;

    rc = fs_list_dir(s_state.path, s_state.win_start, s_state.window,
                     (uint32_t)browser_page(), &got, &total);
    if (rc != 0) {
        log_printf("browser: list '%s' failed (%d)", s_state.path, rc);
        s_state.win_count = 0;
        s_state.total = 0;
        s_state.mounted = true;
        return;
    }
    s_state.win_count = got;
    s_state.total = total;
    s_state.mounted = true;
    browser_clamp_view();
}

/* Keep cursor inside [top, top + page) and refetch when the page moves. */
static void browser_ensure_visible(void)
{
    uint32_t old_start = s_state.win_start;

    if (s_state.cursor < s_state.top)
        s_state.top = s_state.cursor;
    if (s_state.cursor > s_state.top + browser_page() - 1)
        s_state.top = s_state.cursor - browser_page() + 1;
    if (s_state.top < 0)
        s_state.top = 0;

    s_state.win_start = (uint32_t)s_state.top;
    if (s_state.win_start != old_start || !s_state.mounted)
        browser_fetch_window();
}

/* ---- path handling ----------------------------------------------------- */

static void browser_path_copy(char *dst, const char *src, int cap)
{
    snprintf(dst, (size_t)cap, "%s", src);
}

/* Compare paths ignoring trailing slashes ("/music/" == "/music"). */
static bool browser_same_path(const char *a, const char *b)
{
    size_t la = strlen(a);
    size_t lb = strlen(b);

    while (la > 1 && a[la - 1] == '/')
        la--;
    while (lb > 1 && b[lb - 1] == '/')
        lb--;
    return la == lb && strncmp(a, b, la) == 0;
}

/* Load `path` with the given cursor/scroll (BRNOMT-93 state restoration). */
static void browser_load_at(const char *path, int cursor, int top)
{
    browser_path_copy(s_state.path, path, BROWSER_PATH_MAX);
    s_state.cursor = cursor;
    s_state.top = top;
    s_state.win_start = (uint32_t)(top > 0 ? top : 0);
    s_state.win_count = 0;
    s_state.mounted = false;
    browser_ensure_visible();
}

/* A NEW directory starts at the top. */
static void browser_load(const char *path)
{
    browser_load_at(path, 0, 0);
}

static void browser_enter_dir(const char *name)
{
    char next[BROWSER_PATH_MAX];
    size_t len = strlen(s_state.path);
    size_t nlen = strlen(name);

    /* "<path>/<name>", bounded construction (no truncation possible) */
    if (len + nlen + 2 > BROWSER_PATH_MAX) {
        log_printf("browser: path too long for '%s'", name);
        return;
    }
    memcpy(next, s_state.path, len);
    if (len == 0 || next[len - 1] != '/')
        next[len++] = '/';
    memcpy(next + len, name, nlen);
    next[len + nlen] = '\0';

    /* Remember this level's cursor so BACK returns to the folder (BRNOMT-93). */
    if (s_nav_depth < BROWSER_NAV_DEPTH) {
        s_nav[s_nav_depth].cursor = s_state.cursor;
        s_nav[s_nav_depth].top = s_state.top;
        s_nav_depth++;
    }
    browser_load(next);
}

/* Move to the parent directory, clamped at the configured root.
 * Returns false when already at the root (caller decides to pop). */
static bool browser_to_parent(void)
{
    char next[BROWSER_PATH_MAX];
    char *slash;

    if (browser_same_path(s_state.path, s_root))
        return false;

    browser_path_copy(next, s_state.path, BROWSER_PATH_MAX);
    slash = strrchr(next, '/');
    if (slash == NULL)
        return false;
    if (slash == next)
        next[1] = '\0';   /* keep the leading "/" */
    else
        *slash = '\0';

    /* never navigate above the configured root */
    if (strlen(next) < strlen(s_root) &&
        strncmp(next, s_root, strlen(next)) == 0) {
        browser_path_copy(next, s_root, BROWSER_PATH_MAX);
    }
    /* Restore the parent level's cursor/scroll (BRNOMT-93). */
    if (s_nav_depth > 0)
        s_nav_depth--;
    if (s_nav_depth < BROWSER_NAV_DEPTH) {
        browser_load_at(next, s_nav[s_nav_depth].cursor,
                        s_nav[s_nav_depth].top);
    } else {
        browser_load(next);
    }
    return true;
}

/* ---- screen vtable ----------------------------------------------------- */

static void browser_screen_on_enter(void)
{
    if (s_state.path[0] == '\0')
        browser_path_copy(s_state.path, "/", BROWSER_PATH_MAX);
    s_use_skin = theme_ui_load(THEME_UI_BROWSER);
    if (s_use_skin) {
        int rows = theme_ui_rows_hint();

        if (rows <= 0 || rows > BROWSER_WINDOW) {
            s_use_skin = 0;           /* skin window does not fit this screen */
            theme_ui_disarm();
        }
    }
    /* Preserve cursor/scroll when a child screen pops back (BRNOMT-93); force
     * a refetch so an inserted/removed card is re-read. */
    s_state.mounted = false;
    browser_ensure_visible();
    s_source.count = (int)s_state.total;
    if (s_use_skin)
        theme_ui_list(&s_source);
}

static void browser_screen_draw(void)
{
    int page = browser_page();
    int row;
    char footer[32];

    if (s_use_skin) {
        theme_skin_state_t st;

        s_source.count = (int)s_state.total;
        theme_ui_list(&s_source);
        theme_ui_fill_state(&st, s_state.path);
        theme_ui_draw(&st, s_state.cursor, s_state.top);
        return;
    }

    display_clear(THEME_COLOR_BG);

    /* title bar: current path */
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_TITLE_BG);
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                      s_state.path, THEME_COLOR_ACCENT, THEME_COLOR_TITLE_BG);

    /* visible rows from the fetch window */
    for (row = 0; row < page; row++) {
        int abs_idx = s_state.top + row;
        uint32_t widx = (uint32_t)abs_idx - s_state.win_start;
        int y = THEME_MENU_TOP + row * THEME_ITEM_H;
        const fs_dirent_t *ent;
        char label[FS_NAME_MAX + 2];
        bool selected;

        if (abs_idx >= (int)s_state.total || widx >= s_state.win_count)
            break;
        ent = &s_state.window[widx];
        selected = abs_idx == s_state.cursor;

        if (ent->is_dir) {
            snprintf(label, sizeof(label), "%s/", ent->name);
            menu_draw_item(y, label, NULL, selected);
        } else {
            menu_draw_item(y, ent->name, NULL, selected);
        }
    }

    /* footer: names + counts only */
    snprintf(footer, sizeof(footer), "%u / %u",
             (unsigned)(s_state.total ? s_state.cursor + 1 : 0),
             (unsigned)s_state.total);
    menu_draw_item(DISPLAY_H - THEME_ITEM_H, footer, NULL, false);
}

static void browser_screen_handle_event(const input_event_t *ev)
{
    int key = ev->type;

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;                       /* listings act on PRESS/REPEAT only */

    if (s_use_skin) {
        int nav;

        theme_ui_note_input();
        s_source.count = (int)s_state.total;
        theme_ui_list(&s_source);
        nav = theme_ui_nav(ev->type);
        if (nav == THEME_SKIN_PREV)
            key = KEY_UP;
        else if (nav == THEME_SKIN_NEXT)
            key = KEY_DOWN;
        else if (nav == THEME_SKIN_SELECT)
            key = KEY_SELECT;
        else if (nav == THEME_SKIN_BACK)
            key = KEY_BACK;
        else
            return;
    }

    switch (key) {
    case KEY_UP:
        if (s_state.total > 0) {
            /* cyclic wrap-around (BRNOMT-93) */
            s_state.cursor = (s_state.cursor <= 0)
                             ? (int)s_state.total - 1
                             : s_state.cursor - 1;
            browser_ensure_visible();
            ui_invalidate_all();
        }
        break;
    case KEY_DOWN:
        if (s_state.total > 0) {
            s_state.cursor = (s_state.cursor >= (int)s_state.total - 1)
                             ? 0
                             : s_state.cursor + 1;
            browser_ensure_visible();
            ui_invalidate_all();
        }
        break;
    case KEY_SELECT: {
        uint32_t widx = (uint32_t)s_state.cursor - s_state.win_start;
        const fs_dirent_t *ent;

        if (widx >= s_state.win_count)
            break;
        ent = &s_state.window[widx];
        if (ent->is_dir) {
            browser_enter_dir(ent->name);
            ui_invalidate_all();
        } else {
            /* Open with the player module and show Now Playing.
             * Full path = "<dir>/<name>" (same bounded build as enter_dir). */
            char full[BROWSER_PATH_MAX + 2];
            size_t len = strlen(s_state.path);
            size_t nlen = strlen(ent->name);
            int rc;

            if (len + nlen + 2 > sizeof(full)) {
                log_printf("browser: path too long for '%s'", ent->name);
                break;
            }
            memcpy(full, s_state.path, len);
            if (len == 0 || full[len - 1] != '/')
                full[len++] = '/';
            memcpy(full + len, ent->name, nlen + 1);

            {
                unsigned n = 0;
                const uint16_t *list = 0;
                int start = -1;

                /* BRNOMT-90/98: queue the whole folder and resolve the start
                 * position during that same scan (no second pass over the
                 * index, which used to be one media_db_get per track). */
                if (mq_folder_init_start(s_state.path, full, &start) == 0)
                    list = mq_filter_indices(&n);
                if (start >= 0 && list != 0) {
                    rc = player_play_list(list, (int)n, start);
                } else {
                    rc = player_open(full);     /* not indexed: single track */
                    if (rc == PLAYER_OK)
                        rc = player_play();
                }
            }
            if (rc == PLAYER_OK) {
                track_info_t ti;

                player_screen_set_status(NULL);
                if (player_get_track(&ti) == PLAYER_OK)
                    player_screen_set_track(
                        ti.title,
                        ti.duration_ms > 0
                            ? (uint32_t)(ti.duration_ms / 1000) : 0);
            } else {
                /* Show the raw error code (-1 io, -2 format, -3 out of the
                 * codec subset e.g. FLAC block/seektable pool, -4 no codec):
                 * "cannot open file" hides three distinct causes. */
                char eb[28];
                extern unsigned flac_diag_block, flac_diag_channels;
                extern unsigned long flac_diag_need;
                extern int flac_diag_reason;
                if (rc == -3 && flac_diag_reason == 2)
                    snprintf(eb, sizeof eb, "flac pool %lu", flac_diag_need);
                else if (rc == -3 && flac_diag_reason)
                    snprintf(eb, sizeof eb, "flac %d ch%u b%u", flac_diag_reason,
                             flac_diag_channels, flac_diag_block);
                else
                    snprintf(eb, sizeof eb, "cannot open (%d)", rc);
                player_screen_set_track(ent->name, 0);
                player_screen_set_status(
                    rc == PLAYER_ERR_UNSUPPORTED ? eb : eb);
                log_printf("browser: open '%s' failed (%d) rsn=%d blk=%u ch=%u st=%lu",
                           full, rc, flac_diag_reason, flac_diag_block,
                           flac_diag_channels, flac_diag_need);
            }
            ui_push_screen(&player_screen);
        }
        break;
    }
    case KEY_BACK:
        if (browser_to_parent()) {
            ui_invalidate_all();
        } else {
            /* at the root: leave the browser (the app layer pushed it) */
            ui_pop_screen();
        }
        break;
    default:
        break;
    }
}

const ui_screen_t browser_screen = {
    "browser",
    browser_screen_on_enter,
    NULL,
    browser_screen_draw,
    browser_screen_handle_event,
};

void browser_screen_set_root(const char *path)
{
    if (path == NULL || path[0] == '\0')
        path = "/";
    browser_path_copy(s_root, path, BROWSER_PATH_MAX);
    browser_path_copy(s_state.path, path, BROWSER_PATH_MAX);
    s_state.cursor = 0;
    s_state.top = 0;
    s_state.win_start = 0;
    s_state.win_count = 0;
    s_state.total = 0;
    s_state.mounted = false;
    s_nav_depth = 0;
}

const browser_state_t *browser_screen_state(void)
{
    return &s_state;
}

/* Open the browser at `path` with the cursor on `row` (BRNOMT-87 resume).
 * The BACK clamp (s_root) is deliberately left alone, so navigation keeps
 * walking up the real tree; on_enter refetches the listing. */
void browser_screen_open_at(const char *path, int row)
{
    if (path == NULL || path[0] != '/')
        return;
    if (row < 0)
        row = 0;
    browser_load_at(path, row, row > 3 ? row - 3 : 0);
}
