/*
 * test_media_db.c — host tests for the media index (BRNOMT-74).
 *
 * Covers: a build in the inactive slot, the commit, reading the records back,
 * a rebuild flipping slots with the sequence advancing, and both fallbacks a
 * damaged index can need (one slot corrupt -> the other; both corrupt -> "no
 * index" rather than garbage).
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "media_db.h"
#include "scratch.h"
#include "fs.h"
#include "fs_host.h"

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

static void mkrec(media_rec_t *r, const char *path, const char *title,
                  uint16_t trk)
{
    memset(r, 0, sizeof *r);
    snprintf(r->path, sizeof r->path, "%s", path);
    snprintf(r->title, sizeof r->title, "%s", title);
    snprintf(r->artist, sizeof r->artist, "Artist %u", (unsigned)trk);
    snprintf(r->album, sizeof r->album, "Album");
    snprintf(r->genre, sizeof r->genre, "Rock");
    r->track_no = trk;
    r->duration_ms = 1000u * trk;
}

static int build(const char *t1, const char *t2, const char *t3)
{
    media_rec_t r;
    static uint8_t buf[SCRATCH_BYTES];   /* the caller now supplies staging */

    if (media_db_begin(buf, (uint32_t)sizeof buf) != FS_OK)
        return -1;
    mkrec(&r, "Internal/Music/01.flac", t1, 1);
    (void)media_db_add(&r);
    mkrec(&r, "SD/Music/02.mp3", t2, 2);
    (void)media_db_add(&r);
    mkrec(&r, "SD/Music/03.wav", t3, 3);
    (void)media_db_add(&r);
    return media_db_end();
}

int main(void)
{
    media_rec_t r;

    printf("== media_db: index slots + commit ==\n");
    (void)mkdir(".work", 0777);
    (void)mkdir(".work/Internal", 0777);
    (void)mkdir(".work/Internal/.rechord", 0777);
    fs_host_set_root(".work");
    media_db_close();
    (void)remove(".work/Internal/.rechord/INDEX.A");
    (void)remove(".work/Internal/.rechord/INDEX.B");

    /* No index yet. */
    CHECK(media_db_open() == FS_ERR_OPEN);
    CHECK(media_db_is_open() == 0);
    CHECK(media_db_count() == 0);

    /* Argument guards on the caller-supplied staging buffer. */
    CHECK(media_db_begin(0, 4096u) == FS_ERR_ARG);
    CHECK(media_db_begin((void *)1, 0u) == FS_ERR_ARG);

    /* First build -> slot A, seq 1. */
    CHECK(build("One", "Two", "Three") == FS_OK);
    CHECK(media_db_open() == FS_OK);
    CHECK(media_db_is_open() == 1);                 /* BRNOMT-98 */
    CHECK(media_db_count() == 3);
    CHECK(media_db_get(0, &r) == FS_OK);
    CHECK(strcmp(r.title, "One") == 0 && r.track_no == 1);
    CHECK(strcmp(r.path, "Internal/Music/01.flac") == 0);
    CHECK(media_db_get(2, &r) == FS_OK);
    CHECK(strcmp(r.title, "Three") == 0 && r.duration_ms == 3000u);
    CHECK(media_db_get(3, &r) == FS_ERR_ARG);       /* out of range */
    media_db_close();
    CHECK(media_db_is_open() == 0);

    /* Rebuild -> the other slot, seq 2, the new content wins. */
    CHECK(build("Uno", "Dos", "Tres") == FS_OK);
    CHECK(media_db_open() == FS_OK);
    CHECK(media_db_count() == 3);
    CHECK(media_db_get(1, &r) == FS_OK);
    CHECK(strcmp(r.title, "Dos") == 0);
    media_db_close();

    /* Corrupt ONLY the newest slot's seq word: magic/version/count still pass
     * the shape check, so the reader would trust it — but the global CRC
     * covers the header, so it must reject B and fall back to A. The old
     * records-only CRC could NOT catch a header field like this. */
    {
        uint32_t bad_seq = 0xFFFFFFFFu;

        CHECK(fs_patch(MEDIA_DB_B, 8u, &bad_seq,
                       (uint32_t)sizeof bad_seq) == FS_OK);
    }
    CHECK(media_db_open() == FS_OK);
    CHECK(media_db_get(0, &r) == FS_OK);
    CHECK(strcmp(r.title, "One") == 0);             /* the seq-1 build */
    media_db_close();

    /* Damage the newest slot's header: the older one must be used. */
    {
        media_hdr_t bad;

        memset(&bad, 0xA5, sizeof bad);
        CHECK(fs_patch(MEDIA_DB_B, 0u, &bad, sizeof bad) == FS_OK);
    }
    CHECK(media_db_open() == FS_OK);
    CHECK(media_db_get(0, &r) == FS_OK);
    CHECK(strcmp(r.title, "One") == 0);             /* the seq-1 build */
    media_db_close();

    /* Damage both: "no index", never garbage. */
    {
        media_hdr_t bad;

        memset(&bad, 0, sizeof bad);
        CHECK(fs_patch(MEDIA_DB_A, 0u, &bad, sizeof bad) == FS_OK);
    }
    CHECK(media_db_open() == FS_ERR_OPEN);

    /* A build after that starts from a clean slate (seq 1 again). */
    CHECK(build("A", "B", "C") == FS_OK);
    CHECK(media_db_open() == FS_OK);
    CHECK(media_db_count() == 3);
    media_db_close();

    if (s_failures == 0)
        printf("test_media_db: %d checks, OK\n", s_checks);
    else
        printf("test_media_db: %d checks, %d FAILURES\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
