/*
 * test_flac_prescan.c — the FLAC adapter's prescan geometry hand-off.
 *
 * Core B sizes its decode pool from the geometry the AP publishes while
 * prescanning a file (IPC_C_PRE_*, codec_t.prescan_info). The AP build is
 * metadata-only, and that build used to leave the diagnostic fields unset, so
 * prescan_info() returned 0 and B repeated a full header walk on every OPEN.
 * This test runs in BOTH builds (the runner compiles it a second time with
 * -DRECHORD_AP_METADATA_ONLY=1) and pins:
 *   - prescan_info() publishes max block / channels in either build,
 *   - only a real SEEKTABLE (metadata block type 3) counts as seektable bytes:
 *     an embedded PICTURE (type 6) must not be mistaken for one.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codec.h"
#include "test_util.h"

extern const codec_t codec_flac;

#define RATE      44100
#define BLOCKSIZE 4096
#define PICTURE_LEN 48u
#define SEEKPOINTS 2u
#define SEEKTABLE_LEN (SEEKPOINTS * 18u)   /* 18 bytes per seekpoint */

static void put8(unsigned char *b, long *n, unsigned v)
{
    b[(*n)++] = (unsigned char)v;
}

static void put16be(unsigned char *b, long *n, unsigned v)
{
    put8(b, n, v >> 8);
    put8(b, n, v);
}

static void put24be(unsigned char *b, long *n, unsigned v)
{
    put8(b, n, v >> 16);
    put8(b, n, v >> 8);
    put8(b, n, v);
}

static void put_meta_hdr(unsigned char *b, long *n, int last, int type,
                         unsigned len)
{
    put8(b, n, (unsigned)((last ? 0x80 : 0) | type));
    put8(b, n, len >> 16);
    put8(b, n, len >> 8);
    put8(b, n, len);
}

static void put_streaminfo(unsigned char *b, long *n, unsigned long total,
                           unsigned blocksize, int bps)
{
    unsigned bsp1 = (unsigned)(bps - 1);

    put16be(b, n, blocksize);           /* min block size */
    put16be(b, n, blocksize);           /* max block size */
    put24be(b, n, 0);                   /* min frame size */
    put24be(b, n, 0);                   /* max frame size */
    put8(b, n, RATE >> 12);
    put8(b, n, RATE >> 4);
    put8(b, n, ((RATE & 0x0F) << 4) | (1 << 1) | ((bsp1 >> 4) & 1));
    put8(b, n, ((bsp1 & 0x0F) << 4) | ((total >> 32) & 0xF));
    put8(b, n, (total >> 24) & 0xFF);
    put8(b, n, (total >> 16) & 0xFF);
    put8(b, n, (total >> 8) & 0xFF);
    put8(b, n, total & 0xFF);
    {
        int i;
        for (i = 0; i < 16; i++)
            put8(b, n, 0);              /* md5: unknown */
    }
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

int main(void)
{
    static unsigned char buf[1024];
    long n = 0;
    char path[] = "/tmp/rechord_codec_flac_prescan.flac";
    codec_prescan_t p;
    codec_format_t fmt;
    void *st;
    unsigned i;

    TEST("flac: prescan geometry published to Core B");

    put8(buf, &n, 'f'); put8(buf, &n, 'L');
    put8(buf, &n, 'a'); put8(buf, &n, 'C');
    put_meta_hdr(buf, &n, 0, 0, 34);    /* STREAMINFO */
    put_streaminfo(buf, &n, BLOCKSIZE, BLOCKSIZE, 16);
    put_meta_hdr(buf, &n, 0, 6, PICTURE_LEN);   /* PICTURE (art) */
    for (i = 0; i < PICTURE_LEN; i++)
        put8(buf, &n, 0x5A);
    put_meta_hdr(buf, &n, 1, 3, SEEKTABLE_LEN); /* SEEKTABLE, last */
    for (i = 0; i < SEEKTABLE_LEN; i++)
        put8(buf, &n, 0);

    if (write_file(path, buf, n) != 0) {
        printf("  FAIL: cannot write fixture\n");
        return 1;
    }

    st = calloc(1, PLAYER_CODEC_STATE_MAX);
    if (!st) {
        printf("  FAIL: alloc\n");
        remove(path);
        return 1;
    }

    CHECK_EQI(codec_flac.open(st, path), 0, "open fixture");

    memset(&p, 0, sizeof p);
    CHECK_EQI(codec_flac.prescan_info == 0 ? -1
                                           : codec_flac.prescan_info(st, &p),
              1, "prescan_info() publishes geometry");
    CHECK_EQI(p.max_block, BLOCKSIZE, "max block from STREAMINFO");
    CHECK_EQI(p.channels, 2, "channels from STREAMINFO");
    CHECK_EQI(p.seektable_bytes, SEEKTABLE_LEN,
              "only the type-3 SEEKTABLE is counted (not the PICTURE)");

    memset(&fmt, 0, sizeof fmt);
    CHECK_EQI(codec_flac.format(st, &fmt), 0, "format()");
    CHECK_EQI(fmt.sample_rate, RATE, "sample rate");
    CHECK_EQI(fmt.channels, 2, "channels");
    CHECK_EQI(fmt.bits, 16, "source bit depth");

    codec_flac.close(st);
    free(st);
    remove(path);

    if (failures == 0) {
        printf("test_flac_prescan: OK\n");
        return 0;
    }
    printf("test_flac_prescan: %d FAILURE(S)\n", failures);
    return failures;
}
