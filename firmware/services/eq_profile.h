/*
 * eq_profile.h — EQ profiles for BRNOMT-76 (AP side).
 *
 * The device keeps one ACTIVE curve in /Internal/.rechord/eq.cfg and up to
 * eight user slots in /Internal/.rechord/eq/customN.cfg. Built-in presets
 * (Flat, Rock, Jazz, EDM, Vocal) live in flash. Profiles are plain
 * key=value text, editable from a PC with no recompilation:
 *
 *     format=1
 *     name=Rock
 *     band0=31,70,+40        # freq_hz, Q*100, gain in tenths of dB
 *     ...
 *     band9=16000,70,-10
 *
 * The name is a UI label only: Core B receives just the curve (bands, Q and
 * gains) through the shared block (eq_ipc.h).
 *
 * Only the AP parses files; the filter engine runs on Core B. The publish
 * seam keeps this service host-testable without the player/IPC stack.
 */
#ifndef RECHORD_SERVICES_EQ_PROFILE_H
#define RECHORD_SERVICES_EQ_PROFILE_H

#include <stdint.h>

#include "dsp/eq_bank.h"

#define EQ_PROFILE_NAME_MAX 20
#define EQ_PROFILE_SLOTS    8     /* customN.cfg, N = 1..8 */

/* Publish callback the app wires to ipc_b_set_eq(); tests record calls. */
typedef void (*eq_publish_fn)(int enabled, const eq_curve_t *curve);

/* Load the active curve + scan the custom slots, then publish. Safe to call
 * on every boot; a missing/rotten file leaves the defaults in place. */
int eq_profile_boot(void);

/* 1 once boot() has run (the screen lazily boots on a direct entry). */
int eq_profile_ready(void);

void eq_profile_set_publisher(eq_publish_fn fn);

/* Re-send the active curve/enable flag (no file I/O). */
int eq_profile_apply(void);

/* Persist the active curve to /Internal/.rechord/eq.cfg. */
int eq_profile_save(void);

/* Rows the picker shows: 5 built-ins followed by every present custom slot. */
int eq_profile_count(void);
const char *eq_profile_entry_name(int index);
int eq_profile_entry_is_custom(int index);

/* Load row `index` into the active curve (built-in or custom file), persist
 * the selection and publish it. Returns 0, or < 0 when the file is bad. */
int eq_profile_select(int index);

/* Store the ACTIVE curve into the custom row `index` (>= 5), using that
 * slot's current label, then persist and publish. Returns 0, or < 0. */
int eq_profile_save_to_row(int index);

/* Re-scan /Internal/.rechord/eq for customN.cfg slots. */
int eq_profile_scan(void);

const eq_curve_t *eq_profile_curve(void);
const char *eq_profile_name(void);
int eq_profile_enabled(void);
int eq_profile_is_dirty(void);

void eq_profile_set_enabled(int on);

/* Band edit: `gain_tdb` is clamped to +/-12.0 dB. */
void eq_profile_set_band_gain(int band, int gain_tdb);

/* Picker/UI helper: +/- 0.5 dB, clamped. */
void eq_profile_cycle_band(int band, int dir);

/* Format tenths of dB as "+3.5" / "-12.0" / "0.0". Returns the length. */
int eq_profile_format_gain(int gain_tdb, char *buf, unsigned cap);

/* Pure parser/serializer (host-tested, no I/O). */
int eq_profile_parse(const char *text, int len, eq_curve_t *out,
                     char *name, unsigned name_cap);
int eq_profile_format(const eq_curve_t *curve, const char *name,
                      char *buf, unsigned cap);

#endif /* RECHORD_SERVICES_EQ_PROFILE_H */
