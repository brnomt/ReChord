/*
 * bb_telem.h — Core B cycle counter for the hi-res telemetry (perf/core-b-hires).
 *
 * Core B's own SysTick counts the core clock (200 MHz) down over a 1 ms period
 * and interrupts once per wrap. The interrupt is also what wakes the decoder
 * loop from WFI (BRNOMT-97: B used to spin at full clock when it had nothing
 * to do). The handler (bb_main.c) counts the wraps, so bt_now() is a 32-bit
 * up-counting cycle clock: spans up to ~21 s are exact.
 */
#ifndef RECHORD_BB_TELEM_H
#define RECHORD_BB_TELEM_H

#include <stdint.h>

#define BT_CSR (*(volatile uint32_t *)0xE000E010u)
#define BT_RVR (*(volatile uint32_t *)0xE000E014u)
#define BT_CVR (*(volatile uint32_t *)0xE000E018u)

#define BT_PERIOD 200000u           /* cycles per SysTick wrap: 1 ms @ 200 MHz */

extern volatile uint32_t g_bt_ticks; /* SysTick wraps (bb_main.c handler) */

static inline void bt_init(void)
{
    g_bt_ticks = 0u;
    BT_RVR = BT_PERIOD - 1u;
    BT_CVR = 0u;
    BT_CSR = 7u;                    /* CLKSOURCE | TICKINT | ENABLE */
}

/* Core cycles since bt_init(), modulo 2^32. Re-read if a wrap was counted in
 * between, so the tick count and the down-counter belong together. */
static inline uint32_t bt_now(void)
{
    uint32_t t, c;

    do {
        t = g_bt_ticks;
        c = BT_CVR & 0x00FFFFFFu;
    } while (t != g_bt_ticks);
    return t * BT_PERIOD + (BT_PERIOD - 1u - c);
}

/* Cycles elapsed since `t0` (a bt_now() value). */
static inline uint32_t bt_since(uint32_t t0)
{
    return bt_now() - t0;
}

#endif /* RECHORD_BB_TELEM_H */
