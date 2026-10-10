/*
 * panel_io.h -- ReChord display transport abstraction (clean-room).
 *
 * The display driver speaks to the panel only through these five calls,
 * so the same driver code runs:
 *   - on target:   panel_io_rknanoc.c (RKNanoC VOP MCU register interface),
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

#endif /* RECHORD_PANEL_IO_H */
