/*
 * test_settings_view.c — host test for the target view<->store glue
 * (firmware/ui/settings_view.c) over the real schema converters
 * (settings_schema.c). The service I/O is stubbed to an in-memory store.
 *
 * The UI test binary stubs settings_view_load/save itself (mock_services.c),
 * so this binary is what covers the real glue: enum text <-> view value,
 * screen_off minutes <-> seconds, ints, and the load-error fallback.
 */
#include <stdio.h>
#include <string.h>

#include "settings_schema.h"

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

/* ---- stubbed service I/O ------------------------------------------------ */

static settings_t s_store;
static int        s_load_rc;
static int        s_save_count;

int settings_load(settings_t *s)
{
    /* Same contract as services/settings_store.c: defaults are always
     * applied first, so a failed load never leaves the view uninitialised. */
    settings_defaults(s);
    if (s_load_rc != 0)
        return s_load_rc;
    *s = s_store;
    return 0;
}

int settings_save(const settings_t *s)
{
    s_store = *s;
    s_save_count++;
    return 0;
}

/* Value of an enum option by its config text (from the schema itself). */
static int opt_value(const char *key, const char *text)
{
    const setting_schema_t *sc = settings_schema_find(key);
    int i;

    if (sc == 0 || sc->kind != SETTING_KIND_ENUM)
        return -999;
    for (i = 0; i < sc->u.e.count; i++)
        if (strcmp(sc->u.e.opts[i].text, text) == 0)
            return sc->u.e.opts[i].value;
    return -999;
}

int main(void)
{
    rechord_settings_t v, defs;

    printf("== settings: view<->store glue ==\n");

    /* 1. load maps the documented sample through the schema */
    settings_defaults(&s_store);
    snprintf(s_store.play, sizeof s_store.play, "repeat_track");
    s_store.brightness = 8;
    s_store.screen_off_min = 30;
    s_store.auto_off_min = 15;
    s_store.volume = -23;
    s_store.cursor = 3;
    s_load_rc = 0;
    memset(&v, 0, sizeof v);
    CHECK(settings_view_load(&v) == 0);
    CHECK(v.brightness == 8);
    CHECK(v.screen_off_s == 30);         /* same unit in store and view */
    CHECK(v.auto_off_min == 15);
    CHECK(v.volume_db == -23);
    CHECK(v.cursor == 3);
    CHECK(v.play_mode == opt_value("play", "repeat_track"));

    /* 2. save maps the view back (ints straight across, enums -> text) */
    s_save_count = 0;
    v.screen_off_s = 60;
    v.brightness = 4;
    v.play_mode = opt_value("play", "shuffle");
    CHECK(v.play_mode != -999);
    CHECK(settings_view_save(&v) == 0);
    CHECK(s_save_count == 1);
    CHECK(s_store.screen_off_min == 60);
    CHECK(s_store.brightness == 4);
    CHECK(strcmp(s_store.play, "shuffle") == 0);

    /* 3. round-trip through load again */
    memset(&v, 0, sizeof v);
    CHECK(settings_view_load(&v) == 0);
    CHECK(v.screen_off_s == 60);
    CHECK(v.brightness == 4);
    CHECK(v.play_mode == opt_value("play", "shuffle"));

    /* 4. a timer value outside the option list keeps the default */
    s_store.screen_off_min = 45;         /* not an option */
    CHECK(settings_view_load(&v) == 0);
    CHECK(v.screen_off_s == 30);         /* default from the schema     */

    /* 5. load failure -> the view still holds defaults */
    s_load_rc = -2;
    memset(&v, 0xFF, sizeof v);
    CHECK(settings_view_load(&v) == -2);
    settings_view_defaults(&defs);
    CHECK(memcmp(&v, &defs, sizeof v) == 0);

    /* 5. arguments */
    CHECK(settings_view_load(NULL) == -1);
    CHECK(settings_view_save(NULL) == -1);

    if (s_failures == 0)
        printf("test_settings_view: %d checks, OK\n", s_checks);
    else
        printf("test_settings_view: %d checks, %d FAILURES\n", s_checks,
               s_failures);
    return (s_failures == 0) ? 0 : 1;
}
