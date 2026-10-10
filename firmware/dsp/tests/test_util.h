/*
 * test_util.h — shared helpers for firmware/dsp/tests/ host tests.
 *
 * Style follows firmware/test_dsp_core.c: PASS/FAIL lines and a final tally.
 * Signals are INT16 STEREO INTERLEAVED (the dsp_module_t stream contract),
 * so the sine helpers fill L=R and the RMS helpers read any channel stream.
 */
#ifndef DSP_TEST_UTIL_H
#define DSP_TEST_UTIL_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define T_PI 3.14159265358979323846
#define T_FS 48000        /* Hz, the Echo Mini's max PCM rate              */
#define T_FRAMES 24000    /* 0.5 s per test signal                         */
#define T_SAMPLES (T_FRAMES * 2)

static int failures = 0;

#define CHECK(cond, name) do { \
    if (cond) { printf("  PASS: %s\n", name); } \
    else { printf("  FAIL: %s\n", name); failures++; } \
} while (0)

/* Interleaved stereo sine at amplitude `amp` of int16 full scale. */
static inline void gen_sine16(int16_t *b, int frames, double freq, double amp)
{
    int i;
    for (i = 0; i < frames; i++) {
        double s = amp * 32767.0 * sin(2.0 * T_PI * freq * i / (double)T_FS);
        b[2 * i] = b[2 * i + 1] = (int16_t)s;
    }
}

/* Fill both channels with a DC level (ramp/step response probes). */
static inline void gen_dc16(int16_t *b, int frames, int16_t level)
{
    int i;
    for (i = 0; i < frames; i++)
        b[2 * i] = b[2 * i + 1] = level;
}

/* RMS over int16 values (normalized to full scale). */
static inline double rms16(const int16_t *b, int n)
{
    double s = 0.0;
    int i;
    for (i = 0; i < n; i++) {
        double v = (double)b[i] / 32767.0;
        s += v * v;
    }
    return (n > 0) ? sqrt(s / n) : 0.0;
}

/* RMS of the second half of the buffer: steady state only, so filter
 * start-up transients cannot skew relative-gain assertions. */
static inline double rms16_steady(const int16_t *b, int n)
{
    return rms16(b + n / 2, n - n / 2);
}

/* Deterministic pseudo-random int16 values in [-24000, 23999] (no libc
 * rand dependency — identical on every host). */
static inline void gen_pseudo_random16(int16_t *b, int n, uint32_t seed)
{
    int i;
    uint32_t s = seed;
    for (i = 0; i < n; i++) {
        s = s * 1664525u + 1013904223u;      /* Numerical Recipes LCG */
        b[i] = (int16_t)((int32_t)((s >> 16) % 48000u) - 24000);
    }
}

#endif /* DSP_TEST_UTIL_H */
