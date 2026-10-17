/*
 * test_tags.c — host tests for the container tag readers (tags.c).
 *
 * Builds synthetic ID3v2 v2.3/v2.4 and Vorbis comment blobs in temp files,
 * opens them through the stdio codec_io backend, and asserts the readers
 * recover both the text metadata (title/artist/album/genre/track) and the
 * REPLAYGAIN_* values.
 *
 * The Vorbis blobs follow the SPEC layout (vendor_length + vendor + count +
 * entries): the pre-2026-10-17 fixtures omitted the vendor field, which is
 * exactly the shape the reader wrongly assumed — so they kept a real bug
 * green (HANDOVER §Q). test_vorbis_no_vendor pins the other direction.
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

/* ---- ID3v2 byte helpers ---- */

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

/* Append any ID3v2 frame; `ver` picks the v2.4 synchsafe frame size. */
static long append_frame(unsigned char *out, long at, int ver, const char *id,
                         const unsigned char *body, long body_len)
{
    unsigned char *p = out + at;

    memcpy(p, id, 4);
    if (ver == 4)
        put_synchsafe(p + 4, (unsigned int)body_len);
    else
        put_u32_be(p + 4, (unsigned int)body_len);
    p[8] = 0;
    p[9] = 0;                          /* no frame modifiers */
    memcpy(p + 10, body, (size_t)body_len);
    return at + 10 + body_len;
}

/* Text-frame body: encoding byte + text bytes (no NUL, per spec). */
static long text_body(unsigned char *out, int enc, const char *text)
{
    long n = (long)strlen(text);

    out[0] = (unsigned char)enc;
    memcpy(out + 1, text, (size_t)n);
    return n + 1;
}

static void put_id3v2_header(unsigned char *blob, int ver)
{
    blob[0] = 'I'; blob[1] = 'D'; blob[2] = '3';
    blob[3] = (unsigned char)ver;
    blob[4] = 0; blob[5] = 0;          /* no revision / flags */
}

/* ---- ID3v2 tests ---- */

static int test_id3v2_txxx(void)
{
    unsigned char blob[512];
    long n;
    codec_io_t *io;
    codec_replaygain_t rg;
    int rc;

    /* ID3v2 header: "ID3" v3.0, no flags, then the TXXX frames. Reserve the
     * 10-byte header first so append_txxx writes at offset 10. */
    put_id3v2_header(blob, 3);
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

/* v2.3 and v2.4 differ only in the frame-size encoding here; both must
 * deliver the same text fields (and no RG flag when there are no TXXX). */
static int test_id3v2_text(int ver)
{
    unsigned char blob[512], body[128];
    char path[64];
    long n;
    codec_io_t *io;
    codec_meta_t meta;
    codec_replaygain_t rg;
    int rc;

    snprintf(path, sizeof path, "/tmp/rechord_tags_text_v%d.bin", ver);
    put_id3v2_header(blob, ver);
    n = 10;
    n = append_frame(blob, n, ver, "TIT2", body,
                     text_body(body, 0, "Blue Monday"));
    n = append_frame(blob, n, ver, "TPE1", body,
                     text_body(body, 0, "New Order"));
    n = append_frame(blob, n, ver, "TALB", body,
                     text_body(body, 0, "Power, Corruption & Lies"));
    n = append_frame(blob, n, ver, "TCON", body,
                     text_body(body, 0, "Synthpop"));
    n = append_frame(blob, n, ver, "TRCK", body,
                     text_body(body, 0, "3/8"));
    put_synchsafe(blob + 6, (unsigned int)(n - 10));

    if (write_blob(path, blob, n) != 0)
        return -1;

    io = cio_open(path);
    if (!io)
        return -1;
    rc = tags_read_id3v2(io, &meta, &rg);
    cio_close(io);

    CHECK_EQI(rc & TAG_HAVE_META, TAG_HAVE_META, "id3v2 text: meta flag set");
    CHECK_EQI(rc & TAG_HAVE_RG, 0, "id3v2 text: no RG frames -> no RG flag");
    CHECK(strcmp(meta.title, "Blue Monday") == 0, "id3v2 text: TIT2");
    CHECK(strcmp(meta.artist, "New Order") == 0, "id3v2 text: TPE1");
    CHECK(strcmp(meta.album, "Power, Corruption & Lies") == 0,
          "id3v2 text: TALB");
    CHECK(strcmp(meta.genre, "Synthpop") == 0, "id3v2 text: TCON");
    CHECK_EQI(meta.track, 3, "id3v2 text: TRCK \"3/8\" -> 3");
    return failures ? -1 : 0;
}

static int test_id3v2_utf16(void)
{
    unsigned char blob[256], body[80];
    long n;
    codec_io_t *io;
    codec_meta_t meta;
    int rc;

    /* UTF-16LE with BOM (encoding 1): the reader narrow-decodes it, so the
     * ASCII text survives instead of appearing as NUL-laced garbage. */
    put_id3v2_header(blob, 3);
    n = 10;
    body[0] = 1;                       /* encoding: UTF-16 with BOM */
    body[1] = 0xFF;
    body[2] = 0xFE;                    /* BOM = little endian */
    body[3] = 'S'; body[4] = 0;
    body[5] = 'a'; body[6] = 0;
    body[7] = 'm'; body[8] = 0;
    n = append_frame(blob, n, 3, "TIT2", body, 9);
    put_synchsafe(blob + 6, (unsigned int)(n - 10));

    if (write_blob("/tmp/rechord_tags_utf16.bin", blob, n) != 0)
        return -1;

    io = cio_open("/tmp/rechord_tags_utf16.bin");
    if (!io)
        return -1;
    rc = tags_read_id3v2(io, &meta, 0);
    cio_close(io);

    CHECK_EQI(rc & TAG_HAVE_META, TAG_HAVE_META, "id3v2 utf16: meta flag set");
    CHECK(strcmp(meta.title, "Sam") == 0, "id3v2 utf16: TIT2 narrow-decoded");
    return failures ? -1 : 0;
}

static int test_id3v2_text_truncation(void)
{
    unsigned char blob[512], body[300];
    long n;
    codec_io_t *io;
    codec_meta_t meta;
    int i;

    for (i = 0; i < 280; i++)
        body[i + 1] = 'x';
    body[0] = 0;                       /* encoding: Latin-1 */
    put_id3v2_header(blob, 3);
    n = 10;
    n = append_frame(blob, n, 3, "TIT2", body, 281);
    put_synchsafe(blob + 6, (unsigned int)(n - 10));

    if (write_blob("/tmp/rechord_tags_long.bin", blob, n) != 0)
        return -1;

    io = cio_open("/tmp/rechord_tags_long.bin");
    if (!io)
        return -1;
    (void)tags_read_id3v2(io, &meta, 0);
    cio_close(io);

    CHECK_EQI((long)strlen(meta.title), (long)sizeof meta.title - 1,
              "id3v2 text: 280-byte title truncated to the field capacity");
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

/* Spec block head: vendor_length + vendor string. Returns the new size. */
static long vorbis_vendor(unsigned char *out, const char *vendor)
{
    long len = (long)strlen(vendor);

    put_u32_le(out, (unsigned int)len);
    memcpy(out + 4, vendor, (size_t)len);
    return 4 + len;
}

static int test_vorbis_comments(void)
{
    unsigned char blob[512];
    long n;
    codec_io_t *io;
    codec_meta_t meta;
    codec_replaygain_t rg;
    int rc;

    n = vorbis_vendor(blob, "reference libFLAC 1.3.2 20170101");
    put_u32_le(blob + n, 9);                   /* count, right after vendor */
    n += 4;
    n = vorbis_entry(blob, n, "ENCODER=TestEncoder-1.0");
    n = vorbis_entry(blob, n, "TITLE=Blue Monday");
    n = vorbis_entry(blob, n, "ARTIST=New Order");
    n = vorbis_entry(blob, n, "ALBUM=Power, Corruption & Lies");
    n = vorbis_entry(blob, n, "GENRE=Synthpop");
    n = vorbis_entry(blob, n, "TRACKNUMBER=3/8");
    n = vorbis_entry(blob, n, "REPLAYGAIN_TRACK_GAIN=-6.0 dB");
    n = vorbis_entry(blob, n, "REPLAYGAIN_TRACK_PEAK=0.75");
    n = vorbis_entry(blob, n, "REPLAYGAIN_ALBUM_GAIN=-3.0 dB");
    /* (album peak omitted -> stays 0) */

    if (write_blob("/tmp/rechord_tags_vorbis.bin", blob, n) != 0)
        return -1;

    io = cio_open("/tmp/rechord_tags_vorbis.bin");
    if (!io)
        return -1;
    rc = tags_read_vorbis(io, 0, n, &meta, &rg);
    cio_close(io);

    CHECK_EQI(rc & TAG_HAVE_META, TAG_HAVE_META, "vorbis: meta flag set");
    CHECK_EQI(rc & TAG_HAVE_RG, TAG_HAVE_RG, "vorbis: RG flag set");
    CHECK(strcmp(meta.title, "Blue Monday") == 0, "vorbis: TITLE");
    CHECK(strcmp(meta.artist, "New Order") == 0, "vorbis: ARTIST");
    CHECK(strcmp(meta.album, "Power, Corruption & Lies") == 0,
          "vorbis: ALBUM");
    CHECK(strcmp(meta.genre, "Synthpop") == 0, "vorbis: GENRE");
    CHECK_EQI(meta.track, 3, "vorbis: TRACKNUMBER \"3/8\" -> 3");
    CHECK_EQI(rg.track_gain_tenth_db, -60, "vorbis: TRACK_GAIN -6.0 dB => -60");
    CHECK_EQI(rg.album_gain_tenth_db, -30, "vorbis: ALBUM_GAIN -3.0 dB => -30");
    CHECK(rg.track_peak_q15 >= 24575 && rg.track_peak_q15 <= 24577,
          "vorbis: TRACK_PEAK 0.75");
    CHECK_EQI(rg.album_peak_q15, 0, "vorbis: ALBUM_PEAK absent -> 0");
    return failures ? -1 : 0;
}

/* The pre-fix fixtures' shape (count first, no vendor): must NOT be parsed —
 * the count field is only valid after the vendor string. */
static int test_vorbis_no_vendor(void)
{
    unsigned char blob[128];
    long n;
    codec_io_t *io;
    codec_meta_t meta;
    codec_replaygain_t rg;
    int rc;

    n = 0;
    put_u32_le(blob + n, 1);                   /* vendor-less: "count" */
    n += 4;
    n = vorbis_entry(blob, n, "TITLE=No Vendor");

    if (write_blob("/tmp/rechord_tags_novendor.bin", blob, n) != 0)
        return -1;

    io = cio_open("/tmp/rechord_tags_novendor.bin");
    if (!io)
        return -1;
    rc = tags_read_vorbis(io, 0, n, &meta, &rg);
    cio_close(io);

    CHECK_EQI(rc, 0, "vorbis: vendor-less block parses nothing");
    CHECK_EQI(meta.title[0], 0, "vorbis: vendor-less TITLE not taken");
    return failures ? -1 : 0;
}

static int test_no_tags(void)
{
    unsigned char blob[16];
    long n;
    codec_io_t *io;
    codec_meta_t meta;
    codec_replaygain_t rg;
    int rc;

    /* A file with no ID3v2 tag (all zero padding) -> reader finds nothing,
     * outputs zeroed. */
    memset(blob, 0, sizeof(blob));
    n = 16;
    if (write_blob("/tmp/rechord_tags_none.bin", blob, n) != 0)
        return -1;
    io = cio_open("/tmp/rechord_tags_none.bin");
    if (!io)
        return -1;
    rc = tags_read_id3v2(io, &meta, &rg);
    CHECK_EQI(rc, 0, "id3v2: no tag -> nothing found");
    CHECK_EQI(rg.track_gain_tenth_db, 0, "id3v2: out zeroed when not found");
    CHECK_EQI(meta.title[0], 0, "id3v2: meta zeroed when not found");
    rc = tags_replaygain_id3v2(io, &rg);
    CHECK_EQI(rc, -1, "id3v2: replaygain wrapper still reports -1");
    cio_close(io);
    return failures ? -1 : 0;
}

int main(void)
{
    int failed = 0;
    printf("== tags: text metadata + ReplayGain readers ==\n");
    if (test_id3v2_txxx() != 0) failed++;
    if (test_id3v2_text(3) != 0) failed++;
    if (test_id3v2_text(4) != 0) failed++;
    if (test_id3v2_utf16() != 0) failed++;
    if (test_id3v2_text_truncation() != 0) failed++;
    if (test_vorbis_comments() != 0) failed++;
    if (test_vorbis_no_vendor() != 0) failed++;
    if (test_no_tags() != 0) failed++;
    return failed;
}
