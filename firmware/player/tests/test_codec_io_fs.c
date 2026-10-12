/*
 * test_codec_io_fs.c — proves a WAV decodes identically through the
 * target codec_io_fs backend (fs.h) as the analytical sine.
 *
 * codec_io_fs.c + codec_io.c + fs_host.c are linked into this test binary
 * (via run_tests.sh), so cio_open picks the fs backend against the fs.h
 * API backed by fs_host on the host. On the device the same backend links
 * against fs_target.c instead — the seam is exercised end-to-end.
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../codec.h"
#include "../codec_io.h"
#include "test_util.h"

/* fs_host.h isn't public; declare the root-setter we need to point the
 * fs backend at a real temp dir for the host run. */
extern void fs_host_set_root(const char *dir);

extern const codec_t codec_wav;

#define T_PI 3.14159265358979323846
#define RATE 44100
#define NSAMPLES 1024

static void write_u32(FILE *f, uint32_t v)
{
    uint8_t b[4];
    b[0] = (uint8_t)(v & 0xFF);
    b[1] = (uint8_t)((v >> 8) & 0xFF);
    b[2] = (uint8_t)((v >> 16) & 0xFF);
    b[3] = (uint8_t)((v >> 24) & 0xFF);
    fwrite(b, 1, 4, f);
}

static void write_wav(const char *path, const int16_t *samples, int n, int ch)
{
    FILE *f = fopen(path, "wb");
    uint32_t data_bytes = (uint32_t)(n * ch * 2);

    fwrite("RIFF", 1, 4, f);
    write_u32(f, 36 + data_bytes);
    fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f);
    write_u32(f, 16);                       /* fmt chunk size (PCM)        */
    {
        uint8_t b[16];
        uint32_t r = (uint32_t)RATE;
        uint32_t ab = (uint32_t)RATE * (uint32_t)ch * 2;
        b[0] = 1; b[1] = 0;
        b[2] = (uint8_t)(ch & 0xFF); b[3] = 0;
        b[4] = (uint8_t)(r & 0xFF); b[5] = (uint8_t)((r >> 8) & 0xFF);
        b[6] = (uint8_t)((r >> 16) & 0xFF); b[7] = (uint8_t)((r >> 24) & 0xFF);
        b[8] = (uint8_t)(ab & 0xFF); b[9] = (uint8_t)((ab >> 8) & 0xFF);
        b[10] = (uint8_t)((ab >> 16) & 0xFF); b[11] = (uint8_t)((ab >> 24) & 0xFF);
        b[12] = (uint8_t)((ch * 2) & 0xFF); b[13] = 0;
        b[14] = 16; b[15] = 0;
        fwrite(b, 1, 16, f);
    }
    fwrite("data", 1, 4, f);
    write_u32(f, data_bytes);
    fwrite(samples, 2, (size_t)(n * ch), f);
    fclose(f);
}

static void gen_sine(int16_t *s, int n, int ch, double tone)
{
    for (int i = 0; i < n * ch; i++) {
        double phase = tone * (2.0 * T_PI) * (double)(i / ch) / (double)RATE;
        s[i] = (int16_t)(30000.0 * sin(phase));
    }
}

int main(void)
{
    TEST("codec_io_fs: WAV decode through fs backend");

    const char *workdir = "/tmp/rechord_iofs_work";
    const char *devname = "/rechord_iofs_test.wav";
    char path[256];
    int16_t samples[NSAMPLES * 2];
    int16_t pcm_a[NSAMPLES * 2];
    int16_t pcm_b[NSAMPLES * 2];
    uint8_t st_mem[4096];
    codec_format_t fmt = {0, 0, 0};
    codec_t local = codec_wav;

    gen_sine(samples, NSAMPLES, 2, 440.0);
    fs_host_set_root(workdir);
    snprintf(path, sizeof path, "%s/rechord_iofs_test.wav", workdir);
    (void)mkdir(workdir, 0755);   /* may already exist on re-runs */
    write_wav(path, samples, NSAMPLES, 2);

    memset(st_mem, 0, sizeof st_mem);
    CHECK_EQI(local.open(st_mem, devname), 0, "fs backend: open succeeds");

    CHECK_EQI(local.format != NULL, 1, "fs backend: format callback present");
    CHECK_EQI(local.format(st_mem, &fmt), 0, "fs backend: format reports");
    CHECK_EQI(fmt.sample_rate, RATE, "fs backend: sample rate == 44100");
    CHECK_EQI(fmt.channels, 2, "fs backend: channels == 2");
    CHECK_EQI(fmt.bits, 16, "fs backend: bits == 16");

    CHECK_EQI(local.decode(st_mem, pcm_a, NSAMPLES), NSAMPLES,
              "fs backend: decoded a full block");
    CHECK_EQI(local.seek(st_mem, 0), 0, "fs backend: seek(0) ok");
    CHECK_EQI(local.decode(st_mem, pcm_b, NSAMPLES), NSAMPLES,
              "fs backend: re-decode after seek");
    CHECK(memcmp(pcm_a, pcm_b, (size_t)NSAMPLES * 2 * sizeof(int16_t)) == 0,
          "fs backend: seek->decode byte-exact vs first decode");

    /* Cross-check the FS-backed decode against the analytical sine. */
    int bad = 0;
    for (int i = 0; i < NSAMPLES * 2; i++) {
        double phase = 440.0 * (2.0 * T_PI) * (double)(i / 2) / (double)RATE;
        int16_t expect = (int16_t)(30000.0 * sin(phase));
        if (pcm_a[i] != expect) {
            bad = 1; break;
        }
    }
    CHECK(!bad, "fs backend: decoded samples match the sine formula");

    CHECK_EQI(local.seek(st_mem, 10), 0,
              "fs backend: seek to a mid-track position (10 ms)");
    CHECK_EQI(local.decode(st_mem, pcm_a, 8), 8,
              "fs backend: decode a few frames after mid seek");

    memset(st_mem, 0, sizeof st_mem);
    local.close(st_mem);

    remove(path);
    printf("\n");
    return failures;
}
