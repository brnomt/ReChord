/*
 * mock_codec.c — scripted codec implementation (see mock_codec.h).
 */
#include <string.h>

#include "mock_codec.h"

typedef struct
{
    long frame;           /* next frame to produce                        */
    mock_codec_cfg_t cfg;
} mock_state_t;

static mock_codec_cfg_t g_cfg = { 44100, 44100, 2, 0, -1 };

void mock_codec_configure(const mock_codec_cfg_t *cfg)
{
    g_cfg.total_frames = 44100;
    g_cfg.sample_rate = 44100;
    g_cfg.channels = 2;
    g_cfg.fail_open = 0;
    g_cfg.fail_at_frame = -1;
    if (cfg != 0)
        g_cfg = *cfg;
    if (g_cfg.sample_rate <= 0)
        g_cfg.sample_rate = 44100;
    if (g_cfg.channels < 1 || g_cfg.channels > 2)
        g_cfg.channels = 2;
    if (g_cfg.total_frames < 0)
        g_cfg.total_frames = 0;
}

int16_t mock_sample(long frame, int ch)
{
    return (int16_t)(((frame * 37 + ch * 101) % 20000) - 10000);
}

static int mock_open(void *vst, const char *path)
{
    mock_state_t *st = (mock_state_t *)vst;

    if (st == 0 || path == 0)
        return CODEC_ERR_STATE;
    if (g_cfg.fail_open)
        return CODEC_ERR_IO;

    st->frame = 0;
    st->cfg = g_cfg;
    return CODEC_OK;
}

static int mock_decode(void *vst, int16_t *pcm, int max_frames)
{
    mock_state_t *st = (mock_state_t *)vst;
    int i, ch, n;

    if (st == 0 || pcm == 0)
        return CODEC_ERR_STATE;

    if (st->cfg.fail_at_frame >= 0 && st->frame >= st->cfg.fail_at_frame)
        return CODEC_ERR_IO;
    if (st->frame >= st->cfg.total_frames)
        return 0;                      /* end of stream */

    n = max_frames;
    if (st->frame + n > st->cfg.total_frames)
        n = (int)(st->cfg.total_frames - st->frame);
    /* Fail mid-stream at an exact frame: clamp this chunk to the failure
     * point; the next decode() call returns the error. */
    if (st->cfg.fail_at_frame >= 0 &&
        st->frame + n > st->cfg.fail_at_frame)
        n = (int)(st->cfg.fail_at_frame - st->frame);
    if (n <= 0)
        return CODEC_ERR_IO;

    for (i = 0; i < n; i++) {
        for (ch = 0; ch < st->cfg.channels; ch++)
            pcm[i * st->cfg.channels + ch] =
                mock_sample(st->frame + i, ch);
    }
    st->frame += n;
    return n;
}

static int mock_seek(void *vst, long ms)
{
    mock_state_t *st = (mock_state_t *)vst;
    long frame;

    if (st == 0)
        return CODEC_ERR_STATE;
    if (ms < 0)
        ms = 0;
    frame = (ms * st->cfg.sample_rate) / 1000;
    if (frame > st->cfg.total_frames)
        frame = st->cfg.total_frames;
    st->frame = frame;
    return CODEC_OK;
}

static long mock_duration_ms(void *vst)
{
    mock_state_t *st = (mock_state_t *)vst;

    if (st == 0 || st->cfg.sample_rate <= 0)
        return -1;
    return (st->cfg.total_frames * 1000) / st->cfg.sample_rate;
}

static void mock_close(void *vst)
{
    (void)vst;   /* nothing to release */
}

static int mock_format(void *vst, codec_format_t *out)
{
    mock_state_t *st = (mock_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    out->sample_rate = (int)st->cfg.sample_rate;
    out->channels = st->cfg.channels;
    out->bits = 16;
    return CODEC_OK;
}

static const codec_t g_mock_codec =
{
    "mock",
    mock_open,
    mock_decode,
    mock_seek,
    mock_duration_ms,
    mock_close,
    (int)sizeof(mock_state_t),
    mock_format
};

const codec_t *mock_codec_get(void)
{
    return &g_mock_codec;
}
