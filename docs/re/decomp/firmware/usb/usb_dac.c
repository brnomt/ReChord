#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/firmware/usb/usb_dac.c
 * Reconstructed from Ghidra decompilation of:
 *   USB_DAC_OpenStream @ 0x0302b80a
 *
 * Called from MusicInit when g_usb_state == 99 (USB DAC mode).
 * Talks to ROM USB helpers to open a host-facing stream; toggles an
 * internal mode byte between 2 (player) and 4 (DAC), and on success
 * posts event 0xf5 (USB mode change).
 *
 * UI resources: USB_DAC_*.BMP / USB_DATA_*.BMP / USB_PLAYER_*.BMP
 * (theme variants A–E). Actual USB connect/disconnect is ROM:
 *   rom_usb_connect    @ 0x02ffb2e0
 *   rom_usb_disconnect @ 0x02ffb3e6
 */

#include "../../firmware.h"
#include "../os/os_api.h"
#include "usb_api.h"

extern int  rom_usb_stream_open(void *dev, int mode);  /* func_0x02ffe3a0 */
extern int  rom_usb_query(void);                       /* func_0x02ff7890 */
extern void rom_usb_set_mode(uint8_t mode);            /* func_0x02ffbf72 */
extern void usb_dac_ui_enter(void);                    /* FUN_0300ac14 */
extern void usb_dac_ui_leave(void);                    /* FUN_0300ac16 */

extern uint8_t *g_usb_mode_ctx;   /* DAT_0302b9a4 — +0xe mode, +0x1f flag */
extern int16_t *g_usb_stream_id;  /* DAT_0302b9b8 */
extern void    *g_usb_dev;        /* DAT_0302b9c8 */

/*
 * USB_DAC_OpenStream @ 0x0302b80a
 *
 * param_1 = stream mode (MusicInit passes 3).
 * Returns 1 if mode switch completed (event 0xf5 posted), else 0.
 */
int USB_DAC_OpenStream(int mode)
{
    int rc;
    uint8_t *ctx = g_usb_mode_ctx;

    rc = rom_usb_stream_open(g_usb_dev, mode);
    *g_usb_stream_id = (int16_t)rc;
    if (rc != 0)
        return 0;

    if (ctx[0xe] == 2) {
        /* Currently in player mode — try enter DAC */
        if (ctx[0x1f] != 0 && rom_usb_query() == 1) {
            ctx[0xe] = 4;
            usb_dac_ui_enter();
            rom_usb_set_mode(ctx[0xe]);
        }
        rc = rom_usb_stream_open(g_usb_dev, mode);
        *g_usb_stream_id = (int16_t)rc;
    } else if (ctx[0xe] == 4) {
        /* Currently DAC — drop back to player if flag clear path */
        if (ctx[0x1f] == 0)
            return 0;
        ctx[0xe] = 2;
        rom_usb_set_mode(2);
        usb_dac_ui_leave();
        rc = rom_usb_stream_open(g_usb_dev, mode);
        *g_usb_stream_id = (int16_t)rc;
    } else {
        return 0;
    }

    if (rc == 0) {
        ctx[0xe] = 2;
        rom_usb_set_mode(2);
        usb_dac_ui_leave();
        event_set(0xf5);
        return 1;
    }
    return 0;
}
