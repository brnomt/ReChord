/*
 * panel_init_rknanoc.c -- Echo Mini panel power-on DCS init (clean-room).
 *
 * This file encodes the panel INIT SEQUENCE as hardware CONTRACT DATA
 * (command bytes + parameter bytes + delays) extracted from the reference
 * firmware image -- no reference code is copied or paraphrased. The
 * extraction method and the byte-level evidence live in
 * docs/rewrite/drivers.md ("Panel init sequence").
 *
 * Despite the file name, this code is transport-agnostic: it emits the
 * stream only through panel_io.h, so it is linked into BOTH the target
 * image (panel_io_rknanoc.c backend) and the host test builds
 * (panel_io_sim.c backend).
 *
 * Background (extracted facts, docs/rewrite/drivers.md has the trace):
 * the reference's display bring-up is a function at RAM 0x03051F14 that
 *   1. resets the panel (GPIO2.15 active-low pulse) and probes a 4-byte
 *      panel ID through the GPIO0 bus (input port 0x40160050 bits 16..23),
 *   2. selects one of TWO init variants by that ID:
 *        (id & 0x00FF0000) == 0x00E80000 -> variant A
 *        anything else (incl. a board-strap path that forces the ID to
 *        0x66666666)                    -> variant B
 *   3. sends the DCS command stream of the selected variant, then clears
 *      the framebuffer and flushes it.
 *
 * We cannot probe the ID yet (panel_io.h has no bus-read primitive and
 * the ID probe sequence is unverified), so the variant is chosen at
 * compile time here. UNVERIFIED ON HARDWARE: which variant the Echo Mini
 * panel actually needs. If the panel lights up with wrong colors or
 * gamma, rebuild with the other value of PANEL_INIT_VARIANT -- or call
 * panel_init_run_variant() from bring-up code to try one at runtime.
 */

#include <stdint.h>

#include "display.h"
#include "panel_io.h"

/*
 * Compile-time variant selection: 0 = variant A, 1 = variant B.
 * Default 1 (variant B): its 0x2B window row range (0x0023..0x00CC =
 * rows 35..204) exactly matches the +35 row offset this device's flush
 * path uses (PANEL_ROW_OFFSET), which makes it the variant whose table
 * was authored for this geometry. UNVERIFIED on hardware (see header).
 */
#ifndef PANEL_INIT_VARIANT
#define PANEL_INIT_VARIANT 1
#endif

#if (PANEL_INIT_VARIANT != 0) && (PANEL_INIT_VARIANT != 1)
#error "PANEL_INIT_VARIANT must be 0 (variant A) or 1 (variant B)"
#endif

#define PANEL_INIT_MAX_DATA 16   /* longest command payload: 14 (gamma) */

typedef struct {
    uint8_t  cmd;                    /* DCS command byte                  */
    uint8_t  nbytes;                 /* parameter bytes in data[] (0..16) */
    uint8_t  data[PANEL_INIT_MAX_DATA];
    uint16_t delay_ms;               /* wait AFTER this step              */
} panel_init_step_t;

/* Terminator row: cmd 0x00 is DCS NOP, a safe no-op if it ever leaked. */
#define PANEL_INIT_END 0x00

/*
 * Variant A -- sent by the reference when the probed ID byte (bits
 * 16..23 of the assembled 4-byte ID) is 0xE8.
 * Command meanings are ST7789-family inferences (the 0xDF "5A 69 02 01"
 * unlock is the ST7789V2 "Enable Command 2" password); register-level
 * effects are UNVERIFIED, the bytes are exact.
 */
static const panel_init_step_t panel_init_variant_a[] = {
    /* cmd   n  data (hex)                                 delay  meaning   */
    { 0x11, 0, { 0 },                                     120 }, /* sleep out */
    { 0xDF, 4, { 0x5A, 0x69, 0x02, 0x01, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* vendor CMD2 unlock */
    { 0x36, 1, { 0x60, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* MADCTL: MX|MV */
    { 0x3A, 1, { 0x05, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* COLMOD: 16 bpp */
    { 0xB2, 5, { 0x0B, 0x0B, 0x00, 0x33, 0x33, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* porch control */
    { 0xB7, 1, { 0x75, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* gate control */
    { 0xBB, 1, { 0x28, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* VCOMS */
    { 0xC0, 1, { 0x2C, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* LCMCTRL */
    { 0xC2, 1, { 0x01, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* VDVVRHEN */
    { 0xC3, 1, { 0x1F, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* VRHS */
    { 0xC5, 1, { 0x2A, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* VDVS */
    { 0xC6, 1, { 0x13, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* FRCTRL2 */
    { 0xD0, 1, { 0xA7, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* PWCTRL1 (1 arg) */
    { 0xD0, 2, { 0xA4, 0xA1, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* PWCTRL1 (2 args) */
    { 0xD6, 1, { 0xA1, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* gate EQ */
    { 0xE0, 14, { 0xF0, 0x05, 0x0A, 0x06, 0x06, 0x03, 0x2B, 0x32,
                  0x43, 0x36, 0x11, 0x10, 0x2B, 0x32, 0, 0 }, 0 }, /* gamma + */
    { 0xE1, 14, { 0xF0, 0x08, 0x0C, 0x0B, 0x09, 0x24, 0x2B, 0x22,
                  0x43, 0x38, 0x15, 0x16, 0x2F, 0x37, 0, 0 }, 0 }, /* gamma - */
    { 0x21, 0, { 0 },                                        0 }, /* INVON */
    { 0x29, 0, { 0 },                                        0 }, /* DISPON */
    { 0x2C, 0, { 0 },                                        0 }, /* RAMWR (empty) */
    { PANEL_INIT_END, 0, { 0 }, 0 }
};

/*
 * Variant B -- the reference's fallback for every other panel ID.
 * Same caveat: meanings are ST7789-family inferences, bytes are exact.
 * Note the 0x2A/0x2B window pair: columns 0..0x00EF, rows 0x0023..0x00CC.
 * The column range disagrees with the runtime flush path (which programs
 * 0..0x013F); every flush re-programs the window, so the discrepancy is
 * documented but kept verbatim (extracted fact).
 */
static const panel_init_step_t panel_init_variant_b[] = {
    /* cmd   n  data (hex)                                 delay  meaning   */
    { 0x11, 0, { 0 },                                     120 }, /* sleep out */
    { 0x36, 1, { 0x60, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* MADCTL: MX|MV */
    { 0x3A, 1, { 0x05, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* COLMOD: 16 bpp */
    { 0xA4, 2, { 0x44, 0x44, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* vendor (unknown) */
    { 0xB2, 5, { 0x0C, 0x0C, 0x00, 0x33, 0x33, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* porch control */
    { 0xB7, 1, { 0x35, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* gate control */
    { 0xBB, 1, { 0x1A, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* VCOMS */
    { 0xC0, 1, { 0x2C, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* LCMCTRL */
    { 0xC2, 1, { 0x01, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* VDVVRHEN */
    { 0xC3, 1, { 0x0B, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* VRHS */
    { 0xC4, 1, { 0x20, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* VDVS */
    { 0xC6, 1, { 0x0F, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* FRCTRL2 */
    { 0xD0, 2, { 0xA4, 0xA1, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* PWCTRL1 */
    { 0xD6, 1, { 0xA1, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* gate EQ */
    { 0xE0, 14, { 0xF0, 0x06, 0x0B, 0x07, 0x07, 0x24, 0x2E, 0x32,
                  0x46, 0x37, 0x13, 0x13, 0x2D, 0x33, 0, 0 }, 0 }, /* gamma + */
    { 0xE1, 14, { 0xF0, 0x02, 0x06, 0x09, 0x08, 0x05, 0x29, 0x44,
                  0x42, 0x38, 0x14, 0x14, 0x2A, 0x30, 0, 0 }, 0 }, /* gamma - */
    { 0x2A, 4, { 0x00, 0x00, 0x00, 0xEF, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* CASET 0..239 */
    { 0x2B, 4, { 0x00, 0x23, 0x00, 0xCC, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0 },                   0 }, /* RASET 35..204 */
    { 0x21, 0, { 0 },                                        0 }, /* INVON */
    { 0x29, 0, { 0 },                                        0 }, /* DISPON */
    { 0x2C, 0, { 0 },                                        0 }, /* RAMWR (empty) */
    { PANEL_INIT_END, 0, { 0 }, 0 }
};

static void panel_init_run_table(const panel_init_step_t *table)
{
    const panel_init_step_t *step;

    for (step = table; step->cmd != PANEL_INIT_END; step++) {
        int i;

        panel_io_send_cmd(step->cmd);
        for (i = 0; i < step->nbytes && i < PANEL_INIT_MAX_DATA; i++) {
            panel_io_send_data(step->data[i]);
        }
        if (step->delay_ms > 0) {
            panel_io_delay_ms(step->delay_ms);
        }
    }
}

int panel_init_run_variant(int variant)
{
    if (variant == 0) {
        panel_init_run_table(panel_init_variant_a);
        return 0;
    }
    if (variant == 1) {
        panel_init_run_table(panel_init_variant_b);
        return 0;
    }
    return -1;
}

void panel_init_sequence(void)
{
    (void)panel_init_run_variant(PANEL_INIT_VARIANT);
}
