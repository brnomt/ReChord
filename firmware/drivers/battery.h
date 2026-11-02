/*
 * battery.h — battery gauge (BRNOMT-83).
 *
 * Measurement (stock SDK constants, firmware/rockchip/.../battery.h, Adc.h):
 *   - SARADC channel 0 (the key ladder is channel 1)
 *   - mV = ((adc << 1) * 2500) >> 10   (2500 mV reference, board 1/2 divider)
 *
 * Percentage: a piecewise-linear Li-ion curve over the cell voltage (typical
 * open-circuit values, not calibrated to this board). The old 5-step ladder
 * read 100 % from 3.8 V up, i.e. for most of the discharge. While the cable is
 * in the cell reads ~40 mV high, which is subtracted, and the voltage is
 * smoothed over a few polls so the number does not flicker.
 *
 * The gauge is read at a slow cadence (never per frame) and cached; -1 means
 * "unknown" and callers must render a placeholder rather than a fake level.
 */
#ifndef RECHORD_DRIVERS_BATTERY_H
#define RECHORD_DRIVERS_BATTERY_H

#define BATTERY_LEVELS 5

/* Pure mapping (host-tested): millivolts -> 0..BATTERY_LEVELS-1, or -1. */
int battery_level_from_mv(int mv);

/* Li-ion curve: millivolts -> 0..100 %, or -1 when unknown. */
int battery_pct_from_mv(int mv);

/* Tell the gauge whether the charger is connected (compensates the reading).
 * Call before battery_poll(). */
void battery_set_charging(int on);

/* One measurement step (what battery_poll does after reading the ADC): apply
 * the charge offset, smooth, and update the cached values. Exposed for tests. */
void battery_feed_mv(int mv, int charging);

/* Read the gauge now (8-sample average) and cache it. */
void battery_poll(void);

/* Cached values: -1 while unavailable. */
int battery_mv(void);
int battery_level(void);      /* legacy 0..4 step of the smoothed voltage */
int battery_percent(void);    /* 0..100, smoothed + charge-compensated    */

#endif /* RECHORD_DRIVERS_BATTERY_H */
