/*
 * mock_services.h — MOCK service backends for host tests:
 * input queue, log capture, in-memory settings, fake fs_list_dir tree.
 */
#ifndef RECHORD_UI_MOCK_SERVICES_H
#define RECHORD_UI_MOCK_SERVICES_H

#include <stdint.h>

#include "internal/shims/input.h"
#include "internal/shims/fs.h"
#include "internal/shims/player.h"
#include "settings_schema.h"

/* ---- input ------------------------------------------------------------- */
void mock_services_reset(void);

/* Battery level reported to the skin bridge (default 76 %). */
void mock_battery_set(int pct);   /* input queue + log + settings + fs tree */
void mock_input_push(input_key_t key, uint16_t adc);
int  mock_input_pending(void);

/* ---- log --------------------------------------------------------------- */
int         mock_log_count(void);
const char *mock_log_last(void);

/* ---- settings ---------------------------------------------------------- */
void mock_settings_reset(void);
void mock_settings_set(const rechord_settings_t *s); /* preset for load    */
void mock_settings_get(rechord_settings_t *s);       /* last saved copy    */
int  mock_settings_save_count(void);
void mock_settings_set_load_rc(int rc);              /* load() return code */

/* ---- Bluetooth (services/bt_link.c) ------------------------------------ */
void mock_bt_reset(void);
void mock_bt_set(int on, int state, int count);
void mock_bt_set_device(int index, const char *name);
int  mock_bt_action_count(void);
int  mock_bt_last_device(void);
int  mock_bt_screen_opens(void);

/* ---- fs ---------------------------------------------------------------- */
#define MOCK_FS_MAX_DIRS    8
#define MOCK_FS_MAX_ENTRIES 64
#define MOCK_FS_MAX_CALLS   64

void mock_fs_reset(void);
void mock_fs_add_dir(const char *path);   /* register an empty listing     */
/* append one entry to `path`'s listing; returns 0 on success */
int  mock_fs_add_entry(const char *path, const char *name, int is_dir);

int         mock_fs_call_count(void);
uint32_t    mock_fs_call_index(int i);    /* start index of i-th call      */
const char *mock_fs_call_path(int i);

/* ---- player (browser / player_screen link seam) ------------------------- */
void        mock_player_reset(void);
/* scripted player_open() result (default PLAYER_OK) */
void        mock_player_set_open_rc(int rc);
int         mock_player_open_count(void);
const char *mock_player_open_path(void);   /* last player_open() path       */
int         mock_player_play_count(void);
int         mock_player_pause_count(void);
int         mock_player_next_count(void);
int         mock_player_prev_count(void);
int         mock_player_seek_count(void);
long        mock_player_last_seek_ms(void);
int         mock_player_volume_db(void);   /* last player_set_volume() dB   */
int         mock_player_dac_filter(void);  /* last player_set_dac_filter()  */
int         mock_player_bt_level(void);    /* last player_set_bluetooth_... */
int         mock_player_replaygain_mode(void); /* last RG mode applied      */
/* scripted view of the player state */
void        mock_player_set_state(int state);
void        mock_player_set_position(long ms);
void        mock_player_set_track_present(int present); /* get_track OK?     */

#endif /* RECHORD_UI_MOCK_SERVICES_H */
