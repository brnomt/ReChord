/*
 * bb_telem.h — Core B cycle counter for the hi-res telemetry (perf/core-b-hires).
 *
 * Core B's own SysTick, free-running (no interrupt), counts the core clock
 * down from 0xFFFFFF. Spans must be shorter than 2^24 cycles (~84 ms at
 * 200 MHz): a decode round or a ring wait is far below that; a pathological
 * wait that exceeds it only under-reports. If the core had no SysTick the
 * counter would read 0 and every cycle figure would stay 0 (harmless).
 */
#ifndef RECHORD_BB_TELEM_H
#define RECHORD_BB_TELEM_H

#include <stdint.h>

#define BT_CSR (*(volatile uint32_t *)0xE000E010u)
#define BT_RVR (*(volatile uint32_t *)0xE000E014u)
#define BT_CVR (*(volatile uint32_t *)0xE000E018u)

static inline void bt_init(void)
{
    BT_RVR = 0x00FFFFFFu;
    BT_CVR = 0u;
    BT_CSR = 5u;                    /* CLKSOURCE | ENABLE, no TICKINT */
}

static inline uint32_t bt_now(void)
{
    return BT_CVR & 0x00FFFFFFu;
}

/* Cycles elapsed since `t0` (a bt_now() value). */
static inline uint32_t bt_since(uint32_t t0)
{
    return (t0 - bt_now()) & 0x00FFFFFFu;
}

#endif /* RECHORD_BB_TELEM_H */
