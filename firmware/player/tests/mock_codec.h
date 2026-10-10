/*
 * mock_codec.h — scripted codec for the player host tests.
 *
 * Generates a deterministic pseudo-signal (mock_sample) so tests can
 * verify streamed PCM content end-to-end against the same formula the
 * decoder emits. Failure injection covers open errors and mid-stream
 * decode errors (-> PLAYER_ERROR).
 */
#ifndef PLAYER_MOCK_CODEC_H
#define PLAYER_MOCK_CODEC_H

#include "../codec.h"

typedef struct
{
    long total_frames;    /* track length in frames                       */
    long sample_rate;     /* Hz (default 44100)                           */
    int  channels;        /* 1 or 2 (default 2)                           */
    int  fail_open;       /* nonzero: open() returns CODEC_ERR_IO         */
    long fail_at_frame;   /* decode() fails at this frame; -1 = never     */
} mock_codec_cfg_t;

/* Reset to defaults (44100 frames @ 44100 Hz stereo, no failures) and
 * apply `cfg` (may be NULL). */
void mock_codec_configure(const mock_codec_cfg_t *cfg);

/* The codec_t to register with player_register_codec("mock", ...). */
const codec_t *mock_codec_get(void);

/* The exact sample the mock decoder produces for (frame, channel). */
int16_t mock_sample(long frame, int ch);

#endif /* PLAYER_MOCK_CODEC_H */
