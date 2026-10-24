/*
 * eq_bank.c — runtime-configurable EQ bank for the Core B PCM block.
 *
 * The filter math (Q2.30 Direct Form I biquads, RBJ cookbook) comes from
 * rechord_dsp_core.c, included privately with its entry points renamed
 * exactly like mod_eq.c does, so there is one source of truth for the
 * coefficients and no symbol clash with the SDK/core build.
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "eq_bank.h"

/* Private instantiation of the core (renamed entry points). */
#define rch_dsp_configure eq_bank_core_configure
#define rch_dsp_process   eq_bank_core_process
#define rch_dsp_reset     eq_bank_core_reset
#include "../rechord_dsp_core.c"

/* Reference the core glue the renamed entry points do not reach, so the
 * inclusion stays warning-clean under -Wall -Werror (same trick as mod_eq). */
static inline void eq_bank_core_keepalive(void)
{
    (void)configure_param_eq;
    (void)configure_bass;
    (void)rch_dsp_process;
}

#define EQ_BANK_CHANNELS 2   /* Core B upmixes to stereo before this runs */

typedef struct
{
    Biquad     bank[EQ_BANK_CHANNELS][EQ_BANDS_MAX];
    eq_curve_t curve;
    int32_t    pre_scale;                    /* headroom, Q2.30           */
    int        sample_rate;
    uint8_t    band_active[EQ_BANDS_MAX];    /* 0 = skipped (gain 0/Nyq)  */
} eq_bank_impl_t;

_Static_assert(sizeof(eq_bank_impl_t) <= EQ_BANK_STORAGE_BYTES,
               "EQ_BANK_STORAGE_BYTES is too small");

static eq_bank_impl_t *impl_of(eq_bank_state_t *st)
{
    return (eq_bank_impl_t *)(void *)st->bytes;
}

static const eq_bank_impl_t *cimpl_of(const eq_bank_state_t *st)
{
    return (const eq_bank_impl_t *)(const void *)st->bytes;
}

/* A band only costs CPU when it actually changes the signal and its centre
 * lies below Nyquist (8 kHz streams skip the top bands instead of failing). */
static int band_designable(const eq_band_t *b, int sample_rate)
{
    if (b->gain_tdb == 0)
        return 0;
    return (long)b->freq_hz * 2L < (long)sample_rate;
}

static void band_design(Biquad *f, const eq_band_t *b, int index, int count,
                        int sample_rate)
{
    double fs = (double)sample_rate;
    double f0 = (double)b->freq_hz;
    double q  = (double)b->q_x100 / 100.0;
    double g  = (double)b->gain_tdb / 10.0;

    if (index == 0)
        biquad_lowshelf(f, fs, f0, g, q);
    else if (index == count - 1)
        biquad_highshelf(f, fs, f0, g, q);
    else
        biquad_peaking(f, fs, f0, g, q);
}

static void copy_coeffs(Biquad *dst, const Biquad *src)
{
    dst->b0 = src->b0;
    dst->b1 = src->b1;
    dst->b2 = src->b2;
    dst->a1 = src->a1;
    dst->a2 = src->a2;
}

/* Max linear boost the curve can produce; only positive bands count. */
static double curve_max_gain(const eq_curve_t *c, int sample_rate)
{
    double m = 1.0;
    int i;

    for (i = 0; i < c->count; i++) {
        double g;

        if (c->band[i].gain_tdb <= 0 || !band_designable(&c->band[i], sample_rate))
            continue;
        g = pow(10.0, (double)c->band[i].gain_tdb / 200.0);
        if (g > m)
            m = g;
    }
    return m;
}

int eq_bank_init(eq_bank_state_t *st, int sample_rate)
{
    eq_bank_impl_t *s;

    if (st == 0 || sample_rate <= 0)
        return -1;

    memset(st->bytes, 0, sizeof st->bytes);
    s = impl_of(st);
    s->sample_rate = sample_rate;
    s->pre_scale = Q30_ONE;
    s->curve.count = 0;
    s->curve.enabled = 0;
    eq_bank_core_keepalive();
    return 0;
}

int eq_bank_set_rate(eq_bank_state_t *st, int sample_rate)
{
    eq_bank_impl_t *s;

    if (st == 0 || sample_rate <= 0)
        return -1;

    s = impl_of(st);
    if (s->sample_rate == sample_rate)
        return 0;
    s->sample_rate = sample_rate;

    if (s->curve.count == 0)
        return 0;
    if (eq_bank_apply(st, &s->curve) != 0)
        return -1;
    eq_bank_reset(st);
    return 0;
}

int eq_bank_apply(eq_bank_state_t *st, const eq_curve_t *curve)
{
    eq_bank_impl_t *s;
    int was_on, now_on, i, ch;

    if (st == 0 || curve == 0)
        return -1;
    if (!eq_curve_valid(curve))
        return -1;

    s = impl_of(st);
    was_on = s->curve.enabled && s->curve.count > 0;
    now_on = curve->enabled && curve->count > 0;

    for (i = 0; i < curve->count; i++) {
        int live = band_designable(&curve->band[i], s->sample_rate);
        int same;

        if (!live) {
            s->band_active[i] = 0;
            continue;
        }

        /* Same band geometry -> keep the history (gain-only moves must not
         * click). A count change moves the shelf positions, so it resets. */
        same = was_on && s->curve.count == curve->count &&
               s->band_active[i] &&
               s->curve.band[i].freq_hz == curve->band[i].freq_hz &&
               s->curve.band[i].q_x100 == curve->band[i].q_x100;

        {
            Biquad nb;

            band_design(&nb, &curve->band[i], i, curve->count, s->sample_rate);
            for (ch = 0; ch < EQ_BANK_CHANNELS; ch++) {
                if (same)
                    copy_coeffs(&s->bank[ch][i], &nb);
                else
                    s->bank[ch][i] = nb;
            }
        }
        s->band_active[i] = 1;
    }
    for (i = curve->count; i < EQ_BANDS_MAX; i++)
        s->band_active[i] = 0;

    s->curve = *curve;

    /* Unity unless a boost needs headroom; a cut-only curve stays 1.0. */
    {
        double m = curve_max_gain(curve, s->sample_rate);

        s->pre_scale = (m > 1.0) ? f2q30(0.9 / m) : Q30_ONE;
    }

    if (now_on && !was_on)
        eq_bank_reset(st);      /* history from a bypassed stream is stale */

    return 0;
}

void eq_bank_reset(eq_bank_state_t *st)
{
    eq_bank_impl_t *s;
    int ch, b;

    if (st == 0)
        return;

    s = impl_of(st);
    for (ch = 0; ch < EQ_BANK_CHANNELS; ch++) {
        for (b = 0; b < EQ_BANDS_MAX; b++) {
            s->bank[ch][b].x1 = 0;
            s->bank[ch][b].x2 = 0;
            s->bank[ch][b].y1 = 0;
            s->bank[ch][b].y2 = 0;
        }
    }
}

void eq_bank_process(eq_bank_state_t *st, int32_t *pcm, int n)
{
    eq_bank_impl_t *s;
    int ch, b, i;

    if (st == 0 || pcm == 0 || n <= 0)
        return;

    s = impl_of(st);
    if (!s->curve.enabled || s->curve.count == 0)
        return;

    for (ch = 0; ch < EQ_BANK_CHANNELS; ch++) {
        Biquad *bands = s->bank[ch];

        for (i = ch; i < n; i += EQ_BANK_CHANNELS) {
            int64_t y = ((int64_t)pcm[i] * s->pre_scale) >> 30;

            for (b = 0; b < s->curve.count; b++) {
                if (s->band_active[b])
                    y = biquad_run(&bands[b], (int32_t)y);
            }
            pcm[i] = (int32_t)y;
        }
    }
}

const eq_curve_t *eq_bank_curve(const eq_bank_state_t *st)
{
    return (st != 0) ? &cimpl_of(st)->curve : 0;
}

int eq_bank_is_enabled(const eq_bank_state_t *st)
{
    if (st == 0)
        return 0;
    return cimpl_of(st)->curve.enabled != 0;
}
