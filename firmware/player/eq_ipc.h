/*
 * eq_ipc.h — AP <-> Core B EQ curve transport (BRNOMT-76).
 *
 * A single-producer/single-consumer seqlock over the shared control block:
 * the AP marks the generation odd, writes the whole curve, then publishes
 * the next even generation. B reads only even generations and re-checks the
 * generation after copying, so it never installs a half-written curve.
 *
 * The helpers are static inline so the target code and the host tests use
 * the exact same packing logic. Band word layout:
 *
 *   bits  0..14  centre frequency, Hz (<= 20000, fits 15 bits)
 *   bits 15..23  Q * 100 (10..400, fits 9 bits)
 *   bits 24..31  gain in tenths of dB, signed (fits int8)
 */
#ifndef RECHORD_EQ_IPC_H
#define RECHORD_EQ_IPC_H

#include <stdint.h>

#include "dsp/eq_bank.h"
#include "ipc_shared.h"

#if defined(__arm__) || defined(__ARM_ARCH)
#define EQ_IPC_BARRIER() __asm__ volatile("dmb" ::: "memory")
#else
#define EQ_IPC_BARRIER() __asm__ volatile("" ::: "memory")
#endif

static inline uint32_t eq_ipc_pack_band(const eq_band_t *b)
{
    return ((uint32_t)b->freq_hz & 0x7FFFu) |
           (((uint32_t)b->q_x100 & 0x1FFu) << 15) |
           (((uint32_t)(uint8_t)(int8_t)b->gain_tdb) << 24);
}

static inline void eq_ipc_unpack_band(uint32_t w, eq_band_t *b)
{
    b->freq_hz = (uint16_t)(w & 0x7FFFu);
    b->q_x100 = (uint16_t)((w >> 15) & 0x1FFu);
    b->gain_tdb = (int16_t)(int8_t)((w >> 24) & 0xFFu);
}

/* AP side: publish `c` atomically. */
static inline void eq_ipc_publish(volatile uint32_t *sh, const eq_curve_t *c)
{
    uint32_t gen = sh[IPC_C_EQ_GEN];
    int i;

    sh[IPC_C_EQ_GEN] = gen | 1u;         /* odd: update in progress */
    EQ_IPC_BARRIER();
    sh[IPC_C_EQ_EN] = c->enabled ? 1u : 0u;
    sh[IPC_C_EQ_N] = c->count;
    for (i = 0; i < EQ_BANDS_MAX; i++) {
        eq_band_t b;

        if (i < c->count)
            b = c->band[i];
        else {
            b.freq_hz = 0;
            b.q_x100 = 0;
            b.gain_tdb = 0;
        }
        sh[IPC_C_EQ_BASE + i] = eq_ipc_pack_band(&b);
    }
    EQ_IPC_BARRIER();
    sh[IPC_C_EQ_GEN] = (gen | 1u) + 1u;  /* even again: stable */
}

/*
 * B side: 1 = a new stable curve was copied into `out`, 0 = nothing new (or
 * a writer is mid-update), -1 = malformed count (marked seen, so it is not
 * retried every block). `last_gen` is the consumer's own memory.
 */
static inline int eq_ipc_read(const volatile uint32_t *sh, uint32_t *last_gen,
                              eq_curve_t *out)
{
    uint32_t gen = sh[IPC_C_EQ_GEN];
    eq_curve_t c;
    int i;

    if ((gen & 1u) != 0u || gen == *last_gen)
        return 0;

    EQ_IPC_BARRIER();
    c.enabled = sh[IPC_C_EQ_EN] ? 1u : 0u;
    c.count = (uint8_t)sh[IPC_C_EQ_N];

    if (c.count > EQ_BANDS_MAX) {
        *last_gen = gen;
        return -1;
    }
    for (i = 0; i < EQ_BANDS_MAX; i++) {
        if (i < c.count) {
            eq_ipc_unpack_band(sh[IPC_C_EQ_BASE + i], &c.band[i]);
        } else {
            c.band[i].freq_hz = 0;
            c.band[i].q_x100 = 0;
            c.band[i].gain_tdb = 0;
        }
    }
    EQ_IPC_BARRIER();

    if (sh[IPC_C_EQ_GEN] != gen)
        return 0;                        /* torn: retry on the next block */

    *last_gen = gen;
    *out = c;
    return 1;
}

#endif /* RECHORD_EQ_IPC_H */
