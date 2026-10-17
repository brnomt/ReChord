/*
 * rg_math.c — ReplayGain dB->linear gain (host + target).
 *
 * Converts a dB*10 gain and a Q1.15 peak fraction into a Q1.15 fixed-point
 * linear multiplier applied to the int32 left-justified PCM (32768 == 1.0x),
 * with peak-limited clipping so the (known) peak never exceeds full scale.
 * The public names keep the historical "_q16" suffix for call-site
 * compatibility; values are Q1.15.
 *
 * FIXED-POINT ONLY (bare-metal requirement): no floats, no libm, no malloc.
 * dB->linear is table-based: integer-dB and 0.1-dB Q1.15 factors multiply
 * out to Q1.15. Exact at integer-dB points (tests pin 0 / +-20 / -60 dB);
 * within one rounding step (<=0.01 dB) elsewhere. Range: -100 dB (clamped
 * at 0) to +24.9 dB (clamped).
 */
#include "rg_math.h"

/* round(32768 * 10^(-d/20)), d = 0..100 dB */
static const uint32_t ao_atten_db_q15[101] = {
    32768, 29205, 26029, 23198, 20675, 18427, 16423, 14637,
    13045, 11627, 10362, 9235, 8231, 7336, 6538, 5827,
    5193, 4629, 4125, 3677, 3277, 2920, 2603, 2320,
    2068, 1843, 1642, 1464, 1305, 1163, 1036, 924,
    823, 734, 654, 583, 519, 463, 413, 368,
    328, 292, 260, 232, 207, 184, 164, 146,
    130, 116, 104, 92, 82, 73, 65, 58,
    52, 46, 41, 37, 33, 29, 26, 23,
    21, 18, 16, 15, 13, 12, 10, 9,
    8, 7, 7, 6, 5, 5, 4, 4,
    3, 3, 3, 2, 2, 2, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
};

/* round(32768 * 10^(d/20)), d = 0..24 dB (24.9 dB max boost) */
static const uint32_t ao_boost_db_q15[25] = {
    32768, 36766, 41252, 46286, 51934, 58271, 65381, 73358,
    82309, 92353, 103622, 116265, 130452, 146369, 164229, 184268,
    206752, 231980, 260285, 292045, 327680, 367663, 412525, 462860,
    519338,
};

/* round(32768 * 10^(-t/200)) / round(32768 * 10^(t/200)), t = 0..9 tenths */
static const uint32_t ao_atten_tenth_q15[10] = {
    32768, 32393, 32022, 31656, 31293, 30935, 30581, 30231, 29885, 29543,
};
static const uint32_t ao_boost_tenth_q15[10] = {
    32768, 33147, 33531, 33920, 34312, 34710, 35112, 35518, 35929, 36345,
};

/* dB*10 -> linear Q1.15 (32768 = 1.0x). Positive dB boosts, negative attenuates. */
int32_t rg_gain_q16(int gain_tenth_db)
{
    int neg = (gain_tenth_db < 0);
    int mag = neg ? -gain_tenth_db : gain_tenth_db;
    uint32_t db_q15, tenth_q15, gain_q15;
    int d, t;

    if (mag > 1009)
        mag = 1009;                      /* -100.9 .. +100.9 dB input    */
    d = mag / 10;
    t = mag % 10;
    if (neg) {
        if (d > 100)
            d = 100;                     /* deep attenuation floors to 0  */
        db_q15 = ao_atten_db_q15[d];
        tenth_q15 = ao_atten_tenth_q15[t];
    } else {
        if (d > 24) {                    /* boost clamps at +24.9 dB      */
            d = 24;
            t = 9;
        }
        db_q15 = ao_boost_db_q15[d];
        tenth_q15 = ao_boost_tenth_q15[t];
    }
    gain_q15 = (uint32_t)(((uint64_t)db_q15 * (uint64_t)tenth_q15) >> 15);
    return (int32_t)gain_q15;
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

/* Scale `n` interleaved int32 samples (left-justified pipeline format) by
 * `gain_q15` (Q1.15, 32768 = 1.0x, stored in the int32_t arg historically
 * named gain_q16), in place, saturating to the int32 range. */
void rg_scale_pcm(int32_t *pcm, long n, int32_t gain_q15)
{
    long i;
    if (gain_q15 <= 0) {
        for (i = 0; i < n; i++)
            pcm[i] = 0;
        return;
    }
    for (i = 0; i < n; i++) {
        int64_t v = ((int64_t)pcm[i] * (int64_t)gain_q15) >> 15;
        if (v > 2147483647LL) v = 2147483647LL;
        if (v < -2147483648LL) v = -2147483648LL;
        pcm[i] = (int32_t)v;
    }
}
