/*
 * audio_hal.h — internal seams of the audio output HAL.
 *
 * 1) Slot packing: the wire format is ONE 32-bit FIFO word per channel
 *    sample, LEFT-JUSTIFIED, and a stereo frame is TWO words, L slot then
 *    R slot (stock DMA payload, Service.c Bit_Convertor_*). The internal
 *    pipeline sample is already an int32 left-justified to 32 bits
 *    (16-bit source -> `sample << 16`), so it IS the slot word:
 *    ao_slot_word(int32_t) is the identity, and ao_i16_to_s32() lifts a
 *    plain s16 sample (e.g. the polled DAC test tone) into that format.
 *    The hardware feed/ISR in rknano_i2s.c keep the raw expressions inline
 *    (verbatim-move constraint) — the host test asserts the two are
 *    identical, so this header is the executable spec.
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

/* int16 sample -> internal int32 sample (left-justified to 32 bits). */
static inline int32_t ao_i16_to_s32(int16_t s)
{
    return (int32_t)s << 16;
}

/* internal int32 sample -> 32-bit left-justified TXDR slot word. The
 * pipeline sample is already in slot format, so this is the identity. */
static inline uint32_t ao_slot_word(int32_t s)
{
    return (uint32_t)s;
}

/* Stream-depth gate at the HAL boundary. 16, 24 and 32 are all supported:
 * the internal PCM is left-justified to 32 bits, so a 32-bit ASP word length
 * reads every depth correctly (a 16-bit sample is `s16 << 16`, a 24-bit one
 * `s24 << 8` — both full-scale in the 32-bit slot). Other widths are rejected.
 * This is the executable spec of the DAC's ASP word length, which the
 * bring-up fixes at 32 bits (see audio_output.c: ASP_SPSIZE = 00). */
static inline int ao_format_bits_ok(int bits)
{
    return bits == 16 || bits == 24 || bits == 32;
}

/* User dB -> CS43131 PCM_VOLUME attenuation code (pure integer).
 * Datasheet register map p.128 / volume decode: "from 0 to -127.5 dB,
 * 0000 0001 = -0.5 dB" -> 2 codes per dB, 0x00 = 0 dB (unattenuated),
 * 0xFF = -127.5 dB. Positive dB clamps at 0 (the registers attenuate
 * only — no digital boost). */
static inline uint8_t ao_volume_atten_code(int db)
{
    int atten = -2 * db;

    if (atten < 0)
        atten = 0;
    if (atten > 255)
        atten = 255;
    return (uint8_t)atten;
}

/* Feeder backend ops (implemented once for now; see header comment). The
 * pull callback fills the two 32-bit slot words (*left, *right) of one
 * stereo frame and returns 1, or 0 when the ring is dry. */
typedef struct
{
    void (*attach)(int (*pull)(uint32_t *left, uint32_t *right, void *user),
                   void *user);
    void (*detach)(void);
    void (*kick)(void);
    void (*hold)(int held);
} ao_feeder_ops;

/* The single binding today: TX-empty IRQ ring feeder. */
extern const ao_feeder_ops ao_feeder_ring_irq;

#endif /* RECHORD_AUDIO_HAL_H */
