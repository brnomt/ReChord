/*
 * core_b.h — Core B (CAL_M3) start, BRNOMT-68 phase 1.
 *
 * Starts our tiny image on the second Cortex-M3 and polls its heartbeat in
 * the shared block (0x01020000). Failure is non-blocking: the caller keeps
 * running and logs/does nothing — the audio path is untouched.
 */
#ifndef RECHORD_CORE_B_H
#define RECHORD_CORE_B_H

#include <stdint.h>

/* 0 = Core B wrote BB_MAGIC ('BRDY'); -1 = timeout/too big. */
int ap_core_b_start(void);

/* Last value observed at shared[0] (diagnostics). */
uint32_t ap_core_b_magic(void);

#endif /* RECHORD_CORE_B_H */
