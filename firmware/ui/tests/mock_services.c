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
    mock_player_reset();
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

/* ---- mock player (firmware/player API seam for browser/player_screen) ---- */

static int  s_pl_open_rc;
static int  s_pl_open_count;
static int  s_pl_play_count;
static int  s_pl_pause_count;
static int  s_pl_volume = -23;
static int  s_pl_state = PLAYER_STOPPED;
static long s_pl_pos_ms;
static int  s_pl_present;               /* player_get_track() has a track?  */
static char s_pl_path[160];

void mock_player_reset(void)
{
    s_pl_open_rc = PLAYER_OK;
    s_pl_open_count = 0;
    s_pl_play_count = 0;
    s_pl_pause_count = 0;
    s_pl_volume = -23;
    s_pl_state = PLAYER_STOPPED;
    s_pl_pos_ms = 0;
    s_pl_present = 0;
    s_pl_path[0] = '\0';
}

void mock_player_set_open_rc(int rc)
{
    s_pl_open_rc = rc;
}

int mock_player_open_count(void)
{
    return s_pl_open_count;
}

const char *mock_player_open_path(void)
{
    return s_pl_path;
}

int mock_player_play_count(void)
{
    return s_pl_play_count;
}

int mock_player_pause_count(void)
{
    return s_pl_pause_count;
}

int mock_player_volume_db(void)
{
    return s_pl_volume;
}

void mock_player_set_state(int state)
{
    s_pl_state = state;
}

void mock_player_set_position(long ms)
{
    s_pl_pos_ms = ms;
}

void mock_player_set_track_present(int present)
{
    s_pl_present = present ? 1 : 0;
}

int player_open(const char *path)
{
    s_pl_open_count++;
    if (s_pl_open_rc != PLAYER_OK)
        return s_pl_open_rc;
    snprintf(s_pl_path, sizeof s_pl_path, "%s", path ? path : "");
    s_pl_present = 1;
    s_pl_state = PLAYER_STOPPED;
    s_pl_pos_ms = 0;
    return PLAYER_OK;
}

int player_play(void)
{
    if (!s_pl_present)
        return PLAYER_ERR_EMPTY;
    s_pl_play_count++;
    s_pl_state = PLAYER_PLAYING;
    return PLAYER_OK;
}

int player_pause(void)
{
    if (s_pl_state != PLAYER_PLAYING)
        return PLAYER_ERR_STATE;
    s_pl_pause_count++;
    s_pl_state = PLAYER_PAUSED;
    return PLAYER_OK;
}

int player_get_state(void)
{
    return s_pl_state;
}

long player_get_position_ms(void)
{
    return s_pl_pos_ms;
}

int player_get_volume(void)
{
    return s_pl_volume;
}

int player_set_volume(int db)
{
    if (db < -60)
        db = -60;
    if (db > 20)
        db = 20;
    s_pl_volume = db;
    return PLAYER_OK;
}

int player_get_track(track_info_t *out)
{
    const char *base;
    const char *p;
    size_t i;

    if (out == NULL)
        return PLAYER_ERR;
    if (!s_pl_present)
        return PLAYER_ERR_EMPTY;

    memset(out, 0, sizeof *out);
    base = s_pl_path;
    for (p = s_pl_path; *p; p++)
        if (*p == '/' || *p == '\\')
            base = p + 1;

    /* bounded copies (snprintf("%s") trips -Werror=format-truncation here) */
    for (i = 0; base[i] && i + 1 < sizeof out->title; i++)
        out->title[i] = base[i];
    out->title[i] = '\0';
    for (i = 0; s_pl_path[i] && i + 1 < sizeof out->path; i++)
        out->path[i] = s_pl_path[i];
    out->path[i] = '\0';

    out->duration_ms = 120000;          /* canned 2 min */
    out->track_no = 1;
    return PLAYER_OK;
}

/* FLAC decode diagnostics referenced by browser_screen (lives in
 * player/codecs/flac.c on target; zeroed stubs for UI tests). Types match
 * flac.c exactly. */
unsigned      flac_diag_block, flac_diag_channels;
unsigned long flac_diag_seektable, flac_diag_need;
int           flac_diag_reason;

/* ---- media index (screens/library_screen.c) ------------------------------
 * The UI tests do not have a filesystem: the index reports "none", which is
 * the screen's empty-state path. */
#include "../../services/media_db.h"

int media_db_open(void) { return FS_ERR_OPEN; }
uint32_t media_db_count(void) { return 0u; }
int media_db_get(uint32_t i, media_rec_t *out) { (void)i; (void)out; return FS_ERR_ARG; }
void media_db_close(void) { }
