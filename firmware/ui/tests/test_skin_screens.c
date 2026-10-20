/*
 * test_skin_screens.c — per-screen skins and dynamic list sources (BRNOMT-69).
 *
 * The compiled screens hand their rows to the engine through a bounded
 * callback table; the theme only owns the layout. This test covers the
 * window (top/rows), value column, nav mapping, runtime strings and that the
 * real Companion Bot screen skins parse.
 */
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_services.h"
#include "screens/screens.h"
#include "theme.h"
#include "../../theme/theme_skin.h"

static const char *const k_names[10] = {
    "zero", "one", "two", "three", "four",
    "five", "six", "seven", "eight", "nine"
};

static const char *src_label(int index, char *buf, int cap)
{
    if (index < 0 || index >= 10)
        return "";
    snprintf(buf, (size_t)cap, "%s", k_names[index]);
    return buf;
}

static const char *src_value(int index, char *buf, int cap)
{
    snprintf(buf, (size_t)cap, "V%d", index);
    return buf;
}

static theme_skin_list_source_t s_source = { 10, src_label, src_value };

static int write_skin(void)
{
    FILE *f = fopen("rechord_screens.skin", "w");

    if (f == 0)
        return -1;
    fputs(
        "[head]\n"
        "type=text\nx=4\ny=2\ntext=heading\ncolor=text\n"
        "\n"
        "[time]\n"
        "type=text\nx=4\ny=12\ntext=pos_fmt + \" / \" + dur_fmt\ncolor=text\n"
        "\n"
        "[menu]\n"
        "type=list\naxis=vertical\nx=4\ny=24\nheight=10\n"
        "rows=4\nsource=test\nprev=down\nnext=up\n"
        "\n"
        "[foot]\n"
        "type=text\nx=4\ny=150\ntext=(sel + 1) + \" / \" + n\ncolor=text\n",
        f);
    fclose(f);
    return 0;
}

void test_skin_screens(void)
{
    theme_skin_state_t st;

    ui_init();
    mock_display_reset();
    CHECK_EQ_INT(write_skin(), 0);
    CHECK_EQ_INT(theme_skin_load(".", "", "rechord_screens.skin"), 0);
    theme_skin_set_list(&s_source);

    CHECK_EQ_INT(theme_skin_item_count(), 10);
    CHECK_EQ_INT(theme_skin_list_rows(), 4);
    CHECK_EQ_INT(theme_skin_nav(THEME_SKIN_KEY_DOWN), THEME_SKIN_PREV);
    CHECK_EQ_INT(theme_skin_nav(THEME_SKIN_KEY_UP), THEME_SKIN_NEXT);
    CHECK_EQ_INT(theme_skin_nav(THEME_SKIN_KEY_SELECT), THEME_SKIN_SELECT);
    CHECK_EQ_INT(theme_skin_nav(THEME_SKIN_KEY_BACK), THEME_SKIN_BACK);

    memset(&st, 0, sizeof st);
    st.heading = "Settings";
    st.position_ms = 65000;
    st.duration_ms = 245000;
    theme_skin_render(1000, 0, 5, 4, &st);

    CHECK(mock_display_text_find("Settings") >= 0);
    CHECK(mock_display_text_find("1:05 / 4:05") >= 0);
    CHECK(mock_display_text_find("6 / 10") >= 0);
    CHECK(mock_display_text_find("four") >= 0);
    CHECK(mock_display_text_find("five") >= 0);
    CHECK(mock_display_text_find("seven") >= 0);
    CHECK(mock_display_text_find("V5") >= 0);
    CHECK(mock_display_text_find("three") < 0);   /* above the window  */

    /* The app calls player_screen_refresh() every pass, even while another
     * screen is on top. It must not load the Now Playing skin and clobber the
     * active screen's skin. */
    player_screen_refresh();
    CHECK(theme_skin_text() != 0);
    CHECK_EQ_INT(theme_skin_item_count(), 10);

    /* Real package screens must parse without any compiled theme code. */
    CHECK_EQ_INT(theme_skin_load("../../../themes", "companion-bot",
                              "settings.skin"), 0);
    theme_skin_set_list(&s_source);
    mock_display_reset();
    memset(&st, 0, sizeof st);
    st.heading = "Settings";
    theme_skin_render(1000, 0, 1, 0, &st);
    CHECK(mock_display_text_find("Settings") >= 0);
    CHECK(mock_display_text_find("one") >= 0);
    CHECK(mock_display_text_find("V1") >= 0);

    CHECK_EQ_INT(theme_skin_load("../../../themes", "companion-bot",
                              "now_playing.skin"), 0);
    mock_display_reset();
    memset(&st, 0, sizeof st);
    st.heading = "Now playing";
    st.title = "Song";
    st.artist = "Artist";
    st.album = "Album";
    st.state = "Playing";
    st.playing = 1;
    st.volume_db = -12;
    st.position_ms = 65000;
    st.duration_ms = 245000;
    st.bits = 16;
    st.sample_rate = 44100;
    st.playlist_pos = 3;
    st.playlist_count = 9;
    st.shuffle = 1;
    st.repeat = 2;
    theme_skin_render(1000, 0, 0, 0, &st);
    CHECK(mock_display_text_find("Now playing") >= 0);
    CHECK(mock_display_text_find("Song") >= 0);
    CHECK(mock_display_text_find("Playing") >= 0);
    CHECK(mock_display_text_find("1:05 / 4:05") >= 0);
    CHECK(mock_display_text_find("16/44kHz") >= 0);
    CHECK(mock_display_text_find("3/9") >= 0);
    CHECK(mock_display_text_find("REP1") >= 0);
    CHECK(mock_display_text_find("-12 dB") >= 0);

    CHECK_EQ_INT(theme_skin_load("../../../themes", "companion-bot",
                              "library.skin"), 0);
    theme_skin_set_list(&s_source);
    /* Vertical lists use the intuitive mapping: vol+ (DOWN) = next/down. */
    CHECK_EQ_INT(theme_skin_nav(THEME_SKIN_KEY_DOWN), THEME_SKIN_NEXT);
    CHECK_EQ_INT(theme_skin_nav(THEME_SKIN_KEY_UP), THEME_SKIN_PREV);
    CHECK_EQ_INT(theme_skin_load("../../../themes", "companion-bot",
                              "library_cat.skin"), 0);
    CHECK_EQ_INT(theme_skin_load("../../../themes", "companion-bot",
                              "browser.skin"), 0);

    remove("rechord_screens.skin");
}
