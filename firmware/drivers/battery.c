/*
 * battery.c — SARADC battery gauge (BRNOMT-83).
 *
 * Reuses the keys' SARADC sequence (panel_io.c adc_read_channel) on channel 0
 * and the stock SDK's conversion/ladder constants; see battery.h. Approximation
 * only: 5 steps, no charge-state machine (usb_cable() answers "charging").
 */
#include "battery.h"
#include "panel_io.h"

/* Stock Adc.h: ADC_CHANEL_BATTERY = 0 (the keys use 1). */
#define BATTERY_CHANNEL 0

/* Stock battery.h (BATTERY_LEVEL 5). */
#define BATT_EMPTY_MV 3350
#define BATT_L0_MV    3500
#define BATT_L1_MV    3600
#define BATT_L2_MV    3800

static int s_mv = -1;
static int s_level = -1;

int battery_level_from_mv(int mv)
{
    if (mv < 0)
        return -1;
    if (mv < BATT_EMPTY_MV)
        return 0;
    if (mv < BATT_L0_MV)
        return 1;
    if (mv < BATT_L1_MV)
        return 2;
    if (mv < BATT_L2_MV)
        return 3;
    return BATTERY_LEVELS - 1;
}

int battery_pct_from_mv(int mv)
{
    int level = battery_level_from_mv(mv);

    return level < 0 ? -1 : level * 100 / (BATTERY_LEVELS - 1);
}

/* One conversion -> millivolts, mirroring RealBattValue(). */
static int read_mv_once(void)
{
    int adc = adc_read_channel(BATTERY_CHANNEL);

    if (adc < 0 || adc > 1023)
        return -1;
    return (int)((((unsigned)adc << 1) * 2500u) >> 10);
}

void battery_poll(void)
{
    int sum = 0;
    int n = 0;
    int i;

    for (i = 0; i < 8; i++) {
        int mv = read_mv_once();

        if (mv >= 0) {
            sum += mv;
            n++;
        }
    }
    if (n == 0) {
        s_mv = -1;
        s_level = -1;
        return;
    }
    s_mv = sum / n;
    s_level = battery_level_from_mv(s_mv);
}

int battery_mv(void)      { return s_mv; }
int battery_level(void)   { return s_level; }
int battery_percent(void) { return s_level < 0 ? -1 : s_level * 25; }
