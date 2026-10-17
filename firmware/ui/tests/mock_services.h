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
void mock_services_reset(void);   /* input queue + log + settings + fs tree */
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
int         mock_player_volume_db(void);   /* last player_set_volume() dB   */
/* scripted view of the player state */
void        mock_player_set_state(int state);
void        mock_player_set_position(long ms);
void        mock_player_set_track_present(int present); /* get_track OK?     */
/* tags player_get_track() reports (Now Playing artist/album line); empty
 * strings = untagged. */
void        mock_player_set_tags(const char *artist, const char *album);
/* scripted player_read_meta() responses for the browser's visible page:
 * register the tags of one path (unknown paths report PLAYER_ERR). */
void        mock_player_set_meta(const char *path, const char *title,
                                 const char *artist);

#endif /* RECHORD_UI_MOCK_SERVICES_H */
