/*
 * test_mod_volume.c — host test for the mod_volume gain/ramp module.
 *
 * Verifies the UX contract of a volume control on a DAP:
 *   1. unity gain is an exact passthrough (a volume module must be
 *      bit-transparent when the user asks for 0 dB);
 *   2. the soft ramp is monotonic with NO OVERSHOOT past the target and
 *      NO CLICKS (max sample-to-sample step is bounded);
 *   3. downward ramps behave the same way (no undershoot);
 *   4. at full scale a boost SATURATES at 32767 instead of wrapping
 *      negative (the classic int16 overflow "pop to the opposite rail");
 *   5. -60 dB attenuates to (near) silence;
 *   6. set_param/get_param contract: round-trip, ranges, unknown-param.
 *
 * Compile (host):
 *   cc -Wall -Werror -O2 -o test_mod_volume \
 *      firmware/dsp/tests/test_mod_volume.c firmware/dsp/mod_volume.c -lm
 */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "../dsp_module.h"

static int16_t buf[T_SAMPLES];

/* Expected steady output for a DC input under gain g (tenths of dB). */
static double dc_target(double level, int gain_tdb)
{
    return level * pow(10.0, gain_tdb / 200.0);
}

int main(void)
{
    DSP_STATE_STORAGE(st, 256);
    const dsp_module_t *m = &dsp_mod_volume;
    int i, n = T_SAMPLES;

    printf("mod_volume: state_size = %d bytes\n", m->state_size);
    CHECK(m->state_size > 0 && m->state_size <= 256, "state_size fits 256B");
    CHECK(m->init(st.bytes, T_FS) == 0, "init() accepts 48 kHz");

    /* ---- 1. unity is exact passthrough ----------------------------- */
    CHECK(m->set_param(st.bytes, MOD_VOLUME_RAMP_MS, 0) == 0,
          "ramp set to instant");
    gen_pseudo_random16(buf, n, 424242u);
    {
        static int16_t in_copy[T_SAMPLES];
        memcpy(in_copy, buf, sizeof buf);
        m->process(st.bytes, buf, n);
        CHECK(memcmp(buf, in_copy, sizeof buf) == 0,
              "0 dB gain is bit-exact passthrough");
    }

    /* ---- 2. ramp up: monotonic, no overshoot, no click -------------- */
    {
        int ramp_samples = T_FS * 20 / 1000;       /* default 20 ms ramp */
        int16_t expected = (int16_t)dc_target(8000.0, 60);
        int monotonic = 1, no_overshoot = 1;
        int16_t peak = 0;
        int max_step = 0;
        /* Per gain-step the level moves ~8 LSB; allow a 12 LSB per-sample
         * ceiling (a "click" would be a jump of hundreds of LSB). */
        int step_limit = (int)((expected - 8000) / ramp_samples) + 4;

        CHECK(m->set_param(st.bytes, MOD_VOLUME_RAMP_MS, 20) == 0,
              "ramp set to 20 ms");
        gen_dc16(buf, T_FRAMES, 8000);
        CHECK(m->set_param(st.bytes, MOD_VOLUME_GAIN, 60) == 0,
              "gain +6 dB while playing");
        m->process(st.bytes, buf, n);

        for (i = 1; i < n; i++) {                  /* every sample */
            int diff = (int)buf[i] - buf[i - 1];
            if (diff < 0) diff = -diff;
            if (diff > max_step) max_step = diff;
        }
        for (i = 2; i < n; i += 2) {               /* left channel for trend */
            if (buf[i] < buf[i - 2]) monotonic = 0;
            if (buf[i] > expected + 1) no_overshoot = 0;
            if (buf[i] > peak) peak = buf[i];
        }
        printf("  info: ramp peak = %d, target = %d, max step %d\n",
               (int)peak, (int)expected, max_step);
        CHECK(monotonic, "ramp up is monotonic");
        CHECK(no_overshoot, "ramp up never overshoots the target");
        CHECK(max_step <= step_limit, "ramp up has no clicks (bounded step)");
        CHECK(peak >= expected - 2 && peak <= expected + 1,
              "ramp up settles on the target level");
    }

    /* ---- 3. ramp down: monotonic, no undershoot --------------------- */
    {
        int16_t expected = 8000;
        int monotonic = 1, no_undershoot = 1;
        int16_t floor_v = 32767;

        gen_dc16(buf, T_FRAMES, 8000);
        CHECK(m->set_param(st.bytes, MOD_VOLUME_GAIN, 0) == 0,
              "gain back to 0 dB while playing");
        m->process(st.bytes, buf, n);
        for (i = 2; i < n; i += 2) {
            if (buf[i] > buf[i - 2]) monotonic = 0;
            if (buf[i] < expected - 1) no_undershoot = 0;
            if (buf[i] < floor_v) floor_v = buf[i];
        }
        CHECK(monotonic, "ramp down is monotonic");
        CHECK(no_undershoot, "ramp down never undershoots the target");
        CHECK(floor_v >= expected - 1, "ramp down settles at the target");
    }

    /* ---- 4. full-scale saturation ---------------------------------- */
    {
        int all_rail = 1, any_negative = 0;
        gen_dc16(buf, T_FRAMES, 32767);
        CHECK(m->set_param(st.bytes, MOD_VOLUME_GAIN, 60) == 0,
              "gain +6 dB at full scale");
        m->process(st.bytes, buf, n);
        for (i = 0; i < n; i++) {
            if (buf[i] != 32767) all_rail = 0;
            if (buf[i] < 0) any_negative = 1;
        }
        CHECK(all_rail, "boosted full scale saturates at +32767");
        CHECK(!any_negative, "saturation never wraps to negative");
    }

    /* ---- 5. deep attenuation --------------------------------------- */
    {
        double r;
        gen_sine16(buf, T_FRAMES, 1000.0, 0.5);
        CHECK(m->set_param(st.bytes, MOD_VOLUME_GAIN, -600) == 0,
              "gain -60 dB");
        m->process(st.bytes, buf, n);
        r = rms16_steady(buf, T_SAMPLES);
        CHECK(r < 0.005, "-60 dB is (near) silence");
    }

    /* ---- 6. parameter contract ------------------------------------- */
    CHECK(m->set_param(st.bytes, MOD_VOLUME_GAIN, -123) == 0 &&
          m->get_param(st.bytes, MOD_VOLUME_GAIN) == -123,
          "set/get round-trip (-12.3 dB)");
    CHECK(m->set_param(st.bytes, MOD_VOLUME_RAMP_MS, 150) == 0 &&
          m->get_param(st.bytes, MOD_VOLUME_RAMP_MS) == 150,
          "set/get round-trip (ramp 150 ms)");
    CHECK(m->set_param(st.bytes, MOD_VOLUME_GAIN, 61) == -2,
          "gain above +6 dB is rejected");
    CHECK(m->set_param(st.bytes, MOD_VOLUME_GAIN, -601) == -2,
          "gain below -60 dB is rejected");
    CHECK(m->set_param(st.bytes, MOD_VOLUME_RAMP_MS, 1001) == -2,
          "ramp above 1000 ms is rejected");
    CHECK(m->set_param(st.bytes, 42, 0) == -1, "unknown param is rejected");
    CHECK(m->get_param(st.bytes, 42) == INT_MIN,
          "get_param() error is INT_MIN");

    printf("\n%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
