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
#include "../../services/workbuf.h"

/* Debug diag for the flac_open subset check (why a file returns -3). */
unsigned flac_diag_block, flac_diag_channels;
unsigned long flac_diag_seektable, flac_diag_need;
int        flac_diag_reason;

/* Peer-supplied stream geometry (Core B: the AP prescanned the file while
 * pre-opening it, see codec.h). NULL = no source, walk the header ourselves. */
static flac_prescan_fn s_prescan_src;

void flac_set_prescan_source(flac_prescan_fn fn)
{
    s_prescan_src = fn;
}

/* ---- static pool allocator (replaces DRFLAC_MALLOC/REALLOC/FREE) -------- */

/* The decode pool is the shared workbuf region (workbuf.h): a whole FLAC
 * frame must be buffered, and the window for an 8192-sample stereo block
 * (sizeof(drflac) 4336 + 8192*4*2 + 64 ~= 69.9 KiB, plus SEEKTABLE) no longer
 * fits a dedicated static under the .bss ceiling. WORKBUF_BYTES is exactly
 * the old pool size plus the fw_update staging it also serves (mutually
 * exclusive) — see DFIU R-8cd5fb0eb24a. */

typedef struct flac_blk
{
    unsigned size;                 /* payload bytes incl. header, 8-aligned */
    unsigned free;                 /* 1 = available                         */
} flac_blk_t;

static unsigned char *s_pool;      /* workbuf region (claimed on open)      */
static unsigned s_pool_bytes;
static int s_pool_up;

#define BLK_HDR ((unsigned)sizeof(flac_blk_t))
#define ALIGN8(x) (((x) + 7u) & ~7u)

static void pool_reset(void)
{
    flac_blk_t *b;

    if (s_pool == 0 || s_pool_bytes < BLK_HDR) {
        s_pool_up = 0;
        return;
    }
    b = (flac_blk_t *)(void *)s_pool;
    b->size = s_pool_bytes;
    b->free = 1;
    s_pool_up = 1;
}

static void pool_merge_forward(void)
{
    unsigned char *p = s_pool;
    unsigned char *end = s_pool + s_pool_bytes;

    if (s_pool == 0)
        return;

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
    if (s_pool == 0)
        return NULL;
    need = ALIGN8(need + BLK_HDR);
    if (need > s_pool_bytes)
        return NULL;

    p = s_pool;
    end = s_pool + s_pool_bytes;
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

    if (ptr == NULL || s_pool == 0)
        return;
    p = (unsigned char *)ptr - BLK_HDR;
    if (p < base || p >= base + s_pool_bytes)
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
#ifndef RECHORD_AP_METADATA_ONLY
#define DR_FLAC_IMPLEMENTATION
#endif
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
    int             bits;               /* source depth (16/24/32)          */
    long            total_samples;      /* -1 unknown                       */
    long            duration_ms;        /* -1 unknown                       */
    codec_replaygain_t rg;
    codec_meta_t    meta;               /* Vorbis comments (may be empty)   */
} flac_state_t;

_Static_assert(sizeof(flac_state_t) <= PLAYER_CODEC_STATE_MAX,
               "flac_state_t exceeds the player state pool");

/* ---- helpers ------------------------------------------------------------ */

static unsigned be32(const unsigned char *p)
{
    return ((unsigned)p[0] << 24) | ((unsigned)p[1] << 16) |
           ((unsigned)p[2] << 8) | (unsigned)p[3];
}

static unsigned be24(const unsigned char *p)
{
    return ((unsigned)p[0] << 16) | ((unsigned)p[1] << 8) | (unsigned)p[2];
}

/* Some FLAC files carry a non-standard ID3v2 tag before "fLaC" (dr_flac
 * skips it at open). The prescan must skip the same tags, otherwise the AP
 * rejects a file Core B could decode ("format not supported" after a couple
 * of shuffled tracks). Up to four tags; a bogus size is rejected. */
static int skip_id3v2(codec_io_t *io)
{
    int n;

    for (n = 0; n < 4; n++) {
        unsigned char h[10];
        long pos = cio_tell(io);
        unsigned size, skip;

        if (pos < 0)
            return -1;
        if (cio_read(io, h, 10) != 10)
            return -1;
        if (h[0] != 'I' || h[1] != 'D' || h[2] != '3') {
            if (cio_seek(io, pos) != pos)
                return -1;
            return 0;
        }
        size = ((unsigned)(h[6] & 0x7Fu) << 21) |
               ((unsigned)(h[7] & 0x7Fu) << 14) |
               ((unsigned)(h[8] & 0x7Fu) << 7) |
               (unsigned)(h[9] & 0x7Fu);
        if (size > (1u << 20))          /* runaway/invalid tag */
            return -1;
        skip = 10u + size + ((h[5] & 0x10u) ? 10u : 0u);  /* v2.4 footer */
        if (cio_seek(io, pos + (long)skip) != pos + (long)skip)
            return -1;
    }
    return -1;                          /* absurd chain */
}

/* Walk the metadata chain: RG comments -> out; SEEKTABLE bytes summed;
 * STREAMINFO yields max-block and channels for the budget check. Returns 0
 * on success, <0 if the stream is not a parseable FLAC. */
static int flac_prescan(codec_io_t *io, codec_meta_t *meta,
                        codec_replaygain_t *rg,
                        unsigned long *seektable_bytes, unsigned *max_block,
                        unsigned *channels, unsigned *sample_rate,
                        unsigned *bits, unsigned long *total_samples)
{
    unsigned char hdr[4], si[34];
    unsigned type, len;

    if (skip_id3v2(io) != 0 || cio_read(io, hdr, 4) != 4 ||
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
    *sample_rate = ((unsigned)si[10] << 12) | ((unsigned)si[11] << 4) |
                   ((unsigned)si[12] >> 4);
    *bits = (unsigned)((((si[12] & 1u) << 4) | (si[13] >> 4)) + 1);
    *total_samples = (unsigned long)(((uint64_t)(si[13] & 0x0Fu) << 32) |
                                     (uint64_t)be32(si + 14));

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
            (void)tags_read_vorbis(io, pos, (long)len, meta, rg);
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

#ifndef RECHORD_AP_METADATA_ONLY
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
    /* dr_flac drops a seektable > 16 KB (see its open path): it contributes
     * nothing to the allocation then. Budget only what dr_flac keeps. */
    if (seektable_bytes > 16384u)
        seektable_bytes = 0;
    need = (unsigned)sizeof(drflac) + window + vec + (unsigned)seektable_bytes;
    return need <= (unsigned)WORKBUF_BYTES;
}
#endif /* !RECHORD_AP_METADATA_ONLY */

/* Geometry handed over by the peer core: lets this instance skip the header
 * walk (Core B, whose open sits inside the gapless boundary). */
static int flac_prescan_supplied(unsigned *max_block, unsigned *channels,
                                 unsigned long *seektable_bytes)
{
    unsigned mb = 0, ch = 0;
    unsigned long stb = 0;

    if (s_prescan_src == 0 || !s_prescan_src(&mb, &ch, &stb))
        return 0;
    if (mb == 0 || mb > 65535u || ch < 1 || ch > 2)
        return 0;                       /* implausible: walk it ourselves */
    *max_block = mb;
    *channels = ch;
    *seektable_bytes = stb;
    return 1;
}

/* Publish the geometry this open discovered (codec_t.prescan_info). */
static int flac_prescan_info(void *vst, codec_prescan_t *out)
{
    if (vst == 0 || out == 0 || flac_diag_block == 0)
        return 0;
    out->max_block = flac_diag_block;
    out->channels = flac_diag_channels;
    out->seektable_bytes = flac_diag_seektable;
    return 1;
}

/* ---- codec_t callbacks --------------------------------------------------- */

static int flac_open(void *vst, const char *path)
{
    flac_state_t *st = (flac_state_t *)vst;
    unsigned long seektable_bytes = 0;
    unsigned max_block = 0;
    unsigned scan_channels = 0;
    unsigned scan_rate = 0, scan_bits = 0;
    unsigned long scan_total = 0;
    int rc;

    if (st == 0 || path == 0)
        return CODEC_ERR_STATE;

#ifndef RECHORD_AP_METADATA_ONLY
    /* Claim the shared decode pool: FLAC, mp3 and fw_update are mutually
     * exclusive, so a busy region means one of them is up (skip the file).
     * The metadata-only AP build never decodes, so it skips the pool. */
    if (workbuf_claim(WORKBUF_OWNER_FLAC) == 0)
        return CODEC_ERR_IO;
    s_pool = (unsigned char *)workbuf_for(WORKBUF_OWNER_FLAC);
    s_pool_bytes = WORKBUF_BYTES;
#endif

    st->io = cio_open(path);
    if (st->io == 0) {
        rc = CODEC_ERR_IO;
        goto fail;
    }
    st->f = 0;
    st->total_samples = -1;
    st->duration_ms = -1;
    memset(&st->rg, 0, sizeof st->rg);
    memset(&st->meta, 0, sizeof st->meta);

    /* Core B re-opens a file the AP already prescanned while pre-opening it;
     * that walk costs a seek round-trip per metadata block and lands inside
     * the gapless cover budget. Use the peer's geometry when supplied and
     * plausible; walk the header only otherwise. */
    if (!flac_prescan_supplied(&max_block, &scan_channels, &seektable_bytes) &&
        flac_prescan(st->io, &st->meta, &st->rg, &seektable_bytes, &max_block,
                     &scan_channels, &scan_rate, &scan_bits,
                     &scan_total) != 0) {
        cio_close(st->io);
        st->io = 0;
        flac_diag_reason = -2;
        rc = CODEC_ERR_FORMAT;
        goto fail;
    }
#ifndef RECHORD_AP_METADATA_ONLY
    flac_diag_block = max_block;
    flac_diag_channels = scan_channels;
    flac_diag_seektable = seektable_bytes;
    flac_diag_need = (unsigned long)sizeof(drflac) +
                     ((max_block + 15u) / 16u) * 64u * scan_channels + 64u +
                     seektable_bytes;
#endif
    if (scan_channels > 2) {           /* player contract: mono/stereo     */
        cio_close(st->io);
        st->io = 0;
        flac_diag_reason = 1;          /* >2 channels                     */
        rc = CODEC_ERR_UNSUPPORTED;
        goto fail;
    }
#ifndef RECHORD_AP_METADATA_ONLY
    if (!flac_would_fit(max_block, scan_channels, seektable_bytes)) {
        cio_close(st->io);
        st->io = 0;
        flac_diag_reason = 2;          /* block/seektable > pool           */
        rc = CODEC_ERR_UNSUPPORTED;
        goto fail;
    }
#endif

#ifndef RECHORD_AP_METADATA_ONLY
    pool_reset();
    if (cio_seek(st->io, 0) != 0) {    /* dr_flac reads from stream start */
        cio_close(st->io);
        st->io = 0;
        rc = CODEC_ERR_IO;
        goto fail;
    }
    st->f = drflac_open(fr_read, fr_seek, fr_tell, (void *)st->io, NULL);
    if (st->f == 0) {
        cio_close(st->io);
        st->io = 0;
        rc = CODEC_ERR_FORMAT;
        goto fail;
    }

    st->rate = (long)st->f->sampleRate;
    st->channels = (int)st->f->channels;
    st->bits = (int)st->f->bitsPerSample;
    st->total_samples = (long)st->f->totalPCMFrameCount;
#else
    /* Metadata only: STREAMINFO already carries rate/depth/total; Core B
     * opens its own dr_flac instance for the PCM. */
    (void)flac_prescan_supplied;
    st->rate = (long)scan_rate;
    st->channels = (int)scan_channels;
    st->bits = (int)scan_bits;
    st->total_samples = (long)scan_total;
#endif
    if (st->rate <= 0 || st->channels < 1 || st->channels > 2) {
#ifndef RECHORD_AP_METADATA_ONLY
        drflac_close(st->f);
#endif
        cio_close(st->io);
        st->f = 0;
        st->io = 0;
        if (st->channels > 2) {
            flac_diag_reason = 3;          /* dr_flac saw >2 channels      */
            rc = CODEC_ERR_UNSUPPORTED;
        } else {
            flac_diag_reason = -2;         /* bad rate/format              */
            rc = CODEC_ERR_FORMAT;
        }
        goto fail;
    }
    if (st->total_samples > 0)
        /* int64 math: samples*1000 overflows 32-bit `long` past ~48 s at
         * 44.1 kHz, and the wrapped (often negative) value reached the UI
         * as a bogus few-seconds length (BRNOMT-65). */
        st->duration_ms = (long)(((int64_t)st->total_samples * 1000) /
                                 st->rate);
    return CODEC_OK;

fail:
    pool_reset();
    workbuf_release(WORKBUF_OWNER_FLAC);
    s_pool = 0;
    s_pool_bytes = 0;
    return rc;
}

static int flac_decode(void *vst, int32_t *pcm, int max_frames)
{
    flac_state_t *st = (flac_state_t *)vst;
    size_t got;

    if (st == 0 || pcm == 0 || max_frames <= 0)
        return CODEC_ERR_STATE;
#ifdef RECHORD_AP_METADATA_ONLY
    /* The AP never owns the decode (Core B does). */
    return CODEC_ERR_UNSUPPORTED;
#else
    if (st->f == 0)
        return CODEC_ERR_STATE;
    /* s32 output is left-justified to 32 bits (dr_flac shifts by 32-bps),
     * which is exactly the internal pipeline format: no re-scaling here. */
    got = drflac_read_pcm_frames_s32(st->f, (size_t)max_frames,
                                     (drflac_int32 *)pcm);
    return (int)got;                     /* 0 == EOF (codec contract)      */
#endif
}

static int flac_seek(void *vst, long ms)
{
    flac_state_t *st = (flac_state_t *)vst;
    long frame;

    if (st == 0)
        return CODEC_ERR_STATE;
#ifdef RECHORD_AP_METADATA_ONLY
    /* Core B repositions its own decoder (ipc_b_seek); nothing to move here. */
    (void)ms;
    return CODEC_OK;
#else
    if (st->f == 0)
        return CODEC_ERR_STATE;
    if (ms < 0)
        ms = 0;
    if (st->duration_ms > 0 && ms > st->duration_ms)
        ms = st->duration_ms;
    /* int64 math: ms*rate overflows 32-bit `long` past ~48 s at 44.1 kHz. */
    frame = (st->rate > 0)
        ? (long)(((int64_t)ms * st->rate) / 1000) : 0;
    if (!drflac_seek_to_pcm_frame(st->f, (drflac_uint64)frame))
        return CODEC_ERR_IO;
    return CODEC_OK;
#endif
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
    /* Release the codec_io handle on BOTH builds. Missing this leaked one of
     * fs_target's four read handles per track; after 1-2 shuffled tracks the
     * gapless pre-open could no longer open the next file. */
    if (st->io != 0) {
        cio_close(st->io);
        st->io = 0;
    }
#ifndef RECHORD_AP_METADATA_ONLY
    if (st->f != 0)
        drflac_close(st->f);            /* returns memory to the pool     */
    st->f = 0;
    pool_reset();
    /* Hand the shared region back (no-op if this state never claimed it). */
    workbuf_release(WORKBUF_OWNER_FLAC);
    s_pool = 0;
    s_pool_bytes = 0;
#else
    st->f = 0;
#endif
}

static int flac_format(void *vst, codec_format_t *out)
{
    flac_state_t *st = (flac_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    out->sample_rate = (int)st->rate;
    out->channels = st->channels;
    out->bits = st->bits;                /* source depth (16/24/32)         */
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

/* Vorbis-comment text metadata (TITLE/ARTIST/ALBUM/GENRE/TRACKNUMBER),
 * parsed by the same prescan pass that reads the REPLAYGAIN_* comments.
 * Empty fields mean "no such tag". */
static int flac_meta(void *vst, codec_meta_t *out)
{
    flac_state_t *st = (flac_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    *out = st->meta;
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
    flac_tags,
    flac_meta,
    flac_prescan_info
};
