/*
 * panel_io_rknanoc.c -- TARGET panel_io backend for the RKNanoC (Cortex-M3).
 *
 * Implements the panel_io.h contract on the VOP MCU-panel interface, the
 * same documented hardware path the vendor SDK uses for its DCS panels
 * (firmware/rockchip/driver/vop/vop.c: VopSendCmd/VopSendData write the
 * VopMcuCmd / VopMcuData registers of the VOP block at 0x60070000;
 * register map in firmware/rockchip/driver/vop/hw_vop.h).
 *
 * Observed register semantics from the SDK (facts, not copied code):
 *   - VopMcuCmd  takes one command byte per write
 *   - VopMcuData takes one parameter byte per write, OR one full RGB565
 *     pixel per write after RAMWR (the block serializes it to the 8-bit
 *     panel bus)
 *
 * Link this file ONLY in target builds, in place of panel_io_sim.c.
 *
 * TODO (bring-up, see docs/rewrite/drivers.md "open items"):
 *   - VOP clock gating / pinmux bring-up is assumed to be done by earlier
 *     boot stages; verify and move it into panel_io_init().
 *   - FIFO flow control: the SDK paces transfers via VopMcuStatus /
 *     VopMcuFIFOWaterMark and VOP "split" mode around rows. Whether the
 *     bare register writes above are safe at 200 MHz without that pacing
 *     must be validated on hardware.
 *   - panel_io_delay_ms(): the loop below is an uncalibrated placeholder.
 */

#include <stdint.h>

#include "panel_io.h"

/* VOP MCU register block -- matches vendor SDK hw_vop.h (VOP struct),
 * base from firmware/rockchip/driver/hw_memap.h. Kept local so this
 * file has no dependency on SDK headers. */
typedef struct {
    volatile uint32_t con;
    volatile uint32_t version;
    volatile uint32_t timing;
    volatile uint32_t lcd_size;
    volatile uint32_t fifo_watermark;
    volatile uint32_t srt;
    volatile uint32_t int_en;
    volatile uint32_t int_clear;
    volatile uint32_t int_status;
    volatile uint32_t status;
    volatile uint32_t cmd;
    volatile uint32_t data;
    volatile uint32_t start;
} vop_mcu_regs_t;

#define VOP_BASE_ADDR 0x60070000u
#define VOP0 ((vop_mcu_regs_t *)VOP_BASE_ADDR)

void panel_io_init(void)
{
    /* TODO: move VOP clock/pinmux bring-up here (see file header). */
}

void panel_io_send_cmd(uint8_t cmd)
{
    VOP0->cmd = cmd;
}

void panel_io_send_data(uint8_t data)
{
    VOP0->data = data;
}

void panel_io_send_pixel(uint16_t rgb565)
{
    /* One register write per pixel: the VOP serializes high byte first
     * onto the panel bus (matches the SDK's pixel path). */
    VOP0->data = rgb565;
}

void panel_io_delay_ms(uint16_t ms)
{
    /* TODO: replace with a calibrated timer/SysTick delay at bring-up.
     * Busy loop scaled crudely for the 200 MHz boot clock. */
    volatile uint32_t count = (uint32_t)ms * 20000u;
    while (count-- > 0u) {
        /* spin */
    }
}
