/*
 * test_jack_sense.c — headphone-jack mapping and debounce (BRNOMT-70).
 */
#include <stdio.h>

#include "jack_sense.h"

static int s_failures;

#define TEST(name) printf("== %s ==\n", name)
#define CHECK(cond, name) do {                                             \
    if (cond) printf("  PASS: %s\n", name);                                \
    else { printf("  FAIL: %s\n", name); s_failures++; }                    \
} while (0)
#define CHECK_EQI(got, want, name) do {                                    \
    long g_ = (long)(got), w_ = (long)(want);                              \
    if (g_ == w_) printf("  PASS: %s\n", name);                            \
    else { printf("  FAIL: %s (got %ld, want %ld)\n", name, g_, w_);        \
           s_failures++; }                                                \
} while (0)

static void test_pins(void)
{
    TEST("jack_sense: pin mapping");

    CHECK_EQI(jack_kind_of(0, 1), JACK_NONE, "neither pin -> nothing");
    CHECK_EQI(jack_kind_of(1, 1), JACK_SINGLE_ENDED, "single-ended sense");
    CHECK_EQI(jack_kind_of(0, 0), JACK_BALANCED, "balanced sense (0)");
    CHECK_EQI(jack_kind_of(1, 0), JACK_BALANCED, "balanced sense (1)");
}

static void test_debounce(void)
{
    jack_sense_t sense;
    int i;

    TEST("jack_sense: first reading, seven-sample debounce, bounce");

    sense.started = 0;
    CHECK_EQI(jack_sense_sample(&sense, JACK_SINGLE_ENDED), 1,
              "first reading adopted");
    CHECK_EQI(sense.stable, JACK_SINGLE_ENDED, "stable single-ended");

    for (i = 1; i < JACK_SENSE_SAMPLES; i++)
        CHECK_EQI(jack_sense_sample(&sense, JACK_NONE), 0,
                  "change still debouncing");
    CHECK_EQI(sense.stable, JACK_SINGLE_ENDED, "not stable yet");
    CHECK_EQI(jack_sense_sample(&sense, JACK_NONE), 1, "change accepted");
    CHECK_EQI(sense.stable, JACK_NONE, "unplugged");

    /* A bouncing contact restarts the count. */
    sense.started = 0;
    jack_sense_sample(&sense, JACK_NONE);
    for (i = 1; i < JACK_SENSE_SAMPLES; i++)
        jack_sense_sample(&sense, JACK_BALANCED);
    CHECK_EQI(jack_sense_sample(&sense, JACK_NONE), 0,
              "bounce resets the candidate");
    for (i = 1; i < JACK_SENSE_SAMPLES; i++)
        CHECK_EQI(jack_sense_sample(&sense, JACK_BALANCED), 0,
                  "re-debouncing");
    CHECK_EQI(sense.stable, JACK_NONE, "bounce did not change the state");
    CHECK_EQI(jack_sense_sample(&sense, JACK_BALANCED), 1,
              "settled change accepted");
    CHECK_EQI(sense.stable, JACK_BALANCED, "balanced plugged");
}

int main(void)
{
    test_pins();
    test_debounce();
    if (s_failures == 0) {
        printf("test_jack_sense: OK\n");
        return 0;
    }
    printf("test_jack_sense: %d FAILURE(S)\n", s_failures);
    return s_failures;
}
