/*
 * rechord_dsp.c — from-source modular DSP framework (ReChord).
 *
 * Replaces RkNano_EQ_24BIT_20150630.lib (a 31-band fixed-point graphic EQ)
 * with a compile-time plugin framework. The per-sample path is integer
 * (Q2.30 biquad coefficients, Direct Form I, 64-bit MAC); coefficient math
 * runs once per preset change in double (soft-float is fine at that rate).
 *
 *   bypass   : passthrough                        (EQ_NOR)
 *   param_eq : 5-band parametric EQ               (ROCK/POP/JAZZ/CLASS/
 *                                                   HEAVY/UNIQUE/USER)
 *   bass     : low-shelf bass boost               (EQ_BASS)
 *
 * The SDK's RKEffect is the parameter contract: Mode selects the plugin,
 * and for EQ_USER the 5 gains come from RKCoef.dbGain[5] in "dB+12" form
 * (12 == flat), i.e. gain_dB = dbGain[i] - 12.
 */
#include <math.h>

#include "SysInclude.h"
#include "audio_globals.h"
#include "AudioControl.h"
#include "rechord_dsp.h"

/* AudioIOBuf / pAudioRegKey / IsEQNOR are defined by AudioControl.c. */

#define RCH_PI          3.14159265358979323846
#define RCH_DSP_MAX_CH  2        /* Echo Mini is stereo */

/* Q2.30 fixed point: 2 integer bits, 30 fractional bits (range [-2, 2)). */
#define Q30_ONE         0x40000000   /* 1.0 */
static inline int32_t f2q30(double x)
{
    double v = x * 1073741824.0;      /* 2^30 */
    if (v >  2147483647.0) v =  2147483647.0;
    if (v < -2147483647.0) v = -2147483647.0;
    return (int32_t)v;
}

/* ------------------------------------------------------------------ */
/* Biquad (Direct Form I, Q2.30 coefficients, Q0 24-bit signal).       */
/* ------------------------------------------------------------------ */
typedef struct
{
    int32_t b0, b1, b2, a1, a2;   /* normalized by a0, Q2.30 */
    int32_t x1, x2, y1, y2;       /* history (24-bit signal)  */
} BiquadSection;

static void biquad_set(BiquadSection *f,
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

static inline int32_t biquad_run(BiquadSection *f, int32_t x)
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

/* RBJ Audio EQ Cookbook coefficient forms (all normalized by a0). */
static void biquad_peaking(BiquadSection *f, double fs, double f0,
                           double dbgain, double Q)
{
    double A = pow(10.0, dbgain / 40.0);
    double w0 = 2.0 * RCH_PI * f0 / fs;
    double cw = cos(w0);
    double alpha = sin(w0) / (2.0 * Q);
    biquad_set(f,
               1.0 + alpha * A,
               -2.0 * cw,
               1.0 - alpha * A,
               1.0 + alpha / A,
               -2.0 * cw,
               1.0 - alpha / A);
}

static void biquad_lowshelf(BiquadSection *f, double fs, double f0,
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

static void biquad_highshelf(BiquadSection *f, double fs, double f0,
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

/* ------------------------------------------------------------------ */
/* param_eq: 5-band parametric EQ.                                     */
/* ------------------------------------------------------------------ */
#define EQ_Q     0.7071
#define EQ_SHELF 1.0

/* Standard 5-band graphic-EQ centre frequencies (Hz). */
static const double g_band_freq[5] = { 60.0, 250.0, 1000.0, 4000.0, 8000.0 };

/* Preset curves (dB), indexed by eEQMode. EQ_BASS(5)/EQ_USER(7)/EQ_NOR(8)
 * are not stored here — they route to bass / dbGain / bypass respectively. */
static const int8_t g_preset_db[EQ_ROCK + 1][5] = {
    [EQ_HEAVY]  = {  6,  4, -2,  4,  6 },
    [EQ_POP]    = {  2,  3,  1,  3,  2 },
    [EQ_JAZZ]   = {  3,  2, -1,  2,  4 },
    [EQ_UNIQUE] = {  5,  3, -1,  1,  3 },
    [EQ_CLASS]  = {  4,  2, -1,  2,  4 },
    [EQ_ROCK]   = {  5,  3, -1,  3,  5 },
};

typedef struct
{
    BiquadSection band[RCH_DSP_MAX_CH][5];
    int           active;
} ParamEqState;
static ParamEqState g_param;

static long param_eq_adjust(const RKEffect *cfg, unsigned long sr)
{
    double fs = (double)sr;
    int db[5];
    int i, ch;

    if (cfg->Mode == EQ_USER)
        for (i = 0; i < 5; i++)
            db[i] = cfg->RKCoef.dbGain[i] - 12;      /* dB+12 -> dB */
    else
        for (i = 0; i < 5; i++)
            db[i] = g_preset_db[cfg->Mode][i];

    g_param.active = 0;
    for (i = 0; i < 5; i++)
    {
        if (db[i] >  12) db[i] =  12;
        if (db[i] < -12) db[i] = -12;
        if (db[i] != 0)  g_param.active = 1;
    }

    for (ch = 0; ch < RCH_DSP_MAX_CH; ch++)
    {
        biquad_lowshelf (&g_param.band[ch][0], fs, g_band_freq[0], db[0], EQ_SHELF);
        biquad_peaking  (&g_param.band[ch][1], fs, g_band_freq[1], db[1], EQ_Q);
        biquad_peaking  (&g_param.band[ch][2], fs, g_band_freq[2], db[2], EQ_Q);
        biquad_peaking  (&g_param.band[ch][3], fs, g_band_freq[3], db[3], EQ_Q);
        biquad_highshelf(&g_param.band[ch][4], fs, g_band_freq[4], db[4], EQ_SHELF);
    }
    return 0;
}

static long param_eq_process(EQ_TYPE *buf, long samples, int channels)
{
    int ch, b;
    long i;

    if (!g_param.active)
        return 0;
    if (channels < 1) channels = 1;
    if (channels > RCH_DSP_MAX_CH) channels = RCH_DSP_MAX_CH;

    for (ch = 0; ch < channels; ch++)
    {
        BiquadSection *bands = g_param.band[ch];
        for (i = ch; i < samples; i += channels)
        {
            int32_t y = buf[i];
            for (b = 0; b < 5; b++)
                y = biquad_run(&bands[b], y);
            buf[i] = y;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* bass: single low-shelf bass boost.                                  */
/* ------------------------------------------------------------------ */
#define BASS_GAIN_DB 10.0
#define BASS_FREQ    80.0

typedef struct
{
    BiquadSection shelf[RCH_DSP_MAX_CH];
    int           active;
} BassState;
static BassState g_bass;

static long bass_adjust(const RKEffect *cfg, unsigned long sr)
{
    double fs = (double)sr;
    int ch;

    g_bass.active = (cfg->Mode == EQ_BASS);
    for (ch = 0; ch < RCH_DSP_MAX_CH; ch++)
        biquad_lowshelf(&g_bass.shelf[ch], fs, BASS_FREQ, BASS_GAIN_DB, EQ_SHELF);
    return 0;
}

static long bass_process(EQ_TYPE *buf, long samples, int channels)
{
    int ch;
    long i;

    if (!g_bass.active)
        return 0;
    if (channels < 1) channels = 1;
    if (channels > RCH_DSP_MAX_CH) channels = RCH_DSP_MAX_CH;

    for (ch = 0; ch < channels; ch++)
        for (i = ch; i < samples; i += channels)
            buf[i] = biquad_run(&g_bass.shelf[ch], buf[i]);
    return 0;
}

/* ------------------------------------------------------------------ */
/* bypass.                                                             */
/* ------------------------------------------------------------------ */
static long bypass_adjust(const RKEffect *cfg, unsigned long sr)
{
    (void)cfg; (void)sr;
    return 0;
}
static long bypass_process(EQ_TYPE *buf, long samples, int channels)
{
    (void)buf; (void)samples; (void)channels;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Dispatch table.                                                     */
/* ------------------------------------------------------------------ */
typedef enum { DSP_BYPASS = 0, DSP_PARAM_EQ, DSP_BASS, DSP_COUNT } dsp_id_t;

static const RchDspPlugin g_plugins[DSP_COUNT] = {
    [DSP_BYPASS]   = { "bypass",   bypass_adjust,   bypass_process   },
    [DSP_PARAM_EQ] = { "param_eq", param_eq_adjust, param_eq_process },
    [DSP_BASS]     = { "bass",     bass_adjust,     bass_process     },
};

static const RchDspPlugin *g_active   = &g_plugins[DSP_BYPASS];
static int                 g_channels = 2;

static const RchDspPlugin *select_plugin(eEQMode mode)
{
    switch (mode)
    {
    case EQ_BASS: return &g_plugins[DSP_BASS];
    case EQ_NOR:  return &g_plugins[DSP_BYPASS];
    default:      return &g_plugins[DSP_PARAM_EQ];
    }
}

/* ------------------------------------------------------------------ */
/* Public EQ API (what AudioControl.c / Service.c call).               */
/* ------------------------------------------------------------------ */
long EffectInit(void)
{
    g_active   = &g_plugins[DSP_BYPASS];
    g_channels = 2;
    return 0;
}

long EffectEnd(void)
{
    g_active   = &g_plugins[DSP_BYPASS];
    g_channels = 2;
    return 0;
}

long RKEQAdjust(RKEffect *pEft)
{
    /* Kept for API parity with the old Effect.c; routes into the framework. */
    unsigned long sr = 44100;
    if (pEft == 0)
        return 1;
    CodecGetSampleRate(&sr);
    g_active = select_plugin(pEft->Mode);
    g_active->adjust(pEft, sr);
    return 0;
}

long EffectAdjust(void)
{
    RKEffect *cfg = &AudioIOBuf.EffectCtl;
    unsigned long sr = 44100;
    unsigned long ch = 2;

    /* Stock behavior: a fully-flat user EQ collapses to EQ_NOR. */
    if (cfg->Mode == EQ_USER)
    {
        int i, zero = 1;
        for (i = 0; i < 5; i++)
            if (cfg->RKCoef.dbGain[i] != 12)
                zero = 0;
        if (zero)
        {
            cfg->Mode = EQ_NOR;
            IsEQNOR = TRUE;
        }
    }

    CodecGetSampleRate(&sr);
    CodecGetChannels(&ch);
    if (ch >= 1 && ch <= RCH_DSP_MAX_CH)
        g_channels = (int)ch;

    g_active = select_plugin(cfg->Mode);
    g_active->adjust(cfg, sr);
    return 0;
}

long EffectProcess(EQ_TYPE *pBuffer, long PcmLen)
{
    if (pBuffer == 0)
        return 1;
    if (PcmLen <= 0)
        return 0;
    if (pAudioRegKey->IsEQUpdate)
        return 0;
    if (AudioIOBuf.EffectCtl.Mode == EQ_NOR)
        return 0;

    return g_active->process(pBuffer, PcmLen, g_channels);
}
