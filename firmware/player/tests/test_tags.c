/*
 * test_tags.c — host tests for the ReplayGain tag parsers (tags.c).
 *
 * Builds synthetic ID3v2 v2.3 (TXXX) and Vorbis comment blobs in temp files,
 * opens them through the stdio codec_io backend, and asserts the parsers
 * recover the REPLAYGAIN_* fields.
 */
#include "codec_io.h"
#include "codec.h"
#include "tags.h"
#include "test_util.h"

#include <stdio.h>
#include <string.h>

static int write_blob(const char *path, const unsigned char *buf, long n)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return -1;
    long w = (long)fwrite(buf, 1, (size_t)n, f);
    fclose(f);
    return (w == n) ? 0 : -1;
}

/* ---- ID3v2 v2.3 (big-endian frame sizes) ---- */

static void put_u32_be(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v >> 24);
    p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >> 8);
    p[3] = (unsigned char)(v);
}

/* Encode a 28-bit synchsafe integer (4 bytes, MSB of each clear). */
static void put_synchsafe(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)((v >> 21) & 0x7F);
    p[1] = (unsigned char)((v >> 14) & 0x7F);
    p[2] = (unsigned char)((v >> 7) & 0x7F);
    p[3] = (unsigned char)(v & 0x7F);
}

/* Append a TXXX frame to `out` (grows the buffer); returns new size. */
static long append_txxx(unsigned char *out, long at,
                        const char *desc, const char *value)
{
    long body = 1 + (long)strlen(desc) + 1 + (long)strlen(value) + 1; /* enc + desc\0 + value + NUL */
    long frame_size = body;           /* body excludes the 10-byte frame header */
    long total = 10 + frame_size;
    unsigned char *p = out + at;

    memcpy(p, "TXXX", 4);
    put_u32_be(p + 4, (unsigned int)frame_size);
    p[8] = 0; p[9] = 0;               /* flags */
    p[10] = 0;                         /* encoding (Latin-1 / UTF-8, no BOM) */
    memcpy(p + 11, desc, strlen(desc) + 1);
    memcpy(p + 11 + strlen(desc) + 1, value, strlen(value) + 1);
    return at + total;
}

static int test_id3v2_txxx(void)
{
    unsigned char blob[512];
    long n;
    codec_io_t *io;
    codec_replaygain_t rg;
    int rc;

    /* ID3v2 header: "ID3" v3.0, no flags, then the TXXX frames. Reserve the
     * 10-byte header first so append_txxx writes at offset 10. */
    blob[0] = 'I'; blob[1] = 'D'; blob[2] = '3';
    blob[3] = 3; blob[4] = 0; blob[5] = 0;
    n = 10;
    n = append_txxx(blob, n, "REPLAYGAIN_TRACK_GAIN", "-3.0 dB");
    n = append_txxx(blob, n, "REPLAYGAIN_TRACK_PEAK", "0.5");
    n = append_txxx(blob, n, "REPLAYGAIN_ALBUM_GAIN", "-1.5 dB");
    n = append_txxx(blob, n, "REPLAYGAIN_ALBUM_PEAK", "0.25");
    put_synchsafe(blob + 6, (unsigned int)(n - 10));   /* tag payload size */

    if (write_blob("/tmp/rechord_tags_id3.bin", blob, n) != 0)
        return -1;

    io = cio_open("/tmp/rechord_tags_id3.bin");
    if (!io)
        return -1;
    rc = tags_replaygain_id3v2(io, &rg);
    cio_close(io);

    CHECK_EQI(rc, 0, "id3v2: at least one tag found");
    CHECK_EQI(rg.track_gain_tenth_db, -30, "id3v2: TRACK_GAIN -3.0 dB => -30");
    CHECK_EQI(rg.album_gain_tenth_db, -15, "id3v2: ALBUM_GAIN -1.5 dB => -15");
    /* 0.5 of full scale -> ~16383/16384 in Q1.15 */
    CHECK(rg.track_peak_q15 >= 16383 && rg.track_peak_q15 <= 16385,
          "id3v2: TRACK_PEAK 0.5");
    CHECK(rg.album_peak_q15 >= 8190 && rg.album_peak_q15 <= 8193,
          "id3v2: ALBUM_PEAK 0.25");
    return failures ? -1 : 0;
}

/* ---- Vorbis comment block ---- */

static void put_u32_le(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v);
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

static long vorbis_entry(unsigned char *out, long at, const char *entry)
{
    long len = (long)strlen(entry);
    put_u32_le(out + at, (unsigned int)len);
    memcpy(out + at + 4, entry, (size_t)len);
    return at + 4 + len;
}

static int test_vorbis_comments(void)
{
    unsigned char blob[128];
    long n;
    codec_io_t *io;
    codec_replaygain_t rg;
    int rc;

    n = 0;
    put_u32_le(blob + n, 4);                    /* count */
    n += 4;
    n = vorbis_entry(blob, n, "ENCODER=TestEncoder-1.0");
    n = vorbis_entry(blob, n, "REPLAYGAIN_TRACK_GAIN=-6.0 dB");
    n = vorbis_entry(blob, n, "REPLAYGAIN_TRACK_PEAK=0.75");
    n = vorbis_entry(blob, n, "REPLAYGAIN_ALBUM_GAIN=-3.0 dB");
    /* (album peak omitted -> stays 0) */

    if (write_blob("/tmp/rechord_tags_vorbis.bin", blob, n) != 0)
        return -1;

    io = cio_open("/tmp/rechord_tags_vorbis.bin");
    if (!io)
        return -1;
    rc = tags_replaygain_vorbis(io, 0, n, &rg);
    cio_close(io);

    CHECK_EQI(rc, 0, "vorbis: at least one tag found");
    CHECK_EQI(rg.track_gain_tenth_db, -60, "vorbis: TRACK_GAIN -6.0 dB => -60");
    CHECK_EQI(rg.album_gain_tenth_db, -30, "vorbis: ALBUM_GAIN -3.0 dB => -30");
    CHECK(rg.track_peak_q15 >= 24575 && rg.track_peak_q15 <= 24577,
          "vorbis: TRACK_PEAK 0.75");
    CHECK_EQI(rg.album_peak_q15, 0, "vorbis: ALBUM_PEAK absent -> 0");
    return failures ? -1 : 0;
}

static int test_no_tags(void)
{
    unsigned char blob[16];
    long n;
    codec_io_t *io;
    codec_replaygain_t rg;
    int rc;

    /* A file with no ID3v2 tag (all zero padding) -> parser returns -1,
     * out zeroed. */
    memset(blob, 0, sizeof(blob));
    n = 16;
    if (write_blob("/tmp/rechord_tags_none.bin", blob, n) != 0)
        return -1;
    io = cio_open("/tmp/rechord_tags_none.bin");
    if (!io)
        return -1;
    rc = tags_replaygain_id3v2(io, &rg);
    cio_close(io);
    CHECK_EQI(rc, -1, "id3v2: no tag -> -1");
    CHECK_EQI(rg.track_gain_tenth_db, 0, "id3v2: out zeroed when not found");
    return failures ? -1 : 0;
}

int main(void)
{
    int failed = 0;
    printf("== tags: ReplayGain parsers ==\n");
    if (test_id3v2_txxx() != 0) failed++;
    if (test_vorbis_comments() != 0) failed++;
    if (test_no_tags() != 0) failed++;
    return failed;
}
