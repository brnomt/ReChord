/*
 * codec_flac_stub.c — dr_flac adapter scaffold (ReChord player module).
 *
 * STATUS: scaffold only — every callback returns CODEC_ERR_UNSUPPORTED.
 * The goal is to back this vtable with a build-from-source, open-source C
 * FLAC decoder (architecture.md §4: "prefer open-source C codecs").
 *
 * VENDORING PLAN (details in docs/rewrite/player.md §"Codec vendoring"):
 *   - implementation : dr_flac (single-header, MIT-0/Unlicense dual —
 *                      verify license text at vendoring time)
 *   - lives at       : firmware/player/codecs/dr_flac/dr_flac.h
 *                      (upstream file unmodified, version pinned in
 *                       docs/rewrite/player.md)
 *   - THIS file then implements the adapter:
 *       open()     -> cio_open() + drflac_open() on the stream callbacks
 *                     bridging codec_io; expose totalPCMFrameCount and
 *                     sample rate/channels (dr_flac gives both)
 *       decode()   -> drflac_read_pcm_frames_s16() directly into `pcm`
 *                     (dr_flac has a native s16 path — matches codec_t)
 *       seek()     -> drflac_seek_to_pcm_frame() (frame-accurate)
 *       duration_ms-> totalPCMFrameCount * 1000 / sampleRate
 *       state_size -> sizeof(drflac + io bridging fields)
 *
 * TODO(decide): stereo decorrelation/stereo-independence CPU budget on the
 * B-core clock (see the clock facts table in docs/rewrite/player.md) — the
 * host tests measure frames-per-poll before the format subset is frozen.
 *
 * Nothing here may come from third-party CFW code — only the upstream
 * open-source codec, compiled from its .c/.h in this tree (build-from-source
 * rule).
 */
#include "codec.h"
#include "codec_io.h"

/* TODO(vendor dr_flac): embed the drflac handle here (dr_flac allocates
 * through DRFLAC_MALLOC — we will route that to the same static pool the
 * player uses, keeping the no-malloc target build honest). */
typedef struct
{
    codec_io_t *io;
    /* TODO: drflac handle, rate, channels, total frames */
    long sample_rate;
    int  channels;
    long duration_ms;
} flac_state_t;

static int flac_open(void *st, const char *path)
{
    (void)st;
    (void)path;
    /* TODO(vendor dr_flac): drflac_open() with codec_io read/seek
     * callbacks; reject > 2 channels or > 192 kHz here (subset policy). */
    return CODEC_ERR_UNSUPPORTED;
}

static int flac_decode(void *st, int16_t *pcm, int max_frames)
{
    (void)st;
    (void)pcm;
    (void)max_frames;
    /* TODO(vendor dr_flac): drflac_read_pcm_frames_s16(); return frames
     * produced (0 at EOF). */
    return CODEC_ERR_UNSUPPORTED;
}

static int flac_seek(void *st, long ms)
{
    (void)st;
    (void)ms;
    /* TODO(vendor dr_flac): drflac_seek_to_pcm_frame(ms * rate / 1000). */
    return CODEC_ERR_UNSUPPORTED;
}

static long flac_duration_ms(void *st)
{
    (void)st;
    return -1;
}

static void flac_close(void *st)
{
    flac_state_t *f = (flac_state_t *)st;

    if (f == 0 || f->io == 0)
        return;
    cio_close(f->io);
    f->io = 0;
}

static int flac_format(void *st, codec_format_t *out)
{
    flac_state_t *f = (flac_state_t *)st;

    if (f == 0 || out == 0)
        return CODEC_ERR_STATE;
    out->sample_rate = (int)f->sample_rate;
    out->channels    = f->channels;
    out->bits        = 16;
    return CODEC_OK;
}

const codec_t codec_flac =
{
    "flac",
    flac_open,
    flac_decode,
    flac_seek,
    flac_duration_ms,
    flac_close,
    (int)sizeof(flac_state_t),
    flac_format
};
