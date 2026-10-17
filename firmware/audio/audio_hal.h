/*
 * audio_hal.h — internal seams of the audio output HAL.
 *
 * 1) Slot packing: the wire format is ONE 32-bit FIFO word per channel
 *    sample, left-justified (sample << 16); a stereo frame is TWO words,
 *    L slot then R slot (stock DMA payload, Service.c Bit_Convertor_*).
 *    ao_slot_word() is the canonical, host-testable expression of that
 *    packing. The hardware feed/ISR in rknano_i2s.c keeps the raw
 *    expression inline (verbatim-move constraint) — the host test asserts
 *    the two are identical, so this header is the executable spec.
 *
 * 2) Feeder seam: playback streams PCM from a caller-owned SPSC ring into
 *    the I2S FIFO. The current backend is the TX-empty IRQ ring feeder
 *    (rknano_i2s_attach/kick/hold — hardware-verified, DFIU 75271e5f).
 *    A future DMA ping-pong backend (the stock's descriptor ring at
 *    0x03090000 -> TXDR 0x40030024, ref FUN_03056550 tail) plugs in behind
 *    the same ops without touching codecs, player, or ipc_b_target.
 */
#ifndef RECHORD_AUDIO_HAL_H
#define RECHORD_AUDIO_HAL_H

#include <stdint.h>

/* 16-bit PCM sample -> 32-bit left-justified TXDR slot word. */
static inline uint32_t ao_slot_word(int16_t s)
{
    return (uint32_t)(uint16_t)s << 16;
}

/* Feeder backend ops (implemented once for now; see header comment). */
typedef struct
{
    void (*attach)(int (*pull)(uint32_t *word, void *user), void *user);
    void (*detach)(void);
    void (*kick)(void);
    void (*hold)(int held);
} ao_feeder_ops;

/* The single binding today: TX-empty IRQ ring feeder. */
extern const ao_feeder_ops ao_feeder_ring_irq;

#endif /* RECHORD_AUDIO_HAL_H */
