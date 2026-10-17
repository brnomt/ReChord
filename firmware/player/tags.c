/*
 * tags.c — container tag readers: ID3v2 text/TXXX frames + Vorbis comments.
 *
 * Self-contained: only the small codec_io primitives are used, so this
 * compiles freestanding on the target (no libc, no allocation). One pass
 * per container fills both the text metadata (title/artist/album/genre/
 * track) and the ReplayGain values, so a codec pays for the header walk
 * once.
 *
 * Layouts walked (both spec'd):
 *   ID3v2.3/2.4: 10-byte header (ID3 + version + flags + 4-byte synchsafe
 *   size), then 10-byte frame headers (4-char ID + size + 2 flag bytes).
 *   Text frames carry 1 encoding byte + text; TXXX carries 1 encoding byte
 *   + NUL-terminated description + value.
 *   Vorbis comment block: vendor_length(4LE) + vendor + count(4LE) + count
 *   x [ length(4LE) + "KEY=value" ]. The vendor field is why the block body
 *   start is NOT the count offset — 2026-10-17 fix (HANDOVER §Q): reading
 *   the vendor length as the count silently killed ReplayGain on real FLAC
 *   files (only the vendor-less synthetic fixtures worked).
 *
 * Deliberately simple: case-insensitive key match, bounded copies into the
 * caller's struct, a tiny float reader for dB (gain) and 0..1 (peak)
 * values. Unsupported layouts (ID3v2.2, unsync/extended headers, frame
 * modifiers) are reported as "nothing found" instead of being misparsed.
 */
#include "tags.h"

/* Per-frame/per-entry scratch: enough for a text field (destinations are
 * 32/64 bytes), capped so a huge frame is never buffered whole. */
#define TAG_SLAB 128

/* Bounded FLAC metadata-chain walk (a chain longer than this is malformed). */
#define TAG_FLAC_BLOCKS 64

/* ---- little helpers over codec_io ---- */

static int read_u32_le(codec_io_t *io, unsigned int *out)
{
    unsigned char b[4];
    if (cio_read(io, b, 4) != 4)
        return -1;
    *out = (unsigned int)(b[0] | (b[1] << 8) |
                          (b[2] << 16) | (b[3] << 24));
    return 0;
}

static unsigned int read_u32_be(const unsigned char *p)
{
    return ((unsigned int)p[0] << 24) | ((unsigned int)p[1] << 16) |
           ((unsigned int)p[2] << 8)  | ((unsigned int)p[3]);
}

/* ID3v2.4 synchsafe integer (4 bytes, MSB set clear). */
static unsigned int synchsafe(const unsigned char *p)
{
    return ((unsigned int)(p[0] & 0x7F) << 21) |
           ((unsigned int)(p[1] & 0x7F) << 14) |
           ((unsigned int)(p[2] & 0x7F) << 7)  |
           ((unsigned int)(p[3] & 0x7F));
}

/* Case-insensitive match of `n` bytes of `p` against the NUL-terminated
 * `key` (key must be exactly `n` chars). Used for Vorbis comment keys,
 * which sit in "KEY=value" runs (not NUL-terminated at the key). */
static int key_eq_n(const char *p, long n, const char *key)
{
    long i = 0;
    while (i < n && key[i]) {
        char c = p[i], k = key[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        if (k >= 'A' && k <= 'Z') k += 32;
        if (c != k)
            return 0;
        i++;
    }
    return (i == n && key[i] == '\0') ? 1 : 0;
}

/* Case-insensitive match of NUL-terminated `p` against `key`. */
static int key_eq(const char *p, const char *key)
{
    int i = 0;
    while (p[i] && key[i]) {
        char c = p[i], k = key[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        if (k >= 'A' && k <= 'Z') k += 32;
        if (c != k)
            return 0;
        i++;
    }
    return p[i] == key[i];
}

/* Parse a signed decimal with optional fractional part; returns the value
 * in dB * 10 (so "6.0" -> 60, "-6.0" -> -60). */
static int parse_db10(const char *s, int *out)
{
    int sign = 1;
    long intpart = 0;
    long frac = 0;
    int ndec = 0;

    while (*s == ' ' || *s == '\t' || *s == '"')
        s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }
    while (*s >= '0' && *s <= '9') {
        if (intpart < 100000)           /* clamp sane */
            intpart = intpart * 10 + (*s - '0');
        s++;
    }
    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9') {
            if (ndec < 6)
                frac = frac * 10 + (*s - '0');
            ndec++;
            s++;
        }
        while (ndec > 1) { frac /= 10; ndec--; }
    }
    /* drop a trailing "dB" unit */
    while (*s == ' ' || *s == '\t' || *s == 'd' || *s == 'D' ||
           *s == 'b' || *s == 'B')
        s++;
    *out = sign * (int)(intpart * 10 + (frac % 10));
    return 0;
}

/* Parse a 0.0..1.0 float into Q1.15 (0..32767). */
static int parse_peak_q15(const char *s, int *out)
{
    long intpart = 0;
    long frac = 0;
    int ndec = 0;
    long scale;
    long v;

    while (*s == ' ' || *s == '\t' || *s == '"')
        s++;
    if (*s == '-') s++;
    else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') { intpart = intpart * 10 + (*s - '0'); s++; }
    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9') { frac = frac * 10 + (*s - '0'); ndec++; s++; }
    }
    scale = 1;
    while (ndec > 0) { scale *= 10; ndec--; }
    /* peak is in [0,1]; map intpart.frac -> Q1.15 = val*32767, clamped. */
    v = intpart * 32767 + (long)(frac * 32767) / scale;
    if (v < 0) v = 0;
    if (v > 32767) v = 32767;
    *out = (int)v;
    return 0;
}

/* Parse a track number: leading decimal of "N" or "N/M" (0 = unknown). */
static int parse_track_no(const char *s)
{
    int v = 0;
    int digits = 0;

    while (*s == ' ' || *s == '\t')
        s++;
    while (*s >= '0' && *s <= '9' && digits < 4) {
        v = v * 10 + (*s - '0');
        s++;
        digits++;
    }
    return v;
}

/* ---- text-field helpers ---- */

static void meta_zero(codec_meta_t *m)
{
    if (m == 0)
        return;
    m->title[0] = '\0';
    m->artist[0] = '\0';
    m->album[0] = '\0';
    m->genre[0] = '\0';
    m->track = 0;
}

static void rg_zero(codec_replaygain_t *rg)
{
    if (rg == 0)
        return;
    rg->track_gain_tenth_db = 0;
    rg->album_gain_tenth_db = 0;
    rg->track_peak_q15 = 0;
    rg->album_peak_q15 = 0;
}

/* Copy a tag value into a bounded, NUL-terminated destination. Stops at a
 * NUL (Vorbis values and ID3 text can both carry one) and at `cap-1`.
 * Returns the number of bytes stored. */
static int copy_text(char *dst, int cap, const char *src, long n)
{
    int len = 0;

    if (cap <= 0)
        return 0;
    dst[0] = '\0';
    if (src == 0)
        return 0;
    while (len < n && src[len] != '\0' && len < cap - 1) {
        dst[len] = src[len];
        len++;
    }
    dst[len] = '\0';
    return len;
}

/* ID3v2 text-frame body -> narrow NUL-terminated string. Encodings 0
 * (ISO-8859-1) and 3 (UTF-8) are copied as bytes; 1/2 (UTF-16 with BOM /
 * UTF-16BE) are narrow-decoded so a UTF-16 tagged file still shows its
 * ASCII/Latin-1 text instead of NUL-separated garbage — the UI font is
 * ASCII-only, so anything above 0xFF becomes '?'. v2.4 NUL-joined values:
 * the first one wins. Returns the stored length. */
static int id3_text_narrow(char *dst, int cap, const unsigned char *b, long n)
{
    int len = 0;
    int enc;
    long i;

    if (cap <= 0)
        return 0;
    dst[0] = '\0';
    if (b == 0 || n <= 0)
        return 0;

    enc = b[0];
    i = 1;
    if (enc == 0 || enc == 3) {
        for (; i < n && b[i] != '\0'; i++) {
            if (len >= cap - 1)
                break;
            dst[len++] = (char)b[i];
        }
    } else if (enc == 1 || enc == 2) {
        int be = (enc == 2);

        if (enc == 1 && i + 1 < n) {            /* BOM decides the order */
            if (b[i] == 0xFF && b[i + 1] == 0xFE) { be = 0; i += 2; }
            else if (b[i] == 0xFE && b[i + 1] == 0xFF) { be = 1; i += 2; }
        }
        for (; i + 1 < n; i += 2) {
            unsigned int cp = be ? (((unsigned int)b[i] << 8) | b[i + 1])
                                 : (((unsigned int)b[i + 1] << 8) | b[i]);
            if (cp == 0)
                break;
            if (len >= cap - 1)
                break;
            dst[len++] = (char)(cp < 0x100 ? cp : '?');
        }
    }
    dst[len] = '\0';
    return len;
}

/* Read up to TAG_SLAB-1 bytes of a frame/entry body into `slab` and
 * NUL-terminate it. Returns the byte count, or -1 (nothing usable). */
static long read_body(codec_io_t *io, long body, unsigned long size,
                      unsigned char *slab)
{
    long n = (size > (TAG_SLAB - 1)) ? (TAG_SLAB - 1) : (long)size;

    if (n <= 0)
        return -1;
    if (cio_seek(io, body) != body)
        return -1;
    if (cio_read(io, slab, n) != n)
        return -1;
    slab[n] = '\0';
    return n;
}

/* ---- ID3v2 ---- */

/* TXXX frame body -> REPLAYGAIN_* (encoding + description + value). */
static int id3_txxx(codec_io_t *io, long body, unsigned long size,
                    codec_replaygain_t *rg)
{
    unsigned char slab[TAG_SLAB];
    long n, klen;
    const char *val;

    if (rg == 0 || size == 0)
        return 0;
    n = read_body(io, body, size, slab);
    if (n < 0)
        return 0;

    /* slab[0] = encoding; the description is NUL-terminated, then the value.
     * A UTF-16 description would break this scan (rare, documented). */
    klen = 1;
    while (klen < n && slab[klen] != '\0')
        klen++;
    if (klen >= n)
        return 0;
    val = (const char *)slab + klen + 1;

    if (key_eq((const char *)&slab[1], "REPLAYGAIN_TRACK_GAIN")) {
        parse_db10(val, &rg->track_gain_tenth_db);
        return TAG_HAVE_RG;
    }
    if (key_eq((const char *)&slab[1], "REPLAYGAIN_ALBUM_GAIN")) {
        parse_db10(val, &rg->album_gain_tenth_db);
        return TAG_HAVE_RG;
    }
    if (key_eq((const char *)&slab[1], "REPLAYGAIN_TRACK_PEAK")) {
        parse_peak_q15(val, &rg->track_peak_q15);
        return TAG_HAVE_RG;
    }
    if (key_eq((const char *)&slab[1], "REPLAYGAIN_ALBUM_PEAK")) {
        parse_peak_q15(val, &rg->album_peak_q15);
        return TAG_HAVE_RG;
    }
    return 0;
}

/* Year out of a date-ish string ("2015", "2015-06-01", "2015/06"): the first
 * four digits, and only when they form a plausible year. Anything else (empty,
 * "20xx", "0000", out of range) is 0 -- the caller renders that as "Undefined"
 * rather than showing a wrong number. */
static int parse_year(const char *s)
{
    int y = 0, digits = 0;

    if (s == 0)
        return 0;
    while (*s == ' ' || *s == '\t')
        s++;
    while (*s >= '0' && *s <= '9') {
        if (digits < 4)
            y = y * 10 + (*s - '0');
        digits++;
        s++;
    }
    if (digits != 4 || y < 1000 || y > 9999)
        return 0;
    return y;
}

/* Text frame (TIT2/TPE1/TALB/TCON/TRCK/TYER/TDRC/TPUB/TPE2/TCOM). */
static int id3_text(codec_io_t *io, const unsigned char *fh, long body,
                    unsigned long size, codec_meta_t *meta)
{
    unsigned char slab[TAG_SLAB];
    char num[16];
    long n;
    int len;

    n = read_body(io, body, size, slab);        /* also does the encoding */
    if (n < 0)
        return 0;

    if (fh[0] == 'T' && fh[1] == 'I' && fh[2] == 'T' && fh[3] == '2')
        len = id3_text_narrow(meta->title, (int)sizeof meta->title, slab, n);
    else if (fh[0] == 'T' && fh[1] == 'P' && fh[2] == 'E' && fh[3] == '1')
        len = id3_text_narrow(meta->artist, (int)sizeof meta->artist, slab, n);
    else if (fh[0] == 'T' && fh[1] == 'A' && fh[2] == 'L' && fh[3] == 'B')
        len = id3_text_narrow(meta->album, (int)sizeof meta->album, slab, n);
    else if (fh[0] == 'T' && fh[1] == 'C' && fh[2] == 'O' && fh[3] == 'N')
        len = id3_text_narrow(meta->genre, (int)sizeof meta->genre, slab, n);
    else if (fh[0] == 'T' && fh[1] == 'Y' && fh[2] == 'E' && fh[3] == 'R') {
        if (id3_text_narrow(num, (int)sizeof num, slab, n) <= 0)
            return 0;
        meta->year = parse_year(num);
        return meta->year ? TAG_HAVE_META : 0;
    } else if (fh[0] == 'T' && fh[1] == 'D' && fh[2] == 'R' && fh[3] == 'C') {
        if (id3_text_narrow(num, (int)sizeof num, slab, n) <= 0)
            return 0;
        meta->year = parse_year(num);
        return meta->year ? TAG_HAVE_META : 0;
    } else if (fh[0] == 'T' && fh[1] == 'P' && fh[2] == 'U' && fh[3] == 'B')
        len = id3_text_narrow(meta->label, (int)sizeof meta->label, slab, n);
    else if (fh[0] == 'T' && fh[1] == 'P' && fh[2] == 'E' && fh[3] == '2')
        len = id3_text_narrow(meta->album_artist, (int)sizeof meta->album_artist, slab, n);
    else if (fh[0] == 'T' && fh[1] == 'C' && fh[2] == 'O' && fh[3] == 'M')
        len = id3_text_narrow(meta->composer, (int)sizeof meta->composer, slab, n);
    else if (fh[0] == 'T' && fh[1] == 'R' && fh[2] == 'C' && fh[3] == 'K') {
        if (id3_text_narrow(num, (int)sizeof num, slab, n) <= 0)
            return 0;
        meta->track = parse_track_no(num);
        return TAG_HAVE_META;
    } else {
        return 0;                               /* a text frame we do not keep */
    }
    return (len > 0) ? TAG_HAVE_META : 0;
}

int tags_read_id3v2(codec_io_t *io, codec_meta_t *meta, codec_replaygain_t *rg)
{
    unsigned char hdr[10];
    unsigned int tag_end, file_size;
    int ver, flags = 0;
    long pos;

    meta_zero(meta);
    rg_zero(rg);
    if (io == 0)
        return 0;

    if (cio_seek(io, 0) != 0)
        return 0;
    if (cio_read(io, hdr, 10) != 10)
        return 0;
    if (hdr[0] != 'I' || hdr[1] != 'D' || hdr[2] != '3')
        return 0;
    /* v2.2 frame headers are 6 bytes with 3-char IDs; unsynchronised tags
     * rewrite every byte 0xFF. Both are reported as "no tags" rather than
     * walked with the v2.3/2.4 layout. */
    if (hdr[3] != 3 && hdr[3] != 4)
        return 0;
    if (hdr[5] & 0xC0)                  /* unsync (0x80) / extended header */
        return 0;
    if (hdr[6] & 0x80 || hdr[7] & 0x80 || hdr[8] & 0x80 || hdr[9] & 0x80)
        return 0;                       /* malformed synchsafe size */
    ver = hdr[3];

    tag_end = 10 + synchsafe(&hdr[6]);
    file_size = (unsigned int)cio_size(io);
    if (file_size > 0 && tag_end > file_size)
        tag_end = file_size;

    for (pos = 10; pos + 10 <= (long)tag_end; ) {
        unsigned char fh[10];
        unsigned long fsize;
        long body;

        if (cio_seek(io, pos) != pos)
            break;
        if (cio_read(io, fh, 10) != 10)
            break;
        if (fh[0] == '\0')
            break;                      /* padding: end of the frames */
        fsize = (ver == 4) ? synchsafe(&fh[4]) : read_u32_be(&fh[4]);
        if ((long)fsize > (long)tag_end - (pos + 10))
            break;                      /* frame runs past the tag */
        body = pos + 10;

        /* Frame modifiers (v2.3 fh[8], v2.4 fh[9]: compression, encryption,
         * grouping, unsync, data-length indicator) change the body layout. */
        if ((ver == 3 && fh[8] != 0) || (ver == 4 && fh[9] != 0)) {
            pos += 10 + (long)fsize;
            continue;
        }

        if (fh[0] == 'T' && fh[1] == 'X' && fh[2] == 'X' && fh[3] == 'X')
            flags |= id3_txxx(io, body, fsize, rg);
        else if (meta != 0 && fh[0] == 'T')
            flags |= id3_text(io, fh, body, fsize, meta);

        pos += 10 + (long)fsize;
    }
    return flags;
}

/* ---- Vorbis comment block ---- */

int tags_read_vorbis(codec_io_t *io, long offset, long length,
                     codec_meta_t *meta, codec_replaygain_t *rg)
{
    unsigned int count, i, vendor_len;
    long pos, end;
    int flags = 0;

    meta_zero(meta);
    rg_zero(rg);
    if (io == 0 || length <= 0)
        return 0;
    if (cio_seek(io, offset) != offset)
        return 0;

    /* Spec layout starts with the vendor string, so the count is NOT the
     * first u32 of the block body (see the file header comment). */
    if (read_u32_le(io, &vendor_len) != 0)
        return 0;
    pos = offset + 4 + (long)vendor_len;
    if (vendor_len > (unsigned int)length || pos > offset + length)
        return 0;                       /* malformed vendor field */
    if (cio_seek(io, pos) != pos)
        return 0;
    if (read_u32_le(io, &count) != 0)
        return 0;
    pos += 4;
    end = offset + length;

    for (i = 0; i < count && pos < end; i++) {
        unsigned char slab[TAG_SLAB];
        unsigned int len;
        long n, klen;
        const char *val;

        if (read_u32_le(io, &len) != 0)
            break;
        pos += 4;
        if (len == 0 || pos + (long)len > end)
            break;                      /* malformed entry length */

        n = read_body(io, pos, len, slab);
        pos += (long)len;
        if (cio_seek(io, pos) != pos)   /* skip the rest of a long entry */
            break;
        if (n < 0)
            continue;

        klen = 0;
        while (klen < n && slab[klen] != '=')
            klen++;
        if (klen >= n)
            continue;                   /* not a key/value pair */
        slab[klen] = '\0';
        val = (const char *)slab + klen + 1;

        if (rg != 0) {
            if (key_eq_n((const char *)slab, klen, "REPLAYGAIN_TRACK_GAIN")) {
                parse_db10(val, &rg->track_gain_tenth_db);
                flags |= TAG_HAVE_RG;
            } else if (key_eq_n((const char *)slab, klen, "REPLAYGAIN_ALBUM_GAIN")) {
                parse_db10(val, &rg->album_gain_tenth_db);
                flags |= TAG_HAVE_RG;
            } else if (key_eq_n((const char *)slab, klen, "REPLAYGAIN_TRACK_PEAK")) {
                parse_peak_q15(val, &rg->track_peak_q15);
                flags |= TAG_HAVE_RG;
            } else if (key_eq_n((const char *)slab, klen, "REPLAYGAIN_ALBUM_PEAK")) {
                parse_peak_q15(val, &rg->album_peak_q15);
                flags |= TAG_HAVE_RG;
            }
        }
        if (meta != 0) {
            int len;

            if (key_eq_n((const char *)slab, klen, "TITLE"))
                len = copy_text(meta->title, (int)sizeof meta->title, val, n - klen - 1);
            else if (key_eq_n((const char *)slab, klen, "ARTIST"))
                len = copy_text(meta->artist, (int)sizeof meta->artist, val, n - klen - 1);
            else if (key_eq_n((const char *)slab, klen, "ALBUM"))
                len = copy_text(meta->album, (int)sizeof meta->album, val, n - klen - 1);
            else if (key_eq_n((const char *)slab, klen, "GENRE"))
                len = copy_text(meta->genre, (int)sizeof meta->genre, val, n - klen - 1);
            else if (key_eq_n((const char *)slab, klen, "DATE") ||
                     key_eq_n((const char *)slab, klen, "YEAR")) {
                char ybuf[16];

                if (copy_text(ybuf, (int)sizeof ybuf, val, n - klen - 1) > 0) {
                    meta->year = parse_year(ybuf);
                    if (meta->year)
                        flags |= TAG_HAVE_META;
                }
                continue;
            } else if (key_eq_n((const char *)slab, klen, "LABEL"))
                len = copy_text(meta->label, (int)sizeof meta->label, val, n - klen - 1);
            else if (key_eq_n((const char *)slab, klen, "ALBUMARTIST") ||
                     key_eq_n((const char *)slab, klen, "ALBUM ARTIST"))
                len = copy_text(meta->album_artist, (int)sizeof meta->album_artist, val, n - klen - 1);
            else if (key_eq_n((const char *)slab, klen, "COMPOSER"))
                len = copy_text(meta->composer, (int)sizeof meta->composer, val, n - klen - 1);
            else if (key_eq_n((const char *)slab, klen, "TRACKNUMBER")) {
                meta->track = parse_track_no(val);
                flags |= TAG_HAVE_META;
                continue;
            } else {
                continue;               /* a comment we do not keep */
            }
            if (len > 0)
                flags |= TAG_HAVE_META;
        }
    }
    return flags;
}

/* ---- per-file reader (listings) ---- */

static int ext_is(const char *ext, const char *want)
{
    int i = 0;

    while (ext[i] && want[i]) {
        char c = ext[i];
        if (c >= 'A' && c <= 'Z')
            c += 32;
        if (c != want[i])
            return 0;
        i++;
    }
    return ext[i] == '\0' && want[i] == '\0';
}

/* Walk a FLAC metadata chain and read its VORBIS_COMMENT (type 4) block.
 * Only the header region is touched: the chain ends at the first audio
 * frame. Returns TAG_HAVE_* flags. */
static int flac_read_vorbis(codec_io_t *io, codec_meta_t *meta,
                            codec_replaygain_t *rg)
{
    unsigned char hdr[4];
    int flags = 0, i;

    if (cio_seek(io, 0) != 0)
        return 0;
    if (cio_read(io, hdr, 4) != 4 ||
        hdr[0] != 'f' || hdr[1] != 'L' || hdr[2] != 'a' || hdr[3] != 'C')
        return 0;

    for (i = 0; i < TAG_FLAC_BLOCKS; i++) {
        long pos, len;

        if (cio_read(io, hdr, 4) != 4)
            break;
        len = ((long)hdr[1] << 16) | ((long)hdr[2] << 8) | (long)hdr[3];
        pos = cio_tell(io);
        if (pos < 0)
            break;
        if ((hdr[0] & 0x7F) == 4)
            flags |= tags_read_vorbis(io, pos, len, meta, rg);
        if (cio_seek(io, pos + len) != pos + len)
            break;
        if (hdr[0] & 0x80)
            break;                      /* last-metadata-block flag */
    }
    return flags;
}

int tags_read_file(const char *path, codec_meta_t *meta, codec_replaygain_t *rg)
{
    codec_io_t *io;
    const char *dot, *p;
    int flags;

    meta_zero(meta);
    rg_zero(rg);
    if (path == 0 || path[0] == '\0')
        return 0;

    dot = 0;
    for (p = path; *p; p++) {
        if (*p == '.')
            dot = p;
        else if (*p == '/' || *p == '\\')
            dot = 0;
    }
    if (dot == 0 || dot[1] == '\0')
        return 0;

    if (!ext_is(dot + 1, "mp3") && !ext_is(dot + 1, "flac"))
        return 0;                       /* no tag reader for this container */

    io = cio_open(path);
    if (io == 0)
        return 0;
    if (ext_is(dot + 1, "mp3"))
        flags = tags_read_id3v2(io, meta, rg);
    else
        flags = flac_read_vorbis(io, meta, rg);
    cio_close(io);
    return flags;
}

/* ---- ReplayGain-only wrappers (kept contract) ---- */

int tags_replaygain_id3v2(codec_io_t *io, codec_replaygain_t *out)
{
    return (tags_read_id3v2(io, 0, out) & TAG_HAVE_RG) ? 0 : -1;
}

int tags_replaygain_vorbis(codec_io_t *io, long offset, long length,
                           codec_replaygain_t *out)
{
    return (tags_read_vorbis(io, offset, length, 0, out) & TAG_HAVE_RG) ? 0 : -1;
}
