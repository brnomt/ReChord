/*
 * test_skin_home.c — generic declarative skin engine + UI adapter (BRNOMT-69).
 *
 * The skin file below is theme data: no Eye/Companion symbols exist in the
 * firmware. The test verifies expressions, the interactive list, the
 * prev/next key mapping (prev=down makes vol+ move left) and the screen
 * adapter wiring.
 */
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_services.h"
#include "screens/skin_screen.h"
#include "theme.h"
#include "../../theme/theme_skin.h"
#include "../../theme/theme_assets.h"

static int s_lib, s_player, s_settings;

static void open_lib(void)      { s_lib++; }
static void open_player(void)   { s_player++; }
static void open_settings(void) { s_settings++; }

static input_event_t key(int type)
{
    input_event_t e;

    e.type = type;
    e.adc_value = -1;
    e.kind = INPUT_PRESS;
    e.long_press = 0;
    return e;
}

static int write_skin(void)
{
    FILE *f = fopen("rechord_test.skin", "w");

    if (f == NULL)
        return -1;
    fputs(
        "[skin]\n"
        "refresh_ms=100\n"
        "refresh_h=80\n"
        "\n"
        "[bg]\n"
        "type=fill\n"
        "x=0\ny=0\nw=W\nh=H\ncolor=background\n"
        "\n"
        "[title]\n"
        "type=text\n"
        "x=4\ny=4\ntext=title\ncolor=text\n"
        "\n"
        "[items]\n"
        "type=list\n"
        "axis=horizontal\n"
        "y=100\n"
        "height=14\n"
        "selected=1\n"
        "prev=down\n"
        "next=up\n"
        "item0=LIBRARY:library\n"
        "item1=PLAYER:player\n"
        "item2=SETTINGS:settings\n",
        f);
    fclose(f);
    return 0;
}

void test_skin_home(void)
{
    theme_skin_state_t st;
    int sel = 0;
    int idx;

    ui_init();
    mock_display_reset();
    mock_services_reset();
    theme_layout_reset();

    CHECK(write_skin() == 0);
    CHECK_EQ_INT(theme_skin_load(".", "", "rechord_test.skin"), 0);
    remove("rechord_test.skin");
    CHECK(theme_skin_text() != NULL);
    CHECK_EQ_INT(theme_skin_item_count(), 3);
    CHECK_EQ_INT(theme_skin_selected_default(), 1);
    CHECK_EQ_INT(theme_skin_refresh_ms(), 100);
    CHECK_EQ_INT(theme_skin_refresh_h(), 80);

    /* prev=down / next=up: the hardware's vol+ (DOWN) moves left. */
    sel = 1;
    CHECK_EQ_INT(theme_skin_key(THEME_SKIN_KEY_DOWN, &sel), THEME_SKIN_REDRAW);
    CHECK_EQ_INT(sel, 0);
    CHECK_EQ_INT(theme_skin_key(THEME_SKIN_KEY_UP, &sel), THEME_SKIN_REDRAW);
    CHECK_EQ_INT(sel, 1);
    CHECK_EQ_INT(theme_skin_key(THEME_SKIN_KEY_SELECT, &sel),
                 THEME_SKIN_PLAYER);

    /* Draw the data-driven scene. */
    memset(&st, 0, sizeof st);
    st.playing = 1;
    st.volume_db = -10;
    st.title = "Test Track";
    st.artist = "Artist";
    st.album = "Album";
    st.state = "Playing";
    mock_display_reset();
    theme_skin_render(1000, 0, 1, 0, &st);

    idx = mock_display_text_find("Test Track");
    CHECK(idx >= 0);
    idx = mock_display_text_find("LIBRARY");
    CHECK(idx >= 0);
    idx = mock_display_text_find("PLAYER");
    CHECK(idx >= 0);
    if (idx >= 0) {
        CHECK_EQ_INT(mock_display_text(idx)->fg, THEME_COLOR_BG);
        CHECK_EQ_INT(mock_display_text(idx)->bg, THEME_COLOR_FG);
    }
    CHECK(mock_display_text_find("SETTINGS") >= 0);

    /* Adapter: keys go through the engine and actions reach the app. */
    s_lib = s_player = s_settings = 0;
    skin_home_set_actions(open_lib, open_player, open_settings);
    skin_home_tick(2000);
    CHECK(ui_push_screen(&skin_home_screen));
    mock_display_reset();
    ui_render();
    CHECK(mock_display_text_find("PLAYER") >= 0);

    {
        input_event_t e = key(KEY_DOWN);      /* prev=down -> LIBRARY */
        ui_handle_event(&e);
        e = key(KEY_SELECT);
        ui_handle_event(&e);
        CHECK_EQ_INT(s_lib, 1);
        CHECK_EQ_INT(s_player, 0);
    }
    {
        input_event_t e = key(KEY_UP);        /* next=up -> PLAYER */
        ui_handle_event(&e);
        e = key(KEY_SELECT);
        ui_handle_event(&e);
        CHECK_EQ_INT(s_player, 1);
    }
    {
        input_event_t e = key(KEY_BACK);
        ui_handle_event(&e);
        CHECK_EQ_INT(ui_stack_depth(), 0);
    }

    /* The real package from the repo: theme.cfg + home.skin + assets, with
     * no theme code in the firmware. */
    theme_layout_reset();
    CHECK_EQ_INT(theme_layout_load("../../../themes/companion-bot/theme.cfg"), 0);
    CHECK_EQ_INT(theme_layout()->home, THEME_HOME_SKIN);
    CHECK_EQ_INT(theme_assets_load("../../../themes", "companion-bot",
                                   theme_layout()), 0);
    CHECK_EQ_INT(theme_skin_load("../../../themes", "companion-bot",
                                 theme_layout()->skin), 0);
    CHECK_EQ_INT(theme_skin_item_count(), 3);
    CHECK_EQ_INT(theme_skin_key(THEME_SKIN_KEY_DOWN, &sel),
                 THEME_SKIN_REDRAW);          /* prev=down */
    CHECK_EQ_INT(sel, 0);

    memset(&st, 0, sizeof st);
    st.playing = 1;
    st.title = "Companion";
    st.state = "Playing";
    mock_display_reset();
    theme_skin_render(1000, 0, 1, 0, &st);
    CHECK(mock_display_bitmap_count() == 0);  /* vector skin */
    CHECK(mock_display_rect_count() > 0);     /* eyes + rule + list */
    CHECK(mock_display_text_find("LIBRARY") >= 0);
    CHECK(mock_display_text_find("NOW PLAYING") >= 0);
    CHECK(mock_display_text_find("SETTINGS") >= 0);

    theme_skin_reset();
    theme_assets_reset();
    theme_layout_reset();

    /* Runtime state exposed to the theme: title/album/position/duration/
     * depth/rate/playlist/modes. */
    {
        static const char *state_skin =
            "[skin]\nrefresh_ms=1000\n"
            "[t]\ntype=text\nx=0\ny=0\ntext=title\n"
            "[a]\ntype=text\nx=0\ny=12\ntext=album\n"
            "[p]\ntype=text\nx=0\ny=24\ntext=pos\n"
            "[d]\ntype=text\nx=0\ny=36\ntext=dur\n"
            "[b]\ntype=text\nx=0\ny=48\ntext=bits\n"
            "[r]\ntype=text\nx=0\ny=60\ntext=rate\n"
            "[pi]\ntype=text\nx=0\ny=72\ntext=pl_i\n"
            "[pn]\ntype=text\nx=0\ny=84\ntext=pl_n\n"
            "[s]\ntype=text\nx=0\ny=96\ntext=shuffle\n"
            "[m]\ntype=text\nx=0\ny=108\ntext=repeat\n"
            "[plx]\ntype=text\nx=0\ny=120\ntext=pl_i + \"/\" + pl_n\n"
            "[fmt]\ntype=text\nx=0\ny=132\ntext=bits + \"/\" + (rate / 1000) + \"kHz\"\n";
        FILE *f = fopen("rechord_state.skin", "w");

        CHECK(f != NULL);
        if (f != NULL) {
            fputs(state_skin, f);
            fclose(f);
        }
        CHECK_EQ_INT(theme_skin_load(".", "", "rechord_state.skin"), 0);
        remove("rechord_state.skin");
        memset(&st, 0, sizeof st);
        st.title = "TitleX";
        st.album = "AlbumX";
        st.state = "Playing";
        st.playing = 1;
        st.position_ms = 42000;
        st.duration_ms = 125000;
        st.bits = 24;
        st.sample_rate = 96000;
        st.playlist_pos = 3;
        st.playlist_count = 7;
        st.shuffle = 1;
        st.repeat = 2;
        mock_display_reset();
        theme_skin_render(1000, 0, 0, 0, &st);
        CHECK(mock_display_text_find("TitleX") >= 0);
        CHECK(mock_display_text_find("AlbumX") >= 0);
        CHECK(mock_display_text_find("42") >= 0);
        CHECK(mock_display_text_find("125") >= 0);
        CHECK(mock_display_text_find("24") >= 0);
        CHECK(mock_display_text_find("96000") >= 0);
        CHECK(mock_display_text_find("3") >= 0);
        CHECK(mock_display_text_find("7") >= 0);
        CHECK(mock_display_text_find("3/7") >= 0);
        CHECK(mock_display_text_find("24/96kHz") >= 0);
    }

    theme_skin_reset();
    theme_layout_reset();
}
