/*
 * test_fs_bulk.c — fs_read() multi-sector path (fs_target.c).
 *
 * Usage: test_fs_bulk <fresh FAT image>   (run_tests.sh makes it with
 * mkfs.fat). Two files are grown by alternating appends through the firmware's
 * own writer, so their cluster chains interleave (fragmented). Every read
 * pattern must return exactly the bytes written: odd chunks, aligned 4 KiB
 * chunks (the bulk path), unaligned buffers, the player's sector-trimmed ring
 * refills after arbitrary seeks — and the same again with every multi-sector
 * read failing, which must fall back to single sectors with no data change.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "fs_target.h"

static FILE *s_img;
static unsigned s_multi;           /* rd() calls with count > 1 */
static int s_fail_multi;           /* inject: every count > 1 read fails */
static int s_failures;

#define CHECK(c, m) do { if (c) printf("  PASS: %s\n", m); \
    else { printf("  FAIL: %s (line %d)\n", m, __LINE__); s_failures++; } } while (0)

static int img_read(uint32_t lba, uint32_t count, void *buf)
{
    if (count > 1u) {
        s_multi++;
        if (s_fail_multi)
            return -5;
    }
    return fseek(s_img, (long)lba * 512L, SEEK_SET) ||
           fread(buf, 512, count, s_img) != count;
}

static int img_write(uint32_t lba, uint32_t count, const void *buf)
{
    return fseek(s_img, (long)lba * 512L, SEEK_SET) ||
           fwrite(buf, 512, count, s_img) != count;
}

#define FILE_BYTES (200u * 1024u + 77u)

static uint8_t pat(int file, uint32_t i)
{
    return (uint8_t)((i * 131u + (i >> 9) * 7u + (unsigned)file * 101u) & 0xFFu);
}

static uint8_t s_buf[8192 + 8];

/* Read `path` in `chunk`-byte reads into s_buf + `skew`; 1 = all bytes match. */
static int read_all(const char *path, int file, unsigned chunk, unsigned skew)
{
    fs_file_t *f = fs_open_read(path);
    uint32_t pos = 0;
    long r;

    if (f == 0)
        return 0;
    while ((r = fs_read(f, s_buf + skew, chunk)) > 0) {
        for (long k = 0; k < r; k++)
            if (s_buf[skew + k] != pat(file, pos + (uint32_t)k)) {
                fs_close(f);
                return 0;
            }
        pos += (uint32_t)r;
    }
    fs_close(f);
    return r == 0 && pos == FILE_BYTES;
}

/* bfs.c-style refills after a seek: 4 KiB reads trimmed to END on a file
 * sector boundary, destination offset = file offset mod 512 (the ring base is
 * sector-aligned, so dest alignment follows the file position). */
static int read_ring(const char *path, int file, uint32_t seek)
{
    fs_file_t *f = fs_open_read(path);
    uint32_t pos = seek;

    if (f == 0 || fs_seek(f, (long)seek) != (long)seek)
        return 0;
    while (pos < FILE_BYTES) {
        uint32_t n = 4096u, end, at = pos % 512u;
        long r;

        end = (pos + n) & 511u;
        if (n > 512u)
            n -= end;
        r = fs_read(f, s_buf + at, n);
        if (r <= 0)
            break;
        for (long k = 0; k < r; k++)
            if (s_buf[at + k] != pat(file, pos + (uint32_t)k)) {
                fs_close(f);
                return 0;
            }
        pos += (uint32_t)r;
    }
    fs_close(f);
    return pos == FILE_BYTES;
}

static void patterns(const char *tag)
{
    static const uint32_t seeks[] = { 0, 1, 511, 512, 513, 4095, 4096,
                                      70001, FILE_BYTES - 600 };
    unsigned i;
    int ok;
    char m[96];

    for (i = 0; i < 2; i++) {
        const char *p = i ? "/SD/b.bin" : "/SD/a.bin";

        snprintf(m, sizeof m, "%s %s: 777-byte reads", tag, p);
        CHECK(read_all(p, (int)i, 777, 0), m);
        snprintf(m, sizeof m, "%s %s: aligned 4 KiB reads", tag, p);
        CHECK(read_all(p, (int)i, 4096, 0), m);
        snprintf(m, sizeof m, "%s %s: 8 KiB reads into an odd buffer", tag, p);
        CHECK(read_all(p, (int)i, 8192, 3), m);
        ok = 1;
        for (unsigned k = 0; k < sizeof seeks / sizeof seeks[0]; k++)
            ok &= read_ring(p, (int)i, seeks[k]);
        snprintf(m, sizeof m, "%s %s: ring refills after 9 seeks", tag, p);
        CHECK(ok, m);
    }
}

int main(int argc, char **argv)
{
    static uint8_t chunk[5000];
    uint32_t done = 0;

    if (argc != 2 || (s_img = fopen(argv[1], "r+b")) == 0)
        return 2;
    printf("=== fs_target bulk reads (%s) ===\n", argv[1]);
    CHECK(fs_target_mount(1, "SD", img_read, 0) == 0, "mount");
    CHECK(fs_target_set_writer(1, img_write) == 0, "writer");
    CHECK(fs_target_file_create("/SD/a.bin", 0) == 0 &&
          fs_target_file_create("/SD/b.bin", 0) == 0, "create two files");
    /* Alternate appends: the two chains interleave cluster by cluster. */
    while (done < FILE_BYTES) {
        uint32_t n = FILE_BYTES - done < sizeof chunk ? FILE_BYTES - done
                                                      : (uint32_t)sizeof chunk;
        int file;

        for (file = 0; file < 2; file++) {
            for (uint32_t k = 0; k < n; k++)
                chunk[k] = pat(file, done + k);
            if (fs_target_file_append(file ? "/SD/b.bin" : "/SD/a.bin",
                                      chunk, n) != 0) {
                CHECK(0, "append");
                return 1;
            }
        }
        done += n;
    }

    s_multi = 0;
    patterns("bulk");
    CHECK(s_multi > 0, "the bulk path issued multi-sector reads");

    /* A card that rejects CMD18: the first failure switches the volume to
     * single sectors and the data must still be exact. */
    fs_target_unmount(1);
    CHECK(fs_target_mount(1, "SD", img_read, 0) == 0, "remount");
    s_fail_multi = 1;
    s_multi = 0;
    patterns("fallback");
    CHECK(s_multi == 1, "one failed multi-sector read, then single sectors");
    CHECK(fs_target_multi_failed(1) == 1, "the fallback is reported");

    printf("%s (%d failure%s)\n", s_failures ? "FAILURES" : "ALL FS BULK TESTS PASSED",
           s_failures, s_failures == 1 ? "" : "s");
    fclose(s_img);
    return s_failures != 0;
}
