/*
 * render_theme.c — host previewer for a theme package.
 *
 * Links the real skin interpreter, package runtime and RGB565 display driver
 * against the simulated panel (panel_io_sim), draws the screens the firmware
 * would draw and dumps one PPM per state. run.sh converts them to PNG.
 *
 * Usage: render_theme <themes-root> <package-id> <out-dir>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display.h"
#include "panel_io_sim.h"
#include "theme.h"
#include "theme_pkg.h"
#include "theme_runtime.h"
#include "theme_skin.h"

#define COVER_PX 120

static int s_art_ok;
static int s_art_draw(int x, int y, int large);

static theme_skin_art_t s_art = {
    1, 0x18E4, 0xF423, 0x2966, 0xFD88,
    0x0841, 0x18E4, 0x2966, 0x18E4, 0xF7BE, 0x18E4, 0x8C71,
    COVER_PX, 48, s_art_draw
};

static void row(int x0, int x1, int y, int c)
{
    display_fill_rect(x0, y, x1, y, (uint16_t)c);
}

/* A deliberately simple cover so the `art` meter has pixels to show. */
static int s_art_draw(int x, int y, int large)
{
    int i, r = large ? COVER_PX : 48;
    int sky_a = 0x301A, sky_b = 0x98AA, sun = 0xFDC5, hill = 0x110A;

    if (!s_art_ok)
        return -1;
    for (i = 0; i < r; i++) {
        int t = (i * 255) / (r - 1);
        int cr = ((sky_a >> 11) * (255 - t) + (sky_b >> 11) * t) / 255;
        int cg = (((sky_a >> 5) & 63) * (255 - t) + ((sky_b >> 5) & 63) * t) / 255;
        int cb = ((sky_a & 31) * (255 - t) + (sky_b & 31) * t) / 255;
        row(x, x + r - 1, y + i, (cr << 11) | (cg << 5) | cb);
    }
    {
        int cx = r / 2, cy = r * 2 / 5, rad = r / 4;

        for (i = -rad; i <= rad; i++) {
            int half = 0;

            while ((half + 1) * (half + 1) + i * i <= rad * rad)
                half++;
            row(x + cx - half, x + cx + half, y + cy + i, sun);
        }
    }
    for (i = 0; i < r / 3; i++) {
        int half = (r / 2) * (r / 3 - i) / (r / 3);

        row(x + r / 2 - half, x + r / 2 + half, y + r - 1 - i, hill);
    }
    for (i = r * 2 / 3; i < r; i += 4)
        row(x, x + r - 1, y + i, 0x18C6);
    return 0;
}

static void base_state(theme_skin_state_t *st)
{
    memset(st, 0, sizeof *st);
    st->playing = 1;
    st->volume_db = -18;
    st->battery_pct = 76;
    st->title = "Placeholder Song (Demo Mix)";
    st->artist = "Demo Artist";
    st->album = "Mockup Album Vol. 1";
    st->path = "/Internal/Music/Placeholder";
    st->state = "Playing";
    st->position_ms = 102000;
    st->duration_ms = 245000;
    st->bits = 24;
    st->sample_rate = 96000;
    st->playlist_pos = 3;
    st->playlist_count = 12;
    st->shuffle = 1;
    st->repeat = 1;
    st->heading = "";
}

static int load(const char *root, const char *id, const char *path)
{
    int r = theme_skin_load(root, id, path);

    if (r != 0)
        fprintf(stderr, "skin %s: load failed\n", path);
    return r;
}

static void shot(const char *dir, const char *name, unsigned t, unsigned idle,
                 int sel, int top, const theme_skin_state_t *st)
{
    char path[512];
    unsigned step;

    s_art.ok = s_art_ok;
    theme_skin_set_art(&s_art);

    /* Warm the persistent [vars] (needle ballistics) with earlier frames. */
    for (step = (t > 3200u ? t - 3200u : 0); step < t; step += 40) {
        display_clear(theme_get()->background);
        theme_skin_render(step, idle, sel, top, st);
    }
    display_clear(theme_get()->background);
    theme_skin_render(t, idle, sel, top, st);
    display_flush_rows(0, DISPLAY_H - 1);
    snprintf(path, sizeof path, "%s/%s.ppm", dir, name);
    if (panel_io_sim_write_ppm(path) != 0)
        fprintf(stderr, "write %s failed\n", path);
}

/* ---- list sources ------------------------------------------------------- */

static const char *k_lib[] = {
    "All Songs", "Album", "Artist", "Genre", "Year", "Label",
    "Album Artist", "Composer", "Favorites", "Recently played", "Most played"
};
static const char *k_songs[] = {
    "Placeholder Song (Demo Mix)", "Fake Track Two", "Sample Title Three",
    "Lorem Ipsum Groove", "Test Tone Ballad", "Mock Interlude",
    "Dummy Data Dance", "Example Outro"
};
static const char *k_songv[] = { "Fav", "", "", "Fav", "", "", "", "" };
static const char *k_set[] = {
    "Theme", "Volume", "ReplayGain", "Gapless", "Brightness",
    "Auto-off", "Controls", "Bluetooth"
};
static const char *k_setv[] = {
    "Oscilloscope", "-18 dB", "Track", "On", "60%", "10 min", "normal", "Off"
};
static const char *k_files[] = {
    "Music", "Recordings", "Audiobooks", "Playlists", "Root", "SD Card",
    "Internal", "Favorites"
};

static const char *lbl_any(int i, char *buf, int cap)
{
    (void)buf;
    (void)cap;
    (void)i;
    return "";
}

static const char *lib_label(int i, char *b, int cap)
{
    if (i < 0 || i >= (int)(sizeof k_lib / sizeof k_lib[0]))
        return "";
    snprintf(b, (size_t)cap, "%s", k_lib[i]);
    return b;
}

static const char *song_label(int i, char *b, int cap)
{
    if (i < 0 || i >= (int)(sizeof k_songs / sizeof k_songs[0]))
        return "";
    snprintf(b, (size_t)cap, "%s", k_songs[i]);
    return b;
}

static const char *song_value(int i, char *b, int cap)
{
    if (i < 0 || i >= (int)(sizeof k_songv / sizeof k_songv[0]))
        return "";
    snprintf(b, (size_t)cap, "%s", k_songv[i]);
    return b;
}

static const char *set_label(int i, char *b, int cap)
{
    if (i < 0 || i >= (int)(sizeof k_set / sizeof k_set[0]))
        return "";
    snprintf(b, (size_t)cap, "%s", k_set[i]);
    return b;
}

static const char *set_value(int i, char *b, int cap)
{
    if (i < 0 || i >= (int)(sizeof k_setv / sizeof k_setv[0]))
        return "";
    snprintf(b, (size_t)cap, "%s", k_setv[i]);
    return b;
}

static const char *file_label(int i, char *b, int cap)
{
    if (i < 0 || i >= (int)(sizeof k_files / sizeof k_files[0]))
        return "";
    snprintf(b, (size_t)cap, "%s", k_files[i]);
    return b;
}

int main(int argc, char **argv)
{
    const char *root, *id, *dir;
    theme_skin_state_t st;
    theme_skin_list_source_t src;
    unsigned t = 12345;
    int i;

    if (argc < 4) {
        fprintf(stderr, "usage: %s <themes-root> <package-id> <out-dir>\n",
                argv[0]);
        return 2;
    }
    root = argv[1];
    id = argv[2];
    dir = argv[3];

    display_init();
    theme_skin_set_art(&s_art);
    if (theme_pkg_apply(root, id) != 0) {
        fprintf(stderr, "package '%s' rejected\n", id);
        return 1;
    }
    printf("theme '%s' name '%s' home %d assets ok\n",
           theme_pkg_active_id(), theme_get()->name, theme_layout()->home);

    /* ---- home: the six moods ------------------------------------------- */
    load(root, id, "home.skin");
    s_art_ok = 1;

    base_state(&st);
    shot(dir, "home_playing", t, 5000, 1, 0, &st);
    shot(dir, "home_press", t, 150, 1, 0, &st);
    st.playing = 0;
    st.paused = 1;
    st.state = "Paused";
    shot(dir, "home_paused", t, 8000, 1, 0, &st);
    st.playing = 1;
    st.paused = 0;
    st.state = "Playing";
    st.volume_db = -3;
    shot(dir, "home_loud", t, 5000, 1, 0, &st);
    st.volume_db = -18;
    st.battery_pct = 9;
    shot(dir, "home_low", t, 5000, 1, 0, &st);
    st.battery_pct = 76;
    st.playing = 0;
    st.paused = 1;
    st.state = "Paused";
    shot(dir, "home_sleep", t, 90000, 1, 0, &st);

    /* ---- now playing --------------------------------------------------- */
    base_state(&st);
    st.heading = "Now playing";
    if (load(root, id, "now_playing.skin") == 0) {
        s_art_ok = 1;
        shot(dir, "np_cover", t, 5000, 0, 0, &st);
        s_art_ok = 0;
        shot(dir, "np_nocover", t, 5000, 0, 0, &st);
    }

    /* ---- library ------------------------------------------------------- */
    st.heading = "Library";
    if (load(root, id, "library.skin") == 0) {
        src.count = (int)(sizeof k_lib / sizeof k_lib[0]);
        src.label = lib_label;
        src.value = 0;
        theme_skin_set_list(&src);
        shot(dir, "library", t, 5000, 2, 0, &st);
    }

    /* ---- category ------------------------------------------------------ */
    st.heading = "All Songs";
    if (load(root, id, "library_cat.skin") == 0) {
        src.count = (int)(sizeof k_songs / sizeof k_songs[0]);
        src.label = song_label;
        src.value = song_value;
        theme_skin_set_list(&src);
        shot(dir, "library_cat", t, 5000, 2, 0, &st);
    }

    /* ---- browser ------------------------------------------------------- */
    st.heading = "/Internal/Music";
    if (load(root, id, "browser.skin") == 0) {
        src.count = (int)(sizeof k_files / sizeof k_files[0]);
        src.label = file_label;
        src.value = 0;
        theme_skin_set_list(&src);
        shot(dir, "browser", t, 5000, 1, 0, &st);
    }

    /* ---- settings ------------------------------------------------------ */
    st.heading = "Settings";
    if (load(root, id, "settings.skin") == 0) {
        src.count = (int)(sizeof k_set / sizeof k_set[0]);
        src.label = set_label;
        src.value = set_value;
        theme_skin_set_list(&src);
        shot(dir, "settings", t, 5000, 0, 0, &st);
    }

    (void)lbl_any;
    (void)i;
    return 0;
}
