/*
 * output_mem — remember volume and EQ per audio output.
 *
 * The active output is "wired" (headphones/jack) or one Bluetooth device
 * (by address). Each output keeps its own last volume and its own EQ (on/off,
 * profile name and the full curve, so band edits survive). When the output
 * changes the app restores that output's state; when the user changes volume
 * or EQ the active output's entry is updated. An output seen for the first
 * time inherits the current settings (nothing jumps).
 *
 * Storage: /Internal/.rechord/output, one text line per output, oldest first:
 *     o=bt:AABBCCDDEEFF;v=-18;e=1;n=Rock;b=80,70,60 160,140,20 ...
 * The file is preallocated and rewritten in place (fs_store_file). Nothing is
 * kept in RAM beyond the caller's buffer: operations are rare (output change,
 * or a debounced edit), so a 1.8 KB stack buffer replaces permanent .bss.
 *
 * The text handling is pure and host-tested; the file wrappers are thin.
 */
#ifndef RECHORD_SERVICES_OUTPUT_MEM_H
#define RECHORD_SERVICES_OUTPUT_MEM_H

#include <stdint.h>

#include "dsp/eq_bank.h"

#define OM_KEY_MAX      16        /* "bt:AABBCCDDEEFF" + NUL = 16 */
#define OM_NAME_MAX     20        /* matches EQ_PROFILE_NAME_MAX */
#define OM_FILE_MAX     1792      /* working buffer; the file is preallocated larger */
#define OM_MAX_OUTPUTS  8         /* wired + up to 6 Bluetooth devices + spare */

typedef struct {
    int has_vol;
    int vol_db;
    int has_eq;
    int eq_on;
    char name[OM_NAME_MAX];
    eq_curve_t curve;
} om_state_t;

void om_key_wired(char *key);
void om_key_bt(const uint8_t addr[6], char *key);

/* Pure text layer (host-tested). `text` is NUL-terminated. */
int om_find(const char *text, const char *key, om_state_t *out);   /* 1 = found */
/* Insert/replace the entry for `key`, newest last, evicting the oldest lines
 * while it would not fit in `cap`. Returns the new length, or -1. */
int om_upsert(char *text, unsigned cap, const char *key, const om_state_t *st);

/* File wrappers. Return 1 found / 0 not found (load), 0 ok / <0 error (save). */
int om_load(const char *key, om_state_t *out);
int om_save(const char *key, const om_state_t *st);

#endif
