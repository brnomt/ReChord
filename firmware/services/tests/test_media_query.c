/*
 * test_media_query.c — host test for the category queries (BRNOMT-78).
 *
 * Builds an index with known records, then checks the two operations the
 * category screens use: unique sorted names ("Undefined" first) and the
 * filtered record list. This is the part that must not be verified through a
 * panel, which is why it lives in its own module.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "fs.h"
#include "fs_host.h"
#include "media_db.h"
#include "media_query.h"
#include "scratch.h"

static int s_checks, s_failures;
#define CHECK(c) do { s_checks++; if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); s_failures++; } } while (0)

static void add(const char *title, const char *album, unsigned year,
                const char *artist)
{
    media_rec_t r;

    memset(&r, 0, sizeof r);
    snprintf(r.path, sizeof r.path, "SD/%s.flac", title);
    snprintf(r.title, sizeof r.title, "%s", title);
    snprintf(r.album, sizeof r.album, "%s", album);
    snprintf(r.artist, sizeof r.artist, "%s", artist);
    snprintf(r.genre, sizeof r.genre, "Rock");
    r.year = (uint16_t)year;
    (void)media_db_add(&r);
}

int main(void)
{
    static uint8_t s_dbbuf[SCRATCH_BYTES];   /* caller-supplied build staging */
    int i;

    printf("== media_query: names + filters ==\n");
    (void)mkdir(".work", 0777);
    (void)mkdir(".work/Internal", 0777);
    (void)mkdir(".work/Internal/.rechord", 0777);
    fs_host_set_root(".work");
    (void)remove(".work/Internal/.rechord/INDEX.A");
    (void)remove(".work/Internal/.rechord/INDEX.B");

    CHECK(media_db_begin(s_dbbuf, (uint32_t)sizeof s_dbbuf) == FS_OK);
    add("A1", "Undefined", 2015, "Zed");
    add("A2", "Undefined", 2015, "Zed");
    add("B1", "Blue", 2012, "Amy");
    add("C1", "Undefined", 2015, "Amy");
    add("D1", "Blue", 0, "Zed");          /* year 0 -> the Undefined group */
    CHECK(media_db_end() == FS_OK);

    /* Album names: Undefined first, then the rest sorted. */
    CHECK(mq_names_init(MQ_ALBUM) == 0);
    CHECK(mq_names_count() == 2);
    CHECK(strcmp(mq_names_get(0), "Undefined") == 0);
    CHECK(strcmp(mq_names_get(1), "Blue") == 0);

    /* Year names: the 0 year is "Undefined", and it sorts first. */
    CHECK(mq_names_init(MQ_YEAR) == 0);
    CHECK(mq_names_count() == 3);
    CHECK(strcmp(mq_names_get(0), "Undefined") == 0);
    CHECK(strcmp(mq_names_get(1), "2012") == 0);
    CHECK(strcmp(mq_names_get(2), "2015") == 0);

    /* Filter: the songs of 2015. */
    CHECK(mq_filter_init(MQ_YEAR, "2015") == 0);
    CHECK(mq_filter_count() == 3);
    {
        media_rec_t r;

        CHECK(mq_filter_get(0, &r) == FS_OK);
        CHECK(strcmp(r.title, "A1") == 0);
    }

    /* Filter by album, and the whole library (All Songs). */
    CHECK(mq_filter_init(MQ_ALBUM, "Blue") == 0);
    CHECK(mq_filter_count() == 2);
    CHECK(mq_filter_init(MQ_ALL, 0) == 0);
    CHECK(mq_filter_count() == 5);

    /* BRNOMT-98: the browser resolves its start track during the folder scan;
     * the position is the SORTED one (A1, A2, B1, C1, D1). */
    {
        int start = 99;

        CHECK(mq_folder_init_start("SD", "/SD/B1.flac", &start) == 0);
        CHECK(mq_filter_count() == 5);
        CHECK(start == 2);
        CHECK(mq_folder_init_start("SD", "/SD/nope.flac", &start) == 0);
        CHECK(start == -1);
        CHECK(mq_folder_init_start("SD/Missing", "/SD/B1.flac", &start) == 0);
        CHECK(mq_filter_count() == 0);
        CHECK(start == -1);
        CHECK(mq_folder_init("SD") == 0);        /* wrapper still works */
        CHECK(mq_filter_count() == 5);
    }

    /* The resume-queue rebuild uses the same single-pass lookup on a filter:
     * Blue has B1/D1 (album sort key, then record order); All Songs sorts by
     * title and C1 is position 3. */
    {
        int start = 99;

        CHECK(mq_filter_init_start(MQ_ALBUM, "Blue", "/SD/B1.flac", &start) == 0);
        CHECK(mq_filter_count() == 2);
        CHECK(start == 0);
        CHECK(mq_filter_init_start(MQ_ALL, 0, "/SD/C1.flac", &start) == 0);
        CHECK(mq_filter_count() == 5);
        CHECK(start == 3);
        CHECK(mq_filter_init_start(MQ_ALL, 0, "/SD/nope.flac", &start) == 0);
        CHECK(start == -1);
    }
    for (i = 0; i < 2; i++)               /* repeated init is safe */
        CHECK(mq_names_init(MQ_ARTIST) == 0);
    CHECK(mq_names_count() == 2);
    CHECK(strcmp(mq_names_get(0), "Amy") == 0);

    mq_done();
    CHECK(mq_names_count() == 0 && mq_filter_count() == 0);

    /* Case-insensitive grouping (BRNOMT-92): "apple" must sort next to
     * "Banana"/"cherry", not after every uppercase-initial name. Plain strcmp
     * put 'BALLADS' before 'Backstreet's Back' and pushed lowercase-initial
     * albums past the whole uppercase alphabet. */
    CHECK(media_db_begin(s_dbbuf, (uint32_t)sizeof s_dbbuf) == FS_OK);
    add("apple", "apple", 2020, "zed");
    add("Banana", "Banana", 2020, "zed");
    add("cherry", "cherry", 2020, "zed");
    CHECK(media_db_end() == FS_OK);

    CHECK(mq_names_init(MQ_ALBUM) == 0);
    CHECK(mq_names_count() == 3);
    CHECK(strcmp(mq_names_get(0), "apple") == 0);
    CHECK(strcmp(mq_names_get(1), "Banana") == 0);
    CHECK(strcmp(mq_names_get(2), "cherry") == 0);

    CHECK(mq_filter_init(MQ_ALL, 0) == 0);   /* All Songs: same comparer */
    CHECK(mq_filter_count() == 3);
    {
        media_rec_t r;

        CHECK(mq_filter_get(0, &r) == FS_OK);
        CHECK(strcmp(r.title, "apple") == 0);
    }
    mq_done();

    /* No index at all -> the queries report it instead of an empty list. */
    (void)remove(".work/Internal/.rechord/INDEX.A");
    (void)remove(".work/Internal/.rechord/INDEX.B");
    CHECK(mq_names_init(MQ_ALBUM) == FS_ERR_OPEN);
    CHECK(mq_names_count() == 0);

    if (s_failures == 0)
        printf("test_media_query: %d checks, OK\n", s_checks);
    else
        printf("test_media_query: %d checks, %d FAILURES\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
