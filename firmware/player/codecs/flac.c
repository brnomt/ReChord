/*
 * codecs/flac.c — FLAC decoder for the ReChord player: dr_flac adapter.
 *
 * Layout per the codec structure recommendation: this file owns everything
 * FLAC — config, vendored implementation, allocator, metadata pre-scan and
 * the codec_t descriptor. Disable the codec by dropping this .c from the
 * build list.
 *
 * Vendored: dr_flac v0.13.4 (dual public-domain / MIT-0), files
 * unmodified, pinned in codecs/dr_flac/ (see its LICENSE). This TU defines
 * DR_FLAC_IMPLEMENTATION — exactly one such TU.
 *
 * Freestanding: no stdio/wide-char paths (DR_FLAC_NO_STDIO), no Ogg
 * container (native .flac only), no asserts, and ALL heap traffic routes
 * through a static pool allocator below (single open track at a time:
 * flac_open resets the pool, flac_close releases it).
 *
 * Pre-scan (before drflac_open, via codec_io): validates "fLaC", walks the
 * metadata chain for the VORBIS_COMMENT block (ReplayGain via
 * tags_replaygain_vorbis), sums SEEKTABLE bytes, and computes dr_flac's
 * exact allocation (handle + SIMD-rounded decode window + alignment slop,
 * mirroring dr_flac's own formula) so oversized streams are rejected as
 * CODEC_ERR_UNSUPPORTED with a deterministic reason instead of failing
 * somewhere inside the decoder.
 *
 * Gapless: FLAC streams carry no encoder delay/padding — report 0/0 (the
 * player's gapless layer then relies on total PCM frames, exact).
 * Seek: drflac_seek_to_pcm_frame (frame-accurate).
 */
#include <stddef.h>
#include <string.h>

#include "../codec.h"
#include "../codec_io.h"
#include "../tags.h"

/* ---- static pool allocator (replaces DRFLAC_MALLOC/REALLOC/FREE) -------- */

#define FLAC_POOL_B 57344          /* fits handle + 4096-block stereo window
                                     * + a generous SEEKTABLE; bss budget
                                     * checked by the firmware_ap.ld LENGTH
                                     * gate. See codecs/README note in docs. */

typedef struct flac_blk
{
    unsigned size;                 /* payload bytes incl. header, 8-aligned */
    unsigned free;                 /* 1 = available                         */
} flac_blk_t;

static unsigned char s_pool[FLAC_POOL_B] __attribute__((aligned(8)));
static int s_pool_up;

#define BLK_HDR ((unsigned)sizeof(flac_blk_t))
#define ALIGN8(x) (((x) + 7u) & ~7u)

static void pool_reset(void)
{
    flac_blk_t *b = (flac_blk_t *)(void *)s_pool;
    b->size = FLAC_POOL_B;
    b->free = 1;
    s_pool_up = 1;
}

static void pool_merge_forward(void)
{
    unsigned char *p = s_pool;
    unsigned char *end = s_pool + FLAC_POOL_B;

    while (p + BLK_HDR <= end) {
        flac_blk_t *b = (flac_blk_t *)(void *)p;
        if (b->size < BLK_HDR || p + b->size > end)
            return;                       /* corrupt (should never happen) */
        if (b->free) {
            unsigned char *n = p + b->size;
            while (n + BLK_HDR <= end) {
                flac_blk_t *nb = (flac_blk_t *)(void *)n;
                if (nb->size < BLK_HDR || n + nb->size > end)
                    return;
                if (!nb->free)
                    break;
                b->size += nb->size;
                n += nb->size;
            }
        }
        p += b->size;
    }
}

static void *pool_alloc(unsigned need)
{
    unsigned char *p, *end;

    if (!s_pool_up)
        pool_reset();
    need = ALIGN8(need + BLK_HDR);
    if (need > FLAC_POOL_B)
        return NULL;

    p = s_pool;
    end = s_pool + FLAC_POOL_B;
    while (p + BLK_HDR <= end) {
        flac_blk_t *b = (flac_blk_t *)(void *)p;
        if (b->size < BLK_HDR || p + b->size > end)
            return NULL;
        if (b->free && b->size >= need) {
            /* split when the tail can hold its own header */
            if (b->size >= need + BLK_HDR + 8u) {
                flac_blk_t *nb = (flac_blk_t *)(void *)(p + need);
                nb->size = b->size - need;
                nb->free = 1;
                b->size = need;
            }
            b->free = 0;
            return p + BLK_HDR;
        }
        p += b->size;
    }
    return NULL;
}

static void pool_free(void *ptr)
{
    unsigned char *p, *base = s_pool;
    flac_blk_t *b;

    if (ptr == NULL)
        return;
    p = (unsigned char *)ptr - BLK_HDR;
    if (p < base || p >= base + FLAC_POOL_B)
        return;
    b = (flac_blk_t *)(void *)p;
    if (b->free)
        return;
    b->free = 1;
    pool_merge_forward();
}

static void *pool_realloc(void *ptr, unsigned need)
{
    unsigned char *q;
    flac_blk_t *b;
    unsigned old;

    if (ptr == NULL)
        return pool_alloc(need);
    b = (flac_blk_t *)(void *)((unsigned char *)ptr - BLK_HDR);
    old = b->size - BLK_HDR;
    if (ALIGN8(need + BLK_HDR) <= b->size)
        return ptr;
    q = (unsigned char *)pool_alloc(need);
    if (q == NULL)
        return NULL;
    memcpy(q, ptr, old);
    pool_free(ptr);
    return q;
}

/* dr_flac configuration — everything before the implementation include. */
#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#define DR_FLAC_NO_OGG
#define DRFLAC_ASSERT(x)       ((void)0)
#define DRFLAC_MALLOC(sz)      pool_alloc((unsigned)(sz))
#define DRFLAC_REALLOC(p, sz)  pool_realloc((p), (unsigned)(sz))
#define DRFLAC_FREE(p)         pool_free((p))

#include "dr_flac/dr_flac.h"           /* vendored (codecs/dr_flac/) */

/* ---- codec_io -> dr_flac callbacks -------------------------------------- */

static size_t fr_read(void *ud, void *buf, size_t bytes)
{
    codec_io_t *io = (codec_io_t *)ud;
    long n = cio_read(io, buf, (long)bytes);
    return (n < 0) ? 0 : (size_t)n;      /* short/0 == EOF for dr_flac */
}

static drflac_bool32 fr_seek(void *ud, int offset, drflac_seek_origin origin)
{
    codec_io_t *io = (codec_io_t *)ud;
    long cur, target, size;

    if (origin == DRFLAC_SEEK_SET) {
        target = (long)offset;
    } else {
        cur = cio_tell(io);
        if (cur < 0)
            return DRFLAC_FALSE;
        if (origin == DRFLAC_SEEK_CUR)
            target = cur + (long)offset;
        else {                            /* DRFLAC_SEEK_END              */
            size = cio_size(io);
            if (size < 0)
                return DRFLAC_FALSE;
            target = size + (long)offset;
        }
    }
    if (target < 0)
        return DRFLAC_FALSE;
    /* Detect past-EOF seeks explicitly (fs_seek clamps, fseek would not
     * fail): dr_flac relies on the callback reporting failure here. */
    if (cio_size(io) >= 0 && target > cio_size(io))
        return DRFLAC_FALSE;
    return (cio_seek(io, target) == target) ? DRFLAC_TRUE : DRFLAC_FALSE;
}

static drflac_bool32 fr_tell(void *ud, drflac_int64 *cursor)
{
    long t = cio_tell((codec_io_t *)ud);
    if (t < 0)
        return DRFLAC_FALSE;
    *cursor = (drflac_int64)t;
    return DRFLAC_TRUE;
}

/* ---- per-track state ---------------------------------------------------- */

typedef struct
{
    codec_io_t     *io;
    drflac         *f;
    long            rate;
    int             channels;
    long            total_samples;      /* -1 unknown                       */
    long            duration_ms;        /* -1 unknown                       */
    codec_replaygain_t rg;
} flac_state_t;

_Static_assert(sizeof(flac_state_t) <= PLAYER_CODEC_STATE_MAX,
               "flac_state_t exceeds the player state pool");

/* ---- helpers ------------------------------------------------------------ */

static unsigned be24(const unsigned char *p)
{
    return ((unsigned)p[0] << 16) | ((unsigned)p[1] << 8) | (unsigned)p[2];
}

/* Walk the metadata chain: RG comments -> out; SEEKTABLE bytes summed;
 * STREAMINFO yields max-block and channels for the budget check. Returns 0
 * on success, <0 if the stream is not a parseable FLAC. */
static int flac_prescan(codec_io_t *io, codec_replaygain_t *rg,
                        unsigned long *seektable_bytes, unsigned *max_block,
                        unsigned *channels)
{
    unsigned char hdr[4], si[34];
    unsigned type, len;

    if (cio_read(io, hdr, 4) != 4 ||
        hdr[0] != 'f' || hdr[1] != 'L' || hdr[2] != 'a' || hdr[3] != 'C')
        return -1;
    /* First block must be STREAMINFO (type 0, 34 bytes). */
    if (cio_read(io, hdr, 4) != 4)
        return -1;
    if ((hdr[0] & 0x7F) != 0)
        return -1;
    len = be24(hdr + 1);
    if (len != 34 || cio_read(io, si, 34) != 34)
        return -1;
    *max_block = ((unsigned)si[2] << 8) | (unsigned)si[3];
    *channels = (unsigned)(((si[12] >> 1) & 0x07) + 1);

    for (;;) {
        long pos;

        if (cio_read(io, hdr, 4) != 4)
            return -1;
        type = hdr[0] & 0x7F;
        len = be24(hdr + 1);
        pos = cio_tell(io);
        if (pos < 0)
            return -1;
        if (type == 4) {
            (void)tags_replaygain_vorbis(io, pos, (long)len, rg);
        } else if (type == 6) {
            *seektable_bytes += len;
        }
        if (cio_seek(io, pos + (long)len) != pos + (long)len)
            return -1;
        if (hdr[0] & 0x80)
            break;                     /* last-metadata-block flag         */
        if (pos > (1L << 24))           /* sanity: runaway chain           */
            return -1;
    }
    return 0;
}

/* dr_flac's own allocation formula (dr_flac.h, drflac_open__internal):
 * sizeof(drflac) + SIMD-rounded decode window + alignment slop, plus the
 * in-block SEEKTABLE. Recomputed here so the pool check is exact. */
static int flac_would_fit(unsigned max_block, unsigned channels,
                          unsigned long seektable_bytes)
{
    const unsigned vec = (unsigned)DRFLAC_MAX_SIMD_VECTOR_SIZE;
    unsigned elems = vec / 4u;         /* drflac_int32 == 4                 */
    unsigned whole, window, need;

    if (elems == 0 || channels == 0)
        return 0;
    whole = (max_block % elems) ? (max_block / elems + 1u) : (max_block / elems);
    window = whole * vec * channels;
    need = (unsigned)sizeof(drflac) + window + vec + (unsigned)seektable_bytes;
    return need <= (unsigned)FLAC_POOL_B;
}

/* ---- codec_t callbacks --------------------------------------------------- */

static int flac_open(void *vst, const char *path)
{
    flac_state_t *st = (flac_state_t *)vst;
    codec_replaygain_t rg;
    unsigned long seektable_bytes = 0;
    unsigned max_block = 0;
    unsigned scan_channels = 0;

    if (st == 0 || path == 0)
        return CODEC_ERR_STATE;

    st->io = cio_open(path);
    if (st->io == 0)
        return CODEC_ERR_IO;
    st->f = 0;
    st->total_samples = -1;
    st->duration_ms = -1;
    memset(&rg, 0, sizeof rg);

    if (flac_prescan(st->io, &rg, &seektable_bytes, &max_block,
                     &scan_channels) != 0) {
        cio_close(st->io);
        st->io = 0;
        return CODEC_ERR_FORMAT;
    }
    if (scan_channels > 2) {           /* player contract: mono/stereo     */
        cio_close(st->io);
        st->io = 0;
        return CODEC_ERR_UNSUPPORTED;
    }
    if (!flac_would_fit(max_block, scan_channels, seektable_bytes)) {
        cio_close(st->io);
        st->io = 0;
        return CODEC_ERR_UNSUPPORTED;  /* blocksize/seektable > pool       */
    }

    pool_reset();
    if (cio_seek(st->io, 0) != 0) {    /* dr_flac reads from stream start */
        cio_close(st->io);
        st->io = 0;
        return CODEC_ERR_IO;
    }
    st->f = drflac_open(fr_read, fr_seek, fr_tell, (void *)st->io, NULL);
    if (st->f == 0) {
        pool_reset();
        cio_close(st->io);
        st->io = 0;
        return CODEC_ERR_FORMAT;
    }

    st->rate = (long)st->f->sampleRate;
    st->channels = (int)st->f->channels;
    st->total_samples = (long)st->f->totalPCMFrameCount;
    if (st->rate <= 0 || st->channels < 1 || st->channels > 2) {
        drflac_close(st->f);
        pool_reset();
        cio_close(st->io);
        st->f = 0;
        st->io = 0;
        return (st->channels > 2) ? CODEC_ERR_UNSUPPORTED : CODEC_ERR_FORMAT;
    }
    if (st->total_samples > 0)
        st->duration_ms = st->total_samples * 1000 / st->rate;
    st->rg = rg;
    return CODEC_OK;
}

static int flac_decode(void *vst, int16_t *pcm, int max_frames)
{
    flac_state_t *st = (flac_state_t *)vst;
    size_t got;

    if (st == 0 || pcm == 0 || max_frames <= 0)
        return CODEC_ERR_STATE;
    if (st->f == 0)
        return CODEC_ERR_STATE;
    got = drflac_read_pcm_frames_s16(st->f, (size_t)max_frames, pcm);
    return (int)got;                     /* 0 == EOF (codec contract)      */
}

static int flac_seek(void *vst, long ms)
{
    flac_state_t *st = (flac_state_t *)vst;
    long frame;

    if (st == 0 || st->f == 0)
        return CODEC_ERR_STATE;
    if (ms < 0)
        ms = 0;
    if (st->duration_ms > 0 && ms > st->duration_ms)
        ms = st->duration_ms;
    frame = (st->rate > 0) ? (long)ms * st->rate / 1000 : 0;
    if (!drflac_seek_to_pcm_frame(st->f, (drflac_uint64)frame))
        return CODEC_ERR_IO;
    return CODEC_OK;
}

static long flac_duration_ms(void *vst)
{
    flac_state_t *st = (flac_state_t *)vst;
    return (st == 0) ? -1 : st->duration_ms;
}

static void flac_close(void *vst)
{
    flac_state_t *st = (flac_state_t *)vst;

    if (st == 0)
        return;
    if (st->f != 0)
        drflac_close(st->f);            /* returns memory to the pool     */
    st->f = 0;
    pool_reset();
    if (st->io != 0)
        cio_close(st->io);
    st->io = 0;
}

static int flac_format(void *vst, codec_format_t *out)
{
    flac_state_t *st = (flac_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    out->sample_rate = (int)st->rate;
    out->channels = st->channels;
    out->bits = 16;
    out->total_samples = st->total_samples;
    return CODEC_OK;
}

static int flac_gapless(void *vst, codec_gapless_t *out)
{
    (void)vst;
    if (out == 0)
        return CODEC_ERR_STATE;
    out->delay_samples = 0;             /* FLAC: no encoder delay in-stream */
    out->padding_samples = 0;
    return CODEC_OK;
}

static int flac_tags(void *vst, codec_replaygain_t *out)
{
    flac_state_t *st = (flac_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    *out = st->rg;
    return CODEC_OK;
}

const codec_t codec_flac =
{
    "flac",
    "flac",
    flac_open,
    flac_decode,
    flac_seek,
    flac_duration_ms,
    flac_close,
    (int)sizeof(flac_state_t),
    flac_format,
    flac_gapless,
    flac_tags
};
