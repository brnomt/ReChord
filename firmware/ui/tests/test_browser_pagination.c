/*
 * test_browser_pagination.c — browser pagination over fs_list_dir().
 *
 * Contract under test: the fetch window always starts at the view top,
 * one fetch == one visible page (BROWSER_WINDOW rows), the cursor clamps at
 * both ends of the listing, directory entry/leave re-roots the listing, and
 * BACK at the configured root pops the screen.
 */
#include "test_util.h"
#include "ui.h"
#include "screens/screens.h"
#include "mock_display.h"
#include "mock_services.h"

static input_event_t ev(input_key_t key)
{
    input_event_t e;
    e.type = key;
    e.adc_value = 0;
    return e;
}

static void build_tree(void)
{
    char name[8];
    int i;

    mock_fs_reset();
    mock_fs_add_dir("/t");
    mock_fs_add_entry("/t", "sub", 1);
    for (i = 0; i < 39; i++) {           /* + "sub" => 40 entries */
        snprintf(name, sizeof(name), "f%02d", i);
        mock_fs_add_entry("/t", name, 0);
    }
    mock_fs_add_dir("/t/sub");
    mock_fs_add_entry("/t/sub", "s0", 0);
    mock_fs_add_entry("/t/sub", "s1", 0);
    mock_fs_add_entry("/t/sub", "s2", 0);
}

void test_browser_pagination(void)
{
    const browser_state_t *st;
    input_event_t e;
    int i;

    ui_init();
    mock_display_reset();
    mock_services_reset();
    build_tree();

    browser_screen_set_root("/t");
    CHECK(ui_push_screen(&browser_screen));
    st = browser_screen_state();

    /* initial page: window starts at index 0, one page of entries */
    CHECK_STR_EQ(st->path, "/t");
    CHECK_EQ_INT((int)st->total, 40);
    CHECK_EQ_INT((int)st->win_start, 0);
    CHECK_EQ_INT((int)st->win_count, BROWSER_WINDOW);
    CHECK_EQ_INT(st->cursor, 0);
    CHECK_EQ_INT(st->top, 0);
    CHECK_STR_EQ(st->window[0].name, "sub");
    CHECK_EQ_INT(st->window[0].is_dir, 1);
    CHECK_EQ_INT(mock_fs_call_count(), 1);
    CHECK_STR_EQ(mock_fs_call_path(0), "/t");
    CHECK_EQ_INT((int)mock_fs_call_index(0), 0);

    /* scroll past the first page: the window refetches at the new top */
    for (i = 0; i < BROWSER_WINDOW; i++) {
        e = ev(KEY_DOWN);
        ui_handle_event(&e);
    }
    CHECK_EQ_INT(st->cursor, BROWSER_WINDOW);
    CHECK_EQ_INT(st->top, 1);
    CHECK_EQ_INT((int)st->win_start, 1);          /* window start == top */
    CHECK_STR_EQ(st->window[0].name, "f00");      /* listing index 1    */
    CHECK_EQ_INT(mock_fs_call_count(), 2);
    CHECK_EQ_INT((int)mock_fs_call_index(1), 1);

    /* run to the end: cursor and top clamp at the listing bounds */
    for (i = 0; i < 100; i++) {
        e = ev(KEY_DOWN);
        ui_handle_event(&e);
    }
    CHECK_EQ_INT(st->cursor, 39);
    CHECK_EQ_INT(st->top, 39 - BROWSER_WINDOW + 1);
    CHECK_EQ_INT((int)st->win_start, 39 - BROWSER_WINDOW + 1);

    /* and back to the start */
    for (i = 0; i < 100; i++) {
        e = ev(KEY_UP);
        ui_handle_event(&e);
    }
    CHECK_EQ_INT(st->cursor, 0);
    CHECK_EQ_INT(st->top, 0);
    CHECK_EQ_INT((int)st->win_start, 0);

    /* entering a directory re-roots the listing at index 0 */
    e = ev(KEY_SELECT);   /* cursor 0 == "sub" */
    ui_handle_event(&e);
    CHECK_STR_EQ(st->path, "/t/sub");
    CHECK_EQ_INT((int)st->total, 3);
    CHECK_EQ_INT(st->cursor, 0);
    CHECK_EQ_INT((int)st->win_start, 0);
    CHECK_STR_EQ(st->window[0].name, "s0");

    /* nested paging inside the subdirectory */
    e = ev(KEY_DOWN); ui_handle_event(&e);
    e = ev(KEY_DOWN); ui_handle_event(&e);
    e = ev(KEY_DOWN); ui_handle_event(&e);   /* clamps at 2 */
    CHECK_EQ_INT(st->cursor, 2);
    CHECK_EQ_INT(st->top, 0);                /* 3 entries fit one page */

    /* BACK returns to the parent with its listing restored */
    e = ev(KEY_BACK);
    ui_handle_event(&e);
    CHECK_STR_EQ(st->path, "/t");
    CHECK_EQ_INT((int)st->total, 40);
    CHECK_EQ_INT(st->cursor, 0);
    CHECK_EQ_INT((int)st->win_start, 0);

    /* BACK at the configured root pops the screen stack */
    CHECK_EQ_INT(ui_stack_depth(), 1);
    e = ev(KEY_BACK);
    ui_handle_event(&e);
    CHECK_EQ_INT(ui_stack_depth(), 0);

    /* rendering shows the path and the names */
    mock_display_reset();
    browser_screen_set_root("/t");
    CHECK(ui_push_screen(&browser_screen));
    ui_render();
    CHECK(mock_display_text_find("/t") >= 0);
    CHECK(mock_display_text_find("sub/") >= 0);
    CHECK(mock_display_text_find("f00") >= 0);

    /* opening a FILE (cursor 1 = "f00") asks the player and pushes
     * Now Playing with the track loaded and playing */
    e = ev(KEY_DOWN); ui_handle_event(&e);       /* past "sub/" onto "f00" */
    CHECK_EQ_INT(st->window[1].is_dir, 0);
    e = ev(KEY_SELECT); ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_open_count(), 1);
    CHECK_STR_EQ(mock_player_open_path(), "/t/f00");
    CHECK_EQ_INT(mock_player_play_count(), 1);
    CHECK_EQ_INT(ui_stack_depth(), 2);           /* player screen on top  */
    ui_render();
    CHECK(mock_display_text_find("f00") >= 0);   /* track title           */
    CHECK(mock_display_text_find("Playing") >= 0);

    /* BACK leaves the player (playback continues) back to the browser */
    e = ev(KEY_BACK); ui_handle_event(&e);
    CHECK_EQ_INT(ui_stack_depth(), 1);

    /* an unopenable file still pushes Now Playing, showing the reason
     * (browser_screen renders the diagnostic code — "cannot open (-4)"
     * for PLAYER_ERR_UNSUPPORTED; raw codes beat a generic message). */
    mock_player_set_open_rc(PLAYER_ERR_UNSUPPORTED);
    e = ev(KEY_SELECT); ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_open_count(), 2);
    CHECK_EQ_INT(ui_stack_depth(), 2);
    ui_render();
    CHECK(mock_display_text_find("cannot open (-4)") >= 0);
    CHECK(mock_display_text_find("f00") >= 0);   /* file name as title    */
}

/* Tag-aware listing and Now Playing (BRNOMT-78 pt.1): files list their tag
 * title with the artist right-aligned, untagged files and directories keep
 * their names, and Now Playing shows "artist - album" when the track has
 * tags. */
void test_browser_tags(void)
{
    input_event_t e;

    ui_init();
    mock_display_reset();
    mock_services_reset();

    mock_fs_add_dir("/m");
    mock_fs_add_entry("/m", "plain.mp3", 0);
    mock_fs_add_entry("/m", "tagged.flac", 0);
    mock_player_set_meta("/m/tagged.flac", "Blue Monday", "New Order");

    browser_screen_set_root("/m");
    CHECK(ui_push_screen(&browser_screen));
    ui_render();
    CHECK(mock_display_text_find("Blue Monday") >= 0);  /* tag title      */
    CHECK(mock_display_text_find("New Order") >= 0);    /* artist column  */
    CHECK(mock_display_text_find("plain.mp3") >= 0);    /* no tags: name  */

    /* Now Playing for the tagged file: title from the tag + artist - album */
    mock_player_set_tags("New Order", "Power, Corruption & Lies");
    e = ev(KEY_DOWN);
    ui_handle_event(&e);                                /* onto tagged.flac */
    e = ev(KEY_SELECT);
    ui_handle_event(&e);
    ui_render();
    CHECK(mock_display_text_find("Blue Monday") >= 0);  /* tag title       */
    CHECK(mock_display_text_find("New Order - Power") >= 0);
    e = ev(KEY_BACK);
    ui_handle_event(&e);

    /* Untagged file: filename as title, no artist/album line. */
    mock_player_set_tags("", "");
    e = ev(KEY_UP);
    ui_handle_event(&e);                                /* onto plain.mp3  */
    e = ev(KEY_SELECT);
    ui_handle_event(&e);
    mock_display_reset();              /* drop the tagged render's records  */
    ui_render();
    CHECK(mock_display_text_find("plain.mp3") >= 0);
    CHECK(mock_display_text_find(" - ") < 0);
}
