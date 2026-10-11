/*
 * rknanoc.h -- tiny SoC helpers shared by the polled drivers: CRU clock
 * gates / soft resets (reference 0x030513e0 / 0x03051430) and a busy delay.
 */
#ifndef RECHORD_RKNANOC_H
#define RECHORD_RKNANOC_H

#include <stdint.h>

#define RK_CRU ((volatile uint32_t *)0x40180000u)
#define RK_GRF ((volatile uint32_t *)0x50010000u)

/* Write-masked gate registers at CRU+0x80: on = clear the gate bit. */
static inline void cru_gate(unsigned id, int on)
{
    unsigned bit = id & 15u;
    RK_CRU[0x20 + (id >> 4)] = (1u << (bit + 16)) | (on ? 0u : 1u << bit);
}

/* Write-masked soft-reset registers at CRU+0xC0. */
static inline void cru_reset(unsigned id, int assert)
{
    unsigned bit = id & 15u;
    RK_CRU[0x30 + (id >> 4)] = (1u << (bit + 16)) | ((assert ? 1u : 0u) << bit);
}

/* ponytail: uncalibrated busy loop (>= 1 us at the 24 MHz boot clock);
 * callers use it for timeouts/settling, where longer is safe. */
static inline void rk_delay_us(uint32_t us)
{
    volatile uint32_t n = us * 8u;
    while (n--) { }
}

#endif
