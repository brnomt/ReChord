/*
 * test_power_save.c — idle power-down decision (DAC + Core B).
 */
#include <stdio.h>

#include "power_save.h"

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

int main(void)
{
    power_save_t ps;

    TEST("power_save: three-second idle shutdown");
    power_save_init(&ps);

    CHECK_EQI(power_save_step(&ps, 0, 1, 0, 1000), 0,
              "playing: nothing (timer disarmed)");
    CHECK_EQI(power_save_step(&ps, 1, 1, 0, 2000), 0, "first idle pass arms");
    CHECK_EQI(power_save_step(&ps, 1, 1, 0, 4999), 0, "before 3 s: nothing");
    CHECK_EQI(power_save_step(&ps, 1, 1, 0, 5000),
              POWER_SAVE_DAC | POWER_SAVE_CORE_B, "3 s: DAC + Core B off");

    /* A boot preload keeps Core B for a minute, but the DAC still goes. */
    power_save_init(&ps);
    power_save_step(&ps, 1, 1, 1, 1000);
    CHECK_EQI(power_save_step(&ps, 1, 1, 1, 4000), POWER_SAVE_DAC,
              "preloaded: DAC only at 3 s");
    CHECK_EQI(power_save_step(&ps, 1, 1, 1, 60999), POWER_SAVE_DAC,
              "preloaded: still DAC only before 60 s");
    CHECK_EQI(power_save_step(&ps, 1, 1, 1, 61000),
              POWER_SAVE_DAC | POWER_SAVE_CORE_B, "preload expires at 60 s");

    /* Resuming disarms the timer. */
    power_save_init(&ps);
    power_save_step(&ps, 1, 1, 0, 1000);
    CHECK_EQI(power_save_step(&ps, 0, 1, 0, 2000), 0, "play resets");
    CHECK_EQI(power_save_step(&ps, 1, 1, 0, 4500), 0, "re-arms after stop");
    CHECK_EQI(power_save_step(&ps, 1, 0, 0, 7600), POWER_SAVE_CORE_B,
              "no DAC flag when it is already down");

    if (s_failures == 0) {
        printf("test_power_save: OK\n");
        return 0;
    }
    printf("test_power_save: %d FAILURE(S)\n", s_failures);
    return s_failures;
}
