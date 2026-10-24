/*
 * test_theme_cassette.c — the shipped "Cassette" package, end to end.
 *
 * Applies the real package (theme.cfg palette + layout + 1-bit assets), draws
 * the real home/now-playing skins with a fake art source and checks the
 * mockup's key elements: the cassette body, label texts, stripes, reels,
 * status/counter, the info card and the progress strip. Also drives the
 * no-art fallback (theme.cfg palette + the cassette glyph).
 */
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_services.h"
#include "../../theme/theme.h"
#include "../../theme/theme_pkg.h"
#include "../../theme/theme_runtime.h"
#include "../../theme/theme_skin.h"

static theme_skin_art_t s_src;
static int s_draw_calls;
static int s_draw_x, s_draw_y, s_draw_large;

static int fake_draw(int x, int y, int large)
{
    int dim = large ? s_src.large_px : s_src.small_px;

    s_draw_calls++;
    s_draw_x = x;
    s_draw_y = y;
    s_draw_large = large;
    display_fill_rect(x, y, x + dim - 1, y + dim - 1, 0x1234);
    return 0;
}

static void set_state(theme_skin_state_t *st, int playing, long pos)
{
    memset(st, 0, sizeof *st);
    st->playing = playing;
    st->paused = !playing;
    st->state = playing ? "Playing" : "Paused";
    st->volume_db = -14;
    st->battery_pct = -1;
    st->title = "Golden Hour Static";
    st->artist = "The Reel Club";
    st->album = "Side A Sessions";
    st->path = "/Internal/Music/A/01 Track.wav";
    st->position_ms = pos;
    st->duration_ms = 222000;
    st->bits = 24;
    st->sample_rate = 96000;
    st->playlist_pos = 3;
    st->playlist_count = 12;
    st->shuffle = 0;
    st->repeat = 1;
}

static int has_rect_color(uint16_t color)
{
    int i, n = mock_display_rect_count();

    for (i = 0; i < n; i++)
        if (mock_display_rect(i)->color == color)
            return 1;
    return 0;
}

static int has_text(const char *s)
{
    return mock_display_text_find(s) >= 0;
}

/* Exact match (strstr would make "PLAYING" match "NOW PLAYING"). */
static int has_text_exact(const char *s)
{
    int i, n = mock_display_text_count();

    for (i = 0; i < n; i++)
        if (strcmp(mock_display_text(i)->text, s) == 0)
            return 1;
    return 0;
}

void test_theme_cassette(void)
{
    theme_skin_state_t st;
    int i, label_rect = 0;

    ui_init();
    mock_display_reset();
    mock_services_reset();

    /* The real package: palette, layout, assets and the main skin. */
    CHECK_EQ_INT(theme_pkg_apply("../../../themes", "cassette"), 0);
    CHECK_STR_EQ(theme_pkg_active_id(), "cassette");
    CHECK(theme_layout()->home == THEME_HOME_SKIN);
    CHECK(theme_layout()->home_nav == THEME_NAV_HORIZONTAL);

    memset(&s_src, 0, sizeof s_src);
    s_src.ok = 1;
    s_src.c1 = 0x1111;
    s_src.c2 = 0x2222;
    s_src.mean = 0x3333;
    s_src.accent = 0x4444;
    s_src.bg_a = 0x5555;
    s_src.bg_b = 0x6666;
    s_src.glow = 0x7777;
    s_src.panel = 0x8888;
    s_src.label = 0x9999;
    s_src.ink = 0xAAAA;
    s_src.ink_dim = 0xBBBB;
    s_src.large_px = 120;
    s_src.small_px = 48;
    s_src.draw = fake_draw;
    theme_skin_set_art(&s_src);

    /* ---- home ----------------------------------------------------------- */
    set_state(&st, 1, 39800);              /* 0:39 of 3:42, like the mockup */
    s_draw_calls = 0;
    mock_display_reset();
    theme_skin_render(1000, 0, 1, 0, &st);

    CHECK(has_text("Golden Hour Static"));
    CHECK(has_text("The Reel Club"));
    CHECK(has_text("STEREO"));
    CHECK(has_text("24/96K"));
    CHECK(has_text("0:39"));
    CHECK(has_text("LIBRARY"));
    CHECK(has_text("NOW PLAYING"));
    CHECK(has_text("SETTINGS"));
    /* palette-derived pieces: label, stripes (c1/accent/c2), home bar */
    CHECK(has_rect_color(0x9999));
    CHECK(has_rect_color(0x1111));
    CHECK(has_rect_color(0x4444));
    CHECK(has_rect_color(0x2222));
    CHECK(has_rect_color(0x0841));
    /* home does not stream artwork */
    CHECK_EQ_INT(s_draw_calls, 0);

    /* A long title must be cut to 21 chars with '~'. */
    st.title = "A Very Long Track Title That Overflows";
    mock_display_reset();
    theme_skin_render(1000, 0, 1, 0, &st);
    CHECK(has_text("A Very Long Track Ti~"));

    /* ---- now playing ---------------------------------------------------- */
    set_state(&st, 1, 39800);
    CHECK_EQ_INT(theme_skin_load("../../../themes", "cassette",
                                 "now_playing.skin"), 0);
    s_draw_calls = 0;
    mock_display_reset();
    theme_skin_render(1000, 0, 0, 0, &st);

    CHECK_EQ_INT(s_draw_calls, 1);
    CHECK_EQ_INT(s_draw_x, 10);
    CHECK_EQ_INT(s_draw_y, 21);
    CHECK_EQ_INT(s_draw_large, 1);
    CHECK(has_text("NOW PLAYING"));
    CHECK(has_text("3/12"));
    CHECK(has_text_exact("PLAYING"));
    CHECK(has_text("24/96kHz"));
    CHECK(has_text("REP"));
    CHECK(has_text("-14 dB"));
    CHECK(has_text("0:39"));
    CHECK(has_text("3:42"));
    CHECK(has_rect_color(0x8888));         /* info card */

    /* Paused: the state line and the counter change. */
    set_state(&st, 0, 39800);
    mock_display_reset();
    theme_skin_render(1000, 0, 0, 0, &st);
    CHECK(has_text_exact("PAUSED"));
    CHECK(!has_text_exact("PLAYING"));

    /* ---- no art: theme.cfg fallback palette + the cassette glyph -------- */
    s_src.ok = 0;
    memset(&st, 0, sizeof st);
    set_state(&st, 1, 39800);
    s_draw_calls = 0;
    mock_display_reset();
    theme_skin_render(1000, 0, 0, 0, &st);
    CHECK_EQ_INT(s_draw_calls, 0);         /* placeholder, no stream */
    CHECK(has_text_exact("PLAYING"));
    /* fallback palette: the glyph uses text_dim, the label/card the theme's
     * background/text colours (art_* are 0 when art_ok is 0). */
    CHECK(has_rect_color(theme_get()->text_dim));
    CHECK(has_rect_color(theme_get()->background));
    CHECK(has_rect_color(theme_get()->text));

    /* The home label must fall back to theme_get()->text as well. */
    CHECK_EQ_INT(theme_skin_load("../../../themes", "cassette",
                                 "home.skin"), 0);
    mock_display_reset();
    theme_skin_render(1000, 0, 1, 0, &st);
    for (i = 0; i < mock_display_rect_count(); i++) {
        const mock_rect_t *r = mock_display_rect(i);

        if (r->x0 == 58 && r->color == theme_get()->text)
            label_rect = 1;
    }
    CHECK(label_rect);

    /* Every per-screen skin of the package must parse and render. */
    CHECK_EQ_INT(theme_skin_load("../../../themes", "cassette",
                                 "library.skin"), 0);
    CHECK_EQ_INT(theme_skin_load("../../../themes", "cassette",
                                 "library_cat.skin"), 0);
    CHECK_EQ_INT(theme_skin_load("../../../themes", "cassette",
                                 "browser.skin"), 0);
    CHECK_EQ_INT(theme_skin_load("../../../themes", "cassette",
                                 "settings.skin"), 0);
}
