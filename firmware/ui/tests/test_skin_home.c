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
    theme_skin_render(1000, 0, 1, &st);

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
    theme_skin_render(1000, 0, 1, &st);
    CHECK(mock_display_bitmap_count() == 2);  /* left + right eye */
    CHECK(mock_display_text_find("LIBRARY") >= 0);
    CHECK(mock_display_text_find("NOW PLAYING") >= 0);
    CHECK(mock_display_text_find("SETTINGS") >= 0);

    theme_skin_reset();
    theme_assets_reset();
    theme_layout_reset();
}
