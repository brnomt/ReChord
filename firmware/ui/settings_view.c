/*
 * settings_view.c — target glue between the UI view (rechord_settings_t) and
 * the settings service (settings_t, the on-disk 12-key schema).
 *
 * The conversion logic lives in settings_schema.c (settings_view_from_store /
 * settings_view_to_store); this file only adds the load/save calls. It has
 * its own translation unit because the UI host tests substitute these two
 * functions in mock_services.c — the test build compiles the schema but not
 * this file, exactly like the target.
 */
#include "settings_schema.h"

int settings_view_load(rechord_settings_t *s)
{
    settings_t store;
    int rc;

    if (s == 0)
        return -1;
    rc = settings_load(&store);      /* defaults are applied on failure     */
    settings_view_from_store(&store, s);
    return rc;
}

int settings_view_save(const rechord_settings_t *s)
{
    settings_t store;

    if (s == 0)
        return -1;
    settings_view_to_store(s, &store);
    return settings_save(&store);
}
