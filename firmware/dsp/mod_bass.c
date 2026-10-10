/*
 * mod_bass.c — bass-boost effect module built on the ReChord DSP core.
 *
 * Same per-instance instantiation scheme as mod_eq.c (see its header for the
 * full WHY): the core is a stateful singleton and must not be modified, so
 * this module compiles the core's biquad engine into its own TU and keeps the
 * filter bank in the module state. All coefficient math is the core's.
 *
 * Tuning: single low shelf at the core's bass frequency (BASS_FREQ, 80 Hz,
 * RCH_SHELF slope) with a USER-ADJUSTABLE boost. The core's dedicated bass
 * plugin hardcodes +10 dB; going through the shelf band keeps that tuning
 * while letting the user dial the boost (the point of the module system).
 */
#include <stdint.h>
#include <limits.h>
#include <math.h>

#include "dsp_module.h"
#include "dsp_sat.h"

/* Private instantiation of the core (renamed entry points; see header). */
#define rch_dsp_configure mod_bass_core_configure
#define rch_dsp_process   mod_bass_core_process
#define rch_dsp_reset     mod_bass_core_reset
#include "../rechord_dsp_core.c"

/* See mod_eq.c: keep the core's singleton glue referenced so the inclusion
 * is warning-clean without patching the core. */
static inline void mod_bass_core_keepalive(void)
{
    (void)configure_param_eq;
    (void)configure_bass;
    (void)rch_dsp_process;
}

#define MOD_BASS_MAX_TDB 150      /* +15.0 dB ceiling in tenths of dB     */
#define MOD_BASS_CHUNK   64

typedef struct
{
    int     boost_tenth_db;
    int     sample_rate;
    Biquad  bank[RCH_MAX_CH][RCH_DSP_BANDS];
    int32_t pre_scale;
} mod_bass_state_t;

/* Recompute the bank: low shelf at BASS_FREQ with the user boost, followed
 * by the core's identity bands (mirrors the core's configure_bass layout). */
static void mod_bass_apply(mod_bass_state_t *st)
{
    rch_dsp_config_t cfg;
    double fs = (double)st->sample_rate;
    double gain_db = st->boost_tenth_db / 10.0;
    int ch, b;

    for (b = 0; b < RCH_DSP_BANDS; b++)
        cfg.db_gain[b] = 0.0;
    cfg.mode = RCH_DSP_PARAM_EQ;
    cfg.sample_rate = (unsigned long)st->sample_rate;
    cfg.channels = RCH_MAX_CH;
    cfg.db_gain[0] = gain_db;

    for (ch = 0; ch < RCH_MAX_CH; ch++) {
        biquad_lowshelf (&st->bank[ch][0], fs, BASS_FREQ, gain_db, RCH_SHELF);
        biquad_peaking  (&st->bank[ch][1], fs, 1000.0, 0.0, RCH_Q);
        biquad_peaking  (&st->bank[ch][2], fs, 1000.0, 0.0, RCH_Q);
        biquad_peaking  (&st->bank[ch][3], fs, 1000.0, 0.0, RCH_Q);
        biquad_highshelf(&st->bank[ch][4], fs, 1000.0, 0.0, RCH_SHELF);
    }

    /* Core headroom rule: pre-scale by 0.9/max-boost so boost cannot clip. */
    st->pre_scale = f2q30(0.9 / max_linear_gain(&cfg));
}

static int mod_bass_init(void *state, int sample_rate)
{
    mod_bass_state_t *st = (mod_bass_state_t *)state;

    if (st == 0 || sample_rate <= 0)
        return -1;

    st->boost_tenth_db = (int)(BASS_GAIN_DB * 10.0);   /* core default */
    st->sample_rate = sample_rate;
    mod_bass_apply(st);
    mod_bass_core_keepalive();
    return 0;
}

static int mod_bass_process(void *state, int16_t *samples, int n)
{
    mod_bass_state_t *st = (mod_bass_state_t *)state;
    int32_t tmp[MOD_BASS_CHUNK];
    int off, i, ch, b;

    if (st == 0 || samples == 0)
        return -1;
    if (n <= 0)
        return 0;

    for (off = 0; off < n; off += MOD_BASS_CHUNK) {
        int m = n - off;
        if (m > MOD_BASS_CHUNK)
            m = MOD_BASS_CHUNK;

        for (i = 0; i < m; i++)
            tmp[i] = dsp_i16_to_i24(samples[off + i]);

        for (ch = 0; ch < RCH_MAX_CH; ch++) {
            for (i = ch; i < m; i += RCH_MAX_CH) {
                int64_t y = ((int64_t)tmp[i] * st->pre_scale) >> 30;
                for (b = 0; b < RCH_DSP_BANDS; b++)
                    y = biquad_run(&st->bank[ch][b], (int32_t)y);
                tmp[i] = (int32_t)y;
            }
        }

        for (i = 0; i < m; i++)
            samples[off + i] = dsp_i24_to_i16(tmp[i]);
    }
    return 0;
}

static int mod_bass_set_param(void *state, int param, int value)
{
    mod_bass_state_t *st = (mod_bass_state_t *)state;

    if (st == 0)
        return -1;
    switch (param) {
    case MOD_BASS_BOOST:
        if (value < 0 || value > MOD_BASS_MAX_TDB)
            return -2;
        if (st->boost_tenth_db == value)
            return 0;
        st->boost_tenth_db = value;
        mod_bass_apply(st);
        return 0;
    default:
        return -1;
    }
}

static int mod_bass_get_param(void *state, int param)
{
    mod_bass_state_t *st = (mod_bass_state_t *)state;

    if (st == 0)
        return INT_MIN;
    switch (param) {
    case MOD_BASS_BOOST:
        return st->boost_tenth_db;
    default:
        return INT_MIN;
    }
}

const dsp_module_t dsp_mod_bass = {
    "bass",
    mod_bass_init,
    mod_bass_process,
    mod_bass_set_param,
    mod_bass_get_param,
    (int)sizeof(mod_bass_state_t)
};
