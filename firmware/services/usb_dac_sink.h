/*
 * usb_dac_sink.h — USB Audio Class isochronous packets -> PCM ring.
 *
 * The USB DAC session receives one isochronous OUT packet per 1 ms frame
 * (48 frames at 48 kHz) and hands interleaved little-endian samples to the
 * I2S feeder through the same SPSC ring the player uses. The host clock and
 * the DAC clock are independent, so a small proportional correction drops
 * or repeats one frame when the buffer drifts away from its target.
 *
 * Pure (no MMIO, no allocation) so the policy is host-testable.
 */
#ifndef RECHORD_SERVICES_USB_DAC_SINK_H
#define RECHORD_SERVICES_USB_DAC_SINK_H

#include <stdint.h>

#include "ringbuf.h"

#define USB_DAC_MAX_FRAMES 64u   /* largest packet: 196 B / 4 B (16-bit) */

typedef struct
{
    ringbuf_t *ring;
    uint32_t   rate;
    uint32_t   target;      /* frames of cushion (~20 ms)              */
    uint32_t   guard;       /* dead band around the target (~4 ms)     */
    uint8_t    sample_bytes;/* source depth: 2, 3 or 4                 */
    uint8_t    started;
    uint32_t   packets;
    uint32_t   frames;
    uint32_t   dropped;
    uint32_t   repeated;
} usb_dac_sink_t;

/* Attach a ring (already initialised by the caller). */
void usb_dac_sink_init(usb_dac_sink_t *sink, ringbuf_t *ring);

/* Start a stream at `rate` with `sample_bytes` source samples. Does not
 * touch the ring; the caller resets it first. */
void usb_dac_sink_start(usb_dac_sink_t *sink, uint32_t rate, uint8_t sample_bytes);

/* Fill `target` frames of silence so the I2S has a cushion before the host
 * starts sending. Call once after start and after the ring is reset. */
void usb_dac_sink_prefill(usb_dac_sink_t *sink);

/* Push one isochronous packet. Returns the number of frames written
 * (including a drift repeat; excluding a drift drop). */
uint32_t usb_dac_sink_write(usb_dac_sink_t *sink, const uint8_t *bytes, uint32_t length);

/* Frames currently buffered (diagnostics). */
uint32_t usb_dac_sink_fill(const usb_dac_sink_t *sink);

#endif /* RECHORD_SERVICES_USB_DAC_SINK_H */
