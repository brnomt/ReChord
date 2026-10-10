/*
 * mod_eq.c — 3-band parametric EQ effect module (low shelf / mid peaking /
 * high shelf) built on the ReChord DSP core.
 *
 * ==== WHY THE CORE IS COMPILED INTO THIS FILE ============================
 * rechord_dsp_core.c is a STATEFUL SINGLETON: its filter bank lives in file
 * globals and its API (rch_dsp_configure/process/reset) can represent exactly
 * one active configuration. The effect-module interface needs per-instance
 * state so several effects can coexist in one chain (eq + bass + volume) —
 * and the core files must not be modified (the root Makefile uses them).
 *
 * Solution: each core-backed module instantiates the core's OWN biquad
 * engine into its translation unit (the entry points renamed below) and runs
 * a per-instance filter bank stored in the module state. All filter math —
 * Q2.30 Direct Form I, RBJ cookbook coefficients, band frequencies, the
 * 0.9/max-gain headroom rule — still comes verbatim from rechord_dsp_core.c:
 * one source of truth, bit-exact with the core, no duplicated formulas.
 *
 * The module state is therefore: user gains + per-instance bank + headroom.
 * =========================================================================
 */
#include <stdint.h>
#include <limits.h>
#include <math.h>

#include "dsp_module.h"
#include "dsp_sat.h"

/* Private instantiation of the core (renamed entry points; see header). */
#define rch_dsp_configure mod_eq_core_configure
#define rch_dsp_process   mod_eq_core_process
#define rch_dsp_reset     mod_eq_core_reset
#include "../rechord_dsp_core.c"

/*
 * The core's own config glue writes the core's singleton globals; this
 * module runs its per-instance bank instead. Reference those statics so the
 * inclusion stays warning-clean under -Wall -Werror without patching the
 * core (unused-static warnings would otherwise fire).
 */
static inline void mod_eq_core_keepalive(void)
{
    (void)configure_param_eq;
    (void)configure_bass;
    (void)rch_dsp_process;
}

#define MOD_EQ_MAX_TDB 200        /* +/- 20.0 dB in tenths of dB          */
#define MOD_EQ_CHUNK   64         /* int16 samples per stack temp buffer  */

typedef struct
{
    int     gain_tenth_db[MOD_EQ_PARAM_COUNT];
    int     sample_rate;
    Biquad  bank[RCH_MAX_CH][RCH_DSP_BANDS];   /* per-instance filter state */
    int32_t pre_scale;                          /* headroom, Q2.30          */
} mod_eq_state_t;

/*
 * (Re)compute the bank from the user gains. Maps the 3 UI bands onto the
 * core's 5-band grid: LOW -> low shelf 60 Hz, MID -> peaking 1 kHz,
 * HIGH -> high shelf 8 kHz, bands 1/3 left at 0 dB (exact identity).
 */
static void mod_eq_apply(mod_eq_state_t *st)
{
    rch_dsp_config_t cfg;
    double fs = (double)st->sample_rate;
    int ch, b;

    for (b = 0; b < RCH_DSP_BANDS; b++)
        cfg.db_gain[b] = 0.0;
    cfg.mode = RCH_DSP_PARAM_EQ;
    cfg.sample_rate = (unsigned long)st->sample_rate;
    cfg.channels = RCH_MAX_CH;
    cfg.db_gain[0] = st->gain_tenth_db[MOD_EQ_LOW]  / 10.0;
    cfg.db_gain[2] = st->gain_tenth_db[MOD_EQ_MID]  / 10.0;
    cfg.db_gain[4] = st->gain_tenth_db[MOD_EQ_HIGH] / 10.0;

    for (ch = 0; ch < RCH_MAX_CH; ch++) {
        biquad_lowshelf (&st->bank[ch][0], fs, band_freq[0], cfg.db_gain[0], RCH_SHELF);
        biquad_peaking  (&st->bank[ch][1], fs, band_freq[1], cfg.db_gain[1], RCH_Q);
        biquad_peaking  (&st->bank[ch][2], fs, band_freq[2], cfg.db_gain[2], RCH_Q);
        biquad_peaking  (&st->bank[ch][3], fs, band_freq[3], cfg.db_gain[3], RCH_Q);
        biquad_highshelf(&st->bank[ch][4], fs, band_freq[4], cfg.db_gain[4], RCH_SHELF);
    }

    /* Same headroom rule as the core: pre-scale so the maximum possible
     * boost cannot clip the 24-bit path (stock RockEQReduce9dB behavior). */
    st->pre_scale = f2q30(0.9 / max_linear_gain(&cfg));
}

static int mod_eq_init(void *state, int sample_rate)
{
    mod_eq_state_t *st = (mod_eq_state_t *)state;
    int i;

    if (st == 0 || sample_rate <= 0)
        return -1;

    for (i = 0; i < MOD_EQ_PARAM_COUNT; i++)
        st->gain_tenth_db[i] = 0;
    st->sample_rate = sample_rate;
    mod_eq_apply(st);
    mod_eq_core_keepalive();
    return 0;
}

static int mod_eq_process(void *state, int16_t *samples, int n)
{
    mod_eq_state_t *st = (mod_eq_state_t *)state;
    int32_t tmp[MOD_EQ_CHUNK];
    int off, i, ch, b;

    if (st == 0 || samples == 0)
        return -1;
    if (n <= 0)
        return 0;

    /* Chunked so the stack cost is fixed (Cortex-M) regardless of block
     * size; filter history lives in st->bank and flows across chunks. */
    for (off = 0; off < n; off += MOD_EQ_CHUNK) {
        int m = n - off;
        if (m > MOD_EQ_CHUNK)
            m = MOD_EQ_CHUNK;

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

static int mod_eq_set_param(void *state, int param, int value)
{
    mod_eq_state_t *st = (mod_eq_state_t *)state;

    if (st == 0)
        return -1;
    switch (param) {
    case MOD_EQ_LOW:
    case MOD_EQ_MID:
    case MOD_EQ_HIGH:
        if (value < -MOD_EQ_MAX_TDB || value > MOD_EQ_MAX_TDB)
            return -2;
        if (st->gain_tenth_db[param] == value)
            return 0;   /* no-op: avoids needlessly resetting filter state */
        st->gain_tenth_db[param] = value;
        mod_eq_apply(st);   /* note: recomputing coefficients clears history */
        return 0;
    default:
        return -1;
    }
}

static int mod_eq_get_param(void *state, int param)
{
    mod_eq_state_t *st = (mod_eq_state_t *)state;

    if (st == 0)
        return INT_MIN;
    switch (param) {
    case MOD_EQ_LOW:
    case MOD_EQ_MID:
    case MOD_EQ_HIGH:
        return st->gain_tenth_db[param];
    default:
        return INT_MIN;
    }
}

const dsp_module_t dsp_mod_eq = {
    "eq",
    mod_eq_init,
    mod_eq_process,
    mod_eq_set_param,
    mod_eq_get_param,
    (int)sizeof(mod_eq_state_t)
};
