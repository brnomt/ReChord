/*
 * browser_screen.c — file browser skeleton over the abstract fs_list_dir()
 * service (names + counts only — no sizes, no dates).
 *
 * Pagination model (host-tested in tests/test_browser_pagination.c):
 *   - the listing is a flat, stable-ordered sequence of `total` entries;
 *   - the visible page is [top, top + page) with page == BROWSER_WINDOW;
 *   - the fetch window always starts at `top`: whenever the view scrolls,
 *     fs_list_dir(path, top, window, BROWSER_WINDOW, ...) refetches;
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
#include "../theme.h"
#include "../internal/hal.h"

static browser_state_t s_state;
static char            s_root[BROWSER_PATH_MAX];

/* ---- listing / window -------------------------------------------------- */

static int browser_page(void)
{
    return BROWSER_WINDOW;   /* one fetch == one visible page */
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
                     (uint32_t)BROWSER_WINDOW, &got, &total);
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

static void browser_load(const char *path)
{
    browser_path_copy(s_state.path, path, BROWSER_PATH_MAX);
    s_state.cursor = 0;
    s_state.top = 0;
    s_state.win_start = 0;
    s_state.win_count = 0;
    s_state.mounted = false;
    browser_ensure_visible();
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
    browser_load(next);
    return true;
}

/* ---- screen vtable ----------------------------------------------------- */

static void browser_screen_on_enter(void)
{
    if (s_state.path[0] == '\0')
        browser_path_copy(s_state.path, "/", BROWSER_PATH_MAX);
    s_state.mounted = false;
    s_state.win_start = 0;
    s_state.top = 0;
    browser_ensure_visible();
}

static void browser_screen_draw(void)
{
    int page = browser_page();
    int row;
    char footer[32];

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
    switch (ev->type) {
    case KEY_UP:
        if (s_state.cursor > 0) {
            s_state.cursor--;
            browser_ensure_visible();
            ui_invalidate_all();
        }
        break;
    case KEY_DOWN:
        if (s_state.cursor < (int)s_state.total - 1) {
            s_state.cursor++;
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
            /* placeholder: the player module will own file opening */
            log_printf("browser: open '%s/%s'", s_state.path, ent->name);
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
}

const browser_state_t *browser_screen_state(void)
{
    return &s_state;
}
