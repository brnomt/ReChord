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

/* Row positions (12 px pitch): title / artist+album / state, then the bar. */
#define PLAYER_TITLE_Y  32
#define PLAYER_META_Y   44
#define PLAYER_STATE_Y  56

static char     s_title[PLAYER_TITLE_MAX];
static char     s_artist[PLAYER_TITLE_MAX];
static char     s_album[PLAYER_TITLE_MAX];
static uint32_t s_duration_s;
static uint32_t s_position_s;
static int      s_volume_db;   /* -60..0 */
static bool     s_playing;
static char     s_status[40];  /* non-empty: shown instead of Playing/Paused */

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
    char meta_line[2 * PLAYER_TITLE_MAX + 8];
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
    display_draw_text(THEME_PAD, PLAYER_TITLE_Y + THEME_TEXT_YOFF, line,
                      THEME_COLOR_FG, THEME_COLOR_BG);

    /* artist / album line (tags; absent for untagged files) */
    if (s_artist[0] || s_album[0]) {
        if (s_artist[0] && s_album[0])
            snprintf(meta_line, sizeof(meta_line), "%s - %s", s_artist,
                     s_album);
        else
            snprintf(meta_line, sizeof(meta_line), "%s",
                     s_artist[0] ? s_artist : s_album);
        display_draw_text(THEME_PAD, PLAYER_META_Y + THEME_TEXT_YOFF,
                          meta_line, THEME_COLOR_DIM, THEME_COLOR_BG);
    }

    /* play state (or a status line: unsupported file, no renderer, ...) */
    {
        const char *state_line = s_status[0] ? s_status
                                             : (s_playing ? "Playing" : "Paused");
        display_draw_text(THEME_PAD, PLAYER_STATE_Y + THEME_TEXT_YOFF, state_line,
                          s_status[0] ? THEME_COLOR_WARM
                                      : (s_playing ? THEME_COLOR_WARM
                                                   : THEME_COLOR_DIM),
                          THEME_COLOR_BG);
    }

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
    case KEY_DOWN: {
        /* volume through the player module (it clamps -60..+20 dB) */
        int db = player_get_volume() + (ev->type == KEY_UP ? 1 : -1);
        player_set_volume(db);
        player_screen_set_volume(player_get_volume());
        break;
    }
    case KEY_SELECT:
        if (s_status[0])
            break;                        /* error view: inert (BACK leaves) */
        if (s_playing) {
            player_pause();
            player_screen_set_playing(player_get_state() == PLAYER_PLAYING);
        } else {
            player_play();
            player_screen_set_playing(player_get_state() == PLAYER_PLAYING);
        }
        break;
    case KEY_BACK:
        /* playback continues in the background; re-entering re-syncs. */
        ui_pop_screen();
        break;
    default:
        break;
    }
}

/* Sync the screen from the player module when the screen is (re)entered. */
static void player_screen_on_enter(void)
{
    track_info_t ti;

    if (s_status[0])
        return;                       /* error view: keep what's shown */
    if (player_get_track(&ti) != PLAYER_OK) {
        player_screen_set_status("nothing playing");
        return;
    }
    player_screen_set_track(ti.title, ti.artist, ti.album,
                            ti.duration_ms > 0
                                ? (uint32_t)(ti.duration_ms / 1000) : 0);
    player_screen_set_volume(player_get_volume());
    player_screen_set_playing(player_get_state() == PLAYER_PLAYING);
}

const ui_screen_t player_screen = {
    "player",
    player_screen_on_enter,
    NULL,
    player_screen_draw,
    player_screen_handle_event,
};

void player_screen_set_track(const char *title, const char *artist,
                             const char *album, uint32_t duration_s)
{
    snprintf(s_title, sizeof(s_title), "%s", title ? title : "");
    snprintf(s_artist, sizeof(s_artist), "%s", artist ? artist : "");
    snprintf(s_album, sizeof(s_album), "%s", album ? album : "");
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
    ui_invalidate(0, PLAYER_STATE_Y, DISPLAY_W - 1,
                  PLAYER_STATE_Y + THEME_LINE_H);
}

void player_screen_set_status(const char *msg)
{
    snprintf(s_status, sizeof(s_status), "%s", msg ? msg : "");
    ui_invalidate(0, PLAYER_STATE_Y, DISPLAY_W - 1,
                  PLAYER_STATE_Y + THEME_LINE_H);
}

/* Pulled from the app loop every pass: updates position + play state from
 * the player module (no-ops without a track; redraws only on change). */
void player_screen_refresh(void)
{
    track_info_t ti;
    uint32_t pos;
    int st;

    if (player_get_track(&ti) != PLAYER_OK) {
        if (!s_status[0])
            player_screen_set_status("nothing playing");
        return;
    }
    st = player_get_state();
    if (st == PLAYER_ERROR) {
        if (!s_status[0])
            player_screen_set_status("no audio renderer");
        if (s_playing)
            player_screen_set_playing(false);
        return;
    }
    pos = (uint32_t)(player_get_position_ms() / 1000);
    if (pos != s_position_s)
        player_screen_set_position(pos);
    if ((st == PLAYER_PLAYING) != s_playing)
        player_screen_set_playing(st == PLAYER_PLAYING);
}
