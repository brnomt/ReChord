/*
 * panel_io_sim.c -- HOST-SIMULATED panel_io backend (see panel_io_sim.h).
 *
 * Models a DCS panel closely enough to catch driver bugs on the build
 * machine: window registers (CASET/RASET), an auto-incrementing RAMWR
 * cursor, pixel assembly from byte pairs (high byte first), and strict
 * bounds checking with a fault counter.
 */

#include <stdio.h>
#include <string.h>

#include "display.h"
#include "panel_io.h"
#include "panel_io_sim.h"

#if PANEL_SIM_W != DISPLAY_W
#error "PANEL_SIM_W must equal DISPLAY_W"
#endif
#if PANEL_SIM_H != DISPLAY_H
#error "PANEL_SIM_H must equal DISPLAY_H"
#endif

#define CMD_CASET 0x2A
#define CMD_RASET 0x2B
#define CMD_RAMWR 0x2C

#define SIM_CMD_LOG_MAX 256

static uint16_t sim_fb[PANEL_SIM_H][PANEL_SIM_W];

static int cur_cmd;          /* command whose parameters we are collecting */
static uint8_t param[4];     /* collected parameter bytes                 */
static int nparam;
static uint8_t pixel_hi;     /* pending high byte of an RGB565 pixel      */
static int have_hi;

static int xs, ys, xe, ye;   /* window registers (set by CASET/RASET)     */
static int cx, cy;           /* RAMWR cursor                              */
static int have_caset, have_raset;

static unsigned overflow_count;
static unsigned delay_total_ms;

static uint8_t cmd_log[SIM_CMD_LOG_MAX];
static int cmd_log_len;

void panel_io_sim_reset(void)
{
    memset(sim_fb, 0, sizeof(sim_fb));
    cur_cmd = 0;
    nparam = 0;
    have_hi = 0;
    xs = ys = xe = ye = 0;
    cx = cy = 0;
    have_caset = have_raset = 0;
    overflow_count = 0;
    delay_total_ms = 0;
    cmd_log_len = 0;
}

void panel_io_init(void)
{
    panel_io_sim_reset();
}

void panel_io_delay_ms(uint16_t ms)
{
    delay_total_ms += ms;
}

unsigned panel_io_sim_delay_total_ms(void)
{
    return delay_total_ms;
}

void panel_io_send_cmd(uint8_t cmd)
{
    cur_cmd = cmd;
    nparam = 0;
    have_hi = 0;
    if (cmd == CMD_RAMWR) {
        cx = xs;
        cy = ys;
    }
    if (cmd_log_len < SIM_CMD_LOG_MAX) {
        cmd_log[cmd_log_len++] = cmd;
    }
}

int panel_io_sim_cmd_count(void)
{
    return cmd_log_len;
}

uint8_t panel_io_sim_cmd_at(int i)
{
    if (i < 0 || i >= cmd_log_len) {
        return 0;
    }
    return cmd_log[i];
}

void panel_io_sim_last_window(int win[4])
{
    win[0] = have_caset ? xs : -1;
    win[1] = have_raset ? ys : -1;
    win[2] = have_caset ? xe : -1;
    win[3] = have_raset ? ye : -1;
}

static void sim_write_pixel(uint16_t rgb565)
{
    int in_window = (cx >= xs && cx <= xe && cy >= ys && cy <= ye);
    int on_panel  = (cx >= 0 && cx < PANEL_SIM_W &&
                     cy >= 0 && cy < PANEL_SIM_H);

    if (in_window && on_panel) {
        sim_fb[cy][cx] = rgb565;
    } else {
        overflow_count++;
    }

    /* RAMWR cursor auto-increment, wrapping at the window's right edge. */
    cx++;
    if (cx > xe) {
        cx = xs;
        cy++;
    }
}

void panel_io_send_data(uint8_t data)
{
    switch (cur_cmd) {
    case CMD_CASET:
    case CMD_RASET:
        if (nparam < 4) {
            param[nparam++] = data;
        }
        if (nparam == 4) {
            int start = ((int)param[0] << 8) | param[1];
            int end   = ((int)param[2] << 8) | param[3];
            if (cur_cmd == CMD_CASET) {
                xs = start;
                xe = end;
                have_caset = 1;
            } else {
                ys = start;
                ye = end;
                have_raset = 1;
            }
            nparam = 0;   /* tolerate a repeated window set */
        }
        break;

    case CMD_RAMWR:
        if (!have_hi) {
            pixel_hi = data;
            have_hi = 1;
        } else {
            uint16_t rgb565 =
                (uint16_t)(((uint16_t)pixel_hi << 8) | data);
            have_hi = 0;
            sim_write_pixel(rgb565);
        }
        break;

    default:
        /* Unknown/no-parameter commands swallow stray data bytes. */
        break;
    }
}

void panel_io_send_pixel(uint16_t rgb565)
{
    /* Model the wire exactly: two data bytes, high byte first. */
    panel_io_send_data((uint8_t)(rgb565 >> 8));
    panel_io_send_data((uint8_t)(rgb565 & 0xFFu));
}

uint16_t panel_io_sim_pixel(int x, int y)
{
    if (x < 0 || x >= PANEL_SIM_W || y < 0 || y >= PANEL_SIM_H) {
        return 0;
    }
    return sim_fb[y][x];
}

unsigned panel_io_sim_overflow(void)
{
    return overflow_count;
}

int panel_io_sim_write_ppm(const char *path)
{
    FILE *f;
    int y, x;

    if (path == NULL) {
        return -1;
    }
    f = fopen(path, "wb");
    if (f == NULL) {
        return -1;
    }
    fprintf(f, "P6\n%d %d\n255\n", PANEL_SIM_W, PANEL_SIM_H);
    for (y = 0; y < PANEL_SIM_H; y++) {
        for (x = 0; x < PANEL_SIM_W; x++) {
            uint16_t v = sim_fb[y][x];
            unsigned char rgb[3];
            /* RGB565 -> RGB888, full-range scaling */
            rgb[0] = (unsigned char)((((v >> 11) & 0x1F) * 255u) / 31u);
            rgb[1] = (unsigned char)((((v >> 5) & 0x3F) * 255u) / 63u);
            rgb[2] = (unsigned char)(((v & 0x1F) * 255u) / 31u);
            if (fwrite(rgb, 1, 3, f) != 3) {
                fclose(f);
                return -1;
            }
        }
    }
    if (fclose(f) != 0) {
        return -1;
    }
    return 0;
}
