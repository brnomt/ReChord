/*
 * usb_dac.h — USB Audio Class 1.0 playback device (48 kHz/16-bit stereo).
 *
 * The session enumerates a UAC1 audio function instead of mass storage,
 * receives isochronous OUT packets on EP1 and feeds them to the I2S/DAC
 * through services/usb_dac_sink.c. Polled, like the MSC session: no USB
 * interrupts are enabled.
 */
#ifndef RECHORD_DRIVERS_USB_DAC_H
#define RECHORD_DRIVERS_USB_DAC_H

#include <stdint.h>

typedef struct
{
    uint32_t packets;
    uint32_t frames;
    uint32_t dropped;
    uint32_t repeated;
    uint8_t  alt;
    uint8_t  configured;
    int      exit;
} usb_dac_diag_t;

/* Runs until USB_EXIT_* (usb_msc.h). `user_exit` may be NULL. */
int usb_dac_run(int (*user_exit)(void), usb_dac_diag_t *diag);

#endif /* RECHORD_DRIVERS_USB_DAC_H */
