/*
 * codecs/mp3.c — MP3 decoder for the ReChord player: minimp3 adapter.
 *
 * Layout per the codec structure recommendation: this file owns EVERYTHING
 * MP3 — the vendored library (implementation TU below), the codec_t
 * descriptor, and its dispatch key. Swapping libraries or disabling the
 * codec touches only this file + the Makefile source list.
 *
 * Vendored: minimp3 (CC0-1.0), upstream files unmodified, pinned in
 * codecs/minimp3/ (see its LICENSE). This TU defines MINIMP3_IMPLEMENTATION
 * so the header compiles its implementation here — exactly one such TU.
 *
 * Track-level buffers (feed window + 1152-frame residual) are file-statics:
 * the player opens ONE track at a time, and keeping 7 KB out of the
 * per-track state pool keeps PLAYER_CODEC_STATE_MAX small (freestanding
 * budget, see codec.h).
 *
 * Gapless: the Xing/Info header is parsed at open (frames/bytes/TOC and
 * the LAME extension's encoder delay/padding with the standard 528+1
 * decoder-priming adjustment — same interpretation as upstream minimp3_ex).
 * The player's gapless layer then trims delay/padding from the ends; the
 * info frame itself is skipped (its bytes never reach the PCM path).
 *
 * Duration: Xing frames*1152/rate is exact; CBR falls back to
 * file_bytes*8/bitrate (documented accuracy class: whole-file estimate,
 * ID3v1 tail excluded). Seek: Xing TOC when present (byte-accurate to
 * 1%), else linear CBR; PCM resumes frame-aligned after a forward discard.
 *
 * ReplayGain: ID3v2 TXXX frames via tags_replaygain_id3v2() at open
 * (codec_t.tags), same parser the tests pin.
 */
#ifndef RECHORD_AP_METADATA_ONLY
#define MINIMP3_IMPLEMENTATION
#include "minimp3/minimp3.h"           /* vendored (codecs/minimp3/) */
#endif

#include <stddef.h>
#include <string.h>

#include "../codec.h"
#include "../codec_io.h"
#include "../tags.h"
#include "../../services/workbuf.h"

/* One open track at a time: the big per-track buffers live in the shared
 * workbuf region (workbuf.h) — FLAC, mp3 and fw_update are mutually
 * exclusive. s_win at offset 0, s_res right after it (MP3_WIN_BYTES is even,
 * so the int16 residual stays 2-aligned). Feed window must hold a whole frame
 * incl. MAX_FREE_FORMAT_FRAME_SIZE payload. */
#define MP3_WIN_BYTES  2560
#define MP3_MAX_CH     2

static uint8_t *s_win;                       /* workbuf (claimed on open) */
static int      s_win_pos, s_win_len;   /* unconsumed = [pos, len) */
static int      s_win_eof;
static int16_t *s_res;                       /* 1152*2 */
static int      s_res_frames, s_res_i;  /* residual frame cursor */

typedef struct
{
    codec_io_t     *io;
    long            file_size;
    long            file_size_audio;   /* minus the ID3v1 tail            */
    long            tag_end;            /* first audio byte                */
    /* stream facts (first frame) */
    int             rate, channels, spf, layer, bitrate;
    /* Xing/Info */
    int             have_xing;
    unsigned        xing_frames, xing_bytes;
    int             have_toc;
    unsigned char   toc[100];
    int             lame;               /* LAME extension present          */
    int             delay_s, pad_s;     /* gapless (decoder-ready)         */
    /* duration */
    long            total_samples;      /* -1 unknown                       */
    long            duration_ms;        /* -1 unknown                       */
    codec_replaygain_t rg;
    codec_meta_t    meta;               /* ID3v2 text frames (may be empty) */
#ifndef RECHORD_AP_METADATA_ONLY
    mp3dec_t        dec;                /* 6668 B: must be the big tail    */
#endif
} mp3_state_t;

_Static_assert(sizeof(mp3_state_t) <= PLAYER_CODEC_STATE_MAX,
               "mp3_state_t exceeds the player state pool");

#ifdef RECHORD_AP_METADATA_ONLY
/* Header-only frame info for the AP's metadata pre-open: the decoder lives on
 * Core B, so the AP must not link minimp3 (its tables are ~20 KB of code span
 * and the AP sits at the loader gate). Returns 0 when `p` starts a valid
 * MPEG-1/2/2.5 Layer III frame inside `avail`. */
static int mp3_frame_info(const unsigned char *p, int avail, int *hz, int *ch,
                          int *br, int *spf, int *flen)
{
    static const int br_m1[16] =
        { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 };
    static const int br_m2[16] =
        { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 };
    static const int sr_m1[3] = { 44100, 48000, 32000 };
    static const int sr_m2[3] = { 22050, 24000, 16000 };
    static const int sr_m25[3] = { 11025, 12000, 8000 };
    int ver, idx, sri, pad, size, bitrate, rate;

    if (avail < 4 || p[0] != 0xFF || (p[1] & 0xE0) != 0xE0)
        return -1;
    if (((p[1] >> 1) & 3) != 1)          /* Layer III only                  */
        return -1;
    ver = (p[1] >> 3) & 3;               /* 3 = MPEG-1, 2 = MPEG-2, 0 = 2.5  */
    if (ver == 1)
        return -1;
    idx = (p[2] >> 4) & 0x0F;
    sri = (p[2] >> 2) & 3;
    if (idx == 0 || idx == 15 || sri == 3)
        return -1;
    bitrate = (ver == 3) ? br_m1[idx] : br_m2[idx];
    rate = (ver == 3) ? sr_m1[sri] : (ver == 2 ? sr_m2[sri] : sr_m25[sri]);
    if (bitrate == 0 || rate == 0)
        return -1;
    pad = (p[2] >> 1) & 1;
    size = ((ver == 3) ? 144 : 72) * bitrate * 1000 / rate +
           pad * ((ver == 3) ? 4 : 1);
    if (size > avail)
        return -1;
    *hz = rate;
    *br = bitrate;
    *spf = (ver == 3) ? 1152 : 576;
    *ch = (((p[3] >> 6) & 3) == 3) ? 1 : 2;
    *flen = size;
    return 0;
}
#endif

/* ---- helpers ----------------------------------------------------------- */

static unsigned be32(const unsigned char *p)
{
    return ((unsigned)p[0] << 24) | ((unsigned)p[1] << 16) |
           ((unsigned)p[2] << 8) | (unsigned)p[3];
}

/* Xing/Info at the frame's main-data start (== after header[+CRC]+side).
 * Header byte 1: bits 4:3 = MPEG version (11 = MPEG-1), bit 0 = !CRC. */
static int xing_offset(const unsigned char *f, int frame_len)
{
    int mpeg1 = ((f[1] & 0x18) == 0x18);
    int mono = ((f[3] >> 6) & 3) == 3;
    int side = mpeg1 ? (mono ? 17 : 32) : (mono ? 9 : 17);
    int crc = (f[1] & 1) ? 0 : 2;

    if (frame_len < 4 + crc + side + 4)
        return -1;
    return 4 + crc + side;
}

/* Parse Xing + optional LAME extension inside the info frame (the frame's
 * bytes). Semantics copied from upstream minimp3_ex's VBR-tag reader
 * (CC0): flags/frames/bytes/TOC/quality walk, then delay/padding at
 * LAME_start+21 with the +528+1 priming adjustment. */
static void parse_xing(mp3_state_t *st, const unsigned char *frame, int flen)
{
    int off = xing_offset(frame, flen);
    unsigned flags;
    const unsigned char *p;

    if (off < 0)
        return;
    if (memcmp(frame + off, "Xing", 4) != 0 &&
        memcmp(frame + off, "Info", 4) != 0)
        return;
    st->have_xing = 1;
    p = frame + off + 4;
    if ((int)(p - frame) + 4 > flen) return;
    flags = be32(p); p += 4;
    if (flags & 1) { /* FRAMES */
        if ((int)(p - frame) + 4 > flen) return;
        st->xing_frames = be32(p); p += 4;
    }
    if (flags & 2) { /* BYTES */
        if ((int)(p - frame) + 4 > flen) return;
        st->xing_bytes = be32(p); p += 4;
    }
    if (flags & 4) { /* TOC */
        if ((int)(p - frame) + 100 > flen) return;
        memcpy(st->toc, p, 100); p += 100; st->have_toc = 1;
    }
    if (flags & 8) { /* QUALITY */
        if ((int)(p - frame) + 4 > flen) return;
        p += 4;
    }
    /* LAME/extension block: encoder string starts here; delay/padding at
     * +21..+23 (encoder[9]+rev+method+lowpass+peak[4]+radio[2]+audiophile[2]
     * +flags = 21 bytes), packed as 12-bit delay / 12-bit padding. */
    if ((int)(flen - (p - frame)) > 35 && *p != 0) {
        int d = ((p[21] << 4) | (p[22] >> 4)) + (528 + 1);
        int q = (((p[22] & 0xF) << 8) | p[23]) - (528 + 1);
        st->lame = 1;
        st->delay_s = d > 0 ? d : 0;
        st->pad_s = q > 0 ? q : 0;
    }
}

/* ID3v1 tail length (128 B "TAG") if present — excluded from CBR length. */
static long id3v1_len(mp3_state_t *st)
{
    unsigned char t[3];

    if (st->file_size < 128)
        return 0;
    if (cio_seek(st->io, st->file_size - 128) < 0)
        return 0;
    if (cio_read(st->io, t, 3) != 3)
        return 0;
    return (t[0] == 'T' && t[1] == 'A' && t[2] == 'G') ? 128 : 0;
}

#ifndef RECHORD_AP_METADATA_ONLY
/* Size of an MPEG-1/2 Layer III frame starting at p, or 0 if p is not a
 * valid header (used to salvage the last frame when trailing junk — e.g.
 * an ID3v1 tag — follows it at EOF: minimp3 accepts a frame only when the
 * buffer holds frame[+nothing], so the junk must be trimmed away). */
static int tail_frame_len(const unsigned char *p, int avail)
{
    static const int br_tab_m1[16] =
        { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 };
    static const int br_tab_m2[16] =
        { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 };
    static const int sr_tab[4] = { 44100, 48000, 32000, 0 };
    int mpeg1, ver, br, sr, pad, size;

    if (avail < 4)
        return 0;
    if (p[0] != 0xFF || (p[1] & 0xE0) != 0xE0)
        return 0;
    if ((p[1] & 0x06) != 0x02)          /* layer III only                  */
        return 0;
    ver = p[1] & 0x18;
    if (ver != 0x18 && ver != 0x10)      /* MPEG-1 / MPEG-2                */
        return 0;
    mpeg1 = (ver == 0x18);
    br = mpeg1 ? br_tab_m1[(p[2] >> 4) & 0x0F]
               : br_tab_m2[(p[2] >> 4) & 0x0F];
    sr = sr_tab[(p[2] >> 2) & 0x03];
    if (br == 0 || sr == 0)
        return 0;
    pad = (p[2] >> 1) & 0x01;
    size = (mpeg1 ? 144 : 72) * br * 1000 / sr + pad * 4;
    return (size <= avail) ? size : 0;   /* 0: cannot trim to a whole frame */
}

/* Pull the next decoded frame into the residual (file-statics). Returns
 * frames (>0), 0 at end of stream, or a negative CODEC_ERR_*. */
static int fill_decode(mp3_state_t *st)
{
    int eof_trimmed = 0;

    for (;;) {
        if (s_win_pos >= s_win_len) {
            long r;

            if (s_win_eof)
                return 0;
            s_win_pos = s_win_len = 0;
            r = cio_read(st->io, s_win, (long)MP3_WIN_BYTES);
            if (r < 0)
                return CODEC_ERR_IO;
            if (r == 0) {
                s_win_eof = 1;
                return 0;
            }
            s_win_len = (int)r;
            if ((size_t)r < MP3_WIN_BYTES)
                s_win_eof = 1;         /* short read == EOF for a file */
        }

        {
            mp3dec_frame_info_t fi;
            int got;

            memset(&fi, 0, sizeof fi);
            got = mp3dec_decode_frame(&st->dec, s_win + s_win_pos,
                                      s_win_len - s_win_pos, s_res, &fi);
            if (got > 0) {
                if (fi.channels != st->channels || fi.hz != st->rate)
                    return CODEC_ERR_FORMAT;   /* mid-stream format change */
                s_win_pos += fi.frame_bytes;
                s_res_i = 0;
                s_res_frames = got;
                return got;
            }
            /* got == 0: no frame accepted at [pos..len). minimp3 needs
             * frame + NEXT header in-buffer to accept a frame, so a tail
             * frame is rejected with frame_bytes == scanned length — those
             * bytes must NOT be discarded (that destroys straddling
             * frames); collect more data instead. */
            if (s_win_eof) {
                int flen;

                if (!eof_trimmed) {
                    flen = tail_frame_len(s_win + s_win_pos,
                                          s_win_len - s_win_pos);
                    if (flen > 0 &&
                        s_win_pos + flen < s_win_len) {
                        /* Complete last frame + trailing junk (ID3v1):
                         * trim to exactly the frame so the consecutive-
                         * header path accepts it. */
                        s_win_len = s_win_pos + flen;
                        eof_trimmed = 1;
                        continue;
                    }
                }
                s_win_len = s_win_pos;         /* tail can never complete  */
                continue;
            }
            if (s_win_pos > 0) {
                memmove(s_win, s_win + s_win_pos,
                        (size_t)(s_win_len - s_win_pos));
                s_win_len -= s_win_pos;
                s_win_pos = 0;
            }
            if (s_win_len < MP3_WIN_BYTES) {
                long r = cio_read(st->io, s_win + s_win_len,
                                  (long)(MP3_WIN_BYTES - s_win_len));
                if (r < 0)
                    return CODEC_ERR_IO;
                if (r == 0)
                    s_win_eof = 1;
                s_win_len += (int)r;
                continue;
            }
            /* Full window, still no frame (pathological corruption):
             * force one byte of progress so the scan can move on. */
            s_win_pos += 1;
            continue;
        }
    }
}
#endif /* !RECHORD_AP_METADATA_ONLY */

static void reset_feed(mp3_state_t *st)
{
    s_win_pos = s_win_len = 0;
    s_win_eof = 0;
    s_res_frames = s_res_i = 0;
#ifndef RECHORD_AP_METADATA_ONLY
    mp3dec_init(&st->dec);
#else
    (void)st;
#endif
}

/* ---- codec_t callbacks -------------------------------------------------- */

static int mp3_open(void *vst, const char *path)
{
    mp3_state_t *st = (mp3_state_t *)vst;
#ifndef RECHORD_AP_METADATA_ONLY
    mp3dec_frame_info_t fi;
#endif
    unsigned char hdr[10];
    long r;
    int i, off = 0;
#ifndef RECHORD_AP_METADATA_ONLY
    int n = 0;
#endif
    int rc;

    if (st == 0 || path == 0)
        return CODEC_ERR_STATE;

    /* The per-track buffers live in the shared workbuf region (see the header
     * note); a busy region means FLAC or fw_update is up. The metadata-only
     * AP build keeps the claim: its open probe still reads the first frame
     * (and the Xing/LAME tag) through this window. */
    if (workbuf_claim(WORKBUF_OWNER_MP3) == 0)
        return CODEC_ERR_IO;
    s_win = (uint8_t *)workbuf_for(WORKBUF_OWNER_MP3);
    s_res = (int16_t *)(void *)(s_win + MP3_WIN_BYTES);

    /* reset the single-track file-statics */
    s_win_pos = s_win_len = 0;
    s_win_eof = 0;
    s_res_frames = s_res_i = 0;
    memset(&st->rg, 0, sizeof st->rg);
    memset(&st->meta, 0, sizeof st->meta);
    st->have_xing = st->have_toc = st->lame = 0;
    st->xing_frames = st->xing_bytes = 0;
    st->delay_s = st->pad_s = 0;

    st->io = cio_open(path);
    if (st->io == 0) {
        rc = CODEC_ERR_IO;
        goto fail;
    }
    st->file_size = cio_size(st->io);
    st->tag_end = 0;
    st->total_samples = -1;
    st->duration_ms = -1;

    /* ID3v2: skip it for audio, scan it for ReplayGain. */
    if (cio_read(st->io, hdr, 10) == 10 &&
        hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3') {
        unsigned sz;
        if (hdr[6] & 0x80 || hdr[7] & 0x80 ||
            hdr[8] & 0x80 || hdr[9] & 0x80) {
            cio_close(st->io);
            st->io = 0;
            rc = CODEC_ERR_FORMAT;   /* not a synchsafe size */
            goto fail;
        }
        sz = ((unsigned)(hdr[6] & 0x7F) << 21) |
             ((unsigned)(hdr[7] & 0x7F) << 14) |
             ((unsigned)(hdr[8] & 0x7F) << 7) |
             (unsigned)(hdr[9] & 0x7F);
        st->tag_end = 10 + (long)sz + ((hdr[5] & 0x10) ? 10 : 0); /* footer */
        if (st->tag_end > st->file_size) {
            cio_close(st->io);
            st->io = 0;
            rc = CODEC_ERR_FORMAT;
            goto fail;
        }
        (void)tags_read_id3v2(st->io, &st->meta, &st->rg);
    }

    /* ID3v1 tail (length only; CBR duration) */
    {
        long tail = id3v1_len(st);
        st->file_size_audio = st->file_size - tail; /* remembered below */
    }

    if (cio_seek(st->io, st->tag_end) < 0) {
        cio_close(st->io);
        st->io = 0;
        rc = CODEC_ERR_IO;
        goto fail;
    }

    /* first window + header-only probe (pcm == NULL) */
    r = cio_read(st->io, s_win, (long)MP3_WIN_BYTES);
    if (r <= 0) {
        cio_close(st->io);
        st->io = 0;
        rc = CODEC_ERR_FORMAT;
        goto fail;
    }
    s_win_len = (int)r;
    s_win_eof = ((size_t)r < MP3_WIN_BYTES);

#ifndef RECHORD_AP_METADATA_ONLY
    mp3dec_init(&st->dec);
    for (i = 0; i < 9 && n <= 0; i++) {
        memset(&fi, 0, sizeof fi);
        n = mp3dec_decode_frame(&st->dec, s_win + off,
                                s_win_len - off, NULL, &fi);
        if (n <= 0) {
            off += (fi.frame_bytes > 0) ? fi.frame_bytes : 1;
            if (off >= s_win_len)
                break;
        }
    }
    if (n <= 0 || fi.hz == 0 || fi.channels == 0 ||
        fi.channels > MP3_MAX_CH || fi.layer != 3) {
        cio_close(st->io);
        st->io = 0;
        rc = fi.layer && fi.layer != 3 ? CODEC_ERR_UNSUPPORTED
                                       : CODEC_ERR_FORMAT;
        goto fail;
    }
    st->rate = fi.hz;
    st->channels = fi.channels;
    st->spf = n;
    st->layer = fi.layer;
    st->bitrate = fi.bitrate_kbps;

    /* Xing/Info + LAME gapless fields; the info frame is metadata — skip
     * its bytes only when it actually IS a Xing frame (a bare first frame
     * is audio and must decode). */
    parse_xing(st, s_win + off, fi.frame_bytes);
    s_win_pos = off + (st->have_xing ? fi.frame_bytes : 0);
    mp3dec_init(&st->dec);               /* fresh state for the audio */
#else
    /* Header-only probe: no decoder on the AP. Scan for the first Layer III
     * frame header and read its geometry from the tables. */
    {
        int hz = 0, ch = 0, br = 0, spf = 0, flen = 0;

        for (i = 0; i < s_win_len - 4; i++) {
            if (mp3_frame_info(s_win + i, s_win_len - i, &hz, &ch, &br,
                               &spf, &flen) == 0)
                break;
        }
        if (i >= s_win_len - 4) {
            cio_close(st->io);
            st->io = 0;
            rc = CODEC_ERR_FORMAT;
            goto fail;
        }
        off = i;
        st->rate = hz;
        st->channels = ch;
        st->spf = spf;
        st->layer = 3;
        st->bitrate = br;
        parse_xing(st, s_win + off, flen);
        s_win_pos = off + (st->have_xing ? flen : 0);
    }
#endif

    /* duration: Xing (exact frames) > CBR estimate > unknown.
     * int64 math: samples*1000 overflows 32-bit `long` past ~48 s at
     * 44.1 kHz, and the wrapped (often negative) value reached the UI as a
     * bogus few-seconds length (BRNOMT-65). */
    if (st->have_xing && st->xing_frames > 0 && st->rate > 0) {
        st->total_samples = (long)st->xing_frames * st->spf;
        st->duration_ms = (long)(((int64_t)st->total_samples * 1000) /
                                 st->rate);
    } else if (st->bitrate > 0) {
        long audio = st->file_size_audio - st->tag_end;
        if (audio < 0)
            audio = 0;
        st->duration_ms = audio * 8 / st->bitrate; /* bytes*8/kbps = ms */
        st->total_samples = (long)(((int64_t)st->rate * st->duration_ms) /
                                   1000);
    }
    return CODEC_OK;

fail:
    workbuf_release(WORKBUF_OWNER_MP3);
    s_win = 0;
    s_res = 0;
    return rc;
}

static int mp3_decode(void *vst, int32_t *pcm, int max_frames)
{
    mp3_state_t *st = (mp3_state_t *)vst;
    int out = 0;

    if (st == 0 || pcm == 0 || max_frames <= 0)
        return CODEC_ERR_STATE;
#ifdef RECHORD_AP_METADATA_ONLY
    /* The AP never owns the decode (Core B does). */
    (void)out;
    return CODEC_ERR_UNSUPPORTED;
#else
    while (out < max_frames) {
        if (s_res_i < s_res_frames) {
            int avail = s_res_frames - s_res_i;
            int n = (avail < max_frames - out) ? avail : max_frames - out;
            const int16_t *src = s_res + (long)s_res_i * st->channels;
            int32_t *dst = pcm + (long)out * st->channels;
            int k, total = n * st->channels;

            /* minimp3 yields s16; widen to the int32 left-justified pipeline
             * format (sample << 16) so the internal path is width-uniform. */
            for (k = 0; k < total; k++)
                dst[k] = (int32_t)src[k] << 16;
            out += n;
            s_res_i += n;
            continue;
        }
        {
            int r = fill_decode(st);
            if (r < 0)
                return r;
            if (r == 0)
                break;                  /* end of stream */
        }
    }
    return out;                          /* 0 == EOF (codec contract) */
#endif
}

static int mp3_seek(void *vst, long ms)
{
    mp3_state_t *st = (mp3_state_t *)vst;
    long target = st->tag_end;

    if (st == 0 || st->io == 0)
        return CODEC_ERR_STATE;
    if (ms < 0)
        ms = 0;
    if (st->duration_ms > 0 && ms > st->duration_ms)
        ms = st->duration_ms;

    if (st->have_toc && st->xing_bytes > 0 && st->duration_ms > 0) {
        long pct = (long)(((int64_t)ms * 100) / st->duration_ms);
        if (pct > 99)
            pct = 99;
        target += (long)((unsigned long)st->toc[pct] *
                         st->xing_bytes / 256);
    } else if (st->duration_ms > 0) {
        long audio = st->file_size_audio - st->tag_end;
        if (audio < 0)
            audio = 0;
        /* int64 math: bytes*ms overflows 32-bit `long` on the target. */
        target += (long)(((int64_t)audio * ms) / st->duration_ms);
    }
    if (target > st->file_size_audio)
        target = st->file_size_audio;
    if (cio_seek(st->io, target) < 0)
        return CODEC_ERR_IO;

    /* Resume frame-aligned at the first frame header at/after the byte
     * target (TOC accuracy: <= 1 frame of error) — no sample-level
     * forward-scan: the byte position already IS the seek position. */
    reset_feed(st);
    return CODEC_OK;
}

static long mp3_duration_ms(void *vst)
{
    mp3_state_t *st = (mp3_state_t *)vst;
    return (st == 0) ? -1 : st->duration_ms;
}

static void mp3_close(void *vst)
{
    mp3_state_t *st = (mp3_state_t *)vst;

    if (st == 0)
        return;
    if (st->io != 0)
        cio_close(st->io);
    st->io = 0;
    s_win_pos = s_win_len = 0;
    s_win_eof = 0;
    s_res_frames = s_res_i = 0;
    /* Hand the shared region back (no-op if never claimed). */
    workbuf_release(WORKBUF_OWNER_MP3);
    s_win = 0;
    s_res = 0;
}

static int mp3_format(void *vst, codec_format_t *out)
{
    mp3_state_t *st = (mp3_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    out->sample_rate = st->rate;
    out->channels = st->channels;
    out->bits = 16;
    out->total_samples = st->total_samples;
    return CODEC_OK;
}

static int mp3_gapless(void *vst, codec_gapless_t *out)
{
    mp3_state_t *st = (mp3_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    out->delay_samples = st->delay_s;
    out->padding_samples = st->pad_s;
    return CODEC_OK;                     /* 0/0 when no LAME tag */
}

static int mp3_tags(void *vst, codec_replaygain_t *out)
{
    mp3_state_t *st = (mp3_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    *out = st->rg;
    return CODEC_OK;
}

/* ID3v2 text metadata (TIT2/TPE1/TALB/TCON/TRCK), parsed with the same pass
 * that reads the ReplayGain TXXX frames. Empty fields mean "no such tag". */
static int mp3_meta(void *vst, codec_meta_t *out)
{
    mp3_state_t *st = (mp3_state_t *)vst;

    if (st == 0 || out == 0)
        return CODEC_ERR_STATE;
    *out = st->meta;
    return CODEC_OK;
}

const codec_t codec_mp3 =
{
    "mp3",
    "mp3",
    mp3_open,
    mp3_decode,
    mp3_seek,
    mp3_duration_ms,
    mp3_close,
    (int)sizeof(mp3_state_t),
    mp3_format,
    mp3_gapless,
    mp3_tags,
    mp3_meta
};
