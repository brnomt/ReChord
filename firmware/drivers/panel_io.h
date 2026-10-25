/*
 * panel_io.h -- ReChord display transport abstraction (clean-room).
 *
 * The display driver speaks to the panel only through these five calls,
 * so the same driver code runs:
 *   - on target:   panel_io_rknanoc.c (GPIO bit-bang panel interface),
 *   - on the host: panel_io_sim.c (simulated DCS panel, writes PPM files),
 * making every display/font behavior unit-testable with `cc`.
 *
 * Wire contract (MIPI-DCS style, byte oriented):
 *   - commands are one byte (0x2A CASET, 0x2B RASET, 0x2C RAMWR, ...)
 *   - data bytes belong to the most recent command
 *   - an RGB565 pixel is two bytes on the wire, HIGH byte first
 */

#ifndef RECHORD_PANEL_IO_H
#define RECHORD_PANEL_IO_H

#include <stdint.h>

/* Bring the transport up (clocks/pins on target, state reset in sim). */
void panel_io_init(void);

/* Send one DCS command byte; the following data bytes belong to it. */
void panel_io_send_cmd(uint8_t cmd);

/* Send one parameter/data byte to the current command. */
void panel_io_send_data(uint8_t data);

/* Send one RGB565 pixel (two data bytes, high byte first) -- the fast
 * path used after RAMWR (0x2C). */
void panel_io_send_pixel(uint16_t rgb565);

/* Millisecond delay hook for init sequences (power-on delays).
 * The sim backend counts but never sleeps. */
void panel_io_delay_ms(uint16_t ms);

/* Backlight: on/off plus the 1..10 brightness level. The level is remembered
 * while the backlight is off and applied by the next backlight_on(). */
void backlight_on(void);
void backlight_off(void);
void backlight_set_level(int level);

/* Keys: blocking sample of the ADC ladder (0..1023, median of three fresh
 * conversions) and the power key level (1 = pressed). Used by the sampler's
 * fallback and by the blocking mini-loops; the normal path takes samples from
 * keys_scan_take() below. */
int keys_adc(void);
int keys_power(void);

/* Non-blocking key sampler (BRNOMT-98). keys_scan_tick() is the 1 ms hook
 * (call it from SysTick_Handler): it starts one fresh conversion per tick and
 * publishes the median of three as soon as they complete. keys_scan_take()
 * returns 1 once per completed scan and copies the sample out; the caller
 * still runs input_scan() in thread context. keys_scan_failed() turns 1 when
 * the converter never completes (the caller then falls back to the blocking
 * keys_adc()/keys_power() path). */
void keys_scan_tick(void);
int  keys_scan_take(int *adc, int *power);
int  keys_scan_failed(void);
/* One fresh SARADC conversion on `ch` (0 = battery, 1 = key ladder).
 * Returns the raw 10-bit value, or -1 when the channel is unavailable. */
int adc_read_channel(int ch);
void board_power_off(void);   /* does not return */

#endif /* RECHORD_PANEL_IO_H */
