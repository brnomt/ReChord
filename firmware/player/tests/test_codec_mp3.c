/*
 * test_codec_mp3.c — host test for codecs/mp3.c (minimp3 adapter).
 *
 * No encoder exists on the build host, so the fixture is synthesized
 * byte-exactly in C:
 *   - ID3v2.3 tag with two TXXX ReplayGain frames (parser e2e),
 *   - a Xing/Info "info frame" (frames/bytes/TOC flags + LAME extension
 *     with known encoder delay/padding),
 *   - N silent MPEG-1 Layer III frames (valid header + zero side info =
 *     zero spectrum), which minimp3 decodes to exact-zero PCM.
 *
 * Covered: ID3v2 skip, Xing duration (frames*1152/rate), LAME gapless
 * delay/padding (+528+1 adjustment), ReplayGain via codec_t.tags, full
 * decode count, seek accounting (TOC), CBR duration fallback (second file
 * without Xing), EOF, and non-MP3 rejection.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codec.h"
#include "codec_io.h"
#include "test_util.h"

extern const codec_t codec_mp3;

#define RATE        44100
#define SPF         1152
#define FRAME_BYTES 417          /* 128 kbps @ 44.1 kHz, no padding       */
#define NAUDIO      8            /* silent audio frames                    */

/* Expected LAME values (crafted): pack_delay=47, pack_pad=1152 ->
 * delay = 47 + 529 = 576, padding = 1152 - 529 = 623. */
#define EXP_DELAY  576
#define EXP_PAD    623

/* ---- byte-writer --------------------------------------------------------- */

typedef struct { unsigned char *b; long n; } bw_t;

static void put8(bw_t *w, unsigned v) { w->b[w->n++] = (unsigned char)v; }
static void put16be(bw_t *w, unsigned v)
{
    put8(w, v >> 8);
    put8(w, v);
}
static void put32be(bw_t *w, unsigned v)
{
    put8(w, v >> 24); put8(w, v >> 16); put8(w, v >> 8); put8(w, v);
}

/* ---- fixtures ------------------------------------------------------------ */

/* MPEG-1 Layer III, 128 kbps, 44.1 kHz, no CRC, stereo, no padding. */
static void put_mpeg_hdr(bw_t *w)
{
    put8(w, 0xFF); put8(w, 0xFB); put8(w, 0x90); put8(w, 0x00);
}

/* 32-byte side info of all zeroes: main_data_begin=0, every granule's
 * part2_3_length=0 -> no huffman/scalefactor bits, zero spectrum. */
static void put_silence_side(bw_t *w)
{
    int i;
    for (i = 0; i < 32; i++)
        put8(w, 0);
}

static long put_txxx(bw_t *w, const char *desc, const char *value)
{
    long start = w->n;
    long body = 1 + (long)strlen(desc) + 1 + (long)strlen(value) + 1;

    put8(w, 'T'); put8(w, 'X'); put8(w, 'X'); put8(w, 'X');
    put32be(w, (unsigned)body);
    put8(w, 0); put8(w, 0);            /* flags */
    put8(w, 0);                         /* encoding: ISO-8859-1 */
    memcpy(w->b + w->n, desc, strlen(desc) + 1); w->n += (long)strlen(desc) + 1;
    memcpy(w->b + w->n, value, strlen(value) + 1); w->n += (long)strlen(value) + 1;
    (void)start;
    return body + 10;
}

/* ID3v2.3 header (10 B) + two TXXX frames. Returns tag length. */
static long put_id3v2(bw_t *w)
{
    long body_start = w->n;
    long body;

    put8(w, 'I'); put8(w, 'D'); put8(w, '3');
    put8(w, 3); put8(w, 0); put8(w, 0);   /* v2.3.0, no flags */
    put8(w, 0); put8(w, 0); put8(w, 0); put8(w, 0); /* size placeholder */
    put_txxx(w, "REPLAYGAIN_TRACK_GAIN", "-4.0 dB");
    put_txxx(w, "REPLAYGAIN_TRACK_PEAK", "0.500000");
    body = w->n - (body_start + 10);
    w->b[body_start + 6] = (unsigned char)((body >> 21) & 0x7F);
    w->b[body_start + 7] = (unsigned char)((body >> 14) & 0x7F);
    w->b[body_start + 8] = (unsigned char)((body >> 7) & 0x7F);
    w->b[body_start + 9] = (unsigned char)(body & 0x7F);
    return w->n;
}

/* Xing info frame: frames/bytes/TOC flags, LAME extension w/ delay+pad. */
static void put_xing_frame(bw_t *w, unsigned frames, unsigned bytes)
{
    long start = w->n;
    int i;

    put_mpeg_hdr(w);
    put_silence_side(w);                 /* 32 B: keeps minimp3 happy     */
    /* Xing payload (starts right after side info: 4+32) */
    put8(w, 'X'); put8(w, 'i'); put8(w, 'n'); put8(w, 'g');
    put32be(w, 0x0000000Fu);             /* FRAMES|BYTES|TOC|QUALITY       */
    put32be(w, frames);
    put32be(w, bytes);
    /* TOC: byte offset of i% = i*256/100 (the codec multiplies back by
     * bytes/256 when seeking — the standard Xing TOC normalization). */
    for (i = 0; i < 100; i++)
        put8(w, (unsigned)(i * 256 / 100));
    put32be(w, 100);                     /* quality                        */
    /* LAME extension: encoder(9) + rev/method/lowpass(3) + peak(4) +
     * radio(2) + audiophile(2) + flags(1) = 21 bytes, then delay/pad. */
    memcpy(w->b + w->n, "LAME3.100", 9); w->n += 9;
    put8(w, 0); put8(w, 0); put8(w, 0);
    put32be(w, 0);                       /* peak: unknown                 */
    put16be(w, 0); put16be(w, 0);        /* radio/audiophile gain         */
    put8(w, 0);                          /* flags/ATH                     */
    /* delay/padding packed 12+12: delay-529=47 -> 0x02,0xF_ ; pad-529=1152 */
    put8(w, 0x02);
    put8(w, 0xF4);                       /* 47&0xF in high nibble, 4 in lo */
    put8(w, 0x80);                       /* 1152&0xFF = 0x80, hi 4 bits in
                                          * previous byte low nibble       */
    /* pad the frame to FRAME_BYTES with zeroes (bitrate table etc.) */
    while (w->n - start < FRAME_BYTES)
        put8(w, 0);
}

static void put_silent_frame(bw_t *w)
{
    long start = w->n;

    put_mpeg_hdr(w);
    put_silence_side(w);
    while (w->n - start < FRAME_BYTES)
        put8(w, 0);
}

/* ---- helpers -------------------------------------------------------------- */

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

/* Remove a fixture unless KEEP_FIXTURES is set (debug aid). */
static void rm_fix(const char *path)
{
    if (getenv("KEEP_FIXTURES") == NULL)
        remove(path);
}

/* Drive the codec to EOF, counting frames and (optionally) checking the
 * PCM is all-zero (our silence frames). Returns frames decoded. */
static long decode_all(const codec_t *c, void *st, int check_silent,
                       int *bad_samples)
{
    int16_t pcm[SPF * 2];
    long frames = 0;

    *bad_samples = 0;
    for (;;) {
        int got = c->decode(st, pcm, 512);   /* player-pump sized chunks */
        int i;

        if (got < 0) {
            printf("  decode error %d at frame %ld\n", got, frames);
            return -1;
        }
        if (got == 0)
            break;
        if (check_silent) {
            for (i = 0; i < got * 2; i++)
                if (pcm[i] != 0)
                    (*bad_samples)++;
        }
        frames += got;
    }
    return frames;
}

/* ---- tests ---------------------------------------------------------------- */

static void test_xing_file(void)
{
    static unsigned char buf[16384];
    bw_t w = { buf, 0 };
    char path[] = "/tmp/rechord_codec_mp3_xing.mp3";
    codec_format_t fmt;
    codec_gapless_t gap;
    codec_replaygain_t rg;
    void *st;
    long frames;
    int bad = 0;

    TEST("mp3: Xing + LAME + RG + silent frames");
    put_id3v2(&w);
    put_xing_frame(&w, NAUDIO, NAUDIO * FRAME_BYTES);
    {
        int i;
        for (i = 0; i < NAUDIO; i++)
            put_silent_frame(&w);
    }
    if (write_file(path, buf, w.n) != 0) {
        printf("  FAIL: cannot write fixture\n");
        failures++;
        return;
    }

    st = calloc(1, PLAYER_CODEC_STATE_MAX);
    if (!st) { printf("  FAIL: alloc\n"); failures++; return; }

    CHECK_EQI(codec_mp3.open(st, path), 0, "open: ID3v2+Xing file");

    memset(&fmt, 0, sizeof fmt);
    CHECK_EQI(codec_mp3.format(st, &fmt), 0, "format()");
    CHECK_EQI(fmt.sample_rate, RATE, "sample rate 44100");
    CHECK_EQI(fmt.channels, 2, "channels 2");
    CHECK_EQI(fmt.total_samples, NAUDIO * SPF,
              "total_samples = frames*spf (Xing exact)");
    CHECK_EQI(codec_mp3.duration_ms(st), (NAUDIO * SPF) * 1000 / RATE,
              "duration from Xing frame count");

    memset(&gap, 0, sizeof gap);
    CHECK_EQI(codec_mp3.gapless(st, &gap), 0, "gapless()");
    CHECK_EQI(gap.delay_samples, EXP_DELAY, "LAME delay 576 (+529 adjust)");
    CHECK_EQI(gap.padding_samples, EXP_PAD, "LAME padding 623 (-529 adjust)");

    memset(&rg, 0, sizeof rg);
    CHECK_EQI(codec_mp3.tags(st, &rg), 0, "tags()");
    CHECK_EQI(rg.track_gain_tenth_db, -40, "RG track gain -4.0 dB => -40");
    CHECK(rg.track_peak_q15 >= 16383 && rg.track_peak_q15 <= 16385,
          "RG track peak 0.5");

    frames = decode_all(&codec_mp3, st, 1, &bad);
    CHECK_EQI(frames, (long)NAUDIO * SPF,
              "decoded every PCM frame (8 audio mp3 frames)");
    CHECK_EQI(bad, 0, "info frame skipped; silent frames decode to zero");

    /* seek: TOC byte position, resume at the first frame boundary */
    CHECK_EQI(codec_mp3.seek(st, 100), 0, "seek(100 ms)");
    frames = decode_all(&codec_mp3, st, 1, &bad);
    CHECK(frames >= 4 * SPF && frames <= 6 * SPF,
          "seek resumes mid-track (remaining frames plausible)");

    codec_mp3.close(st);
    free(st);
    rm_fix(path);
}

/* A real 78.4 s file (3000 frames @ 44.1 kHz). The target's 32-bit `long`
 * wraps in samples*1000 here — the naive form reported a bogus few-seconds
 * length on Now Playing (BRNOMT-65) instead of the true 78367 ms. */
#define LONG_FRAMES 3000

static void test_xing_long_duration(void)
{
    static unsigned char buf[(LONG_FRAMES + 1) * FRAME_BYTES];
    bw_t w = { buf, 0 };
    char path[] = "/tmp/rechord_codec_mp3_long.mp3";
    codec_format_t fmt;
    long frames;
    void *st;
    int bad = 0;
    int i;

    TEST("mp3: 78 s Xing file reports its exact length");
    put_xing_frame(&w, LONG_FRAMES, (LONG_FRAMES + 1) * FRAME_BYTES);
    for (i = 0; i < LONG_FRAMES; i++)
        put_silent_frame(&w);
    if (write_file(path, buf, w.n) != 0) {
        printf("  FAIL: cannot write fixture\n");
        failures++;
        return;
    }

    st = calloc(1, PLAYER_CODEC_STATE_MAX);
    if (!st) { printf("  FAIL: alloc\n"); failures++; return; }

    CHECK_EQI(codec_mp3.open(st, path), 0, "open: long Xing file");
    memset(&fmt, 0, sizeof fmt);
    CHECK_EQI(codec_mp3.format(st, &fmt), 0, "format()");
    CHECK_EQI(fmt.total_samples, (long)LONG_FRAMES * SPF,
              "total_samples = frames*spf");
    CHECK_EQI(codec_mp3.duration_ms(st),
              (long)(((int64_t)LONG_FRAMES * SPF * 1000) / RATE),
              "duration = 78367 ms (past the 32-bit wrap point)");
    CHECK(codec_mp3.duration_ms(st) > 48000,
          "length is not the wrapped few-seconds value");

    frames = decode_all(&codec_mp3, st, 1, &bad);
    CHECK_EQI(frames, (long)LONG_FRAMES * SPF, "decoded the whole file");
    CHECK_EQI(bad, 0, "silent frames decode to zero");
    CHECK_EQI(codec_mp3.seek(st, 60000), 0, "seek(60 s) works");

    codec_mp3.close(st);
    free(st);
    rm_fix(path);
}

static void test_cbr_file(void)
{
    static unsigned char buf[8192];
    bw_t w = { buf, 0 };
    char path[] = "/tmp/rechord_codec_mp3_cbr.mp3";
    codec_format_t fmt;
    void *st;
    long frames;
    int bad = 0;
    int i;

    TEST("mp3: CBR duration fallback (no Xing)");
    for (i = 0; i < NAUDIO; i++)
        put_silent_frame(&w);
    if (write_file(path, buf, w.n) != 0) {
        printf("  FAIL: cannot write fixture\n");
        failures++;
        return;
    }

    st = calloc(1, PLAYER_CODEC_STATE_MAX);
    if (!st) { printf("  FAIL: alloc\n"); failures++; return; }

    CHECK_EQI(codec_mp3.open(st, path), 0, "open: bare CBR file");
    memset(&fmt, 0, sizeof fmt);
    codec_mp3.format(st, &fmt);
    CHECK_EQI(codec_mp3.duration_ms(st),
              (long)(NAUDIO * FRAME_BYTES) * 8 / 128,
              "duration = bytes*8/bitrate (128 kbps)");
    (void)fmt;

    frames = decode_all(&codec_mp3, st, 1, &bad);
    CHECK_EQI(frames, (long)NAUDIO * SPF, "decoded every PCM frame");
    CHECK_EQI(bad, 0, "CBR frames decode to zero");

    codec_mp3.close(st);
    free(st);
    rm_fix(path);
}

static void test_reject(void)
{
    static unsigned char junk[64];
    char path[] = "/tmp/rechord_codec_mp3_junk.bin";
    void *st;

    TEST("mp3: non-MP3 rejected");
    memset(junk, 0x55, sizeof junk);
    if (write_file(path, junk, (long)sizeof junk) != 0) {
        printf("  FAIL: cannot write fixture\n");
        failures++;
        return;
    }
    st = calloc(1, PLAYER_CODEC_STATE_MAX);
    if (!st) { printf("  FAIL: alloc\n"); failures++; return; }
    CHECK(codec_mp3.open(st, path) < 0, "junk file rejected");
    codec_mp3.close(st);
    free(st);
    rm_fix(path);
}

int main(void)
{
    test_xing_file();
    test_xing_long_duration();
    test_cbr_file();
    test_reject();
    if (failures == 0) {
        printf("test_codec_mp3: OK\n");
        return 0;
    }
    printf("test_codec_mp3: %d FAILURE(S)\n", failures);
    return failures;
}
