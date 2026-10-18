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
    printf("== battery: ladder + gauge ==\n");

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

    CHECK(battery_pct_from_mv(3300) == 0);
    CHECK(battery_pct_from_mv(3350) == 25);
    CHECK(battery_pct_from_mv(3500) == 50);
    CHECK(battery_pct_from_mv(3600) == 75);
    CHECK(battery_pct_from_mv(4000) == 100);
    CHECK(battery_pct_from_mv(-1) == -1);

    /* Sim backend: channel 0 is unavailable -> the gauge stays unknown and the
     * UI renders a placeholder instead of a fake level. */
    battery_poll();
    CHECK(battery_mv() == -1);
    CHECK(battery_level() == -1);
    CHECK(battery_percent() == -1);

    printf("%d checks, %d failures\n", checks, fails);
    return fails == 0 ? 0 : 1;
}
