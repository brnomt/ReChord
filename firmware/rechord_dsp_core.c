/*
 * rechord_dsp_core.c — freestanding ReChord DSP core (no SDK deps).
 *
 * Pure per-sample DSP: Q2.30 biquads (Direct Form I), three plugins:
 *   bypass   : passthrough
 *   param_eq : 5-band parametric (low-shelf + 3 peaking + high-shelf)
 *   bass     : single low-shelf bass boost
 *
 * Compiled both for the host (unit tests) and, later, as a position-
 * independent target blob. No SDK includes, no globals from AudioControl.
 */
#include <math.h>

#include "rechord_dsp_core.h"

#define RCH_PI          3.14159265358979323846
#define RCH_MAX_CH      2        /* Echo Mini is stereo */
#define RCH_Q           0.7071   /* peaking Q */
#define RCH_SHELF       1.0      /* shelf slope */

/* Q2.30 fixed point: 2 integer bits, 30 fractional bits (range [-2, 2)). */
#define Q30_ONE         0x40000000   /* 1.0 */

static inline int32_t f2q30(double x)
{
    double v = x * 1073741824.0;      /* 2^30 */
    if (v >  2147483647.0) v =  2147483647.0;
    if (v < -2147483647.0) v = -2147483647.0;
    return (int32_t)v;
}

typedef struct
{
    int32_t b0, b1, b2, a1, a2;   /* normalized by a0, Q2.30 */
    int32_t x1, x2, y1, y2;       /* history (24-bit signal)  */
} Biquad;

static void biquad_set(Biquad *f,
                       double b0, double b1, double b2,
                       double a0, double a1, double a2)
{
    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
    f->b0 = f2q30(b0);
    f->b1 = f2q30(b1);
    f->b2 = f2q30(b2);
    f->a1 = f2q30(a1);
    f->a2 = f2q30(a2);
    f->x1 = f->x2 = f->y1 = f->y2 = 0;
}

static inline int32_t biquad_run(Biquad *f, int32_t x)
{
    int64_t acc;
    int32_t y;

    acc  = (int64_t)f->b0 * x;
    acc += (int64_t)f->b1 * f->x1;
    acc += (int64_t)f->b2 * f->x2;
    acc -= (int64_t)f->a1 * f->y1;
    acc -= (int64_t)f->a2 * f->y2;
    y = (int32_t)(acc >> 30);

    f->x2 = f->x1; f->x1 = x;
    f->y2 = f->y1; f->y1 = y;
    return y;
}

/* RBJ Audio EQ Cookbook (all normalized by a0). */
static void biquad_peaking(Biquad *f, double fs, double f0,
                           double dbgain, double Q)
{
    double A = pow(10.0, dbgain / 40.0);
    double w0 = 2.0 * RCH_PI * f0 / fs;
    double cw = cos(w0);
    double alpha = sin(w0) / (2.0 * Q);
    biquad_set(f, 1.0 + alpha * A, -2.0 * cw, 1.0 - alpha * A,
               1.0 + alpha / A, -2.0 * cw, 1.0 - alpha / A);
}

static void biquad_lowshelf(Biquad *f, double fs, double f0,
                            double dbgain, double S)
{
    double A = pow(10.0, dbgain / 40.0);
    double w0 = 2.0 * RCH_PI * f0 / fs;
    double cw = cos(w0);
    double alpha = sin(w0) / 2.0 * sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);
    double t = 2.0 * sqrt(A) * alpha;
    biquad_set(f,
               A * ((A + 1.0) - (A - 1.0) * cw + t),
               2.0 * A * ((A - 1.0) - (A + 1.0) * cw),
               A * ((A + 1.0) - (A - 1.0) * cw - t),
               (A + 1.0) + (A - 1.0) * cw + t,
               -2.0 * ((A - 1.0) + (A + 1.0) * cw),
               (A + 1.0) + (A - 1.0) * cw - t);
}

static void biquad_highshelf(Biquad *f, double fs, double f0,
                             double dbgain, double S)
{
    double A = pow(10.0, dbgain / 40.0);
    double w0 = 2.0 * RCH_PI * f0 / fs;
    double cw = cos(w0);
    double alpha = sin(w0) / 2.0 * sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);
    double t = 2.0 * sqrt(A) * alpha;
    biquad_set(f,
               A * ((A + 1.0) + (A - 1.0) * cw + t),
               -2.0 * A * ((A - 1.0) + (A + 1.0) * cw),
               A * ((A + 1.0) + (A - 1.0) * cw - t),
               (A + 1.0) - (A - 1.0) * cw + t,
               2.0 * ((A - 1.0) - (A + 1.0) * cw),
               (A + 1.0) - (A - 1.0) * cw - t);
}

/* Standard 5-band graphic-EQ centre frequencies (Hz). */
static const double band_freq[RCH_DSP_BANDS] =
    { 60.0, 250.0, 1000.0, 4000.0, 8000.0 };

#define BASS_GAIN_DB 10.0
#define BASS_FREQ    80.0

static Biquad g_bands[RCH_MAX_CH][RCH_DSP_BANDS];
static int    g_active = 0;
static int    g_channels = 2;
static int32_t g_pre_scale = Q30_ONE;   /* headroom reduction (Q2.30) */

/* Max linear gain any boost can produce, so we pre-scale to prevent clipping.
 * The stock firmware does the same via RockEQReduce9dB (headroom reduction). */
static double max_linear_gain(const rch_dsp_config_t *cfg)
{
    double m = 1.0;
    if (cfg->mode == RCH_DSP_BASS) {
        m = pow(10.0, BASS_GAIN_DB / 20.0);
    } else if (cfg->mode == RCH_DSP_PARAM_EQ) {
        for (int b = 0; b < RCH_DSP_BANDS; b++) {
            double g = pow(10.0, cfg->db_gain[b] / 20.0);
            if (g > m) m = g;
        }
    }
    return m;
}

static void configure_param_eq(const rch_dsp_config_t *cfg)
{
    double fs = (double)cfg->sample_rate;
    int ch, b;
    for (ch = 0; ch < RCH_MAX_CH; ch++) {
        biquad_lowshelf (&g_bands[ch][0], fs, band_freq[0], cfg->db_gain[0], RCH_SHELF);
        biquad_peaking  (&g_bands[ch][1], fs, band_freq[1], cfg->db_gain[1], RCH_Q);
        biquad_peaking  (&g_bands[ch][2], fs, band_freq[2], cfg->db_gain[2], RCH_Q);
        biquad_peaking  (&g_bands[ch][3], fs, band_freq[3], cfg->db_gain[3], RCH_Q);
        biquad_highshelf(&g_bands[ch][4], fs, band_freq[4], cfg->db_gain[4], RCH_SHELF);
    }
    (void)b;
}

static void configure_bass(const rch_dsp_config_t *cfg)
{
    double fs = (double)cfg->sample_rate;
    int ch;
    for (ch = 0; ch < RCH_MAX_CH; ch++) {
        /* Only the low-shelf band is used; the rest are identity. */
        biquad_lowshelf(&g_bands[ch][0], fs, BASS_FREQ, BASS_GAIN_DB, RCH_SHELF);
        biquad_peaking(&g_bands[ch][1], fs, 1000.0, 0.0, RCH_Q);
        biquad_peaking(&g_bands[ch][2], fs, 1000.0, 0.0, RCH_Q);
        biquad_peaking(&g_bands[ch][3], fs, 1000.0, 0.0, RCH_Q);
        biquad_highshelf(&g_bands[ch][4], fs, 1000.0, 0.0, RCH_SHELF);
    }
}

int rch_dsp_configure(const rch_dsp_config_t *cfg)
{
    if (cfg == 0 || cfg->sample_rate == 0)
        return -1;

    g_channels = (cfg->channels >= 1 && cfg->channels <= RCH_MAX_CH)
                   ? cfg->channels : 2;

    switch (cfg->mode) {
    case RCH_DSP_BYPASS:
        g_active = 0;
        g_pre_scale = Q30_ONE;
        break;
    case RCH_DSP_PARAM_EQ:
        configure_param_eq(cfg);
        g_active = 1;
        g_pre_scale = f2q30(0.9 / max_linear_gain(cfg));
        break;
    case RCH_DSP_BASS:
        configure_bass(cfg);
        g_active = 1;
        g_pre_scale = f2q30(0.9 / max_linear_gain(cfg));
        break;
    default:
        return -1;
    }
    return 0;
}

void rch_dsp_reset(void)
{
    int ch, b;
    for (ch = 0; ch < RCH_MAX_CH; ch++)
        for (b = 0; b < RCH_DSP_BANDS; b++)
            g_bands[ch][b].x1 = g_bands[ch][b].x2 =
            g_bands[ch][b].y1 = g_bands[ch][b].y2 = 0;
}

int rch_dsp_process(int32_t *buf, int samples)
{
    int ch, b, i;

    if (!g_active)
        return 0;
    if (buf == 0 || samples <= 0)
        return -1;

    for (ch = 0; ch < g_channels; ch++) {
        Biquad *bands = g_bands[ch];
        for (i = ch; i < samples; i += g_channels) {
            int64_t y = ((int64_t)buf[i] * g_pre_scale) >> 30;  /* headroom */
            for (b = 0; b < RCH_DSP_BANDS; b++)
                y = biquad_run(&bands[b], (int32_t)y);
            buf[i] = (int32_t)y;
        }
    }
    return 0;
}
