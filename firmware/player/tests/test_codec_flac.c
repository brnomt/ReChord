/*
 * test_codec_flac.c — host test for codecs/flac.c (dr_flac adapter).
 *
 * No FLAC encoder exists on the build host, so the fixture is synthesized
 * byte-exactly in C using VERBATIM subframes (PCM stored raw): a valid
 * fLaC stream with STREAMINFO (44100/2ch/16-bit, 4096-sample single
 * block), a VORBIS_COMMENT block carrying ReplayGain, and one frame whose
 * samples follow a known formula. Decoding must be LOSSLESS: every sample
 * must equal the formula — the strongest golden a codec test can assert.
 *
 * Covered: metadata pre-scan (RG comments), exact duration from
 * STREAMINFO total, lossless decode, frame-accurate seek (dr_flac lands on
 * the exact PCM frame), EOF, gapless report (0/0), and non-FLAC rejection.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codec.h"
#include "codec_io.h"
#include "test_util.h"

extern const codec_t codec_flac;

#define RATE       44100
#define BLOCKSIZE  4096
#define TOTAL      BLOCKSIZE          /* one frame                          */

/* Deterministic sample formula (same family as the mock codec's). */
static int16_t gen_sample(int i, int ch)
{
    return (int16_t)(((i * 37 + ch * 101) % 20000) - 10000);
}

/* ---- byte-writer + checksums --------------------------------------------- */

typedef struct { unsigned char *b; long n; } bw_t;

static void put8(bw_t *w, unsigned v) { w->b[w->n++] = (unsigned char)v; }
static void put16be(bw_t *w, unsigned v) { put8(w, v >> 8); put8(w, v); }
static void put24be(bw_t *w, unsigned v) { put8(w, v >> 16); put8(w, v >> 8); put8(w, v); }
static void put32le(bw_t *w, unsigned v)
{
    put8(w, v); put8(w, v >> 8); put8(w, v >> 16); put8(w, v >> 24);
}

/* FLAC CRC-8: poly 0x07, init 0, no reflection (frame header). */
static unsigned crc8(const unsigned char *b, int n)
{
    unsigned c = 0;
    int i, k;

    for (i = 0; i < n; i++) {
        c ^= b[i];
        for (k = 0; k < 8; k++)
            c = (c & 0x80u) ? ((c << 1) ^ 0x07u) & 0xFFu
                            : (c << 1) & 0xFFu;
    }
    return c;
}

/* FLAC CRC-16: poly 0x8005, init 0, no reflection (whole frame). */
static unsigned crc16(const unsigned char *b, int n)
{
    unsigned c = 0;
    int i, k;

    for (i = 0; i < n; i++) {
        c ^= (unsigned)b[i] << 8;
        for (k = 0; k < 8; k++)
            c = (c & 0x8000u) ? ((c << 1) ^ 0x8005u) & 0xFFFFu
                              : (c << 1) & 0xFFFFu;
    }
    return c;
}

/* ---- fixture -------------------------------------------------------------- */

static void put_meta_hdr(bw_t *w, int last, int type, unsigned len)
{
    put8(w, (unsigned)((last ? 0x80 : 0) | type));
    put8(w, len >> 16);
    put8(w, len >> 8);
    put8(w, len);
}

static void put_streaminfo(bw_t *w, unsigned long total)
{
    /* 34 bytes */
    put16be(w, BLOCKSIZE);              /* min block size                  */
    put16be(w, BLOCKSIZE);              /* max block size                  */
    put24be(w, 0);                      /* min frame size (0 = unknown)    */
    put24be(w, 0);                      /* max frame size (0 = unknown)    */
    /* rate(20) | ch-1(3) | bps-1(5) | total(36) */
    put8(w, RATE >> 12);                                        /* 4      */
    put8(w, RATE >> 4);                                         /* 12     */
    put8(w, ((RATE & 0x0F) << 4) | (1 << 1) | (15 >> 4));       /* 16     */
    put8(w, ((15 & 0x0F) << 4) |
            (unsigned)(((unsigned long)total >> 32) & 0xF));
    put8(w, (total >> 24) & 0xFF);
    put8(w, (total >> 16) & 0xFF);
    put8(w, (total >> 8) & 0xFF);
    put8(w, total & 0xFF);
    {
        int i;
        for (i = 0; i < 16; i++)        /* md5: zeros = unknown/unchecked  */
            put8(w, 0);
    }
}

static void put_vorbis_comment(bw_t *w)
{
    static const char *entries[] = {
        "TITLE=Golden Sine",
        "ARTIST=ReChord Test",
        "ALBUM=Fixtures",
        "GENRE=Test",
        "TRACKNUMBER=7",
        "REPLAYGAIN_TRACK_GAIN=-6.0 dB",
        "REPLAYGAIN_TRACK_PEAK=0.500000",
    };
    static const char *vendor = "reference libFLAC 1.3.2 20170101";
    unsigned count = (unsigned)(sizeof entries / sizeof entries[0]);
    unsigned i, vlen = (unsigned)strlen(vendor);
    long len;

    /* Spec layout: vendor_length + vendor + count + entries (the count is
     * NOT the first u32 of the block body — HANDOVER §Q). */
    put32le(w, vlen);
    memcpy(w->b + w->n, vendor, (size_t)vlen);
    w->n += vlen;
    put32le(w, count);
    for (i = 0; i < count; i++) {
        len = (long)strlen(entries[i]);
        put32le(w, (unsigned)len);
        memcpy(w->b + w->n, entries[i], (size_t)len);
        w->n += len;
    }
}

/* One frame: fixed blocksize 4096 (code 0111 + 16-bit LE at header end),
 * 44100 (code 1010), stereo independent (0001), 16 bps (100), frame #0,
 * VERBATIM subframes (raw PCM, MSB-first two's complement). Byte-aligned
 * throughout, so the writer emits plain bytes. */
static void put_verbatim_frame(bw_t *w)
{
    long start = w->n;
    unsigned crc;
    int ch, i;

    put8(w, 0xFF);                      /* sync (14) + reserved(0)+block(0) */
    put8(w, 0xF8);
    put8(w, 0x79);                      /* blocksize 0111, rate 1001=44100 */
    put8(w, 0x18);                      /* ch 0001, bps 100, reserved 0     */
    put8(w, 0x00);                      /* coded frame number (UTF-8) FIRST */
    put8(w, 0x0F); put8(w, 0xFF);       /* then blocksize-1 = 4095, BITSTREAM
                                         * order (MSB-first 16-bit read)   */
    put8(w, crc8(w->b + start, 7));
    for (ch = 0; ch < 2; ch++) {
        /* subframe header: pad(1)=0, type(6)=000001 VERBATIM, wasted(1)=0
         * (dr_flac: type = (hdr & 0x7E) >> 1; 1 == VERBATIM) */
        put8(w, 0x02);
        for (i = 0; i < TOTAL; i++)
            put16be(w, (unsigned)(uint16_t)gen_sample(i, ch));
    }
    crc = crc16(w->b + start, (int)(w->n - start));
    put16be(w, crc);
}

static int write_file(const char *path, const unsigned char *b, long n)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return -1;
    if (fwrite(b, 1, (size_t)n, f) != (size_t)n) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

/* ---- tests ---------------------------------------------------------------- */

static void test_flac_golden(void)
{
    static unsigned char buf[32768];
    bw_t w = { buf, 0 };
    char path[] = "/tmp/rechord_codec_flac.flac";
    codec_format_t fmt;
    codec_gapless_t gap;
    codec_replaygain_t rg;
    codec_meta_t meta;
    void *st;
    int16_t pcm[TOTAL * 2];
    long got, total = 0, i;
    int mismatches = 0;

    TEST("flac: lossless golden + RG + seek");

    put8(&w, 'f'); put8(&w, 'L'); put8(&w, 'a'); put8(&w, 'C');
    put_meta_hdr(&w, 0, 0, 34);        /* STREAMINFO (not last)            */
    put_streaminfo(&w, TOTAL);
    {
        long before = w.n;
        long clen;

        put_meta_hdr(&w, 1, 4, 0);     /* VORBIS_COMMENT, placeholder len  */
        put_vorbis_comment(&w);
        clen = w.n - before - 4;
        buf[before + 1] = (unsigned char)(clen >> 16);
        buf[before + 2] = (unsigned char)(clen >> 8);
        buf[before + 3] = (unsigned char)clen;
    }
    put_verbatim_frame(&w);

    if (write_file(path, buf, w.n) != 0) {
        printf("  FAIL: cannot write fixture\n");
        failures++;
        return;
    }

    st = calloc(1, PLAYER_CODEC_STATE_MAX);
    if (!st) { printf("  FAIL: alloc\n"); failures++; return; }

    CHECK_EQI(codec_flac.open(st, path), 0, "open: synthesized FLAC");

    memset(&fmt, 0, sizeof fmt);
    CHECK_EQI(codec_flac.format(st, &fmt), 0, "format()");
    CHECK_EQI(fmt.sample_rate, RATE, "sample rate 44100");
    CHECK_EQI(fmt.channels, 2, "channels 2");
    CHECK_EQI(fmt.total_samples, TOTAL, "total_samples from STREAMINFO");
    CHECK_EQI(codec_flac.duration_ms(st), TOTAL * 1000 / RATE,
              "duration = total*1000/rate");

    memset(&gap, 0, sizeof gap);
    CHECK_EQI(codec_flac.gapless(st, &gap), 0, "gapless()");
    CHECK_EQI(gap.delay_samples, 0, "FLAC: no encoder delay");
    CHECK_EQI(gap.padding_samples, 0, "FLAC: no encoder padding");

    memset(&rg, 0, sizeof rg);
    CHECK_EQI(codec_flac.tags(st, &rg), 0, "tags() via metadata pre-scan");
    CHECK_EQI(rg.track_gain_tenth_db, -60, "RG track gain -6.0 dB => -60");
    CHECK(rg.track_peak_q15 >= 16383 && rg.track_peak_q15 <= 16385,
          "RG track peak 0.5");

    /* Text metadata through the same pre-scan pass (spec Vorbis layout). */
    memset(&meta, 0, sizeof meta);
    CHECK_EQI(codec_flac.meta(st, &meta), 0, "meta()");
    CHECK(strcmp(meta.title, "Golden Sine") == 0, "meta: TITLE");
    CHECK(strcmp(meta.artist, "ReChord Test") == 0, "meta: ARTIST");
    CHECK(strcmp(meta.album, "Fixtures") == 0, "meta: ALBUM");
    CHECK(strcmp(meta.genre, "Test") == 0, "meta: GENRE");
    CHECK_EQI(meta.track, 7, "meta: TRACKNUMBER");

    /* Lossless decode: every sample must equal the generator. Each decode
     * call fills pcm from offset 0; `total` tracks the global frame index. */
    while ((got = codec_flac.decode(st, pcm, 512)) > 0) {
        for (i = 0; i < got; i++) {
            int ch;
            for (ch = 0; ch < 2; ch++) {
                int want = gen_sample((int)(total + i), ch);
                if (pcm[i * 2 + ch] != want) {
                    if (mismatches == 0)
                        printf("  DBG first mismatch: frame=%ld ch=%d "
                               "got=%d want=%d\n",
                               total + i, ch, pcm[i * 2 + ch], want);
                    mismatches++;
                }
            }
        }
        total += got;
    }
    CHECK_EQI(got, 0, "decode reaches EOF cleanly");
    CHECK_EQI(total, TOTAL, "decoded every sample of the block");
    CHECK_EQI(mismatches, 0, "lossless: samples match the generator exactly");

    /* Frame-accurate seek: land exactly at PCM frame 2028 (46 ms). */
    CHECK_EQI(codec_flac.seek(st, 46), 0, "seek(46 ms) -> frame 2028");
    got = codec_flac.decode(st, pcm, 16);
    CHECK_EQI(got, 16, "post-seek decode works");
    if (got == 16) {
        int want = gen_sample(2028, 0);
        CHECK_EQI(pcm[0], want, "post-seek first sample = frame 2028");
    }

    codec_flac.close(st);
    free(st);
    remove(path);
}

/* A real >48 s file's header: 5,000,000 frames @ 44.1 kHz = 113.4 s. The
 * target's 32-bit `long` wraps in samples*1000 past ~48 s — that is how Now
 * Playing showed bogus few-seconds lengths (BRNOMT-65), so the exact
 * 113378 ms here is the regression guard. The body deliberately stays one
 * block: the reported length comes from STREAMINFO, not from the body. */
#define LONG_TOTAL 5000000UL

static void test_flac_long_duration(void)
{
    static unsigned char buf[32768];
    bw_t w = { buf, 0 };
    char path[] = "/tmp/rechord_codec_flac_long.flac";
    codec_format_t fmt;
    void *st;

    TEST("flac: 113 s STREAMINFO total reports its exact duration");

    put8(&w, 'f'); put8(&w, 'L'); put8(&w, 'a'); put8(&w, 'C');
    put_meta_hdr(&w, 1, 0, 34);        /* STREAMINFO (last)                */
    put_streaminfo(&w, LONG_TOTAL);
    put_verbatim_frame(&w);

    if (write_file(path, buf, w.n) != 0) {
        printf("  FAIL: cannot write fixture\n");
        failures++;
        return;
    }

    st = calloc(1, PLAYER_CODEC_STATE_MAX);
    if (!st) { printf("  FAIL: alloc\n"); failures++; return; }

    CHECK_EQI(codec_flac.open(st, path), 0, "open: long-total FLAC");
    memset(&fmt, 0, sizeof fmt);
    CHECK_EQI(codec_flac.format(st, &fmt), 0, "format()");
    CHECK_EQI(fmt.sample_rate, RATE, "sample rate 44100");
    CHECK_EQI(fmt.total_samples, (long)LONG_TOTAL,
              "total_samples from STREAMINFO");
    CHECK_EQI(codec_flac.duration_ms(st),
              (long)(((int64_t)LONG_TOTAL * 1000) / RATE),
              "duration = 113378 ms (past the 32-bit wrap point)");
    CHECK(codec_flac.duration_ms(st) > 48000,
          "length is not the wrapped few-seconds value");

    codec_flac.close(st);
    free(st);
    remove(path);
}

static void test_flac_reject(void)
{
    static unsigned char junk[64];
    char path[] = "/tmp/rechord_codec_flac_junk.bin";
    void *st;

    TEST("flac: non-FLAC rejected");
    memcpy(junk, "RIFF", 4);
    memset(junk + 4, 0x55, sizeof junk - 4);
    if (write_file(path, junk, (long)sizeof junk) != 0) {
        printf("  FAIL: cannot write fixture\n");
        failures++;
        return;
    }
    st = calloc(1, PLAYER_CODEC_STATE_MAX);
    if (!st) { printf("  FAIL: alloc\n"); failures++; return; }
    CHECK(codec_flac.open(st, path) < 0, "junk file rejected");
    codec_flac.close(st);
    free(st);
    remove(path);
}

int main(void)
{
    test_flac_golden();
    test_flac_long_duration();
    test_flac_reject();
    if (failures == 0) {
        printf("test_codec_flac: OK\n");
        return 0;
    }
    printf("test_codec_flac: %d FAILURE(S)\n", failures);
    return failures;
}
