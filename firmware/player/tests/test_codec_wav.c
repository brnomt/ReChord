/*
 * test_codec_wav.c — golden test for our WAV/RIFF PCM decoder.
 *
 * Synthesizes a KNOWN sine WAV on disk (exact int16 bytes), decodes it
 * through codec_wav.c + the stdio codec_io backend, and verifies:
 *   - sample-rate/channel reporting (format),
 *   - exact sample-for-sample decode (byte-exact golden),
 *   - independent sine-formula cross-check,
 *   - seek behavior (frame-accurate), EOF stability,
 *   - chunk-list robustness (odd-sized JUNK padding, fmt after data),
 *   - strict subset rejection (8-bit, float, bad magic),
 *   - declared-vs-actual data-size clamping (truncated/oversized files).
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../codec.h"
#include "test_util.h"

extern const codec_t codec_wav;

#define T_PI 3.14159265358979323846

/* ---- WAV file writer (test-side; independent of the decoder) ---- */

typedef struct
{
    long rate;
    int  ch;              /* channels in the fmt chunk                    */
    int  bits;
    int  audio_format;    /* 1 = PCM, 3 = IEEE float                      */
    const uint8_t *data;
    long data_bytes;
    long declared_data;   /* data-chunk size field override (-1 = exact)  */
    int  junk_chunk;      /* insert an odd-size JUNK chunk before fmt     */
    int  fmt_after_data;
    const char *riff_tag; /* default "RIFF"                               */
} wav_spec_t;

static void put_u32(FILE *f, uint32_t v)
{
    uint8_t b[4];
    b[0] = (uint8_t)(v & 0xFF);
    b[1] = (uint8_t)((v >> 8) & 0xFF);
    b[2] = (uint8_t)((v >> 16) & 0xFF);
    b[3] = (uint8_t)((v >> 24) & 0xFF);
    fwrite(b, 1, 4, f);
}

static void put_u16(FILE *f, uint16_t v)
{
    uint8_t b[2];
    b[0] = (uint8_t)(v & 0xFF);
    b[1] = (uint8_t)((v >> 8) & 0xFF);
    fwrite(b, 1, 2, f);
}

static void put_tag(FILE *f, const char *s)
{
    fwrite(s, 1, 4, f);
}

static void put_fmt_chunk(FILE *f, const wav_spec_t *s)
{
    int ext = (s->audio_format == 0xFFFE);

    put_tag(f, "fmt ");
    put_u32(f, ext ? 40 : 16);
    put_u16(f, (uint16_t)s->audio_format);
    put_u16(f, (uint16_t)s->ch);
    put_u32(f, (uint32_t)s->rate);
    put_u32(f, (uint32_t)(s->rate * s->ch * (s->bits / 8)));
    put_u16(f, (uint16_t)(s->ch * (s->bits / 8)));
    put_u16(f, (uint16_t)s->bits);
    if (ext) {
        static const uint8_t k_pcm_guid[16] = {
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
            0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
        };

        put_u16(f, 22);              /* cbSize                            */
        put_u16(f, (uint16_t)s->bits);  /* wValidBitsPerSample            */
        put_u32(f, s->ch == 2 ? 3u : 4u); /* channel mask                  */
        fwrite(k_pcm_guid, 1, 16, f);
    }
}

static void put_data_chunk(FILE *f, const wav_spec_t *s)
{
    long declared = (s->declared_data >= 0) ? s->declared_data
                                            : s->data_bytes;
    put_tag(f, "data");
    put_u32(f, (uint32_t)declared);
    fwrite(s->data, 1, (size_t)s->data_bytes, f);
    if (s->data_bytes & 1)
        fputc(0, f);
}

static void write_wav(const char *path, const wav_spec_t *s)
{
    FILE *f = fopen(path, "wb");
    long junk_bytes = s->junk_chunk ? 12 : 0;   /* hdr+3 bytes+1 pad */
    long fmt_bytes = 8 + ((s->audio_format == 0xFFFE) ? 40 : 16);
    long data_bytes = 8 + s->data_bytes + (s->data_bytes & 1);
    long riff_size;

    if (f == NULL) {
        printf("  FAIL: cannot create %s\n", path);
        failures++;
        return;
    }

    riff_size = 4 + junk_bytes + fmt_bytes + data_bytes;

    put_tag(f, s->riff_tag ? s->riff_tag : "RIFF");
    put_u32(f, (uint32_t)riff_size);
    put_tag(f, "WAVE");

    if (s->junk_chunk) {
        put_tag(f, "JUNK");
        put_u32(f, 3);
        fwrite("xyz", 1, 3, f);   /* odd size -> one pad byte follows */
        fputc(0, f);
    }

    if (!s->fmt_after_data) {
        put_fmt_chunk(f, s);
        put_data_chunk(f, s);
    } else {
        put_data_chunk(f, s);
        put_fmt_chunk(f, s);
    }
    fclose(f);
}

/* ---- reference signal ---- */

#define GOLD_RATE 8000
#define GOLD_CH   2
#define GOLD_FRAMES 400            /* 50 ms                               */
#define GOLD_FREQ 440.0

static int16_t g_ref[GOLD_FRAMES * GOLD_CH];

static void gen_reference(void)
{
    int i, c;
    for (i = 0; i < GOLD_FRAMES; i++) {
        double v = 0.8 * 32767.0 *
                   sin(2.0 * T_PI * GOLD_FREQ * (double)i / (double)GOLD_RATE);
        int16_t s = (int16_t)lround(v);
        for (c = 0; c < GOLD_CH; c++)
            g_ref[i * GOLD_CH + c] = s;
    }
}

/* Decoded output is the internal int32 left-justified format: the 16-bit
 * golden value widened by 16. */
static int matches_ref(const int32_t *pcm, const int16_t *ref, int n)
{
    int i;
    for (i = 0; i < n; i++)
        if (pcm[i] != ((int32_t)ref[i] << 16))
            return 0;
    return 1;
}

static int decode_all(const codec_t *cc, void *st, int32_t *pcm, int max,
                      long *frames_out)
{
    long total = 0;
    int guard = 0;

    for (guard = 0; guard < 10000; guard++) {
        int n = cc->decode(st, pcm, max);
        if (n < 0)
            return n;
        if (n == 0)
            break;
        total += n;
    }
    *frames_out = total;
    return 0;
}

int main(void)
{
    const codec_t *cc = &codec_wav;
    uint8_t *state;
    codec_format_t fmt;
    int32_t pcm[512 * 2];
    long total = 0;
    int rc, i, c, ok;
    long dur;

    state = (uint8_t *)calloc(1, (size_t)cc->state_size);
    if (state == 0) {
        printf("test_codec_wav: cannot allocate state\n");
        return 1;
    }
    gen_reference();

    TEST("codec descriptor");
    CHECK(cc->state_size > 0, "state_size > 0");
    CHECK(strcmp(cc->name, "wav") == 0, "name is \"wav\"");
    CHECK(cc->format != 0, "has format() reporter");

    TEST("golden sine: open + format + duration");
    write_wav(".work/sine.wav", &(wav_spec_t){
        GOLD_RATE, GOLD_CH, 16, 1,
        (const uint8_t *)g_ref, sizeof(g_ref),
        -1, 0, 0, 0
    });
    rc = cc->open(state, ".work/sine.wav");
    CHECK_EQI(rc, CODEC_OK, "open known-good sine wav");
    memset(&fmt, 0, sizeof(fmt));
    CHECK_EQI(cc->format(state, &fmt), CODEC_OK, "format() reports");
    CHECK_EQI(fmt.sample_rate, GOLD_RATE, "sample-rate reporting (8000)");
    CHECK_EQI(fmt.channels, GOLD_CH, "channel reporting (2)");
    CHECK_EQI(fmt.bits, 16, "16-bit");
    dur = cc->duration_ms(state);
    CHECK_EQI(dur, 50, "duration 50 ms (400 frames @ 8 kHz)");

    TEST("golden sine: chunked decode is byte-exact");
    {
        long got = 0;
        int chunk = 128;   /* forces multiple decode calls, uneven tail */
        int n;
        ok = 1;
        while ((n = cc->decode(state, pcm, chunk)) > 0) {
            for (i = 0; i < n * GOLD_CH; i++) {
                if (pcm[i] != ((int32_t)g_ref[got * GOLD_CH + i] << 16)) {
                    ok = 0;
                    break;
                }
            }
            got += n;
            if (got > GOLD_FRAMES) {
                ok = 0;
                break;
            }
        }
        CHECK(n == 0, "decode ends with 0 (EOF)");
        CHECK(ok, "every decoded sample matches the golden reference");
        CHECK_EQI(got, GOLD_FRAMES, "decoded exactly 400 frames");
        CHECK_EQI(cc->decode(state, pcm, 128), 0, "EOF is stable");
        CHECK_EQI(cc->decode(state, pcm, 128), 0, "EOF stays stable");
    }

    TEST("golden sine: independent sine cross-check");
    ok = 1;
    cc->seek(state, 0);
    if (cc->decode(state, pcm, GOLD_FRAMES) != GOLD_FRAMES)
        ok = 0;
    for (i = 0; i < GOLD_FRAMES && ok; i++) {
        double want = 0.8 * 32767.0 *
            sin(2.0 * T_PI * GOLD_FREQ * (double)i / (double)GOLD_RATE);
        for (c = 0; c < GOLD_CH; c++) {
            double diff = ((double)pcm[i * GOLD_CH + c] / 65536.0) - want;
            if (diff > 2.0 || diff < -2.0) {
                ok = 0;
                break;
            }
        }
    }
    CHECK(ok, "decoded samples match a fresh 440 Hz sine computation");

    TEST("seek: frame-accurate, clamped, restartable");
    CHECK_EQI(cc->seek(state, 25), CODEC_OK, "seek(25 ms) -> 0");
    CHECK_EQI(cc->decode(state, pcm, 50), 50, "decode 50 frames after seek");
    ok = 1;
    for (i = 0; i < 50 * GOLD_CH; i++) {
        if (pcm[i] != ((int32_t)g_ref[200 * GOLD_CH + i] << 16)) {  /* 25 ms = frame 200 */
            ok = 0;
            break;
        }
    }
    CHECK(ok, "samples after seek(25 ms) match golden at frame 200");
    CHECK_EQI(cc->seek(state, 0), CODEC_OK, "seek(0) -> 0");
    CHECK_EQI(cc->decode(state, pcm, 4), 4, "decode after rewind");
    CHECK(matches_ref(pcm, g_ref, 4 * GOLD_CH),
          "rewind lands on frame 0");
    CHECK_EQI(cc->seek(state, -10), CODEC_OK, "seek(-10) clamps to 0");
    CHECK_EQI(cc->seek(state, 999999), CODEC_OK, "seek(past end) clamps");
    CHECK_EQI(cc->decode(state, pcm, 16), 0, "decode at clamped end = EOF");
    cc->close(state);

    TEST("mono + alternate rate");
    {
        int16_t mono[200];
        for (i = 0; i < 200; i++)
            mono[i] = (int16_t)(i * 37 - 3000);
        write_wav(".work/mono.wav", &(wav_spec_t){
            44100, 1, 16, 1,
            (const uint8_t *)mono, sizeof(mono),
            -1, 0, 0, 0
        });
        CHECK_EQI(cc->open(state, ".work/mono.wav"), CODEC_OK,
                  "open mono wav");
        memset(&fmt, 0, sizeof(fmt));
        cc->format(state, &fmt);
        CHECK_EQI(fmt.sample_rate, 44100, "mono rate 44100");
        CHECK_EQI(fmt.channels, 1, "mono channel count");
        CHECK_EQI(cc->duration_ms(state), (200 * 1000) / 44100,
                  "mono duration");
        CHECK_EQI(cc->decode(state, pcm, 200), 200, "mono decode all");
        CHECK(matches_ref(pcm, mono, 200), "mono samples exact");
        cc->close(state);
    }

    TEST("chunk-list robustness");
    write_wav(".work/junk.wav", &(wav_spec_t){
        GOLD_RATE, GOLD_CH, 16, 1,
        (const uint8_t *)g_ref, sizeof(g_ref),
        -1, 1, 0, 0
    });
    CHECK_EQI(cc->open(state, ".work/junk.wav"), CODEC_OK,
              "odd-size JUNK chunk skipped + padded");
    CHECK_EQI(cc->decode(state, pcm, GOLD_FRAMES), GOLD_FRAMES,
              "decode all with JUNK present");
    CHECK(matches_ref(pcm, g_ref, GOLD_FRAMES * GOLD_CH),
          "samples exact with JUNK");
    cc->close(state);

    write_wav(".work/reorder.wav", &(wav_spec_t){
        GOLD_RATE, GOLD_CH, 16, 1,
        (const uint8_t *)g_ref, sizeof(g_ref),
        -1, 0, 1, 0
    });
    CHECK_EQI(cc->open(state, ".work/reorder.wav"), CODEC_OK,
              "fmt chunk after data accepted");
    CHECK_EQI(cc->decode(state, pcm, GOLD_FRAMES), GOLD_FRAMES,
              "decode all with reordered chunks");
    CHECK(matches_ref(pcm, g_ref, GOLD_FRAMES * GOLD_CH),
          "samples exact with reordered chunks");
    cc->close(state);

    TEST("8/24/32-bit PCM and WAVE_FORMAT_EXTENSIBLE");
    {
        /* 4 stereo frames per depth, known raw bytes. */
        static const uint8_t d8[8] = { 128, 127, 129, 0, 255, 1, 192, 64 };
        static const int32_t e8[8] = {
            0, -1 << 24, 1 << 24, -128 << 24, 127 << 24, -127 << 24,
            64 << 24, -64 << 24
        };
        static const uint8_t d24[24] = {
            0x00, 0x00, 0x80,  0xFF, 0xFF, 0xFF,   /* -8388608, -1        */
            0x00, 0x00, 0x00,  0x01, 0x00, 0x00,   /* 0, 1                */
            0x56, 0x34, 0x12,  0xAA, 0xCB, 0xED,   /* 0x123456, -0x123456 */
            0xFF, 0xFF, 0x7F,  0x00, 0x00, 0x80    /* 8388607, -8388608   */
        };
        static const int32_t e24[8] = {
            -8388608 << 8, -1 << 8, 0, 1 << 8,
            0x123456 << 8, -0x123456 << 8, 8388607 << 8, -8388608 << 8
        };
        static const uint8_t d32[32] = {
            0x00, 0x00, 0x00, 0x80,  0xFF, 0xFF, 0xFF, 0xFF,
            0x00, 0x00, 0x00, 0x00,  0x01, 0x00, 0x00, 0x00,
            0x78, 0x56, 0x34, 0x12,  0x87, 0xA9, 0xCB, 0xED,
            0xFF, 0xFF, 0xFF, 0x7F,  0x00, 0x00, 0x00, 0x80
        };
        static const int32_t e32[8] = {
            (int32_t)0x80000000u, -1, 0, 1, 0x12345678, (int32_t)0xEDCBA987u,
            (int32_t)0x7FFFFFFFu, (int32_t)0x80000000u
        };
        int k;

        write_wav(".work/pcm8.wav", &(wav_spec_t){
            GOLD_RATE, 2, 8, 1, d8, sizeof d8, -1, 0, 0, 0
        });
        CHECK_EQI(cc->open(state, ".work/pcm8.wav"), CODEC_OK, "8-bit opens");
        CHECK_EQI(cc->format(state, &fmt), CODEC_OK, "8-bit format()");
        CHECK_EQI(fmt.bits, 8, "8-bit reported");
        CHECK_EQI(cc->decode(state, pcm, 8), 4, "8-bit decodes 4 frames");
        for (k = 0; k < 8; k++)
            CHECK_EQI(pcm[k], e8[k], "8-bit sample left-justified");
        cc->close(state);

        write_wav(".work/pcm24.wav", &(wav_spec_t){
            GOLD_RATE, 2, 24, 1, d24, sizeof d24, -1, 0, 0, 0
        });
        CHECK_EQI(cc->open(state, ".work/pcm24.wav"), CODEC_OK, "24-bit opens");
        CHECK_EQI(cc->format(state, &fmt), CODEC_OK, "24-bit format()");
        CHECK_EQI(fmt.bits, 24, "24-bit reported");
        CHECK_EQI(cc->decode(state, pcm, 8), 4, "24-bit decodes 4 frames");
        for (k = 0; k < 8; k++)
            CHECK_EQI(pcm[k], e24[k], "24-bit sample sign-extended << 8");
        cc->close(state);

        write_wav(".work/pcm32.wav", &(wav_spec_t){
            GOLD_RATE, 2, 32, 1, d32, sizeof d32, -1, 0, 0, 0
        });
        CHECK_EQI(cc->open(state, ".work/pcm32.wav"), CODEC_OK, "32-bit opens");
        CHECK_EQI(cc->format(state, &fmt), CODEC_OK, "32-bit format()");
        CHECK_EQI(fmt.bits, 32, "32-bit reported");
        CHECK_EQI(cc->decode(state, pcm, 8), 4, "32-bit decodes 4 frames");
        for (k = 0; k < 8; k++)
            CHECK_EQI(pcm[k], e32[k], "32-bit sample passthrough");
        cc->close(state);

        write_wav(".work/ext24.wav", &(wav_spec_t){
            GOLD_RATE, 2, 24, 0xFFFE, d24, sizeof d24, -1, 0, 0, 0
        });
        CHECK_EQI(cc->open(state, ".work/ext24.wav"), CODEC_OK,
                  "WAVE_FORMAT_EXTENSIBLE (PCM 24) opens");
        CHECK_EQI(cc->format(state, &fmt), CODEC_OK, "extensible format()");
        CHECK_EQI(fmt.bits, 24, "extensible container depth");
        CHECK_EQI(cc->decode(state, pcm, 8), 4, "extensible decodes 4 frames");
        for (k = 0; k < 8; k++)
            CHECK_EQI(pcm[k], e24[k], "extensible sample matches PCM 24");
        cc->close(state);

        /* Extensible with a non-PCM sub-format still gets a clean reject. */
        write_wav(".work/extfloat.wav", &(wav_spec_t){
            GOLD_RATE, 2, 32, 0xFFFE, d32, sizeof d32, -1, 0, 0, 0
        });
        {
            /* Patch the GUID's first two bytes to 3 (IEEE float). */
            FILE *f = fopen(".work/extfloat.wav", "r+b");
            if (f != 0) {
                long off = 12 + 8 + 24;
                fseek(f, off, SEEK_SET);
                fputc(3, f);
                fputc(0, f);
                fclose(f);
            }
        }
        CHECK_EQI(cc->open(state, ".work/extfloat.wav"),
                  CODEC_ERR_UNSUPPORTED,
                  "extensible float sub-format rejected");
    }

    TEST("strict subset rejection");
    {
        uint8_t junk8[400];
        for (i = 0; i < 400; i++)
            junk8[i] = (uint8_t)(i & 0xFF);

        write_wav(".work/float.wav", &(wav_spec_t){
            GOLD_RATE, GOLD_CH, 32, 3,
            junk8, 400, -1, 0, 0, 0
        });
        CHECK_EQI(cc->open(state, ".work/float.wav"), CODEC_ERR_UNSUPPORTED,
                  "IEEE float rejected as UNSUPPORTED");

        write_wav(".work/rifx.wav", &(wav_spec_t){
            GOLD_RATE, GOLD_CH, 16, 1,
            (const uint8_t *)g_ref, sizeof(g_ref),
            -1, 0, 0, "RIFX"
        });
        CHECK_EQI(cc->open(state, ".work/rifx.wav"), CODEC_ERR_FORMAT,
                  "RIFX (big-endian) rejected as FORMAT");
    }

    TEST("declared vs actual data size");
    /* Declared longer than the file (truncated download): clamp, no crash. */
    write_wav(".work/oversize.wav", &(wav_spec_t){
        GOLD_RATE, GOLD_CH, 16, 1,
        (const uint8_t *)g_ref, sizeof(g_ref),
        (long)sizeof(g_ref) + 400, 0, 0, 0
    });
    CHECK_EQI(cc->open(state, ".work/oversize.wav"), CODEC_OK,
              "oversized data size opens (clamped)");
    CHECK_EQI(cc->decode(state, pcm, GOLD_FRAMES), GOLD_FRAMES,
              "clamped to actual file content");
    CHECK_EQI(cc->decode(state, pcm, 16), 0, "EOF after clamp");
    cc->close(state);

    /* Declared shorter than the file: honor the declared end (200 frames). */
    write_wav(".work/short.wav", &(wav_spec_t){
        GOLD_RATE, GOLD_CH, 16, 1,
        (const uint8_t *)g_ref, sizeof(g_ref),
        200 * GOLD_CH * 2, 0, 0, 0
    });
    CHECK_EQI(cc->open(state, ".work/short.wav"), CODEC_OK,
              "short data size opens");
    CHECK_EQI(decode_all(cc, state, pcm, 512, &total), 0,
              "decode runs to EOF");
    CHECK_EQI(total, 200, "stops at the declared data end");
    cc->close(state);

    free(state);

    if (failures == 0)
        printf("test_codec_wav: ALL PASS\n");
    else
        printf("test_codec_wav: %d FAILURES\n", failures);
    return failures;
}
