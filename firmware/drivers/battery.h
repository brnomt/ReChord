/*
 * battery.h — battery gauge (BRNOMT-83), an APPROXIMATION.
 *
 * Constants and the conversion formula come from the stock SDK sources that
 * already ship in this tree (firmware/rockchip/system/sysservice/battery.h and
 * driver/ADC/Adc.h), so no reverse engineering is needed for the measurement:
 *
 *   - SARADC channel 0 (the key ladder is channel 1)
 *   - mV = ((adc << 1) * 2500) >> 10   (2500 mV reference, board 1/2 divider)
 *   - 5-step ladder (BATTERY_LEVEL 5): 3350 / 3500 / 3600 / 3800 / 4000 mV
 *
 * The gauge is read at a slow cadence (never per frame) and cached; -1 means
 * "unknown" and callers must render a placeholder rather than a fake level.
 */
#ifndef RECHORD_DRIVERS_BATTERY_H
#define RECHORD_DRIVERS_BATTERY_H

#define BATTERY_LEVELS 5

/* Pure mapping (host-tested): millivolts -> 0..BATTERY_LEVELS-1, or -1. */
int battery_level_from_mv(int mv);

/* Percentage approximation: level * 25, or -1 when unknown. */
int battery_pct_from_mv(int mv);

/* Read the gauge now (8-sample average) and cache it. */
void battery_poll(void);

/* Cached values: -1 while unavailable. */
int battery_mv(void);
int battery_level(void);
int battery_percent(void);

#endif /* RECHORD_DRIVERS_BATTERY_H */
