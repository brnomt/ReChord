/*
 * audio_output.h — centralized audio output controller (audio HAL facade).
 *
 * Pipeline (docs/rewrite/audio.md):
 *   player -> codec_t (pure) -> ipc_b ring -> audio_output -> rknano_i2s +
 *   cs43131. This controller is the ONLY layer that coordinates hardware
 *   on a format change; codecs never see MMIO.
 *
 * When the format changes, configure() runs the verified bring-up order
 * (moved verbatim from dac_cs43131.c/dac_init, commit c5d571f):
 *   power/reset GPIOs -> CRU gates -> I2C probe -> rate-table lookup ->
 *   ACODEC PLL mux -> I2S1 (CKR/TXCR) -> CS43131 register sequence ->
 *   per-rate DAC byte -> unmute.
 */
#ifndef RECHORD_AUDIO_OUTPUT_H
#define RECHORD_AUDIO_OUTPUT_H

#include <stdint.h>

#include "ringbuf.h"        /* player/ringbuf.h — shared SPSC util          */
#include "cs43131.h"        /* DAC_OK / DAC_ERR_* / diagnostics            */

/* Stream format at the HAL boundary. Codecs standardize on interleaved int32
 * PCM, left-justified to 32 bits (a 16-bit source is `sample << 16`, a 24-bit
 * one `sample << 8`; see codec.h) — which is exactly one I2S slot word.
 * `bits` carries the SOURCE depth (16/24/32). The DAC's ASP word length is
 * fixed at 32 bits by the bring-up (ASP_SPSIZE = 00), so every depth reads at
 * full scale and the source depth does not itself reconfigure the DAC. */
typedef struct
{
    uint32_t sample_rate;   /* Hz                                          */
    uint8_t  bits;          /* SOURCE depth: 16, 24 or 32                  */
    uint8_t  channels;      /* 1 = mono, 2 = stereo (interleaved)          */
} audio_format_t;

/* One-time controller bring-up (state only; hardware comes up in
 * configure()). */
int  audio_output_init(void);

/* (Re)lock clocks + I2S + CS43131 to `fmt`. Runs the full verified
 * bring-up sequence at fmt->sample_rate. Returns DAC_OK / DAC_ERR_*.
 * Accepts bits 16/24/32 and channels 1..2. A same-rate/same-channel re-open
 * with a different source depth keeps the DAC state (the ASP word length is
 * 32 bits for all depths). */
int  audio_output_configure(const audio_format_t *fmt);

/* Arm the TX feeder on `pcm_ring` and start the serializer. ring == NULL
 * selects the polled path (DAC test tone). */
int  audio_output_start(ringbuf_t *pcm_ring);

/* Feeder + I2S off (no DAC power-down: safe for gapless/hard boundaries —
 * the next configure() re-runs the bring-up). */
void audio_output_stop(void);
void audio_output_detach(void);
void audio_output_hold(int held);       /* 1 = pause feeding, 0 = resume  */
void audio_output_kick(void);           /* re-arm feeder after push       */

/* Frames drained to the I2S FIFO since start/flush (progress accounting). */
long audio_output_frames_played(void);
void audio_output_clear_played(void);

/* Full output teardown (hardware mute, DAC mute, amp off, I2S1 reset and
 * pad Hi-Z) — the verified dac_output_off sequence. */
void audio_output_power_off(void);

/* Click-free transitions (stock FUN_03055f48 / FUN_03056e54 pattern):
 * fade the PCM volume to mute before tearing the stream down (stop /
 * pause) and back to the user level when it resumes. Ramping is the
 * stock's 6-code / 2 ms software sequence on top of the PCM_SZC soft
 * ramp. No-op when the DAC is not up. */
void audio_output_fade_out(void);
void audio_output_fade_in(void);
/* Stop/track-change mute with the stock fixed ramp (not the user fade). */
void audio_output_quick_mute(void);

/* BRNOMT-67: play/pause fade duration in milliseconds (0 = immediate).
 * The app seeds it from the `fade` setting at boot and on every change;
 * configure() starts silent when a fade is active so fade_in() has a ramp. */
void audio_output_set_fade_ms(int ms);

/* User volume -> CS43131 PCM_VOLUME attenuation (0.5 dB/step), applied
 * with the stock's click-free soft ramp (= FUN_03055f48 to the user
 * level). `db` is dB*1 attenuation: 0 = 0 dB (max), negative attenuates
 * (-127.5 dB floor = 0xFF), positive clamps at 0 dB (the registers attenuate
 * only). Survives configure() — tracks relock to the current level. */
void audio_output_set_volume_db(int db);

/* Polled feed (DAC test tone): interleaved int16 stereo frames. */
int  audio_output_feed_blocking(const int16_t *pcm, int frames);

#endif /* RECHORD_AUDIO_OUTPUT_H */
