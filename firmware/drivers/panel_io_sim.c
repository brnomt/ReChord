/*
 * panel_io_sim.c -- HOST-SIMULATED panel_io backend (see panel_io_sim.h).
 *
 * Models a DCS panel closely enough to catch driver bugs on the build
 * machine: window registers (CASET/RASET), an auto-incrementing RAMWR
 * cursor, pixel assembly from byte pairs (high byte first), and strict
 * bounds checking with a fault counter.
 *
 * Geometry: the wire carries PHYSICAL panel rows (the driver adds
 * PANEL_ROW_OFFSET to every RASET parameter -- extracted hardware
 * contract, see display.h). The simulated framebuffer models only the
 * visible DISPLAY_H-row band (physical rows PANEL_ROW_OFFSET ..
 * PANEL_ROW_OFFSET+DISPLAY_H-1); panel_io_sim_pixel() and the PPM dump
 * take LOGICAL rows 0..DISPLAY_H-1, so tests stay geometry-agnostic.
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
#define SIM_CMD_MAX_ARGS 16

/* Physical row range of the visible band (extracted contract). */
#define SIM_BAND_Y0 PANEL_ROW_OFFSET
#define SIM_BAND_Y1 (PANEL_ROW_OFFSET + PANEL_SIM_H - 1)

static uint16_t sim_fb[PANEL_SIM_H][PANEL_SIM_W];

static int cur_cmd;          /* command whose parameters we are collecting */
static uint8_t param[4];     /* collected parameter bytes                 */
static int nparam;
static uint8_t pixel_hi;     /* pending high byte of an RGB565 pixel      */
static int have_hi;

static int xs, ys, xe, ye;   /* window registers (CASET/RASET; rows PHYSICAL) */
static int cx, cy;           /* RAMWR cursor (cy physical)                */
static int have_caset, have_raset;

static unsigned overflow_count;
static unsigned delay_total_ms;

static struct {
    uint8_t cmd;
    uint8_t nargs;
    uint8_t args[SIM_CMD_MAX_ARGS];
} cmd_log[SIM_CMD_LOG_MAX];
static int cmd_log_len;
static int cur_entry;        /* index of the command being logged, or -1 */

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
    cur_entry = -1;
}

void panel_io_init(void)
{
    panel_io_sim_reset();
}

void panel_io_delay_ms(uint16_t ms)
{
    delay_total_ms += ms;
}

void backlight_on(void)
{
}

void backlight_off(void)
{
}

int keys_adc(void)
{
    return 1023;   /* idle ladder */
}

int keys_power(void)
{
    return 0;
}

int adc_read_channel(int ch)
{
    /* Sim: the key ladder idles at 1023; the battery channel is unknown. */
    return ch == 1 ? 1023 : -1;
}

void board_power_off(void)
{
    for (;;) { }
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
        cmd_log[cmd_log_len].cmd = cmd;
        cmd_log[cmd_log_len].nargs = 0;
        cur_entry = cmd_log_len;
        cmd_log_len++;
    } else {
        cur_entry = -1;
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
    return cmd_log[i].cmd;
}

int panel_io_sim_cmd_nargs(int i)
{
    if (i < 0 || i >= cmd_log_len) {
        return 0;
    }
    return cmd_log[i].nargs;
}

uint8_t panel_io_sim_cmd_arg(int i, int k)
{
    if (i < 0 || i >= cmd_log_len || k < 0 || k >= cmd_log[i].nargs) {
        return 0;
    }
    return cmd_log[i].args[k];
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
                     cy >= SIM_BAND_Y0 && cy <= SIM_BAND_Y1);

    if (in_window && on_panel) {
        sim_fb[cy - SIM_BAND_Y0][cx] = rgb565;
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
    /* Record the byte in the current command's argument log. */
    if (cur_entry >= 0) {
        if (cmd_log[cur_entry].nargs < SIM_CMD_MAX_ARGS) {
            cmd_log[cur_entry].args[cmd_log[cur_entry].nargs++] = data;
        }
    }

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
