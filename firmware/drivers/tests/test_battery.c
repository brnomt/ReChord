/*
 * test_battery.c — host test for the battery ladder (BRNOMT-83).
 *
 * The SARADC read itself is verified on hardware; this pins the pure mapping
 * (the stock SDK's 5-step ladder) and the "unknown" behaviour of the gauge.
 */
#include <stdio.h>

#include "battery.h"

static int checks;
static int fails;

#define CHECK(cond) do {                                                   \
    checks++;                                                              \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        fails++;                                                           \
    }                                                                      \
} while (0)

int main(void)
{
    printf("== battery: curve + smoothing + gauge ==\n");

    /* Boundaries from the stock SDK (3350/3500/3600/3800/4000 mV). */
    CHECK(battery_level_from_mv(3300) == 0);
    CHECK(battery_level_from_mv(3349) == 0);
    CHECK(battery_level_from_mv(3350) == 1);
    CHECK(battery_level_from_mv(3499) == 1);
    CHECK(battery_level_from_mv(3500) == 2);
    CHECK(battery_level_from_mv(3599) == 2);
    CHECK(battery_level_from_mv(3600) == 3);
    CHECK(battery_level_from_mv(3799) == 3);
    CHECK(battery_level_from_mv(3800) == 4);
    CHECK(battery_level_from_mv(4000) == 4);
    CHECK(battery_level_from_mv(4100) == 4);
    CHECK(battery_level_from_mv(-1) == -1);

    /* Li-ion curve: anchors, interpolation, clamps, monotonic. */
    CHECK(battery_pct_from_mv(3000) == 0);
    CHECK(battery_pct_from_mv(3300) == 0);
    CHECK(battery_pct_from_mv(3500) == 10);
    CHECK(battery_pct_from_mv(3650) == 30);
    CHECK(battery_pct_from_mv(3750) == 50);
    CHECK(battery_pct_from_mv(3850) == 70);
    CHECK(battery_pct_from_mv(3950) == 85);
    CHECK(battery_pct_from_mv(4050) == 95);
    CHECK(battery_pct_from_mv(4150) == 100);
    CHECK(battery_pct_from_mv(4300) == 100);
    CHECK(battery_pct_from_mv(3575) == 20);          /* midway 10..30        */
    CHECK(battery_pct_from_mv(-1) == -1);
    {
        int mv, prev = 0, mono = 1;

        for (mv = 3000; mv <= 4300; mv++) {
            int p = battery_pct_from_mv(mv);

            if (p < prev)
                mono = 0;
            prev = p;
        }
        CHECK(mono);
    }

    /* Smoothing + charge compensation through the feed step. */
    battery_feed_mv(4128, 0);                        /* first reading seeds  */
    CHECK(battery_mv() == 4128);
    CHECK(battery_percent() == battery_pct_from_mv(4128));
    battery_feed_mv(3700, 0);                        /* one low sample       */
    CHECK(battery_percent() > battery_pct_from_mv(3700));   /* damped       */
    CHECK(battery_percent() < battery_pct_from_mv(4128));
    {
        int i;

        for (i = 0; i < 40; i++)
            battery_feed_mv(3700, 0);
        CHECK(battery_percent() == battery_pct_from_mv(3700) ||
              battery_percent() == battery_pct_from_mv(3703));  /* converges */
    }
    battery_feed_mv(-1, 0);                          /* ADC failed -> unknown */
    CHECK(battery_percent() == -1);
    battery_feed_mv(4170, 1);                        /* on the cable: -40 mV */
    CHECK(battery_percent() == battery_pct_from_mv(4130));
    battery_feed_mv(-1, 0);

    /* Sim backend: channel 0 is unavailable -> the gauge stays unknown and the
     * UI renders a placeholder instead of a fake level. */
    battery_poll();
    CHECK(battery_mv() == -1);
    CHECK(battery_level() == -1);
    CHECK(battery_percent() == -1);

    printf("%d checks, %d failures\n", checks, fails);
    return fails == 0 ? 0 : 1;
}
