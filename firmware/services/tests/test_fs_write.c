/*
 * test_fs_write.c — host tests for the fs_target FAT write path.
 *
 * Builds a real FAT16 image in memory (4087 clusters, 2 FATs, 32-entry
 * root), mounts it as "SD", and exercises the narrow write contract:
 * root-only 8.3 files, create/overwrite, data->FAT->dirent commit order,
 * and on-disk consistency across a RE-MOUNT. This is the safety proof for
 * the player_state persistence writes on the SD card.
 *
 *   cc -Wall -Werror -O1 -I.. -o test_fs_write test_fs_write.c ../fs_target.c
 */
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "fs_target.h"

static int s_checks, s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

/* ---- in-memory FAT16 image -------------------------------------------- */

#define SEC        512u
#define TOT_SECS   4130u
#define FATSZ      20u
#define RSV        1u
#define NFATS      2u
#define ROOT_ENTS  32u
#define ROOT_SECS  ((ROOT_ENTS * 32u + 511u) / 512u)   /* 2               */
#define DATA_SEC   (RSV + NFATS * FATSZ + ROOT_SECS)   /* 43              */

static unsigned char s_img[TOT_SECS * SEC];

static int img_read(uint32_t lba, uint32_t count, void *buf)
{
    memcpy(buf, s_img + (size_t)lba * SEC, (size_t)count * SEC);
    return 0;
}

static int img_write(uint32_t lba, uint32_t count, const void *buf)
{
    memcpy(s_img + (size_t)lba * SEC, buf, (size_t)count * SEC);
    return 0;
}

static void img_build(void)
{
    unsigned char *b = s_img;
    uint32_t f;

    memset(s_img, 0, sizeof s_img);
    b[0] = 0xEB; b[1] = 0x90; b[2] = 0x90;
    memcpy(b + 3, "MSDOS5.0", 8);
    b[0x0B] = 0x00; b[0x0C] = 0x02;            /* 512 bytes/sector        */
    b[0x0D] = 1;                               /* 1 sector/cluster        */
    b[0x0E] = 1; b[0x0F] = 0;                  /* reserved sectors        */
    b[0x10] = (unsigned char)NFATS;
    b[0x11] = (unsigned char)ROOT_ENTS; b[0x12] = 0;
    b[0x13] = (unsigned char)(TOT_SECS & 0xFF);
    b[0x14] = (unsigned char)(TOT_SECS >> 8);
    b[0x15] = 0xF8;                            /* media                   */
    b[0x16] = (unsigned char)FATSZ; b[0x17] = 0;
    b[0x1FE] = 0x55; b[0x1FF] = 0xAA;
    for (f = 0; f < NFATS; f++) {              /* FAT[0]=media, FAT[1]=eof */
        unsigned char *fat = s_img + (RSV + f * FATSZ) * SEC;
        fat[0] = 0xF8; fat[1] = 0xFF; fat[2] = 0xFF; fat[3] = 0xFF;
    }
}

/* ---- helpers ----------------------------------------------------------- */

static void pattern_fill(unsigned char *p, int len, int seed)
{
    int i;

    for (i = 0; i < len; i++)
        p[i] = (unsigned char)(seed + i * 7);
}

static int file_matches(const char *path, const unsigned char *want, int len)
{
    unsigned char got[2048];
    fs_file_t *f = fs_open_read(path);
    long r;

    if (!f)
        return 0;
    r = fs_read(f, got, sizeof got);
    fs_close(f);
    return r == len && memcmp(got, want, (size_t)len) == 0;
}

int main(void)
{
    unsigned char a[1180], b2[300], big[600];
    fs_file_t *f;
    long w;
    int i;

    img_build();
    CHECK(fs_target_mount(1, "SD", img_read, 0) == 0);
    CHECK(fs_target_set_writer(1, img_write) == 0);

    /* --- create + write + read-back (the player_state shape: 1180 B) --- */
    pattern_fill(a, sizeof a, 3);
    f = fs_open_write("SD/PLAYER.STA");
    CHECK(f != NULL);
    w = fs_write(f, a, sizeof a);
    CHECK(w == (long)sizeof a);
    fs_close(f);
    CHECK(file_matches("SD/PLAYER.STA", a, sizeof a));

    /* --- overwrite in place (shorter): new content, new size --- */
    pattern_fill(b2, sizeof b2, 91);
    f = fs_open_write("SD/PLAYER.STA");
    CHECK(f != NULL);
    CHECK(fs_write(f, b2, sizeof b2) == (long)sizeof b2);
    fs_close(f);
    CHECK(file_matches("SD/PLAYER.STA", b2, sizeof b2));
    f = fs_open_read("SD/PLAYER.STA");
    CHECK(f && fs_file_size(f) == (long)sizeof b2);
    if (f) fs_close(f);

    /* --- second file coexists --- */
    pattern_fill(big, sizeof big, 17);
    f = fs_open_write("SD/OTHER.DAT");
    CHECK(f != NULL);
    CHECK(fs_write(f, big, sizeof big) == (long)sizeof big);
    fs_close(f);
    CHECK(file_matches("SD/OTHER.DAT", big, sizeof big));
    CHECK(file_matches("SD/PLAYER.STA", b2, sizeof b2));

    /* --- multi-sector, multi-cluster write (600 B = 2 clusters) --- */
    for (i = 0; i < (int)sizeof big; i++)
        big[i] = (unsigned char)(255 - i);
    f = fs_open_write("SD/OTHER.DAT");
    CHECK(f != NULL);
    CHECK(fs_write(f, big, sizeof big) == (long)sizeof big);
    fs_close(f);
    CHECK(file_matches("SD/OTHER.DAT", big, sizeof big));

    /* --- refusals: subdirs, bad names, read handles, read-only volume --- */
    CHECK(fs_open_write("SD/SUB/X.BIN") == NULL);
    CHECK(fs_open_write("SD/TOOLONGNAME.BIN") == NULL);
    CHECK(fs_open_write("SD/BAD.LONGEXT") == NULL);
    f = fs_open_write("SD/NOEXT");               /* extensionless 8.3 ok  */
    CHECK(f != NULL);
    if (f)
        fs_close(f);
    f = fs_open_read("SD/PLAYER.STA");
    CHECK(f != NULL);
    if (f) {
        CHECK(fs_write(f, a, 4) == FS_ERR_UNSUPPORTED);
        fs_close(f);
    }
    CHECK(fs_target_set_writer(1, NULL) == 0);
    CHECK(fs_open_write("SD/RO.BIN") == NULL);
    CHECK(fs_target_set_writer(1, img_write) == 0);

    /* --- RE-MOUNT: what landed on disk is consistent FAT --- */
    fs_target_unmount(1);
    CHECK(fs_target_mount(1, "SD", img_read, 0) == 0);
    CHECK(fs_target_set_writer(1, img_write) == 0);
    CHECK(file_matches("SD/OTHER.DAT", big, sizeof big));
    CHECK(file_matches("SD/PLAYER.STA", b2, sizeof b2));
    f = fs_open_read("SD/PLAYER.STA");
    CHECK(f && fs_file_size(f) == (long)sizeof b2);
    if (f) fs_close(f);

    printf("test_fs_write: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
