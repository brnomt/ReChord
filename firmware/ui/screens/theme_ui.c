/*
 * theme_ui.c — generic per-screen skin bridge (see theme_ui.h).
 */
#include <string.h>

#include "theme_ui.h"
#include "../theme.h"
#include "../internal/hal.h"
#include "../../theme/theme_pkg.h"
#include "../../theme/theme_runtime.h"
#include "../../services/rechord_files.h"

static unsigned s_now_ms;
static unsigned s_last_input_ms;
static int      s_active;

/* The state holds pointers into this track; it must outlive fill_state() and
 * the draw that follows. One screen draws at a time. */
static track_info_t s_track;

const char *theme_ui_path(theme_ui_screen_t id)
{
    const theme_layout_t *l = theme_layout();

    switch (id) {
    case THEME_UI_MAIN:        return l->skin;
    case THEME_UI_NOW_PLAYING: return l->skin_now_playing;
    case THEME_UI_LIBRARY:     return l->skin_library;
    case THEME_UI_LIBRARY_CAT: return l->skin_library_cat;
    case THEME_UI_BROWSER:     return l->skin_browser;
    case THEME_UI_SETTINGS:    return l->skin_settings;
    default:                   return "";
    }
}

int theme_ui_load(theme_ui_screen_t id)
{
    const char *path = theme_ui_path(id);

    if (path == 0 || path[0] == '\0') {
        theme_skin_reset();
        s_active = 0;
        return 0;
    }
    if (theme_skin_load(RECHORD_THEMES_DIR, theme_pkg_active_id(), path) != 0) {
        theme_skin_reset();
        s_active = 0;
        return 0;
    }
    s_active = 1;
    return 1;
}

void theme_ui_tick(unsigned now_ms)
{
    s_now_ms = now_ms;
}

void theme_ui_note_input(void)
{
    s_last_input_ms = s_now_ms;
}

void theme_ui_fill_state(theme_skin_state_t *st, const char *heading)
{
    if (st == 0)
        return;
    memset(st, 0, sizeof *st);
    st->playing = player_get_state() == PLAYER_PLAYING;
    st->paused = player_get_state() == PLAYER_PAUSED;
    st->stopped = player_get_state() == PLAYER_STOPPED;
    st->volume_db = player_get_volume();
    st->battery_pct = -1;
    st->position_ms = player_get_position_ms();
    st->shuffle = player_get_shuffle();
    st->repeat = player_get_repeat();
    st->state = st->playing ? "Playing" : (st->paused ? "Paused" : "Stopped");
    st->title = "";
    st->artist = "";
    st->album = "";
    st->path = "";
    st->heading = heading != 0 ? heading : "";
    if (player_get_track(&s_track) == PLAYER_OK) {
        st->title = s_track.title;
        st->artist = s_track.artist;
        st->album = s_track.album;
        st->path = s_track.path;
        st->duration_ms = s_track.duration_ms;
        st->bits = s_track.bits;
        st->sample_rate = s_track.sample_rate;
        st->playlist_pos = s_track.playlist_pos;
        st->playlist_count = s_track.playlist_count;
    }
}

void theme_ui_draw(const theme_skin_state_t *st, int sel, int top)
{
    display_clear(THEME_COLOR_BG);
    theme_skin_render(s_now_ms, s_now_ms - s_last_input_ms, sel, top, st);
}

void theme_ui_list(const theme_skin_list_source_t *src)
{
    theme_skin_set_list(src);
}

int theme_ui_nav(int key)
{
    return theme_skin_nav(key);
}

int theme_ui_rows(void)
{
    return theme_skin_list_rows();
}

int theme_ui_rows_hint(void)
{
    return theme_skin_list_rows_hint();
}

int theme_ui_count(void)
{
    return theme_skin_item_count();
}

int theme_ui_active(void)
{
    return s_active;
}

void theme_ui_disarm(void)
{
    theme_skin_reset();
    s_active = 0;
}
