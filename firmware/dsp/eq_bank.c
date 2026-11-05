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
    uint8_t    boosting;                     /* soft knee on the output   */
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

/* RBJ shelf from a true Q (Equalizer APO's LSC/HSC): alpha = sin(w0)/(2Q).
 * The core's shelves take the slope S instead, which the positional layout
 * (and every saved curve before BRNOMT-77) relies on. */
static void shelf_q(Biquad *f, double fs, double f0, double dbgain, double q,
                    int high)
{
    double A = pow(10.0, dbgain / 40.0);
    double w0 = 2.0 * RCH_PI * f0 / fs;
    double cw = cos(w0);
    double t = 2.0 * sqrt(A) * (sin(w0) / (2.0 * q));
    double s = high ? -1.0 : 1.0;   /* high shelf: mirror the cos terms */

    biquad_set(f,
               A * ((A + 1.0) - s * (A - 1.0) * cw + t),
               s * 2.0 * A * ((A - 1.0) - s * (A + 1.0) * cw),
               A * ((A + 1.0) - s * (A - 1.0) * cw - t),
               (A + 1.0) + s * (A - 1.0) * cw + t,
               -s * 2.0 * ((A - 1.0) + s * (A + 1.0) * cw),
               (A + 1.0) + s * (A - 1.0) * cw - t);
}

static void band_design(Biquad *f, const eq_band_t *b, int index, int count,
                        int sample_rate)
{
    double fs = (double)sample_rate;
    double f0 = (double)b->freq_hz;
    double q  = (double)b->q_x100 / 100.0;
    double g  = (double)b->gain_tdb / 10.0;

    switch (b->type) {
    case EQ_TYPE_PEAK:
        biquad_peaking(f, fs, f0, g, q);
        return;
    case EQ_TYPE_LSHELF:
        shelf_q(f, fs, f0, g, q, 0);
        return;
    case EQ_TYPE_HSHELF:
        shelf_q(f, fs, f0, g, q, 1);
        return;
    default:
        break;
    }
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

/* Largest boost of the WHOLE cascade over 20 Hz..Nyquist-ish, evaluated from
 * the quantised coefficients that actually run, plus its geometric mean over a
 * log-frequency grid (music has roughly equal energy per octave, so the mean
 * tracks how much louder the curve makes a track). Overlapping bands add up
 * (two neighbouring +4 dB bands make ~+7 dB between them), which a per-band
 * maximum misses. Amplitudes are linear: Core B's libm has no log. */
#define EQ_PEAK_GRID 48

static double curve_response(const eq_bank_impl_t *s, double *mean_amp)
{
    double nyq = (double)s->sample_rate * 0.5;
    double top = nyq * 0.9 < 20000.0 ? nyq * 0.9 : 20000.0;
    double peak = 1.0, prod = 1.0;
    int k, b;

    for (k = 0; k < EQ_PEAK_GRID; k++) {
        double f = 20.0 * pow(top / 20.0, (double)k / (EQ_PEAK_GRID - 1));
        double w = 2.0 * RCH_PI * f / (double)s->sample_rate;
        double c1 = cos(w), s1 = sin(w), c2 = cos(2.0 * w), s2 = sin(2.0 * w);
        double pw = 1.0;                       /* cascade |H|^2 */

        for (b = 0; b < s->curve.count; b++) {
            const Biquad *q = &s->bank[0][b];
            double b0, b1, b2, a1, a2, nr, ni, dr, di;

            if (!s->band_active[b])
                continue;
            b0 = q->b0 / 1073741824.0;
            b1 = q->b1 / 1073741824.0;
            b2 = q->b2 / 1073741824.0;
            a1 = q->a1 / 1073741824.0;
            a2 = q->a2 / 1073741824.0;
            nr = b0 + b1 * c1 + b2 * c2;
            ni = b1 * s1 + b2 * s2;
            dr = 1.0 + a1 * c1 + a2 * c2;
            di = a1 * s1 + a2 * s2;
            pw *= (nr * nr + ni * ni) / (dr * dr + di * di);
        }
        pw = sqrt(pw);
        prod *= pw;
        if (pw > peak)
            peak = pw;
    }
    *mean_amp = pow(prod, 1.0 / EQ_PEAK_GRID);
    return peak;
}

/* Output soft knee: linear to 0.8 FS, then a smooth curve that reaches full
 * scale with zero slope. Keeps a boosted peak from clipping harshly. */
static int32_t soft_knee(int32_t v)
{
    const int64_t T = 1717986918LL;           /* 0.8 FS            */
    const int64_t F = 2147483647LL;
    const int64_t R = F - T;
    int64_t a = v < 0 ? -(int64_t)v : (int64_t)v;
    int64_t e, y;

    if (a <= T)
        return v;
    e = a - T;
    y = (e >= 2 * R) ? F : T + e - (e * e) / (4 * R);
    return (int32_t)(v < 0 ? -y : y);
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
               s->curve.band[i].q_x100 == curve->band[i].q_x100 &&
               s->curve.band[i].type == curve->band[i].type;

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

    /* Preamp: keep loudness roughly where it was instead of pulling the whole
     * track down by the biggest boost. Back off the mean boost (what the ear
     * hears as "louder") but never less than peak^0.4 (40 % of it in dB), and
     * let the soft knee absorb the rest. Cut-only curves stay at unity. */
    {
        double mean = 1.0;
        double pk = curve_response(s, &mean);
        double floor_amp = pow(pk, 0.4);
        double back = mean > floor_amp ? mean : floor_amp;

        s->boosting = pk > 1.0058;               /* > 0.05 dB */
        s->pre_scale = s->boosting ? f2q30(1.0 / back) : Q30_ONE;
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

/* One band over one channel of an interleaved stereo block (stride 2). The
 * coefficients and the four history words live in locals for the whole block,
 * and a1/a2 are negated up front so every term is a multiply-accumulate
 * (SMLAL) instead of a multiply-subtract. Bit-identical to biquad_run(): same
 * 64-bit accumulate, same >>30 and same saturation.
 *
 * On Cortex-M3 the loop needs 14 live values (5 coeffs, 4 history, 64-bit
 * accumulator, pointer, end, input) and GCC spilled the coefficients and the
 * counter to the stack: ~8 extra RAM accesses per band-sample, measured at
 * ~86 cycles per band-sample on Core B. The Thumb-2 version below keeps all
 * of them in r0-r12 + lr. The C body stays the reference (host tests) and
 * tools/eq_asm_check.py runs both on an emulated M3 and compares them. */
#if defined(__thumb2__) && !defined(EQ_BANK_NO_ASM)
/* Biquad: b0 b1 b2 a1 a2 @0..16, x1 x2 y1 y2 @20..32. */
__attribute__((naked, noinline, used))
static void band_block(Biquad *f, int32_t *p, int frames)
{
    __asm__ volatile(
        "push   {r0, r4-r11, lr}        \n"   /* r0 = f, kept for the end  */
        "mov    lr, r0                  \n"
        "add    r12, r1, r2, lsl #3     \n"   /* end = p + frames * 2      */
        "mov    r0, r1                  \n"   /* r0 = p                    */
        "mov    r1, r12                 \n"   /* r1 = end                  */
        "ldr    r2, [lr, #0]            \n"   /* b0                        */
        "ldr    r3, [lr, #4]            \n"   /* b1                        */
        "ldr    r4, [lr, #8]            \n"   /* b2                        */
        "ldr    r5, [lr, #12]           \n"
        "rsb    r5, r5, #0              \n"   /* -a1                       */
        "ldr    r6, [lr, #16]           \n"
        "rsb    r6, r6, #0              \n"   /* -a2                       */
        "ldr    r7, [lr, #20]           \n"   /* x1                        */
        "ldr    r8, [lr, #24]           \n"   /* x2                        */
        "ldr    r9, [lr, #28]           \n"   /* y1                        */
        "ldr    r10, [lr, #32]          \n"   /* y2                        */
        "cmp    r0, r1                  \n"
        "bhs    3f                      \n"   /* frames <= 0               */
    "1:                                 \n"
        "ldr    lr, [r0]                \n"   /* x                         */
        "smull  r11, r12, r2, lr        \n"   /* acc  = b0 * x             */
        "smlal  r11, r12, r3, r7        \n"   /* acc += b1 * x1            */
        "smlal  r11, r12, r4, r8        \n"   /* acc += b2 * x2            */
        "smlal  r11, r12, r5, r9        \n"   /* acc -= a1 * y1            */
        "smlal  r11, r12, r6, r10       \n"   /* acc -= a2 * y2            */
        "mov    r8, r7                  \n"   /* x2 = x1                   */
        "mov    r7, lr                  \n"   /* x1 = x                    */
        "mov    r10, r9                 \n"   /* y2 = y1                   */
        "lsr    r9, r11, #30            \n"   /* y = low word of acc >> 30 */
        "orr    r9, r9, r12, lsl #2     \n"
        "asr    lr, r12, #30            \n"   /* high word of acc >> 30    */
        "cmp    lr, r9, asr #31         \n"   /* fits in int32?            */
        "bne    4f                      \n"
    "2:                                 \n"
        "cmp    r9, #0x80000000         \n"   /* C clamps at -(2^31 - 1)   */
        "it     eq                      \n"
        "addeq  r9, r9, #1              \n"
        "str    r9, [r0], #8            \n"   /* *p = y; p += 2            */
        "cmp    r0, r1                  \n"
        "blo    1b                      \n"
    "3:                                 \n"
        "ldr    lr, [sp]                \n"   /* f                         */
        "str    r7, [lr, #20]           \n"
        "str    r8, [lr, #24]           \n"
        "str    r9, [lr, #28]           \n"
        "str    r10, [lr, #32]          \n"
        "pop    {r0, r4-r11, pc}        \n"
    "4:                                 \n"   /* overflow: saturate by sign */
        "asr    lr, r12, #31            \n"
        "eor    r9, lr, #0x80000000     \n"
        "mvn    r9, r9                  \n"   /* +: 0x7FFFFFFF, -: 0x80000000 */
        "b      2b                      \n");
}
#else
static void band_block(Biquad *f, int32_t *p, int frames)
{
    const int32_t b0 = f->b0, b1 = f->b1, b2 = f->b2;
    const int32_t na1 = -f->a1, na2 = -f->a2;   /* coeffs are clamped, no wrap */
    int32_t x1 = f->x1, x2 = f->x2, y1 = f->y1, y2 = f->y2;

    while (frames-- > 0) {
        int32_t x = *p;
        int64_t acc = (int64_t)b0 * x;
        int64_t t;
        int32_t y;

        acc += (int64_t)b1 * x1;
        acc += (int64_t)b2 * x2;
        acc += (int64_t)na1 * y1;
        acc += (int64_t)na2 * y2;
        t = acc >> 30;
        if (t > 2147483647LL)
            t = 2147483647LL;
        else if (t < -2147483647LL)
            t = -2147483647LL;
        y = (int32_t)t;
        x2 = x1; x1 = x;
        y2 = y1; y1 = y;
        *p = y;
        p += 2;
    }
    f->x1 = x1; f->x2 = x2; f->y1 = y1; f->y2 = y2;
}
#endif

/* Band-major: each active band runs over the whole block before the next one.
 * The bands form a causal cascade per channel, so this gives the same samples
 * as running every band per sample, but keeps a band's state in registers
 * instead of reloading nine words from RAM for every sample. */
void eq_bank_process(eq_bank_state_t *st, int32_t *pcm, int n)
{
    eq_bank_impl_t *s;
    int ch, b, i, frames;

    if (st == 0 || pcm == 0 || n <= 0)
        return;

    s = impl_of(st);
    if (!s->curve.enabled || s->curve.count == 0)
        return;

    frames = n / EQ_BANK_CHANNELS;
    for (ch = 0; ch < EQ_BANK_CHANNELS; ch++) {
        Biquad *bands = s->bank[ch];
        int32_t *p = pcm + ch;

        for (i = 0; i < frames; i++)
            p[i * EQ_BANK_CHANNELS] = (int32_t)(
                ((int64_t)p[i * EQ_BANK_CHANNELS] * s->pre_scale) >> 30);
        for (b = 0; b < s->curve.count; b++) {
            if (s->band_active[b])
                band_block(&bands[b], p, frames);
        }
        if (s->boosting) {
            for (i = 0; i < frames; i++)
                p[i * EQ_BANK_CHANNELS] =
                    soft_knee(p[i * EQ_BANK_CHANNELS]);
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
