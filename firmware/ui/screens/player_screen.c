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
#include "playlist.h"      /* PLAYLIST_REPEAT_* (mode indicator) */
#include "../theme.h"
#include "../internal/hal.h"

#define PLAYER_BAR_X0   (theme_layout()->pad)
#define PLAYER_BAR_X1   (DISPLAY_W - 1 - theme_layout()->pad)
#define PLAYER_BAR_W    (PLAYER_BAR_X1 - PLAYER_BAR_X0 + 1)

/* Layout (BRNOMT-72/69): every origin comes from the active theme package,
 * with built-in defaults equal to what this screen used before. */
#define PLAYER_TITLE_Y   (theme_layout()->np_title_y)
#define PLAYER_ARTIST_Y  (theme_layout()->np_artist_y)
#define PLAYER_STATE_Y   (theme_layout()->np_state_y)
#define PLAYER_INFO_Y    (theme_layout()->np_info_y)
#define PLAYER_BAR_Y     (theme_layout()->np_bar_y)
#define PLAYER_BAR_H     (theme_layout()->np_bar_h)
#define PLAYER_TIME_Y    (theme_layout()->np_time_y)
#define PLAYER_MODE_Y    (theme_layout()->np_mode_y)
#define PLAYER_VOL_Y     (theme_layout()->np_vol_y)
#define PLAYER_VOL_H     (theme_layout()->np_vol_h)

static char     s_title[PLAYER_TITLE_MAX];
static char     s_artist[PLAYER_TITLE_MAX];
static uint32_t s_duration_s;
static uint32_t s_position_s;
static int      s_volume_db;   /* -60..0 */
static int      s_bits;        /* source bit depth (0 = unknown) */
static long     s_rate;        /* sample rate Hz (0 = unknown) */
static int      s_pl_pos;      /* playlist position (0 = no list) */
static int      s_pl_count;    /* playlist length */
static int      s_repeat;      /* PLAYLIST_REPEAT_* (shown on the state row) */
static int      s_shuffle;     /* 1 = shuffle on */
static int      s_track_no = -1; /* player open-sequence: detects a new track */
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
    /* Mono-safe trough: a surface-colored trough vanishes on black, so the bar
     * sits on a 1 px dim baseline and only the filled part is solid. */
    display_fill_rect(PLAYER_BAR_X0, y + h, PLAYER_BAR_X1, y + h,
                      THEME_COLOR_DIM);
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
    /* Same 1 px header rule as every list screen (BRNOMT-72). */
    display_fill_rect(0, THEME_TITLE_H - 1, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_FG);

    /* title, artist beneath */
    snprintf(line, sizeof(line), "%s", s_title);
    display_draw_text(THEME_PAD, PLAYER_TITLE_Y + THEME_TEXT_YOFF, line,
                      THEME_COLOR_FG, THEME_COLOR_BG);
    if (s_artist[0] != '\0')
        display_draw_text(THEME_PAD, PLAYER_ARTIST_Y + THEME_TEXT_YOFF,
                          s_artist, THEME_COLOR_DIM, THEME_COLOR_BG);

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

    /* stream format (left) + playlist position (right) */
    if (s_bits > 0 && s_rate > 0) {
        char f[40];
        long k10 = s_rate / 100;      /* tenths of kHz, no float printf */

        snprintf(f, sizeof(f), "%d/%ld.%ldkHz", s_bits, k10 / 10, k10 % 10);
        display_draw_text(THEME_PAD, PLAYER_INFO_Y + THEME_TEXT_YOFF,
                          f, THEME_COLOR_DIM, THEME_COLOR_BG);
    }
    if (s_pl_count > 0) {
        char pl[24];

        snprintf(pl, sizeof(pl), "%d/%d", s_pl_pos, s_pl_count);
        display_draw_text(DISPLAY_W - THEME_PAD - THEME_TEXT_W((int)strlen(pl)),
                          PLAYER_INFO_Y + THEME_TEXT_YOFF,
                          pl, THEME_COLOR_DIM, THEME_COLOR_BG);
    }

    /* progress bar + elapsed / total */
    draw_bar(PLAYER_BAR_Y, PLAYER_BAR_H,
             player_bar_width(PLAYER_BAR_W, s_position_s, s_duration_s));
    format_time(t_now, (int)sizeof(t_now), s_position_s);
    format_time(t_end, (int)sizeof(t_end), s_duration_s);
    snprintf(line, sizeof(line), "%s / %s", t_now, t_end);
    display_draw_text(THEME_PAD, PLAYER_TIME_Y + THEME_TEXT_YOFF, line,
                      THEME_COLOR_DIM, THEME_COLOR_BG);

    /* bottom: only the ACTIVE playback modes (SHUF / REP1), no clutter */
    {
        int x = THEME_PAD;

        if (s_shuffle) {
            display_draw_text(x, PLAYER_MODE_Y + THEME_TEXT_YOFF, "SHUF",
                              THEME_COLOR_FG, THEME_COLOR_BG);
            x += THEME_TEXT_W(4) + 8;
        }
        if (s_repeat == PLAYLIST_REPEAT_ONE)
            display_draw_text(x, PLAYER_MODE_Y + THEME_TEXT_YOFF, "REP1",
                              THEME_COLOR_FG, THEME_COLOR_BG);
    }

    /* volume: value on that row, slim bar at the very bottom (no label) */
    snprintf(line, sizeof(line), "%d dB", s_volume_db);
    display_draw_text(DISPLAY_W - THEME_PAD - THEME_TEXT_W((int)strlen(line)),
                      PLAYER_MODE_Y + THEME_TEXT_YOFF,
                      line, THEME_COLOR_FG, THEME_COLOR_BG);
    vol_w = (s_volume_db + 60) * PLAYER_BAR_W / 60;
    if (vol_w < 0)
        vol_w = 0;
    draw_bar(PLAYER_VOL_Y, PLAYER_VOL_H, vol_w);
}

/* Now Playing keys (BRNOMT-93):
 *   VOL+/VOL- short  -> one volume step each. The step happens on RELEASE so a
 *                       hold does not also bump the volume before the long
 *                       action.
 *   VOL+ long        -> next track in the current playlist/queue.
 *   VOL- long        -> seek back to 0:00; if still inside the first 3 s,
 *                       previous track instead.
 *   SELECT           -> play/pause.  BACK -> leave (playback continues).
 * A held key also sends REPEATs; on this screen they are not a volume sweep. */
static void player_screen_handle_event(const input_event_t *ev)
{
    if (ev->kind == INPUT_REPEAT)
        return;

    switch (ev->type) {
    case KEY_UP:
        if (s_status[0])
            break;
        if (ev->kind == INPUT_LONG) {
            (void)player_next();
        } else if (ev->kind == INPUT_RELEASE && !ev->long_press) {
            player_set_volume(player_get_volume() + 1);
            player_screen_set_volume(player_get_volume());
        }
        break;
    case KEY_DOWN:
        if (s_status[0])
            break;
        if (ev->kind == INPUT_LONG) {
            if (player_get_position_ms() > 3000)
                (void)player_seek(0);
            else
                (void)player_prev();
        } else if (ev->kind == INPUT_RELEASE && !ev->long_press) {
            player_set_volume(player_get_volume() - 1);
            player_screen_set_volume(player_get_volume());
        }
        break;
    case KEY_SELECT:
        if (ev->kind != INPUT_PRESS)
            break;
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
        if (ev->kind == INPUT_PRESS)
            ui_pop_screen();   /* playback continues; re-entering re-syncs */
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
    player_screen_set_track(ti.title,
                            ti.duration_ms > 0
                                ? (uint32_t)(ti.duration_ms / 1000) : 0);
    player_screen_set_artist(ti.artist);
    player_screen_set_volume(player_get_volume());
    player_screen_set_format(ti.bits, ti.sample_rate);
    player_screen_set_playlist(ti.playlist_pos, ti.playlist_count);
    player_screen_set_modes(player_get_repeat(), player_get_shuffle());
    s_track_no = ti.track_no;
    player_screen_set_playing(player_get_state() == PLAYER_PLAYING);
}

const ui_screen_t player_screen = {
    "player",
    player_screen_on_enter,
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

void player_screen_set_artist(const char *artist)
{
    snprintf(s_artist, sizeof(s_artist), "%s", artist ? artist : "");
    ui_invalidate(0, PLAYER_TITLE_Y, DISPLAY_W - 1,
                  PLAYER_ARTIST_Y + THEME_LINE_H);
}

void player_screen_set_position(uint32_t position_s)
{
    s_position_s = position_s;
    ui_invalidate(0, PLAYER_BAR_Y, DISPLAY_W - 1,
                  PLAYER_TIME_Y + THEME_LINE_H);
}

void player_screen_set_volume(int volume_db)
{
    if (volume_db < -60)
        volume_db = -60;
    if (volume_db > 0)
        volume_db = 0;
    s_volume_db = volume_db;
    ui_invalidate(0, PLAYER_MODE_Y, DISPLAY_W - 1,
                  PLAYER_VOL_Y + PLAYER_VOL_H);
}

void player_screen_set_playlist(int pos, int count)
{
    if (pos == s_pl_pos && count == s_pl_count)
        return;
    s_pl_pos = pos;
    s_pl_count = count;
    ui_invalidate(0, PLAYER_INFO_Y, DISPLAY_W - 1,
                  PLAYER_INFO_Y + THEME_LINE_H);
}

void player_screen_set_format(int bits, long sample_rate)
{
    if (bits == s_bits && sample_rate == s_rate)
        return;
    s_bits = bits;
    s_rate = sample_rate;
    ui_invalidate(0, PLAYER_INFO_Y, DISPLAY_W - 1,
                  PLAYER_INFO_Y + THEME_LINE_H);
}

void player_screen_set_playing(bool playing)
{
    s_playing = playing;
    ui_invalidate(0, PLAYER_STATE_Y, DISPLAY_W - 1,
                  PLAYER_STATE_Y + THEME_LINE_H);
}

/* Repeat / shuffle mode indicator (BRNOMT-90). Shows what the PLAYER has, so
 * a Settings change is verifiable from Now Playing itself. */
void player_screen_set_modes(int repeat, int shuffle)
{
    if (repeat == s_repeat && (shuffle ? 1 : 0) == s_shuffle)
        return;
    s_repeat = repeat;
    s_shuffle = shuffle ? 1 : 0;
    ui_invalidate(0, PLAYER_MODE_Y, DISPLAY_W - 1,
                  PLAYER_MODE_Y + THEME_LINE_H);
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
    /* A new track (auto-advance / next) must refresh the title, duration and
     * format too — the position alone updates every pass. */
    if (ti.track_no != s_track_no) {
        s_track_no = ti.track_no;
        player_screen_set_track(ti.title,
                                ti.duration_ms > 0
                                    ? (uint32_t)(ti.duration_ms / 1000) : 0);
        player_screen_set_artist(ti.artist);
    }
    pos = (uint32_t)(player_get_position_ms() / 1000);
    if (pos != s_position_s)
        player_screen_set_position(pos);
    player_screen_set_format(ti.bits, ti.sample_rate);
    player_screen_set_playlist(ti.playlist_pos, ti.playlist_count);
    player_screen_set_modes(player_get_repeat(), player_get_shuffle());
    if ((st == PLAYER_PLAYING) != s_playing)
        player_screen_set_playing(st == PLAYER_PLAYING);
}
