/*
 * test_media_scan.c — host test for the index scanner (BRNOMT-74).
 *
 * The host FS backend maps /Internal and /SD under .work, and the tag reader
 * finds no tags in these dummy files: the records must then carry the filename
 * (extension stripped) as the title, skip non-audio files and ignore nothing
 * else. An aborting progress callback must leave the previous index current.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "fs.h"
#include "fs_host.h"
#include "media_db.h"
#include "media_scan.h"
#include "scratch.h"

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

static void put(const char *path)
{
    fs_file_t *f = fs_open_write(path);

    if (f != 0) {
        (void)fs_write(f, "not really audio", 16);
        fs_close(f);
    } else {
        printf("FAIL setup: cannot create %s\n", path);
    }
}

static int abort_now(uint32_t files, const char *name)
{
    (void)files;
    (void)name;
    return 1;
}

/* Is a record with this volume-relative path in the open index? */
static int has_path(const char *p)
{
    media_rec_t r;
    uint32_t i, n = media_db_count();

    for (i = 0; i < n; i++) {
        if (media_db_get(i, &r) == FS_OK && strcmp(r.path, p) == 0)
            return 1;
    }
    return 0;
}

int main(void)
{
    media_rec_t r;

    printf("== media_scan: walk + records ==\n");
    (void)mkdir(".work", 0777);
    (void)mkdir(".work/Internal", 0777);
    (void)mkdir(".work/Internal/Music", 0777);
    (void)mkdir(".work/Internal/.rechord", 0777);
    (void)mkdir(".work/SD", 0777);
    fs_host_set_root(".work");
    media_db_close();
    (void)remove(".work/Internal/.rechord/INDEX.A");
    (void)remove(".work/Internal/.rechord/INDEX.B");
    /* the abort case creates extra files: a second run must not see them */
    for (int k = 0; k < 8; k++) {
        char p[64];

        snprintf(p, sizeof p, ".work/Internal/Music/extra %d.wav", k);
        (void)remove(p);
    }
    put("/Internal/Music/01 First.mp3");
    put("/Internal/Music/notes.txt");              /* not audio: ignored */
    put("/SD/Second.flac");

    CHECK(media_scan_build(0) == 0);
    CHECK(media_db_open() == FS_OK);
    CHECK(media_db_count() == 2);
    CHECK(media_db_get(0, &r) == FS_OK);
    CHECK(strcmp(r.path, "Internal/Music/01 First.mp3") == 0);
    CHECK(strcmp(r.title, "01 First") == 0);       /* filename fallback */
    CHECK(media_db_get(1, &r) == FS_OK);
    CHECK(strcmp(r.path, "SD/Second.flac") == 0);
    CHECK(strcmp(r.title, "Second") == 0);
    /* No tags on these files: every category field is the "Undefined" group,
     * never an empty string, and the year is 0 (the views render it as
     * "Undefined" too). */
    CHECK(strcmp(r.artist, "Undefined") == 0);
    CHECK(strcmp(r.album, "Undefined") == 0);
    CHECK(strcmp(r.album_artist, "Undefined") == 0);
    CHECK(strcmp(r.composer, "Undefined") == 0);
    CHECK(strcmp(r.genre, "Undefined") == 0);
    CHECK(strcmp(r.label, "Undefined") == 0);
    CHECK(r.year == 0);
    media_db_close();

    /* Abort: nothing is committed, the previous index stays current. The
     * callback fires every 8 files, so grow the library past that first. */
    for (int k = 0; k < 8; k++) {
        char p[64];

        snprintf(p, sizeof p, "/Internal/Music/extra %d.wav", k);
        put(p);
    }
    CHECK(media_scan_build(abort_now) == MEDIA_SCAN_ABORTED);
    CHECK(media_db_open() == FS_OK);
    CHECK(media_db_count() == 2);
    media_db_close();

    /* Depth cap + iterative walk: a tree deeper than 8 must be ignored, and the
     * walk must not grow the stack per level (it uses an explicit frame stack
     * in the shared scratch region). The depth-9 file must not be indexed; the
     * shallow files still must be. */
    {
        char host[160] = ".work/Internal/Music";
        char dev[160] = "/Internal/Music";
        int d;

        for (d = 1; d <= 10; d++) {
            size_t hl = strlen(host), dl = strlen(dev);

            snprintf(host + hl, sizeof host - hl, "/L%d", d);
            snprintf(dev + dl, sizeof dev - dl, "/L%d", d);
            (void)mkdir(host, 0777);
        }
        {
            size_t hl = strlen(host), dl = strlen(dev);

            snprintf(host + hl, sizeof host - hl, "/deep.wav");
            snprintf(dev + dl, sizeof dev - dl, "/deep.wav");
            put(dev);
        }
    }
    CHECK(media_scan_build(0) == 0);
    CHECK(media_db_open() == FS_OK);
    CHECK(has_path("Internal/Music/L1/L2/L3/L4/L5/L6/L7/L8/L9/L10/deep.wav")
              == 0);
    CHECK(has_path("Internal/Music/01 First.mp3") == 1);
    media_db_close();

    /* Ownership: the scan claims the shared region and releases it on every
     * exit. After a successful build the region is free again; while another
     * owner holds it the build fails cleanly (returns FS_ERR_IO, touches
     * nothing) and the previous index stays current. */
    CHECK(scratch_owner() == SCRATCH_OWNER_NONE);
    {
        uint32_t before = 0;

        CHECK(media_db_open() == FS_OK);
        before = media_db_count();
        media_db_close();

        CHECK(scratch_claim(SCRATCH_OWNER_USB) != 0);
        CHECK(media_scan_build(0) == FS_ERR_IO);
        scratch_release(SCRATCH_OWNER_USB);
        CHECK(scratch_owner() == SCRATCH_OWNER_NONE);

        CHECK(media_db_open() == FS_OK);   /* the previous index survived */
        CHECK(media_db_count() == before);
        media_db_close();
    }

    if (s_failures == 0)
        printf("test_media_scan: %d checks, OK\n", s_checks);
    else
        printf("test_media_scan: %d checks, %d FAILURES\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
