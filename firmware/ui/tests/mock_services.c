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
static int           s_battery_pct = 76;

void mock_services_reset(void)
{
    s_q_head = 0;
    s_q_tail = 0;
    s_log_count = 0;
    s_log_last[0] = '\0';
    mock_settings_reset();
    mock_fs_reset();
    mock_player_reset();
    mock_bt_reset();
    s_battery_pct = 76;
}

/* ---- battery (the skin bridge reads battery_percent) ------------------- */

void mock_battery_set(int pct)
{
    s_battery_pct = pct;
}

int battery_percent(void)
{
    return s_battery_pct;
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
    s->auto_off_min = 5;
    s->volume = -23;
    snprintf(s->vol_orient, sizeof s->vol_orient, "normal");
    snprintf(s->hold_vol, sizeof s->hold_vol, "skip");
}

/* ---- mock player (firmware/player API seam for browser/player_screen) ---- */

static int  s_pl_open_rc;
static int  s_pl_open_count;
static int  s_pl_play_count;
static int  s_pl_pause_count;
static int  s_pl_volume = -23;
static int  s_pl_rg_mode = 3;
static int  s_pl_dac_filter = 1;
static int  s_pl_bt_level = 1;
static int  s_pl_state = PLAYER_STOPPED;
static long s_pl_pos_ms;
static int  s_pl_present;               /* player_get_track() has a track?  */
static char s_pl_path[160];
static int  s_pl_next_count;
static int  s_pl_prev_count;
static int  s_pl_seek_count;
static long s_pl_last_seek_ms;

void mock_player_reset(void)
{
    s_pl_open_rc = PLAYER_OK;
    s_pl_open_count = 0;
    s_pl_play_count = 0;
    s_pl_pause_count = 0;
    s_pl_volume = -23;
    s_pl_rg_mode = 3;               /* auto (BRNOMT-106 default) */
    s_pl_dac_filter = 1;            /* filter 1 (bring-up default) */
    s_pl_bt_level = 1;              /* Louder (wired level) */
    s_pl_state = PLAYER_STOPPED;
    s_pl_pos_ms = 0;
    s_pl_present = 0;
    s_pl_path[0] = '\0';
    s_pl_next_count = 0;
    s_pl_prev_count = 0;
    s_pl_seek_count = 0;
    s_pl_last_seek_ms = 0;
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

int mock_player_next_count(void)
{
    return s_pl_next_count;
}

int mock_player_prev_count(void)
{
    return s_pl_prev_count;
}

int mock_player_seek_count(void)
{
    return s_pl_seek_count;
}

long mock_player_last_seek_ms(void)
{
    return s_pl_last_seek_ms;
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

/* BRNOMT-93: the Now Playing long-press shortcuts call these. */
int player_next(void)
{
    if (!s_pl_present)
        return PLAYER_ERR_EMPTY;
    s_pl_next_count++;
    s_pl_state = PLAYER_PLAYING;
    return PLAYER_OK;
}

int player_prev(void)
{
    if (!s_pl_present)
        return PLAYER_ERR_EMPTY;
    s_pl_prev_count++;
    s_pl_state = PLAYER_PLAYING;
    return PLAYER_OK;
}

int player_seek(long ms)
{
    if (!s_pl_present)
        return PLAYER_ERR_EMPTY;
    if (ms < 0)
        ms = 0;
    s_pl_seek_count++;
    s_pl_last_seek_ms = ms;
    s_pl_pos_ms = ms;
    return PLAYER_OK;
}

/* BRNOMT-90 stubs: the UI tests drive the screens, not the list engine. */
void player_set_resolver(int (*fn)(uint16_t idx, char *path, int cap))
{
    (void)fn;
}

int player_play_list(const uint16_t *idx, int n, int start)
{
    (void)idx;
    (void)n;
    (void)start;
    if (!s_pl_present)
        return PLAYER_ERR_EMPTY;
    s_pl_play_count++;
    s_pl_state = PLAYER_PLAYING;
    return PLAYER_OK;
}

void player_set_shuffle(int on)
{
    (void)on;
}

int player_get_shuffle(void)
{
    return 0;
}

void player_set_repeat(int mode)
{
    (void)mode;
}

int player_get_repeat(void)
{
    return 0;
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

void player_set_fade_ms(int ms)
{
    (void)ms;                       /* no DAC on the host */
}

void player_set_dac_filter(int filter)
{
    s_pl_dac_filter = filter;
}

void player_set_bluetooth_level(int louder)
{
    s_pl_bt_level = louder ? 1 : 0;
}

int mock_player_dac_filter(void)
{
    return s_pl_dac_filter;
}

int mock_player_bt_level(void)
{
    return s_pl_bt_level;
}

void player_set_replaygain_mode(int mode)
{
    s_pl_rg_mode = mode;
}

int player_get_replaygain_mode(void)
{
    return s_pl_rg_mode;
}

int mock_player_replaygain_mode(void)
{
    return s_pl_rg_mode;
}

void player_set_queue_source(int kind, const char *value, int cat)
{
    (void)kind;
    (void)value;
    (void)cat;
}

void player_get_queue_source(int *kind, char *value, int cap, int *cat)
{
    if (kind != NULL)
        *kind = 0;
    if (cat != NULL)
        *cat = 0;
    if (value != NULL && cap > 0)
        value[0] = '\0';
}

int player_list_count(void)
{
    return 0;
}

int player_list_position(void)
{
    return -1;
}

int player_list_title(int index, char *buf, int cap)
{
    (void)index;
    if (buf != NULL && cap > 0)
        buf[0] = '\0';
    return -1;
}

int player_play_at(int index)
{
    (void)index;
    return PLAYER_ERR;
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

/* ---- album art (theme `art` meter, services/art_pack.c) ------------------
 * The UI tests have no ARTPACK: report "no art", so the engine takes the
 * placeholder path. */
#include "../../services/art_pack.h"

uint64_t art_pack_key(const char *path, uint32_t bytes)
{
    (void)path;
    (void)bytes;
    return 0;
}

int art_pack_set_track(const char *path) { (void)path; return -1; }
void art_pack_clear(void) { }
int art_pack_ok(void) { return 0; }
int art_pack_color(int which) { (void)which; return 0; }
int art_pack_dim(int large) { (void)large; return 0; }

int art_pack_draw(int x, int y, int large)
{
    (void)x;
    (void)y;
    (void)large;
    return -1;
}

/* ---- Bluetooth (services/bt_link.c) ------------------------------------
 * The screen is exercised against a scripted policy: no UART, no module. */
#include "../../services/bt_link.h"

static bt_proto_t     s_mock_bt;
static int            s_mock_bt_on;
static int            s_mock_bt_actions;
static int            s_mock_bt_last_device;
static int            s_mock_bt_screen_opens;
static unsigned long  s_mock_now = 1000UL;

void mock_bt_reset(void)
{
    memset(&s_mock_bt, 0, sizeof s_mock_bt);
    s_mock_bt.state = BT_STATE_OFF;
    s_mock_bt.connected = -1;
    s_mock_bt.target = -1;
    s_mock_bt_on = 0;
    s_mock_bt_actions = 0;
    s_mock_bt_last_device = -1;
    s_mock_bt_screen_opens = 0;
    s_mock_now = 1000UL;
}

void mock_bt_set(int on, int state, int count)
{
    s_mock_bt_on = on ? 1 : 0;
    s_mock_bt.state = (bt_state_t)state;
    s_mock_bt.count = (uint8_t)count;
}

void mock_bt_set_device(int index, const char *name)
{
    if (index < 0 || index >= BT_DEV_MAX)
        return;
    snprintf(s_mock_bt.devices[index].name, BT_NAME_MAX, "%s", name);
    s_mock_bt.devices[index].addr[0] = (uint8_t)index;
}

int mock_bt_action_count(void) { return s_mock_bt_actions; }
int mock_bt_last_device(void) { return s_mock_bt_last_device; }
int mock_bt_screen_opens(void) { return s_mock_bt_screen_opens; }

unsigned long log_now_ms(void) { return s_mock_now; }

const bt_proto_t *bt_link_state(void) { return &s_mock_bt; }
int  bt_link_enabled(void) { return s_mock_bt_on; }

void bt_link_set_enabled(int on, unsigned long now)
{
    s_mock_bt_on = on ? 1 : 0;
    s_mock_bt.state = on ? BT_STATE_STARTING : BT_STATE_OFF;
    s_mock_bt_actions++;
    s_mock_now = now;
}

void bt_link_screen_opened(unsigned long now)
{
    s_mock_bt_screen_opens++;
    s_mock_now = now;
}

void bt_link_screen_closed(unsigned long now) { s_mock_now = now; }

void bt_link_scan(unsigned long now)
{
    s_mock_bt_actions++;
    s_mock_bt.state = BT_STATE_SCANNING;
    s_mock_now = now;
}

void bt_link_connect(int device, unsigned long now)
{
    s_mock_bt_actions++;
    s_mock_bt_last_device = device;
    s_mock_bt.state = BT_STATE_CONNECTING;
    s_mock_bt.target = (int8_t)device;
    s_mock_now = now;
}

void bt_link_disconnect(unsigned long now)
{
    s_mock_bt_actions++;
    s_mock_now = now;
}

void bt_link_forget(int device, unsigned long now)
{
    s_mock_bt_actions++;
    s_mock_bt_last_device = device;
    s_mock_now = now;
}

const char *bt_link_status_text(void)
{
    return s_mock_bt_on ? "On" : "Off";
}

/* ---- library extras (favorites / recent / counts) ----------------------
 * The screens only need the calls to exist and to report an empty store. */
#include "../../services/library_extras.h"

void extras_init(void) { }
int  extras_loaded(void) { return 1; }

uint32_t extras_path_hash(const char *path)
{
    uint32_t h = 2166136261u;

    while (path != 0 && *path != '\0') {
        h ^= (uint8_t)*path++;
        h *= 16777619u;
    }
    return h;
}

int      extras_is_favorite(const char *path) { (void)path; return 0; }
int      extras_is_favorite_hash(uint32_t hash) { (void)hash; return 0; }
int      extras_favorite_slot(uint32_t hash) { (void)hash; return -1; }
int      extras_toggle_favorite(const char *path) { (void)path; return 1; }
unsigned extras_favorite_count(void) { return 0; }
uint32_t extras_favorite_hash(unsigned slot) { (void)slot; return 0; }
void     extras_note_play(const char *path) { (void)path; }
int      extras_recent_slot(uint32_t hash) { (void)hash; return -1; }
unsigned extras_recent_count(void) { return 0; }
uint32_t extras_recent_hash(unsigned slot) { (void)slot; return 0; }
unsigned extras_recent_plays(unsigned slot) { (void)slot; return 0; }
unsigned extras_plays_for_hash(uint32_t hash) { (void)hash; return 0; }

unsigned extras_most_played_order(unsigned char *order, unsigned cap)
{
    (void)order;
    (void)cap;
    return 0;
}

/* Audio tap: no PCM ring on the host; the scope idles. */
#include "../../player/ipc_b.h"
int ipc_b_ring_window(const volatile int32_t **ring, uint32_t *rd,
                      uint32_t *mask, int *frames)
{
    (void)ring; (void)rd; (void)mask; (void)frames;
    return 0;
}
