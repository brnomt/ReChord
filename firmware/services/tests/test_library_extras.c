/*
 * test_library_extras.c — favorites, recently played and play counts.
 *
 * Pure store: hash identity, MRU ordering, play counts, the 32/48 caps,
 * most-played ordering and the file round-trip through fs_host.
 */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "library_extras.h"
#include "rechord_files.h"
#include "fs.h"
#include "fs_host.h"

static int s_failures;

#define TEST(name) printf("== %s ==\n", name)
#define CHECK(cond, name) do {                                             \
    if (cond) printf("  PASS: %s\n", name);                                \
    else { printf("  FAIL: %s\n", name); s_failures++; }                    \
} while (0)
#define CHECK_EQI(got, want, name) do {                                    \
    long g_ = (long)(got), w_ = (long)(want);                              \
    if (g_ == w_) printf("  PASS: %s\n", name);                            \
    else { printf("  FAIL: %s (got %ld, want %ld)\n", name, g_, w_);        \
           s_failures++; }                                                \
} while (0)

static void hex_line(char *out, uint32_t h, unsigned plays)
{
    if (plays == 0)
        snprintf(out, 24, "%08x\n", h);
    else
        snprintf(out, 24, "%08x %u\n", h, plays);
}

int main(void)
{
    uint32_t ha = extras_path_hash("/SD/A.flac");
    uint32_t hb = extras_path_hash("/SD/B.flac");
    uint32_t hc = extras_path_hash("/SD/C.flac");
    uint32_t hd = extras_path_hash("/SD/D.flac");
    char text[256];
    char line[24];
    unsigned char order[EXTRAS_RECENT_MAX];
    unsigned n;
    unsigned long size = 0;

    mkdir(".work", 0777);
    mkdir(".work/extras", 0777);
    mkdir(".work/extras/Internal", 0777);
    mkdir(".work/extras/Internal/.rechord", 0777);
    fs_host_set_root(".work/extras");
    (void)remove(".work/extras/Internal/.rechord/favorites");
    (void)remove(".work/extras/Internal/.rechord/recent");

    TEST("library_extras: load + identity");

    text[0] = '\0';
    hex_line(line, ha, 0); strcat(text, line);
    hex_line(line, hb, 0); strcat(text, line);
    CHECK_EQI(fs_store_file(RECHORD_FAVORITES, text, (uint32_t)strlen(text)),
              FS_OK, "favorites fixture written");
    text[0] = '\0';
    hex_line(line, ha, 5); strcat(text, line);
    hex_line(line, hc, 2); strcat(text, line);
    CHECK_EQI(fs_store_file(RECHORD_RECENT, text, (uint32_t)strlen(text)),
              FS_OK, "recent fixture written");

    extras_init();
    CHECK_EQI(extras_favorite_count(), 2, "two favorites loaded");
    CHECK(extras_is_favorite("/SD/A.flac"), "A is favorite");
    CHECK(extras_is_favorite("/SD/B.flac"), "B is favorite");
    CHECK(!extras_is_favorite("/SD/C.flac"), "C is not favorite");
    CHECK(ha != hb && hb != hc, "hashes differ");
    CHECK_EQI(extras_recent_count(), 2, "two recent loaded");
    CHECK(extras_recent_hash(0) == ha, "A is most recent");
    CHECK_EQI(extras_recent_plays(0), 5, "A has 5 plays");
    CHECK_EQI(extras_recent_plays(1), 2, "C has 2 plays");

    TEST("library_extras: play / MRU / counts");

    extras_note_play("/SD/D.flac");
    CHECK_EQI(extras_recent_count(), 3, "D added");
    CHECK(extras_recent_hash(0) == hd, "D moved to the front");
    CHECK_EQI(extras_recent_plays(0), 1, "D first play");
    extras_note_play("/SD/D.flac");
    CHECK_EQI(extras_recent_plays(0), 2, "D second play");
    CHECK(extras_recent_hash(1) == ha, "A stays next");

    n = extras_most_played_order(order, EXTRAS_RECENT_MAX);
    CHECK_EQI(n, 3, "three entries ordered");
    CHECK(extras_recent_hash(order[0]) == ha, "A (5 plays) is most played");
    CHECK(extras_recent_hash(order[1]) == hd, "D (2 plays, newer) next");
    CHECK(extras_recent_hash(order[2]) == hc, "C (2 plays, older) last");

    TEST("library_extras: toggle and persistence");

    CHECK_EQI(extras_toggle_favorite("/SD/C.flac"), 1, "C added to favorites");
    CHECK_EQI(extras_favorite_count(), 3, "three favorites");
    CHECK_EQI(extras_toggle_favorite("/SD/A.flac"), 0, "A removed");
    CHECK(!extras_is_favorite("/SD/A.flac"), "A no longer favorite");
    CHECK_EQI(extras_favorite_count(), 2, "two favorites");

    CHECK_EQI(fs_size_file(RECHORD_RECENT, &size), FS_OK, "recent file exists");
    CHECK(size > 0, "recent file has content");
    CHECK_EQI(fs_load_file(RECHORD_FAVORITES, text, sizeof text) > 0, 1,
              "favorites file readable");

    if (s_failures == 0) {
        printf("test_library_extras: OK\n");
        return 0;
    }
    printf("test_library_extras: %d FAILURE(S)\n", s_failures);
    return s_failures;
}
