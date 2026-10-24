/*
 * cdc_overlay.c -- CDC-ACM console overlay linked at 0x0304AE64.
 *
 * The AP copies this blob from /.rechord/cdc.bin into the free loader-data
 * window and calls cdc_overlay_main(). usb_dwc2_init/stop and usb_cable are
 * AP functions resolved by the overlay link (absolute symbols).
 */
#include "usb_cdc.h"
#include "usb_msc.h"

int cdc_overlay_main(void)
{
    usb_cdc_init();
    usb_cdc_puts("ReChord CDC overlay\r\n"
                 "commands: type to echo, q to quit\r\n");

    for (;;) {
        int c;

        usb_cdc_poll();
        c = usb_cdc_getc();
        if (c < 0)
            continue;
        if (c == 'q')
            break;
        {
            char echo[2];
            echo[0] = (char)c;
            echo[1] = 0;
            usb_cdc_puts(echo);
        }
    }

    usb_dwc2_stop();
    return 0;
}
