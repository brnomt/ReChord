/*
 * mock_services.c — MOCK service backends implementation.
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "mock_services.h"

/* ---- input queue ------------------------------------------------------- */

#define MOCK_INPUT_MAX 32

static input_event_t s_queue[MOCK_INPUT_MAX];
static int           s_q_head;
static int           s_q_tail;
static char          s_log_last[128];
static int           s_log_count;

void mock_services_reset(void)
{
    s_q_head = 0;
    s_q_tail = 0;
    s_log_count = 0;
    s_log_last[0] = '\0';
    mock_settings_reset();
    mock_fs_reset();
}


void mock_input_push(input_key_t key, uint16_t adc)
{
    int next = (s_q_tail + 1) % MOCK_INPUT_MAX;

    if (next == s_q_head)
        return;   /* full: drop */
    s_queue[s_q_tail].type = key;
    s_queue[s_q_tail].adc_value = adc;
    s_q_tail = next;
}

int mock_input_pending(void)
{
    return s_q_head != s_q_tail;
}

int input_poll(input_event_t *ev)
{
    if (s_q_head == s_q_tail)
        return 0;
    *ev = s_queue[s_q_head];
    s_q_head = (s_q_head + 1) % MOCK_INPUT_MAX;
    return 1;
}

/* ---- log capture -------------------------------------------------------- */

int mock_log_count(void)
{
    return s_log_count;
}

const char *mock_log_last(void)
{
    return s_log_last;
}

void log_printf(const char *fmt, ...)
{
    va_list ap;

    s_log_count++;
    va_start(ap, fmt);
    vsnprintf(s_log_last, sizeof(s_log_last), fmt, ap);
    va_end(ap);
}

/* ---- in-memory settings -------------------------------------------------- */

static rechord_settings_t s_store;
static rechord_settings_t s_last_saved;
static int                s_save_count;
static int                s_load_rc;

void mock_settings_reset(void)
{
    settings_view_defaults(&s_store);
    settings_view_defaults(&s_last_saved);
    s_save_count = 0;
    s_load_rc = 0;
}

void mock_settings_set(const rechord_settings_t *s)
{
    s_store = *s;
}

void mock_settings_get(rechord_settings_t *s)
{
    *s = s_last_saved;
}

int mock_settings_save_count(void)
{
    return s_save_count;
}

void mock_settings_set_load_rc(int rc)
{
    s_load_rc = rc;
}

int settings_view_load(rechord_settings_t *s)
{
    if (s_load_rc != 0)
        return s_load_rc;
    *s = s_store;
    return 0;
}

int settings_view_save(const rechord_settings_t *s)
{
    s_last_saved = *s;
    s_store = *s;
    s_save_count++;
    return 0;
}

/* ---- fake directory tree ------------------------------------------------- */

typedef struct mock_dir {
    char        path[128];
    fs_dirent_t entries[MOCK_FS_MAX_ENTRIES];
    int         count;
} mock_dir_t;

typedef struct mock_call {
    char     path[128];
    uint32_t index;
} mock_call_t;

static mock_dir_t  s_dirs[MOCK_FS_MAX_DIRS];
static int         s_dir_count;
static mock_call_t s_calls[MOCK_FS_MAX_CALLS];
static int         s_call_count;

void mock_fs_reset(void)
{
    s_dir_count = 0;
    s_call_count = 0;
}

void mock_fs_add_dir(const char *path)
{
    if (s_dir_count >= MOCK_FS_MAX_DIRS)
        return;
    snprintf(s_dirs[s_dir_count].path, sizeof(s_dirs[s_dir_count].path),
             "%s", path);
    s_dirs[s_dir_count].count = 0;
    s_dir_count++;
}

int mock_fs_add_entry(const char *path, const char *name, int is_dir)
{
    int i;
    mock_dir_t *d = NULL;

    for (i = 0; i < s_dir_count; i++) {
        if (strcmp(s_dirs[i].path, path) == 0) {
            d = &s_dirs[i];
            break;
        }
    }
    if (d == NULL || d->count >= MOCK_FS_MAX_ENTRIES)
        return -1;
    snprintf(d->entries[d->count].name, FS_NAME_MAX, "%s", name);
    d->entries[d->count].is_dir = (uint8_t)(is_dir ? 1 : 0);
    d->count++;
    return 0;
}

int mock_fs_call_count(void)
{
    return s_call_count;
}

uint32_t mock_fs_call_index(int i)
{
    return (i >= 0 && i < s_call_count) ? s_calls[i].index : 0;
}

const char *mock_fs_call_path(int i)
{
    return (i >= 0 && i < s_call_count) ? s_calls[i].path : "";
}

int fs_list_dir(const char *path, uint32_t index, fs_dirent_t *out,
                uint32_t max, uint32_t *out_count, uint32_t *out_total)
{
    int i;
    mock_dir_t *d = NULL;
    uint32_t n = 0;

    if (s_call_count < MOCK_FS_MAX_CALLS) {
        snprintf(s_calls[s_call_count].path,
                 sizeof(s_calls[s_call_count].path), "%s", path);
        s_calls[s_call_count].index = index;
        s_call_count++;
    }

    for (i = 0; i < s_dir_count; i++) {
        if (strcmp(s_dirs[i].path, path) == 0) {
            d = &s_dirs[i];
            break;
        }
    }
    if (d == NULL)
        return -1;

    for (n = 0; n < max && index + n < (uint32_t)d->count; n++)
        out[n] = d->entries[index + n];
    if (out_count)
        *out_count = n;
    if (out_total)
        *out_total = (uint32_t)d->count;
    return 0;
}


/* Storage-layer defaults (services/settings.h) — the UI's storage<->view
 * adapter seeds the store through this; mocked here so UI tests stay
 * decoupled from the real services sources. */
void settings_defaults(settings_t *s)
{
    if (s == NULL)
        return;
    memset(s, 0, sizeof *s);
    snprintf(s->play, sizeof s->play, "repeat_track");
    snprintf(s->usb, sizeof s->usb, "ask");
    snprintf(s->resume, sizeof s->resume, "folder");
    snprintf(s->strip, sizeof s->strip, "off");
    snprintf(s->tags, sizeof s->tags, "on");
    snprintf(s->cpu, sizeof s->cpu, "auto");
    s->brightness = 8;
    s->screen_off_min = 30;
    s->auto_off_min = 15;
    s->volume = -23;
}
