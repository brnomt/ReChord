/*
 * player_screen.c — now-playing skeleton: track title, progress bar, volume.
 *
 * Pure presentation + local state: the audio side (B core / player module)
 * feeds it through the setters; keys adjust the local view (volume up/down,
 * play/pause toggle) which the player module will mirror to the audio
 * service later.
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "../internal/hal.h"

#define PLAYER_BAR_X0   THEME_PAD
#define PLAYER_BAR_X1   (DISPLAY_W - 1 - THEME_PAD)
#define PLAYER_BAR_W    (PLAYER_BAR_X1 - PLAYER_BAR_X0 + 1)
#define PLAYER_BAR_Y    78
#define PLAYER_BAR_H    10

#define PLAYER_VOL_Y    122
#define PLAYER_VOL_H    6

static char     s_title[PLAYER_TITLE_MAX];
static uint32_t s_duration_s;
static uint32_t s_position_s;
static int      s_volume_db;   /* -60..0 */
static bool     s_playing;

/* ---- pure geometry helper (host-tested) -------------------------------- */

int player_bar_width(int width, uint32_t pos, uint32_t dur)
{
    if (width <= 0 || dur == 0 || pos == 0)
        return 0;
    if (pos >= dur)
        return width;
    return (int)(((uint64_t)width * pos) / dur);
}

/* ---- local helpers ------------------------------------------------------ */

static void format_time(char *buf, int cap, uint32_t sec)
{
    snprintf(buf, (size_t)cap, "%u:%02u",
             (unsigned)(sec / 60u), (unsigned)(sec % 60u));
}

static void draw_bar(int y, int h, int fill_w)
{
    display_fill_rect(PLAYER_BAR_X0, y, PLAYER_BAR_X1, y + h - 1,
                      THEME_COLOR_BAR_BG);
    if (fill_w > 0) {
        if (fill_w > PLAYER_BAR_W)
            fill_w = PLAYER_BAR_W;
        display_fill_rect(PLAYER_BAR_X0, y,
                          PLAYER_BAR_X0 + fill_w - 1, y + h - 1,
                          THEME_COLOR_BAR_FG);
    }
}

/* ---- screen vtable ------------------------------------------------------ */

static void player_screen_draw(void)
{
    char line[PLAYER_TITLE_MAX + 16];
    char t_now[16];
    char t_end[16];
    int  vol_w;

    display_clear(THEME_COLOR_BG);

    /* title bar */
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_TITLE_BG);
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                      "Now playing", THEME_COLOR_ACCENT, THEME_COLOR_TITLE_BG);

    /* track title (truncated to fit one line) */
    snprintf(line, sizeof(line), "%s", s_title);
    display_draw_text(THEME_PAD, 32 + THEME_TEXT_YOFF, line,
                      THEME_COLOR_FG, THEME_COLOR_BG);

    /* play state */
    display_draw_text(THEME_PAD, 48 + THEME_TEXT_YOFF,
                      s_playing ? "Playing" : "Paused",
                      s_playing ? THEME_COLOR_WARM : THEME_COLOR_DIM,
                      THEME_COLOR_BG);

    /* progress bar + times */
    draw_bar(PLAYER_BAR_Y, PLAYER_BAR_H,
             player_bar_width(PLAYER_BAR_W, s_position_s, s_duration_s));
    format_time(t_now, (int)sizeof(t_now), s_position_s);
    format_time(t_end, (int)sizeof(t_end), s_duration_s);
    snprintf(line, sizeof(line), "%s / %s", t_now, t_end);
    display_draw_text(THEME_PAD, PLAYER_BAR_Y + PLAYER_BAR_H + 4,
                      line, THEME_COLOR_DIM, THEME_COLOR_BG);

    /* volume bar (-60..0 dB maps to 0..full width) */
    display_draw_text(THEME_PAD, PLAYER_VOL_Y - THEME_LINE_H + 2,
                      "Volume", THEME_COLOR_DIM, THEME_COLOR_BG);
    vol_w = (s_volume_db + 60) * PLAYER_BAR_W / 60;
    if (vol_w < 0)
        vol_w = 0;
    draw_bar(PLAYER_VOL_Y, PLAYER_VOL_H, vol_w);
    snprintf(line, sizeof(line), "%d dB", s_volume_db);
    display_draw_text(DISPLAY_W - THEME_PAD - THEME_TEXT_W((int)strlen(line)),
                      PLAYER_VOL_Y - THEME_LINE_H + 2,
                      line, THEME_COLOR_FG, THEME_COLOR_BG);
}

static void player_screen_handle_event(const input_event_t *ev)
{
    switch (ev->type) {
    case KEY_UP:
        if (s_volume_db < 0) {
            s_volume_db++;
            ui_invalidate(0, PLAYER_VOL_Y - THEME_LINE_H,
                          DISPLAY_W - 1, PLAYER_VOL_Y + PLAYER_VOL_H);
        }
        break;
    case KEY_DOWN:
        if (s_volume_db > -60) {
            s_volume_db--;
            ui_invalidate(0, PLAYER_VOL_Y - THEME_LINE_H,
                          DISPLAY_W - 1, PLAYER_VOL_Y + PLAYER_VOL_H);
        }
        break;
    case KEY_SELECT:
        s_playing = !s_playing;
        ui_invalidate(0, 48, DISPLAY_W - 1, 48 + THEME_LINE_H);
        break;
    case KEY_BACK:
        ui_pop_screen();
        break;
    default:
        break;
    }
}

const ui_screen_t player_screen = {
    "player",
    NULL,
    NULL,
    player_screen_draw,
    player_screen_handle_event,
};

void player_screen_set_track(const char *title, uint32_t duration_s)
{
    snprintf(s_title, sizeof(s_title), "%s", title ? title : "");
    s_duration_s = duration_s;
    s_position_s = 0;
    ui_invalidate_all();
}

void player_screen_set_position(uint32_t position_s)
{
    s_position_s = position_s;
    ui_invalidate(0, PLAYER_BAR_Y, DISPLAY_W - 1,
                  PLAYER_BAR_Y + PLAYER_BAR_H + THEME_LINE_H);
}

void player_screen_set_volume(int volume_db)
{
    if (volume_db < -60)
        volume_db = -60;
    if (volume_db > 0)
        volume_db = 0;
    s_volume_db = volume_db;
    ui_invalidate(0, PLAYER_VOL_Y - THEME_LINE_H,
                  DISPLAY_W - 1, PLAYER_VOL_Y + PLAYER_VOL_H);
}

void player_screen_set_playing(bool playing)
{
    s_playing = playing;
    ui_invalidate(0, 48, DISPLAY_W - 1, 48 + THEME_LINE_H);
}
