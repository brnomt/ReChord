/*
 * audio_scope — real-audio level/waveform tap for the theme engine.
 *
 * Reads the frames the DAC is about to play straight out of the shared PCM
 * ring (interleaved int32, left-justified; 8 bytes per stereo frame). Frames in
 * [rd, wr) belong to the consumer until played, so the producer never writes
 * them: the UI can read them without locking and without touching the ISR.
 * No buffers are copied (the AP's .bss and stack are both tight): the ring is
 * walked in place and reduced to a few numbers.
 *
 * Levels are 0..1000. The PCM is PRE-volume (the DAC applies volume), so
 * skins multiply by the volume themselves when they want a physical scale.
 */
#ifndef RECHORD_AUDIO_SCOPE_H
#define RECHORD_AUDIO_SCOPE_H

#include <stdint.h>

#define AUDIO_SCOPE_N       48     /* waveform points per channel            */
#define AUDIO_SCOPE_FRAMES  1024   /* frames reduced per update (~23 ms)     */

typedef struct {
    int lvl_l, lvl_r;              /* smoothed level, 0..1000 (fast attack)  */
    int peak_l, peak_r;            /* peak hold, 0..1000, falls ~1 s         */
    int16_t wave_l[AUDIO_SCOPE_N]; /* -1000..1000, mean per bucket           */
    int16_t wave_r[AUDIO_SCOPE_N];
} audio_scope_t;

void audio_scope_reset(audio_scope_t *s);

/* Reduce `frames` frames starting at byte offset `rd` of the ring
 * (ring_mask = size-1, power of two). `dt_ms` is the time since the last
 * update, so decay is the same at 20 and 60 fps. */
void audio_scope_feed(audio_scope_t *s, const volatile int32_t *ring,
                      uint32_t rd, uint32_t ring_mask, int frames, int dt_ms);

/* No audio (paused, stopped, underrun): decay levels, flatten the trace. */
void audio_scope_idle(audio_scope_t *s, int dt_ms);

#endif
