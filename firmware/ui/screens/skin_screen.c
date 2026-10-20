/*
 * skin_screen.c — generic UI adapter for the declarative skin engine.
 *
 * The engine (firmware/theme/theme_skin.c) parses the active package's skin
 * file and draws it. This adapter only owns the selected index, the idle
 * timer and the mapping from the engine's actions to app callbacks.
 */
#include <string.h>

#include "skin_screen.h"
#include "../theme.h"
#include "../internal/hal.h"
#include "../../theme/theme_skin.h"

static int      s_sel;
static unsigned s_now_ms;
static unsigned s_last_input_ms;
static track_info_t s_track;

static void (*s_open_lib)(void);
static void (*s_open_np)(void);
static void (*s_open_set)(void);

void skin_home_set_actions(void (*library)(void),
                           void (*player)(void),
                           void (*settings)(void))
{
    s_open_lib = library;
    s_open_np = player;
    s_open_set = settings;
}

void skin_home_tick(unsigned now_ms)
{
    s_now_ms = now_ms;
}

static void skin_on_enter(void)
{
    s_sel = theme_skin_selected_default();
    s_last_input_ms = s_now_ms;
    ui_invalidate_all();
}

static void skin_draw(void)
{
    theme_skin_state_t st;

    memset(&st, 0, sizeof st);
    st.playing = player_get_state() == PLAYER_PLAYING;
    st.paused = player_get_state() == PLAYER_PAUSED;
    st.stopped = player_get_state() == PLAYER_STOPPED;
    st.volume_db = player_get_volume();
    st.battery_pct = -1;
    st.shuffle = player_get_shuffle();
    st.repeat = player_get_repeat();
    st.position_ms = player_get_position_ms();
    st.title = "";
    st.artist = "";
    st.album = "";
    st.path = "";
    st.state = st.playing ? "Playing" : (st.paused ? "Paused" : "Stopped");
    if (player_get_track(&s_track) == PLAYER_OK) {
        st.title = s_track.title;
        st.artist = s_track.artist;
        st.album = s_track.album;
        st.path = s_track.path;
        st.duration_ms = s_track.duration_ms;
        st.bits = s_track.bits;
        st.sample_rate = s_track.sample_rate;
        st.playlist_pos = s_track.playlist_pos;
        st.playlist_count = s_track.playlist_count;
    }
    display_clear(THEME_COLOR_BG);
    theme_skin_render(s_now_ms, s_now_ms - s_last_input_ms, s_sel, &st);
}

static void skin_handle_event(const input_event_t *ev)
{
    int r;

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;
    s_last_input_ms = s_now_ms;
    r = theme_skin_key(ev->type, &s_sel);
    switch (r) {
    case THEME_SKIN_REDRAW: {
        int y, h;
        int band = theme_skin_refresh_h();

        /* The eyes shift and the selected bar changes; the background and
         * rule are static. Redraw only those two bands. */
        ui_invalidate(0, 0, DISPLAY_W - 1,
                      band > 0 ? band - 1 : DISPLAY_H - 1);
        if (theme_skin_list_bounds(&y, &h) == 0 && h > 0)
            ui_invalidate(0, y, DISPLAY_W - 1, y + h - 1);
        break;
    }
    case THEME_SKIN_BACK:
        ui_pop_screen();
        break;
    case THEME_SKIN_LIBRARY:
        if (s_open_lib)
            s_open_lib();
        break;
    case THEME_SKIN_PLAYER:
        if (s_open_np)
            s_open_np();
        break;
    case THEME_SKIN_SETTINGS:
        if (s_open_set)
            s_open_set();
        break;
    default:
        break;
    }
}

const ui_screen_t skin_home_screen = {
    "skin",
    skin_on_enter,
    NULL,
    skin_draw,
    skin_handle_event,
};
