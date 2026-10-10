/*
 * test_mod_bass.c — host test for the mod_bass bass-boost effect module.
 *
 * Verifies:
 *   1. default boost (+10 dB, the core's bass tuning) makes 50 Hz much
 *      louder than 1 kHz (RELATIVE assertion, like test_dsp_core.c — the
 *      0.9/max-gain headroom pre-scale keeps absolute levels below input);
 *   2. boost = 0 is ~0.9x passthrough at both ends (the shelf is identity);
 *   3. set_param/get_param contract: round-trip, range, unknown-param.
 *
 * Compile (host):
 *   cc -Wall -Werror -O2 -o test_mod_bass \
 *      firmware/dsp/tests/test_mod_bass.c firmware/dsp/mod_bass.c \
 *      firmware/rechord_dsp_core.c -lm
 */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "../dsp_module.h"

static int16_t buf[T_SAMPLES];

static double run_sine(const dsp_module_t *m, void *st,
                       double freq, double amp)
{
    gen_sine16(buf, T_FRAMES, freq, amp);
    m->process(st, buf, T_SAMPLES);
    return rms16_steady(buf, T_SAMPLES);
}

int main(void)
{
    DSP_STATE_STORAGE(st, 512);
    const dsp_module_t *m = &dsp_mod_bass;
    double ratio, r50, r1k;

    printf("mod_bass: state_size = %d bytes\n", m->state_size);
    CHECK(m->state_size > 0 && m->state_size <= 512, "state_size fits 512B");
    CHECK(m->init(st.bytes, T_FS) == 0, "init() accepts 48 kHz");

    /* ---- 1. default boost: 50 Hz >> 1 kHz ------------------------- */
    CHECK(m->get_param(st.bytes, MOD_BASS_BOOST) == 100,
          "default boost is +10 dB (core bass tuning)");
    r50 = run_sine(m, st.bytes, 50.0, 0.2);
    r1k = run_sine(m, st.bytes, 1000.0, 0.2);
    ratio = r50 / r1k;
    printf("  info: 50 Hz / 1 kHz RMS ratio = %.2f\n", ratio);
    CHECK(ratio > 2.0, "bass boost: 50 Hz >> 1 kHz");

    /* Stronger boost must push the ratio further up (monotonic effect). */
    CHECK(m->set_param(st.bytes, MOD_BASS_BOOST, 150) == 0,
          "set boost = +15 dB");
    ratio = run_sine(m, st.bytes, 50.0, 0.2) /
            run_sine(m, st.bytes, 1000.0, 0.2);
    CHECK(ratio > 2.5, "+15 dB boost raises the ratio further");

    /* ---- 2. zero boost is passthrough at ~0.9x --------------------- */
    CHECK(m->set_param(st.bytes, MOD_BASS_BOOST, 0) == 0, "set boost = 0");
    r50 = run_sine(m, st.bytes, 50.0, 0.5);
    r1k = run_sine(m, st.bytes, 1000.0, 0.5);
    CHECK(r50 / r1k > 0.9 && r50 / r1k < 1.1,
          "0 dB boost: 50 Hz and 1 kHz pass equally");
    ratio = r50 / (0.5 * 0.7071);   /* RMS of the 0.5-amp input sine */
    CHECK(ratio > 0.85 && ratio < 0.95,
          "0 dB boost passes through at ~0.9x (headroom rule)");

    /* ---- 3. parameter contract ------------------------------------- */
    CHECK(m->set_param(st.bytes, MOD_BASS_BOOST, 45) == 0 &&
          m->get_param(st.bytes, MOD_BASS_BOOST) == 45,
          "set/get round-trip (+4.5 dB)");
    CHECK(m->set_param(st.bytes, MOD_BASS_BOOST, -1) == -2,
          "negative boost is rejected");
    CHECK(m->set_param(st.bytes, MOD_BASS_BOOST, 151) == -2,
          "boost above +15 dB is rejected");
    CHECK(m->set_param(st.bytes, 7, 0) == -1, "unknown param is rejected");
    CHECK(m->get_param(st.bytes, 7) == INT_MIN,
          "get_param() error is INT_MIN");

    printf("\n%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
