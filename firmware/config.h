/*
 * config.h — shared ReChord build configuration (single source of truth).
 *
 * Included by BOTH the AP (fw1) and BB (section_3) builds so they agree on
 * version, feature flags, and protocol headers without duplicating values.
 * Freestanding (no SDK typedefs); safe to include from bare-metal code.
 *
 * Version constants live in rechord_version.h; this header aggregates them
 * and the build-wide feature toggles.
 */
#ifndef RECHORD_CONFIG_H
#define RECHORD_CONFIG_H

#include "rechord_version.h"

/* ------------------------------------------------------------------ */
/* Feature toggles (build-wide; mirrored to the compiler via Makefile) */
/* ------------------------------------------------------------------ */

/* Modular from-source DSP (replaces RkNano_EQ .lib). */
#ifndef RECHORD_DSP_ENABLE
#define RECHORD_DSP_ENABLE      1
#endif

/* Whether the running half is the AP (UI) or BB (audio). Set by the
 * Makefile via -DRECHORD_AP_BUILD / -DRECHORD_BB_BUILD, not here. */
#if defined(RECHORD_AP_BUILD) && defined(RECHORD_BB_BUILD)
#error "RECHORD_AP_BUILD and RECHORD_BB_BUILD are mutually exclusive"
#endif

/* ------------------------------------------------------------------ */
/* Shared protocol — include so both halves use the same mailbox IDs.  */
/* ------------------------------------------------------------------ */
#include "ipc.h"

#endif /* RECHORD_CONFIG_H */
