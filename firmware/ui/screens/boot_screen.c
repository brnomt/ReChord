/*
 * boot_screen.c — ReChord boot screen: branding + status lines.
 *
 * Our own rendering (clean-room): a centered wordmark drawn in the built-in
 * 6x11 font with a 1 px double-pass for weight, a tagline, a version line,
 * and up to BOOT_STATUS_MAX_LINES status lines at 12 px pitch — the same
 * "id x 12 px row" contract the boot flow uses
 * (docs/re/route-b-minimum.md: status_line(id, color, fmt, ...)).
 *
 * Status lines are printf-style; formatting happens here so the boot flow
 * can just report progress ("storage: ok", "display: 320x170", ...).
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "../internal/hal.h"

#define BOOT_WORDMARK   "ReChord"
#define BOOT_TAGLINE    "clean-room firmware"
#define BOOT_FOOTER     "own code, own UI"

#define BOOT_WORDMARK_Y 36
#define BOOT_TAGLINE_Y  56
#define BOOT_VERSION_Y  68
#define BOOT_DIVIDER_Y  84
#define BOOT_STATUS_Y0  98
#define BOOT_FOOTER_Y   (BOOT_STATUS_Y0 + BOOT_STATUS_MAX_LINES * THEME_LINE_H + 8)

static char s_status[BOOT_STATUS_MAX_LINES][BOOT_STATUS_LEN];
static bool s_status_used[BOOT_STATUS_MAX_LINES];

void boot_screen_reset(void)
{
    int i;

    for (i = 0; i < BOOT_STATUS_MAX_LINES; i++) {
        s_status[i][0] = '\0';
        s_status_used[i] = false;
    }
    ui_invalidate_all();
}

void boot_screen_status(int line, const char *fmt, ...)
{
    va_list ap;
    int y;

    if (line < 0 || line >= BOOT_STATUS_MAX_LINES || fmt == NULL)
        return;

    va_start(ap, fmt);
    vsnprintf(s_status[line], BOOT_STATUS_LEN, fmt, ap);
    va_end(ap);
    s_status_used[line] = true;

    y = BOOT_STATUS_Y0 + line * THEME_LINE_H;
    ui_invalidate(0, y, DISPLAY_W - 1, y + THEME_LINE_H - 1);
}

/* Centered text with a 1 px weight pass (our own bolding for the built-in
 * single-weight bitmap font). */
static void draw_centered_weighted(const char *s, int y, uint16_t fg)
{
    int x = (DISPLAY_W - THEME_TEXT_W((int)strlen(s))) / 2;

    if (x < 0)
        x = 0;
    display_draw_text(x + 1, y, s, fg, THEME_COLOR_BG);
    display_draw_text(x, y, s, fg, THEME_COLOR_BG);
}

static void draw_centered(const char *s, int y, uint16_t fg)
{
    int x = (DISPLAY_W - THEME_TEXT_W((int)strlen(s))) / 2;

    if (x < 0)
        x = 0;
    display_draw_text(x, y, s, fg, THEME_COLOR_BG);
}

static void boot_screen_draw(void)
{
    int i;

    display_clear(THEME_COLOR_BG);

    draw_centered_weighted(BOOT_WORDMARK, BOOT_WORDMARK_Y, THEME_COLOR_ACCENT);
    draw_centered(BOOT_TAGLINE, BOOT_TAGLINE_Y, THEME_COLOR_DIM);
    draw_centered("ReChord UI " RECHORD_UI_VERSION, BOOT_VERSION_Y,
                  THEME_COLOR_DIM);

    display_fill_rect(THEME_PAD, BOOT_DIVIDER_Y,
                      DISPLAY_W - 1 - THEME_PAD, BOOT_DIVIDER_Y,
                      THEME_COLOR_DIM);

    for (i = 0; i < BOOT_STATUS_MAX_LINES; i++) {
        if (!s_status_used[i])
            continue;
        display_draw_text(THEME_PAD,
                          BOOT_STATUS_Y0 + i * THEME_LINE_H + THEME_TEXT_YOFF,
                          s_status[i], THEME_COLOR_FG, THEME_COLOR_BG);
    }

    draw_centered(BOOT_FOOTER, BOOT_FOOTER_Y, THEME_COLOR_DIM);
}

const ui_screen_t boot_screen = {
    "boot",
    NULL,
    NULL,
    boot_screen_draw,
    NULL,   /* boot screen is passive; the boot flow drives it */
};
