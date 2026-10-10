/*
 * mock_services.h — MOCK service backends for host tests:
 * input queue, log capture, in-memory settings, fake fs_list_dir tree.
 */
#ifndef RECHORD_UI_MOCK_SERVICES_H
#define RECHORD_UI_MOCK_SERVICES_H

#include <stdint.h>

#include "internal/shims/input.h"
#include "internal/shims/fs.h"
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

#endif /* RECHORD_UI_MOCK_SERVICES_H */
