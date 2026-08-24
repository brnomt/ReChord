/*
 * test_dsp_core.c — host unit test for the freestanding ReChord DSP core.
 *
 * Verifies the biquad math is correct BEFORE it ever touches the device:
 *   1. Bypass is a true passthrough.
 *   2. Bass boost raises low-freq energy RELATIVE to a high-freq tone.
 *   3. A peaking EQ band boosts its centre freq RELATIVE to a far freq.
 *   4. No overflow/clipping at full-scale 24-bit input.
 *
 * EQ with headroom pre-scale makes overall output quieter, so the correct
 * assertions are RELATIVE (boosted band vs unboosted band), matching how the
 * stock RockEQReduce9dB headroom reduction behaves.
 *
 * Compile (host):  cc -O2 -o test_dsp_core test_dsp_core.c rechord_dsp_core.c -lm
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "rechord_dsp_core.h"

#define FS      48000.0
#define N       48000            /* 1 second */
#define FULL    0x7FFFFF          /* 24-bit full scale */

static int failures = 0;

#define CHECK(cond, name) do { \
    if (cond) { printf("  PASS: %s\n", name); } \
    else { printf("  FAIL: %s\n", name); failures++; } \
} while (0)

static double rms(const int32_t *b, int n)
{
    double s = 0.0;
    for (int i = 0; i < n; i++) {
        double v = (double)b[i] / FULL;
        s += v * v;
    }
    return sqrt(s / n);
}

static void gen_sine(int32_t *b, int n, double freq, double amp)
{
    for (int i = 0; i < n; i++)
        b[i] = (int32_t)(amp * FULL * sin(2.0 * M_PI * freq * i / FS));
}

int main(void)
{
    int32_t *buf = (int32_t *)malloc(N * sizeof(int32_t));
    rch_dsp_config_t cfg = {0};

    /* 1. Bypass passthrough */
    gen_sine(buf, N, 1000.0, 0.5);
    int32_t *ref = (int32_t *)malloc(N * sizeof(int32_t));
    for (int i = 0; i < N; i++) ref[i] = buf[i];

    cfg.mode = RCH_DSP_BYPASS;
    cfg.sample_rate = (unsigned long)FS;
    cfg.channels = 1;
    rch_dsp_configure(&cfg);
    rch_dsp_process(buf, N);
    CHECK(rms(buf, N) > 0.99 * rms(ref, N) &&
          rms(buf, N) < 1.01 * rms(ref, N), "bypass is passthrough");
    free(ref);

    /* 2. Bass boost: 50 Hz is louder than 1 kHz by ~10 dB (3.16x) */
    cfg.mode = RCH_DSP_BASS;
    rch_dsp_reset();
    rch_dsp_configure(&cfg);
    gen_sine(buf, N, 50.0, 0.2);
    rch_dsp_process(buf, N);
    double bass = rms(buf, N);
    gen_sine(buf, N, 1000.0, 0.2);
    rch_dsp_process(buf, N);
    double treble = rms(buf, N);
    CHECK(bass > treble * 2.0, "bass boost: 50 Hz >> 1 kHz");

    /* 3. Param EQ: +12 dB @ 1 kHz makes 1 kHz louder than 100 Hz (~4x) */
    cfg.mode = RCH_DSP_PARAM_EQ;
    for (int i = 0; i < RCH_DSP_BANDS; i++) cfg.db_gain[i] = 0.0;
    cfg.db_gain[2] = 12.0;   /* 1000 Hz band */
    rch_dsp_reset();
    rch_dsp_configure(&cfg);
    gen_sine(buf, N, 1000.0, 0.2);
    rch_dsp_process(buf, N);
    double peak = rms(buf, N);
    gen_sine(buf, N, 100.0, 0.2);
    rch_dsp_process(buf, N);
    double offband = rms(buf, N);
    CHECK(peak > offband * 2.0, "1 kHz peaking: 1 kHz >> 100 Hz");

    /* 4. Full-scale does not overflow */
    cfg.mode = RCH_DSP_BASS;
    rch_dsp_reset();
    rch_dsp_configure(&cfg);
    for (int i = 0; i < N; i++) buf[i] = FULL;
    rch_dsp_process(buf, N);
    int clipped = 0;
    for (int i = 0; i < N; i++)
        if (buf[i] > FULL || buf[i] < -FULL) clipped++;
    CHECK(clipped == 0, "no overflow at full scale");

    free(buf);
    printf("\n%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
