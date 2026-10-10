/*
 * test_settings_keys.c — 12-key config schema and its menu binding.
 *
 * Verifies: exact key names/order (the on-device schema), per-key field
 * mapping (cycling one key touches exactly one field), value formatting,
 * wrap behavior, and the settings_screen menu binding + persistence hooks.
 */
#include "test_util.h"
#include "ui.h"
#include "settings_schema.h"
#include "screens/screens.h"
#include "mock_display.h"
#include "mock_services.h"

static const char *const k_expected_names[SETTING_KEY_COUNT] = {
    "play", "brightness", "screen_off", "auto_off", "usb", "resume",
    "strip", "tags", "cpu", "volume", "browse", "cursor",
};

static input_event_t ev(input_key_t key)
{
    input_event_t e;
    e.type = key;
    e.adc_value = 0;
    return e;
}

static void test_schema_table(void)
{
    int i;

    CHECK_EQ_INT(SETTING_KEY_COUNT, 12);
    for (i = 0; i < SETTING_KEY_COUNT; i++)
        CHECK_STR_EQ(setting_schema[i].name, k_expected_names[i]);

    /* lookup by name */
    CHECK(settings_schema_find("play") == &setting_schema[0]);
    CHECK(settings_schema_find("cursor") == &setting_schema[11]);
    CHECK(settings_schema_find("brightness") == &setting_schema[1]);
    CHECK(settings_schema_find("nope") == NULL);
    CHECK(settings_schema_find(NULL) == NULL);
}

static void test_defaults_and_format(void)
{
    rechord_settings_t s;
    char buf[32];

    settings_view_defaults(&s);
    /* seeded from the observed on-device config values */
    CHECK_EQ_INT(s.play_mode, 0);        /* repeat_track */
    CHECK_EQ_INT(s.brightness, 8);
    CHECK_EQ_INT(s.screen_off_s, 30);
    CHECK_EQ_INT(s.auto_off_min, 15);
    CHECK_EQ_INT(s.usb_mode, 0);         /* ask */
    CHECK_EQ_INT(s.resume_mode, 1);      /* folder */
    CHECK_EQ_INT(s.strip, 0);            /* off */
    CHECK_EQ_INT(s.tags, 1);             /* on */
    CHECK_EQ_INT(s.cpu_mode, 0);         /* auto */
    CHECK_EQ_INT(s.volume_db, -23);
    CHECK_STR_EQ(s.browse, "");
    CHECK_EQ_INT(s.cursor, 0);

    /* values format exactly like the config file writes them */
    settings_format(&setting_schema[0], &s, buf, (int)sizeof(buf));
    CHECK_STR_EQ(buf, "repeat_track");
    settings_format(&setting_schema[1], &s, buf, (int)sizeof(buf));
    CHECK_STR_EQ(buf, "8");
    settings_format(&setting_schema[2], &s, buf, (int)sizeof(buf));
    CHECK_STR_EQ(buf, "30");
    settings_format(&setting_schema[6], &s, buf, (int)sizeof(buf));
    CHECK_STR_EQ(buf, "off");
    settings_format(&setting_schema[7], &s, buf, (int)sizeof(buf));
    CHECK_STR_EQ(buf, "on");
    settings_format(&setting_schema[9], &s, buf, (int)sizeof(buf));
    CHECK_STR_EQ(buf, "-23");
}

static void test_key_field_mapping(void)
{
    rechord_settings_t s;
    rechord_settings_t old;
    int i;

    /* expected field after one +1 cycle from defaults (see schema table) */
    static const struct { int field; int want; } k_after[SETTING_KEY_COUNT] = {
        { 0, 1 },    /* play_mode: repeat_track -> repeat_folder */
        { 1, 9 },    /* brightness: 8 -> 9                        */
        { 2, 60 },   /* screen_off_s: 30 -> 60                    */
        { 3, 30 },   /* auto_off_min: 15 -> 30                    */
        { 4, 1 },    /* usb_mode: ask -> charge                   */
        { 5, 2 },    /* resume_mode: folder -> track              */
        { 6, 1 },    /* strip: off -> on                          */
        { 7, 0 },    /* tags: on -> off                           */
        { 8, 1 },    /* cpu_mode: auto -> low                     */
        { 9, -22 },  /* volume_db: -23 -> -22                     */
        { 10, 0 },   /* browse: cleared (string)                  */
        { 11, 1 },   /* cursor: 0 -> 1                            */
    };

    for (i = 0; i < SETTING_KEY_COUNT; i++) {
        int changed = 0;

        settings_view_defaults(&s);
        if (i == 10)   /* the string key: give it something to clear */
            snprintf(s.browse, sizeof(s.browse), "/music");
        old = s;

        settings_cycle(&setting_schema[i], &s, +1);

        /* exactly one field moved */
        if (s.play_mode != old.play_mode) changed++;
        if (s.brightness != old.brightness) changed++;
        if (s.screen_off_s != old.screen_off_s) changed++;
        if (s.auto_off_min != old.auto_off_min) changed++;
        if (s.usb_mode != old.usb_mode) changed++;
        if (s.resume_mode != old.resume_mode) changed++;
        if (s.strip != old.strip) changed++;
        if (s.tags != old.tags) changed++;
        if (s.cpu_mode != old.cpu_mode) changed++;
        if (s.volume_db != old.volume_db) changed++;
        if (strcmp(s.browse, old.browse) != 0) changed++;
        if (s.cursor != old.cursor) changed++;
        CHECK_EQ_INT(changed, 1);

        /* and it is the right one, with the right value */
        switch (k_after[i].field) {
        case 0:  CHECK_EQ_INT(s.play_mode, k_after[i].want); break;
        case 1:  CHECK_EQ_INT(s.brightness, k_after[i].want); break;
        case 2:  CHECK_EQ_INT(s.screen_off_s, k_after[i].want); break;
        case 3:  CHECK_EQ_INT(s.auto_off_min, k_after[i].want); break;
        case 4:  CHECK_EQ_INT(s.usb_mode, k_after[i].want); break;
        case 5:  CHECK_EQ_INT(s.resume_mode, k_after[i].want); break;
        case 6:  CHECK_EQ_INT(s.strip, k_after[i].want); break;
        case 7:  CHECK_EQ_INT(s.tags, k_after[i].want); break;
        case 8:  CHECK_EQ_INT(s.cpu_mode, k_after[i].want); break;
        case 9:  CHECK_EQ_INT(s.volume_db, k_after[i].want); break;
        case 10: CHECK_STR_EQ(s.browse, ""); break;
        case 11: CHECK_EQ_INT(s.cursor, k_after[i].want); break;
        default: CHECK(0); break;
        }
    }

    /* wrap behavior */
    settings_view_defaults(&s);
    s.brightness = 10;
    settings_cycle(&setting_schema[1], &s, +1);
    CHECK_EQ_INT(s.brightness, 0);
    settings_cycle(&setting_schema[1], &s, -1);
    CHECK_EQ_INT(s.brightness, 10);

    s.volume_db = 0;
    settings_cycle(&setting_schema[9], &s, +1);
    CHECK_EQ_INT(s.volume_db, -60);
    settings_cycle(&setting_schema[9], &s, -1);
    CHECK_EQ_INT(s.volume_db, 0);

    s.tags = 1;
    settings_cycle(&setting_schema[7], &s, +1);   /* on -> off -> on */
    settings_cycle(&setting_schema[7], &s, +1);
    CHECK_EQ_INT(s.tags, 1);
}

static void test_settings_screen_binding(void)
{
    const menu_model_t *model;
    input_event_t e;
    int i;

    ui_init();
    mock_display_reset();
    mock_services_reset();

    CHECK(ui_push_screen(&settings_screen));

    model = settings_screen_model();
    CHECK_EQ_INT(model->item_count, SETTING_KEY_COUNT);
    CHECK_STR_EQ(model->title, "Settings");
    for (i = 0; i < SETTING_KEY_COUNT; i++)
        CHECK_STR_EQ(model->items[i].label, k_expected_names[i]);

    /* defaults are live after enter */
    CHECK_EQ_INT(settings_screen_get()->screen_off_s, 30);
    CHECK_EQ_INT(settings_screen_get()->brightness, 8);

    /* navigate to "screen_off" (index 2) and cycle it */
    e = ev(KEY_DOWN); ui_handle_event(&e);
    e = ev(KEY_DOWN); ui_handle_event(&e);
    CHECK_EQ_INT(settings_screen_view()->cursor, 2);
    e = ev(KEY_SELECT); ui_handle_event(&e);
    CHECK_EQ_INT(settings_screen_get()->screen_off_s, 60);
    CHECK_EQ_INT(settings_screen_get()->brightness, 8);   /* untouched */

    /* the menu shows schema values */
    mock_display_reset();
    ui_render();
    CHECK(mock_display_text_find("screen_off") >= 0);
    CHECK(mock_display_text_find("60") >= 0);

    /* BACK saves through the settings service and pops */
    e = ev(KEY_BACK); ui_handle_event(&e);
    CHECK_EQ_INT(ui_stack_depth(), 0);
    CHECK_EQ_INT(mock_settings_save_count(), 1);
}

void test_settings_keys(void)
{
    test_schema_table();
    test_defaults_and_format();
    test_key_field_mapping();
    test_settings_screen_binding();
}
