/*
 * codec_mp3_stub.c — minimp3 adapter scaffold (ReChord player module).
 *
 * STATUS: scaffold only — every callback returns CODEC_ERR_UNSUPPORTED.
 * The goal is to back this vtable with a build-from-source, open-source C
 * MP3 decoder (architecture.md §4: "prefer open-source C codecs").
 *
 * VENDORING PLAN (details in docs/rewrite/player.md §"Codec vendoring"):
 *   - implementation : minimp3 (single-header, CC0-1.0 — verify license
 *                      text at vendoring time)
 *   - lives at       : firmware/player/codecs/minimp3/minimp3.h
 *                      (upstream file unmodified, drop-in version pinned
 *                       in docs/rewrite/player.md)
 *   - THIS file then implements the adapter:
 *       open()     -> cio_open() + minimp3_decode_frame header probe for
 *                     sample rate / channels; estimate duration from file
 *                     size and first-frame bitrate (CBR assumption) or
 *                     parse an Xing/VBR header when present
 *       decode()   -> feed bytes from codec_io through minimp3_decode_frame
 *                     into `pcm` (minimp3 outputs interleaved int16, which
 *                     is exactly the codec_t contract), return frames
 *       seek()     -> file-size proportional seek for CBR; frame-accurate
 *                     via Xing TOC when present (TODO: accuracy class flag)
 *       duration_ms-> from Xing header or size/bitrate estimate
 *       state_size -> sizeof(minimp3 dec state + io + estimate fields)
 *
 * Nothing here may come from third-party CFW code — only the upstream
 * open-source codec, compiled from its .c/.h in this tree (build-from-source
 * rule).
 */
#include "codec.h"
#include "codec_io.h"

/* TODO(vendor minimp3): move the decoder state into this struct and pass
 * it as `st`; keep state_size as the single allocation knob the player
 * uses (static pool — no malloc on the target). */
typedef struct
{
    codec_io_t *io;
    /* TODO: minimp3 buffer/window state, rate, channels, bitrate, ... */
    long sample_rate;
    int  channels;
    long duration_ms;
} mp3_state_t;

static int mp3_open(void *st, const char *path)
{
    (void)st;
    (void)path;
    /* TODO(vendor minimp3): probe the first frame for rate/channels,
     * look for a Xing/Info header, estimate duration. */
    return CODEC_ERR_UNSUPPORTED;
}

static int mp3_decode(void *st, int16_t *pcm, int max_frames)
{
    (void)st;
    (void)pcm;
    (void)max_frames;
    /* TODO(vendor minimp3): stream bytes from codec_io through
     * minimp3_decode_frame; return frames produced (0 at EOF). */
    return CODEC_ERR_UNSUPPORTED;
}

static int mp3_seek(void *st, long ms)
{
    (void)st;
    (void)ms;
    /* TODO(vendor minimp3): Xing TOC seek or CBR size-proportional seek. */
    return CODEC_ERR_UNSUPPORTED;
}

static long mp3_duration_ms(void *st)
{
    (void)st;
    return -1;
}

static void mp3_close(void *st)
{
    mp3_state_t *m = (mp3_state_t *)st;

    if (m == 0 || m->io == 0)
        return;
    cio_close(m->io);
    m->io = 0;
}

static int mp3_format(void *st, codec_format_t *out)
{
    mp3_state_t *m = (mp3_state_t *)st;

    if (m == 0 || out == 0)
        return CODEC_ERR_STATE;
    out->sample_rate = (int)m->sample_rate;
    out->channels    = m->channels;
    out->bits        = 16;
    return CODEC_OK;
}

const codec_t codec_mp3 =
{
    "mp3",
    mp3_open,
    mp3_decode,
    mp3_seek,
    mp3_duration_ms,
    mp3_close,
    (int)sizeof(mp3_state_t),
    mp3_format
};
