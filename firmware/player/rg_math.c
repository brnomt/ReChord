/*
 * rg_math.c — ReplayGain dB->linear gain (host + target).
 *
 * Converts a dB*10 gain and a Q1.15 peak fraction into a Q1.15 fixed-point
 * linear multiplier applied to int16 PCM (32768 == 1.0x), with peak-limited
 * clipping so the (known) peak never exceeds full scale. The public names
 * keep the historical "_q16" suffix for call-site compatibility; values are
 * Q1.15.
 *
 * Host builds link libm; target builds that lack libm can swap this TU for a
 * fixed-point LUT without touching the player (see docs/rewrite/audio.md).
 */
#include "rg_math.h"

#include <math.h>

/* dB*10 -> linear Q1.15 (32768 = 1.0x). Positive dB boosts, negative attenuates. */
int32_t rg_gain_q16(int gain_tenth_db)
{
    double db = (double)gain_tenth_db / 10.0;
    double lin = pow(10.0, db / 20.0);
    double q15 = lin * 32768.0;
    if (q15 < 0.0) q15 = 0.0;
    if (q15 > 2147483647.0) q15 = 2147483647.0;
    return (int32_t)q15;
}

/* Given a dB*10 gain and a Q1.15 peak (32768 = 1.0), return the effective Q1.15
 * gain: min(dB->linear, 1/peak) to avoid clipping a full-scale signal of this
 * track. peak<=0 => no peak info => use the dB gain straight. */
int32_t rg_effective_gain_q16(int gain_tenth_db, int peak_q15)
{
    int32_t db_gain = rg_gain_q16(gain_tenth_db);
    if (peak_q15 <= 0)
        return db_gain;
    {
        /* 1/peak (Q1.15) = 32768 * 32768 / peak_q15. */
        int64_t peak_limit = (int64_t)32768 * 32768 / (int64_t)peak_q15;
        if (peak_limit > 2147483647)
            peak_limit = 2147483647;
        if (db_gain < 0) db_gain = 0;
        if ((int64_t)db_gain > peak_limit)
            db_gain = (int32_t)peak_limit;
        return db_gain;
    }
}

/* Scale `n` interleaved int16 samples by `gain_q15` (Q1.15, 32768 = 1.0x, stored
 * in the int32_t arg historically named gain_q16), in place, saturating. */
void rg_scale_pcm(int16_t *pcm, long n, int32_t gain_q15)
{
    long i;
    if (gain_q15 <= 0) {
        for (i = 0; i < n; i++)
            pcm[i] = 0;
        return;
    }
    for (i = 0; i < n; i++) {
        int64_t v = ((int64_t)pcm[i] * (int64_t)gain_q15) >> 15;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        pcm[i] = (int16_t)v;
    }
}
