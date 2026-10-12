/*
 * tags.c — minimal ReplayGain tag readers (ID3v2 TXXX + Vorbis comments).
 *
 * Self-contained: only the small codec_io primitives and codec_replaygain_t
 * are used, so this compiles freestanding on the target (no libc). Text
 * parsing is deliberately simple: case-insensitive key match + a tiny
 * float reader for dB (gain) and 0..1 (peak) values.
 */
#include "tags.h"

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
        if (c >= 'A' && c <= 'Z') c -= 32;
        if (k >= 'A' && k <= 'Z') k -= 32;
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
        if (c >= 'A' && c <= 'Z') c -= 32;
        if (k >= 'A' && k <= 'Z') k -= 32;
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

/* ---- Vorbis comment scanner ---- */

/* Scan a Vorbis comment region at `offset`..offset+length and fill `out` with
 * any REPLAYGAIN_* values found. Returns 0 if at least one field was found,
 * -1 otherwise. Works on Ogg FLAC / Vorbis / Opus comment blocks (the block
 * body after the 32-bit element-count header). */
int tags_replaygain_vorbis(codec_io_t *io, long offset, long length,
                           codec_replaygain_t *out)
{
    unsigned int count, i;
    long pos, end;
    int found = 0;

    if (io == 0 || out == 0 || length <= 0)
        return -1;
    if (cio_seek(io, offset) != offset)
        return -1;
    if (read_u32_le(io, &count) != 0)
        return -1;

    out->track_gain_tenth_db = 0;
    out->album_gain_tenth_db = 0;
    out->track_peak_q15 = 0;
    out->album_peak_q15 = 0;

    pos = offset + 4;
    end = offset + length;
    for (i = 0; i < count && pos < end; i++) {
        unsigned int len;
        unsigned char slab[80];
        long n;
        const char *val;
        long klen;

        if (read_u32_le(io, &len) != 0)
            return -1;
        pos += 4;
        if (pos + len > end)
            return -1;

        n = (len > 80) ? 80 : (long)len;
        if (cio_read(io, slab, n) != n)
            return -1;
        if (len > 80)
            cio_seek(io, pos + len);       /* skip oversized payload */
        pos += len;

        /* find '=' separating KEY from value */
        klen = 0;
        while (klen < n && slab[klen] != '=')
            klen++;
        if (klen == n)
            continue;                     /* not a key/value pair */
        val = (const char *)slab + klen + 1;

        if (key_eq_n((const char *)slab, klen, "REPLAYGAIN_TRACK_GAIN")) {
            parse_db10(val, &out->track_gain_tenth_db); found = 1;
        } else if (key_eq_n((const char *)slab, klen, "REPLAYGAIN_ALBUM_GAIN")) {
            parse_db10(val, &out->album_gain_tenth_db); found = 1;
        } else if (key_eq_n((const char *)slab, klen, "REPLAYGAIN_TRACK_PEAK")) {
            parse_peak_q15(val, &out->track_peak_q15); found = 1;
        } else if (key_eq_n((const char *)slab, klen, "REPLAYGAIN_ALBUM_PEAK")) {
            parse_peak_q15(val, &out->album_peak_q15); found = 1;
        }
    }
    return found ? 0 : -1;
}

/* ---- ID3v2 TXXX scanner ---- */

/* Scan ID3v2 (v2.3/v2.4) TXXX frames for REPLAYGAIN_* keys. Returns 0 if at
 * least one field was found, -1 otherwise. Only the ID3v2 tag region is
 * walked (bounded by the header's synchsafe size), not the audio body. */
int tags_replaygain_id3v2(codec_io_t *io, codec_replaygain_t *out)
{
    unsigned char hdr[10];
    unsigned int tag_size, tag_end;
    int ver, found = 0;
    unsigned int pos;

    if (io == 0 || out == 0)
        return -1;

    out->track_gain_tenth_db = 0;
    out->album_gain_tenth_db = 0;
    out->track_peak_q15 = 0;
    out->album_peak_q15 = 0;

    if (cio_seek(io, 0) != 0)
        return -1;
    if (cio_read(io, hdr, 10) != 10)
        return -1;
    if (hdr[0] != 'I' || hdr[1] != 'D' || hdr[2] != '3')
        return -1;
    ver = (hdr[3] == 2) ? 2 : (hdr[3] == 3 ? 3 : (hdr[3] == 4 ? 4 : 0));
    if (ver == 0)
        return -1;

    /* The ID3v2 header (bytes 6-9) is a synchsafe length; walk only the tag. */
    if (hdr[6] & 0x80 || hdr[7] & 0x80 || hdr[8] & 0x80 || hdr[9] & 0x80)
        return -1;                         /* malformed size */
    tag_end = 10 + synchsafe(&hdr[6]);
    tag_size = (unsigned int)cio_size(io);
    if (tag_end > tag_size)
        tag_end = tag_size;

    pos = 10;
    while (pos + 10 <= tag_end) {
        unsigned char fh[10];
        unsigned int fsize;
        long want;

        cio_seek(io, (long)pos);
        if (cio_read(io, fh, 10) != 10)
            break;
        if (fh[0] == 0)
            break;                          /* padding */

        fsize = (ver == 4) ? synchsafe(&fh[4]) : read_u32_be(&fh[4]);
        if (fh[0] == 'T' && fh[1] == 'X' && fh[2] == 'X' && fh[3] == 'X') {
            if (fsize == 0) { pos += 10; continue; }
            want = (long)(fsize - 1);       /* -1 for the encoding byte */
            if (want > 0) {
                unsigned char slab[96];
                long n = (want > 95) ? 95 : want;
                long klen;
                const char *val;

                cio_seek(io, (long)(pos + 10));
                if (cio_read(io, slab, n) != n)
                    return -1;
                /* slab[0] = encoding; description ends at first NUL */
                klen = 1;
                while (klen < n && slab[klen] != '\0')
                    klen++;
                val = (const char *)slab + klen + 1;

                if (key_eq((const char *)&slab[1], "REPLAYGAIN_TRACK_GAIN")) {
                    parse_db10(val, &out->track_gain_tenth_db); found = 1;
                } else if (key_eq((const char *)&slab[1], "REPLAYGAIN_ALBUM_GAIN")) {
                    parse_db10(val, &out->album_gain_tenth_db); found = 1;
                } else if (key_eq((const char *)&slab[1], "REPLAYGAIN_TRACK_PEAK")) {
                    parse_peak_q15(val, &out->track_peak_q15); found = 1;
                } else if (key_eq((const char *)&slab[1], "REPLAYGAIN_ALBUM_PEAK")) {
                    parse_peak_q15(val, &out->album_peak_q15); found = 1;
                }
            }
            pos += 10 + fsize;
            continue;
        }
        pos += 10 + fsize;
    }
    return found ? 0 : -1;
}
