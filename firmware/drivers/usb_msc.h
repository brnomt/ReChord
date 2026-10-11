/*
 * usb_msc.h -- polled USB mass storage (DWC2 OTG device at 0x60080000)
 * exposing one eMMC window [base, base+count) as LUN 0.
 */
#ifndef RECHORD_USB_MSC_H
#define RECHORD_USB_MSC_H

#include <stdint.h>

enum {
    USB_EXIT_EJECT = 1,      /* host START STOP UNIT with LoEj */
    USB_EXIT_UNPLUG = 2,     /* VBUS gone */
    USB_EXIT_SUSPEND = 3,    /* host suspended after configuration */
    USB_EXIT_NO_HOST = 4,    /* no bus reset within ~3 s (charger) */
    USB_EXIT_IO = 5,         /* USB transfer failed */
    USB_EXIT_USER = 6,       /* user_exit() returned nonzero */
};

int usb_cable(void);         /* 1 = VBUS present */

/* Last session: [7:0] last SCSI opcode, [15:8] last START STOP byte 4,
 * [19:16] GINTSTS bits 10..13 ever seen (early suspend, suspend, reset, enum). */
extern uint32_t usb_msc_diag;

/* Runs the session until one of USB_EXIT_*; user_exit (may be NULL) is
 * polled continuously. */
int usb_msc_run(uint32_t base, uint32_t count, int (*user_exit)(void),
                uint32_t *rd_sectors, uint32_t *wr_sectors);

#endif
