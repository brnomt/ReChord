/*
 * battery.c — SARADC battery gauge (BRNOMT-83).
 *
 * Reuses the keys' SARADC sequence (panel_io.c adc_read_channel) on channel 0
 * and the stock SDK's conversion constants; see battery.h. The percentage is a
 * Li-ion curve over the smoothed, charge-compensated cell voltage.
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

#define BATT_CHARGE_OFFSET_MV 40   /* cell reads this much high on the cable */

static int s_mv = -1;          /* last raw reading                          */
static int s_avg = -1;         /* smoothed, charge-compensated millivolts   */
static int s_level = -1;
static int s_pct = -1;
static int s_charging;

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

/* Typical Li-ion discharge (open circuit, rested cell). */
static const struct { short mv; short pct; } k_curve[] = {
    { 3300, 0 }, { 3500, 10 }, { 3650, 30 }, { 3750, 50 },
    { 3850, 70 }, { 3950, 85 }, { 4050, 95 }, { 4150, 100 }
};
#define CURVE_N ((int)(sizeof k_curve / sizeof k_curve[0]))

int battery_pct_from_mv(int mv)
{
    int i;

    if (mv < 0)
        return -1;
    if (mv <= k_curve[0].mv)
        return 0;
    if (mv >= k_curve[CURVE_N - 1].mv)
        return 100;
    for (i = 1; i < CURVE_N; i++) {
        if (mv < k_curve[i].mv) {
            int dmv = k_curve[i].mv - k_curve[i - 1].mv;
            int dpc = k_curve[i].pct - k_curve[i - 1].pct;

            return k_curve[i - 1].pct +
                   (mv - k_curve[i - 1].mv) * dpc / dmv;
        }
    }
    return 100;
}

void battery_set_charging(int on)
{
    s_charging = on ? 1 : 0;
}

void battery_feed_mv(int mv, int charging)
{
    int eff;

    if (mv < 0) {
        s_mv = s_avg = s_level = s_pct = -1;
        return;
    }
    eff = mv - (charging ? BATT_CHARGE_OFFSET_MV : 0);
    /* 3/4 old + 1/4 new: ~1 min time constant at the 15 s poll. */
    s_avg = (s_avg < 0) ? eff : (s_avg * 3 + eff + 2) / 4;
    s_mv = mv;
    s_level = battery_level_from_mv(s_avg);
    s_pct = battery_pct_from_mv(s_avg);
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
    battery_feed_mv(n == 0 ? -1 : sum / n, s_charging);
}

int battery_mv(void)      { return s_mv; }
int battery_level(void)   { return s_level; }
int battery_percent(void) { return s_pct; }
