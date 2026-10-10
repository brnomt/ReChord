/*
 * mod_volume.c — volume/gain effect module with soft ramp (no clicks).
 *
 * Requirements (from the DAP UX):
 *   - gain changes must never click: the gain is RAMPED sample-by-sample
 *     from its current value to the target over VOL_RAMP_MS, so a volume
 *     keypress is a smooth glide;
 *   - no overshoot: the ramp approaches the target monotonically and clamps
 *     exactly at it — a step never rings past the final level;
 *   - no wrap-around: the result is saturated to int16 (dsp_sat.h), so a
 *     boost over full scale pins at the rail instead of flipping sign.
 *
 * Note on "saturate helpers from the core": rechord_dsp_core.h exports no
 * saturate helpers (its fixed-point helpers are file-static and the core
 * files must not change), so the module uses the dsp/ layer's own dsp_sat.h.
 * This module is stateless w.r.t. filters, so unlike mod_eq/mod_bass it needs
 * no core instantiation — several volume instances can coexist (preamp +
 * master), which the chain tests rely on.
 */
#include <stdint.h>
#include <limits.h>
#include <math.h>

#include "dsp_module.h"
#include "dsp_sat.h"

#define VOL_GAIN_MIN_TDB  -600   /* -60.0 dB (below: silence)             */
#define VOL_GAIN_MAX_TDB    60   /*  +6.0 dB (int16 Q30 range tops at 2x) */
#define VOL_RAMP_MAX_MS   1000
#define VOL_Q30_ONE   0x40000000 /* 1.0 in Q2.30 (same format as core)    */

typedef struct
{
    int32_t g_cur;             /* current gain, Q2.30                     */
    int32_t g_target;          /* target gain, Q2.30                      */
    int32_t g_step;            /* per-sample step toward target, Q2.30    */
    int     gain_tenth_db;
    int     ramp_ms;
    int     sample_rate;
} mod_volume_state_t;

static int32_t f2q30(double x)
{
    double v = x * 1073741824.0;      /* 2^30, same format as the core */
    if (v >  2147483647.0) v =  2147483647.0;
    if (v < -2147483647.0) v = -2147483647.0;
    return (int32_t)v;
}

/* (Re)start the ramp from g_cur toward g_target. */
static void mod_volume_arm_ramp(mod_volume_state_t *st)
{
    long ramp_samples;

    if (st->sample_rate <= 0 || st->ramp_ms <= 0) {
        st->g_step = 0;
        st->g_cur = st->g_target;      /* instant mode (tests, seek, etc.) */
        return;
    }

    ramp_samples = (long)st->sample_rate * st->ramp_ms / 1000;
    if (ramp_samples < 1)
        ramp_samples = 1;

    st->g_step = (int32_t)((st->g_target - st->g_cur) / (int32_t)ramp_samples);
    if (st->g_step == 0)
        st->g_cur = st->g_target;      /* sub-LSB change: snap, don't creep */
}

/* Advance one sample: step toward target, clamping exactly at it. */
static inline void mod_volume_advance(mod_volume_state_t *st)
{
    if (st->g_step == 0)
        return;
    st->g_cur += st->g_step;
    if ((st->g_step > 0 && st->g_cur > st->g_target) ||
        (st->g_step < 0 && st->g_cur < st->g_target)) {
        st->g_cur = st->g_target;      /* no overshoot */
        st->g_step = 0;
    }
}

static int mod_volume_init(void *state, int sample_rate)
{
    mod_volume_state_t *st = (mod_volume_state_t *)state;

    if (st == 0 || sample_rate <= 0)
        return -1;

    st->g_cur = st->g_target = VOL_Q30_ONE;
    st->g_step = 0;
    st->gain_tenth_db = 0;
    st->ramp_ms = 20;                  /* short default: smooth, still fast */
    st->sample_rate = sample_rate;
    return 0;
}

static int mod_volume_process(void *state, int16_t *samples, int n)
{
    mod_volume_state_t *st = (mod_volume_state_t *)state;
    int i;

    if (st == 0 || samples == 0)
        return -1;

    for (i = 0; i < n; i++) {
        int32_t y;
        mod_volume_advance(st);
        y = (int32_t)(((int64_t)samples[i] * st->g_cur) >> 30);
        samples[i] = dsp_sat16(y);
    }
    return 0;
}

static int mod_volume_set_param(void *state, int param, int value)
{
    mod_volume_state_t *st = (mod_volume_state_t *)state;

    if (st == 0)
        return -1;
    switch (param) {
    case MOD_VOLUME_GAIN:
        if (value < VOL_GAIN_MIN_TDB || value > VOL_GAIN_MAX_TDB)
            return -2;
        if (st->gain_tenth_db == value)
            return 0;
        st->gain_tenth_db = value;
        st->g_target = f2q30(pow(10.0, value / 200.0));   /* tenths dB */
        mod_volume_arm_ramp(st);
        return 0;
    case MOD_VOLUME_RAMP_MS:
        if (value < 0 || value > VOL_RAMP_MAX_MS)
            return -2;
        if (st->ramp_ms == value)
            return 0;
        st->ramp_ms = value;
        mod_volume_arm_ramp(st);
        return 0;
    default:
        return -1;
    }
}

static int mod_volume_get_param(void *state, int param)
{
    mod_volume_state_t *st = (mod_volume_state_t *)state;

    if (st == 0)
        return INT_MIN;
    switch (param) {
    case MOD_VOLUME_GAIN:
        return st->gain_tenth_db;
    case MOD_VOLUME_RAMP_MS:
        return st->ramp_ms;
    default:
        return INT_MIN;
    }
}

const dsp_module_t dsp_mod_volume = {
    "volume",
    mod_volume_init,
    mod_volume_process,
    mod_volume_set_param,
    mod_volume_get_param,
    (int)sizeof(mod_volume_state_t)
};
