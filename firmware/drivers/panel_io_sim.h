/*
 * panel_io_sim.h -- HOST-SIMULATED panel_io backend (test builds only).
 *
 * Implements the panel_io.h contract against an in-RAM model of a DCS
 * panel: it parses CASET (0x2A) / RASET (0x2B) / RAMWR (0x2C) like a real
 * controller would, stores received pixels in a simulated framebuffer,
 * and can dump that framebuffer as a binary PPM (P6) so tests can produce
 * visual artifacts. Everything else (unknown commands) is logged and
 * ignored.
 *
 * The simulated panel geometry equals the real one (DISPLAY_W x DISPLAY_H
 * from display.h; enforced at compile time). Writes outside the current
 * window or outside the panel are counted in a fault counter that tests
 * assert to be zero.
 *
 * Link this file ONLY in host builds, in place of panel_io_rknanoc.c.
 */

#ifndef RECHORD_PANEL_IO_SIM_H
#define RECHORD_PANEL_IO_SIM_H

#include <stdint.h>

/* Simulated panel geometry (must equal DISPLAY_W / DISPLAY_H). */
#define PANEL_SIM_W 320
#define PANEL_SIM_H 170

/* Reset all state: framebuffer to 0, windows/cursor cleared, logs empty. */
void panel_io_sim_reset(void);

/* Read a simulated panel pixel. Out-of-range coordinates return 0. */
uint16_t panel_io_sim_pixel(int x, int y);

/* Write the simulated framebuffer to 'path' as a binary PPM (P6).
 * Returns 0 on success, -1 on I/O error. */
int panel_io_sim_write_ppm(const char *path);

/* Number of pixels written outside the current window/panel bounds. */
unsigned panel_io_sim_overflow(void);

/* Command byte log: the i-th command byte sent since the last reset. */
int panel_io_sim_cmd_count(void);
uint8_t panel_io_sim_cmd_at(int i);

/* Last CASET/RASET values seen since the last reset, as
 * win[0]=xs win[1]=ys win[2]=xe win[3]=ye. Any never-received component
 * is -1. */
void panel_io_sim_last_window(int win[4]);

/* Total milliseconds requested through panel_io_delay_ms since reset. */
unsigned panel_io_sim_delay_total_ms(void);

#endif /* RECHORD_PANEL_IO_SIM_H */
