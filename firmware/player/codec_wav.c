/*
 * codec_wav.c — WAV/RIFF PCM decoder, written by ReChord (reference codec).
 *
 * This is the codec that proves the whole pipeline from .c source: file ->
 * codec_io -> codec_t -> int16 PCM -> ring buffer -> B core. It is a full
 * implementation of the RIFF/WAVE subset we support, not a stub.
 *
 * Supported subset (deliberately strict — anything else is rejected so the
 * user gets a clean "unsupported" instead of garbage audio):
 *   - container : RIFF/WAVE, little-endian fields (both target cores are
 *                 little-endian, so raw PCM copies are byte-exact)
 *   - format    : WAVE_FORMAT_PCM (1) only; no float (3), no extensible
 *   - sample fmt: 16 bits per sample
 *   - channels  : 1 (mono) or 2 (stereo), interleaved
 *   - any sample rate (reported via format(); the player forwards it to
 *     the sink — "sample-rate reporting")
 *   - chunks    : "fmt " and "data" are consumed, everything else is
 *     skipped with correct even-size padding (JUNK/LIST/... safe)
 *
 * Not handled (open item, see docs/rewrite/player.md): streamed WAVs with
 * data size 0xFFFFFFFF (clamped to file size instead), WAVE_FORMAT_EXTENSIBLE,
 * 24/32-bit PCM, RIFF big-endian (RIFX).
 */
#include <stdint.h>

#include "codec.h"
#include "codec_io.h"

typedef struct
{
    codec_io_t *io;
    long        data_off;      /* absolute file offset of PCM data        */
    long        data_size;     /* PCM payload bytes (clamped to file)     */
    long        frame;         /* next frame index within the data chunk  */
    long        total_frames;
    long        sample_rate;
    int         channels;
    int         block_align;   /* bytes per frame = channels * 2          */
} wav_state_t;

static uint32_t rd_u32le(const uint8_t *b)
{
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
           ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static uint16_t rd_u16le(const uint8_t *b)
{
    return (uint16_t)((uint32_t)b[0] | ((uint32_t)b[1] << 8));
}

static int tag_eq(const uint8_t *b, const char *s)
{
    return b[0] == (uint8_t)s[0] && b[1] == (uint8_t)s[1] &&
           b[2] == (uint8_t)s[2] && b[3] == (uint8_t)s[3];
}

/* Read exactly `len` bytes unless EOF/error. Returns bytes read (>= 0) or
 * -1 on I/O error. A short result means end of file. */
static long read_fully(codec_io_t *io, uint8_t *buf, long len)
{
    long got = 0;

    while (got < len) {
        long n = cio_read(io, buf + got, len - got);
        if (n < 0)
            return -1;
        if (n == 0)
            break;
        got += n;
    }
    return got;
}

static int wav_open(void *vst, const char *path)
{
    wav_state_t *st = (wav_state_t *)vst;
    uint8_t hdr[12];
    uint8_t chdr[8];
    uint8_t fmt[40];
    int have_fmt = 0, have_data = 0;
    long file_size;
    long i;

    if (st == 0 || path == 0)
        return CODEC_ERR_STATE;

    st->io = cio_open(path);
    if (st->io == 0)
        return CODEC_ERR_IO;

    file_size = cio_size(st->io);

    /* RIFF header: "RIFF" <size u32> "WAVE" */
    if (read_fully(st->io, hdr, 12) != 12 ||
        !tag_eq(hdr, "RIFF") || !tag_eq(hdr + 8, "WAVE")) {
        cio_close(st->io);
        st->io = 0;
        return CODEC_ERR_FORMAT;
    }

    /* Walk the chunk list. fmt may legally come after data, so keep going
     * until both are seen or the file ends. */
    for (i = 0; i < 1024; i++) {   /* sanity bound on chunk count */
        uint32_t csize;
        long next;

        if (read_fully(st->io, chdr, 8) != 8)
            break;
        csize = rd_u32le(chdr + 4);

        if (tag_eq(chdr, "fmt ")) {
            long want = (csize > (uint32_t)sizeof(fmt))
                        ? (long)sizeof(fmt) : (long)csize;
            if (csize < 16 || read_fully(st->io, fmt, want) != want)
                goto bad_format;
            /* Skip any remainder of the chunk (cbSize etc.) + pad. */
            next = (long)csize;
            if (next < want)
                goto bad_format;
            if (cio_seek(st->io, cio_tell(st->io) + (next - want) +
                         (next & 1)) < 0)
                goto io_error;
            {
                uint16_t audio_format = rd_u16le(fmt + 0);
                uint16_t channels     = rd_u16le(fmt + 2);
                uint32_t rate         = rd_u32le(fmt + 4);
                uint16_t block_align  = rd_u16le(fmt + 12);
                uint16_t bits         = rd_u16le(fmt + 14);

                if (audio_format != 1 || bits != 16 ||
                    (channels != 1 && channels != 2) ||
                    rate == 0 || rate > 192000)
                    goto unsupported;
                if (block_align != 0 && block_align != channels * 2)
                    goto bad_format;

                st->sample_rate = (long)rate;
                st->channels    = (int)channels;
                st->block_align = (int)channels * 2;
            }
            have_fmt = 1;
        } else if (tag_eq(chdr, "data")) {
            long off = cio_tell(st->io);
            long dsz;
            if (off < 0)
                goto io_error;
            /* Streamed WAVs write 0xFFFFFFFF as size; clamp to what the
             * file actually holds (documented leniency). */
            dsz = (csize > 0x7FFFFFFFu) ? -1 : (long)csize;
            if (file_size >= 0 && (dsz < 0 || dsz > file_size - off))
                dsz = file_size - off;
            if (dsz < 0)
                dsz = 0;
            st->data_off = off;
            st->data_size = dsz;
            have_data = 1;
            if (have_fmt)
                break;   /* headers complete; wav_open seeks back below */
            /* fmt still pending: skip the payload (even-size pad). */
            {
                long skip = (csize > 0x7FFFFFFFu) ? dsz : (long)csize;
                if (cio_seek(st->io, off + skip + (skip & 1)) < 0)
                    goto io_error;
            }
        } else {
            /* Unknown chunk: skip with even-size padding. */
            next = (long)csize;
            if (cio_seek(st->io, cio_tell(st->io) + next + (next & 1)) < 0)
                goto io_error;
        }

        if (have_fmt && have_data)
            break;
    }

    if (!have_fmt || !have_data)
        goto bad_format;

    st->total_frames = st->data_size / st->block_align;
    st->frame = 0;
    if (cio_seek(st->io, st->data_off) < 0)
        goto io_error;
    return CODEC_OK;

unsupported:
    cio_close(st->io);
    st->io = 0;
    return CODEC_ERR_UNSUPPORTED;

bad_format:
    cio_close(st->io);
    st->io = 0;
    return CODEC_ERR_FORMAT;

io_error:
    cio_close(st->io);
    st->io = 0;
    return CODEC_ERR_IO;
}

static int wav_decode(void *vst, int16_t *pcm, int max_frames)
{
    wav_state_t *st = (wav_state_t *)vst;
    long remaining, want, got;
    long bytes;

    if (st == 0 || st->io == 0 || pcm == 0)
        return CODEC_ERR_STATE;
    if (max_frames <= 0)
        return 0;

    remaining = st->total_frames - st->frame;
    if (remaining <= 0)
        return 0;                       /* end of stream */
    want = (max_frames < remaining) ? max_frames : remaining;
    bytes = want * st->block_align;

    got = read_fully(st->io, (uint8_t *)pcm, bytes);
    if (got < 0)
        return CODEC_ERR_IO;

    got /= st->block_align;             /* whole frames only */
    st->frame += got;
    return (int)got;
}

static int wav_seek(void *vst, long ms)
{
    wav_state_t *st = (wav_state_t *)vst;
    int64_t frame;

    if (st == 0 || st->io == 0)
        return CODEC_ERR_STATE;
    if (ms < 0)
        ms = 0;

    /* int64 math: ms * rate overflows 32-bit `long` on the target. */
    frame = ((int64_t)ms * (int64_t)st->sample_rate) / 1000;
    if (frame > (int64_t)st->total_frames)
        frame = (int64_t)st->total_frames;

    if (cio_seek(st->io, st->data_off + (long)frame * st->block_align) < 0)
        return CODEC_ERR_IO;

    st->frame = (long)frame;
    return CODEC_OK;
}

static long wav_duration_ms(void *vst)
{
    wav_state_t *st = (wav_state_t *)vst;

    if (st == 0 || st->sample_rate <= 0)
        return -1;
    return (long)(((int64_t)st->total_frames * 1000) /
                  (int64_t)st->sample_rate);
}

static void wav_close(void *vst)
{
    wav_state_t *st = (wav_state_t *)vst;

    if (st == 0 || st->io == 0)
        return;
    cio_close(st->io);
    st->io = 0;
}

static int wav_format(void *vst, codec_format_t *out)
{
    wav_state_t *st = (wav_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    out->sample_rate = (int)st->sample_rate;
    out->channels    = st->channels;
    out->bits        = 16;
    return CODEC_OK;
}

const codec_t codec_wav =
{
    "wav",
    wav_open,
    wav_decode,
    wav_seek,
    wav_duration_ms,
    wav_close,
    (int)sizeof(wav_state_t),
    wav_format
};
