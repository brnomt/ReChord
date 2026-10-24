/*
 * eq_bank.h — runtime-configurable graphic/parametric EQ bank (product path).
 *
 * Unlike the int16 dsp_module_t chain (firmware/dsp/mod_*.c), this engine
 * processes the PCM format Core B actually produces: interleaved stereo
 * int32, LEFT-JUSTIFIED 24-bit (see firmware/player/codec.h). Converting to
 * int16 first would throw away the low 8 bits of every 24-bit sample.
 *
 * The curve describes 5..10 bands in ascending frequency order:
 *   band 0            -> low shelf
 *   band count-1      -> high shelf
 *   every middle band -> peaking
 * Filters use the proven RBJ/Q2.30 biquad math of rechord_dsp_core.c (each
 * translation unit includes its own private copy, exactly like mod_eq.c).
 *
 * Zero-gain bands are not instantiated at all: a flat curve is a bit-exact
 * pass-through and costs no CPU. Bands whose centre frequency is at or above
 * the stream's Nyquist are ignored (8 kHz streams still work).
 */
#ifndef EQ_BANK_H
#define EQ_BANK_H

#include <stdint.h>

#define EQ_BANDS_MIN 5
#define EQ_BANDS_MAX 10

#define EQ_FREQ_MIN_HZ 20
#define EQ_FREQ_MAX_HZ 20000
#define EQ_Q_MIN_X100  10
#define EQ_Q_MAX_X100  400
#define EQ_GAIN_MAX_TDB 120      /* +/- 12.0 dB */

typedef struct
{
    uint16_t freq_hz;
    uint16_t q_x100;             /* Q * 100 (70 = 0.70)                 */
    int16_t  gain_tdb;           /* tenths of dB, -120..+120            */
} eq_band_t;

typedef struct
{
    uint8_t  count;              /* 5..10                               */
    uint8_t  enabled;            /* 0 = bypass                          */
    eq_band_t band[EQ_BANDS_MAX];
} eq_curve_t;

/*
 * Per-instance state, caller-owned (no heap anywhere). The public type is
 * an aligned byte buffer; eq_bank.c asserts the real struct fits.
 */
#define EQ_BANK_STORAGE_BYTES 896
typedef union
{
    double  _align_d;
    void   *_align_p;
    int64_t _align_i;
    uint8_t bytes[EQ_BANK_STORAGE_BYTES];
} eq_bank_state_t;

/* Zero the state and set the stream rate. Returns 0, or < 0 on bad args. */
int eq_bank_init(eq_bank_state_t *st, int sample_rate);

/* Change the stream rate (track change), re-designing the live curve.
 * Filter history is cleared. Returns 0, or < 0 when the curve no longer
 * validates at the new rate. */
int eq_bank_set_rate(eq_bank_state_t *st, int sample_rate);

/*
 * Validate and install a curve. Validation is structural (count, ranges,
 * strictly increasing frequencies) and atomic: on error the previous curve
 * and coefficients are left untouched. If the effect was bypassed, turning
 * it on clears history; a topology change (count/freq/Q) clears only the
 * affected bands' history, so gain-only moves do not click.
 * Returns 0, or < 0 when the curve is rejected.
 */
int eq_bank_apply(eq_bank_state_t *st, const eq_curve_t *curve);

/* Structural validation only (no state). 1 = valid.
 * Header-inline so the AP (profile parser / IPC publisher) can validate a
 * curve without linking the filter engine — only Core B runs it. */
static inline int eq_curve_valid(const eq_curve_t *c)
{
    int i;

    if (c == 0)
        return 0;
    if (c->enabled > 1)
        return 0;
    if (c->count < EQ_BANDS_MIN || c->count > EQ_BANDS_MAX)
        return 0;

    for (i = 0; i < c->count; i++) {
        const eq_band_t *b = &c->band[i];

        if (b->freq_hz < EQ_FREQ_MIN_HZ || b->freq_hz > EQ_FREQ_MAX_HZ)
            return 0;
        if (b->q_x100 < EQ_Q_MIN_X100 || b->q_x100 > EQ_Q_MAX_X100)
            return 0;
        if (b->gain_tdb < -EQ_GAIN_MAX_TDB || b->gain_tdb > EQ_GAIN_MAX_TDB)
            return 0;
        if (i > 0 && b->freq_hz <= c->band[i - 1].freq_hz)
            return 0;
    }
    return 1;
}

/* Reset every filter history (track change / seek). */
void eq_bank_reset(eq_bank_state_t *st);

/* Process `n` interleaved int32 values in place (frames * channels). */
void eq_bank_process(eq_bank_state_t *st, int32_t *pcm, int n);

/* Introspection (the UI/AP keeps its own copy; useful in tests). */
const eq_curve_t *eq_bank_curve(const eq_bank_state_t *st);
int eq_bank_is_enabled(const eq_bank_state_t *st);

#endif /* EQ_BANK_H */
