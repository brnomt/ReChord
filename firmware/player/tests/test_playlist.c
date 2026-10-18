/*
 * test_playlist.c — host tests for the BRNOMT-90 playlist (pure logic).
 */
#include <stdio.h>
#include <string.h>

#include "../playlist.h"

static int fails;
#define CHECK(c, msg) do { if (!(c)) { printf("FAIL: %s\n", msg); fails++; } } while (0)

static void test_order_and_advance(void)
{
    playlist_t pl;
    const uint16_t idx[5] = { 10, 20, 30, 40, 50 };

    memset(&pl, 0, sizeof pl);
    playlist_set_repeat(&pl, PLAYLIST_REPEAT_OFF);
    CHECK(playlist_load(&pl, idx, 5, 2) == 5, "load count");
    CHECK(playlist_count(&pl) == 5, "count");
    CHECK(playlist_position(&pl) == 3, "position is 1-based");
    CHECK(playlist_current(&pl) == 30, "start track");

    CHECK(playlist_advance(&pl) == 3 && playlist_current(&pl) == 40, "advance 3");
    CHECK(playlist_advance(&pl) == 4 && playlist_current(&pl) == 50, "advance 4");
    CHECK(playlist_advance(&pl) == -1, "end of list stops (repeat off)");
    CHECK(playlist_current(&pl) == 50, "stop stays on the last track");

    playlist_set_repeat(&pl, PLAYLIST_REPEAT_ALL);
    CHECK(playlist_advance(&pl) == 0 && playlist_current(&pl) == 10,
          "repeat all wraps");

    playlist_set_repeat(&pl, PLAYLIST_REPEAT_ONE);
    CHECK(playlist_advance(&pl) == 0, "repeat one keeps the position");
    CHECK(playlist_current(&pl) == 10, "repeat one replays the same track");
}

static void test_shuffle(void)
{
    playlist_t pl;
    uint16_t idx[8];
    uint16_t seen_order[8];
    int i, j;

    for (i = 0; i < 8; i++)
        idx[i] = (uint16_t)(100 + i);

    memset(&pl, 0, sizeof pl);
    playlist_set_repeat(&pl, PLAYLIST_REPEAT_OFF);
    playlist_seed(&pl, 0xC0FFEEu);
    playlist_load(&pl, idx, 8, 5);           /* start on 105 */
    CHECK(playlist_current(&pl) == 105, "unshuffled start");

    playlist_set_shuffle(&pl, 1);
    CHECK(pl.shuffle == 1, "shuffle on");
    CHECK(playlist_count(&pl) == 8, "count preserved by shuffle");
    CHECK(playlist_position(&pl) == 1, "current moves to the front");
    CHECK(playlist_current(&pl) == 105, "current track keeps playing");

    /* Walk the whole shuffled order: every original track exactly once. */
    seen_order[0] = playlist_current(&pl);
    for (i = 1; i < 8; i++) {
        int p = playlist_advance(&pl);

        CHECK(p == i, "advance walks the shuffled order");
        seen_order[i] = playlist_current(&pl);
    }
    for (i = 0; i < 8; i++) {
        int dup = 0;

        for (j = 0; j < 8; j++)
            if (j != i && seen_order[j] == seen_order[i])
                dup = 1;
        CHECK(!dup, "no duplicate in the shuffled order");
        CHECK(seen_order[i] >= 100 && seen_order[i] < 108, "value in range");
    }

    /* Turning shuffle off restores the user order and re-points the position. */
    {
        uint16_t cur = playlist_current(&pl);

        playlist_set_shuffle(&pl, 0);
        CHECK(pl.shuffle == 0, "shuffle off");
        for (i = 0; i < 8; i++)
            CHECK(pl.order[i] == idx[i], "user order restored");
        CHECK(playlist_current(&pl) == cur, "still on the same track");
    }
}

static void test_empty(void)
{
    playlist_t pl;

    memset(&pl, 0, sizeof pl);
    CHECK(playlist_load(&pl, 0, 0, 0) == 0, "empty load");
    CHECK(playlist_current(&pl) == PLAYLIST_IDX_NONE, "no current");
    CHECK(playlist_advance(&pl) == -1, "advance on empty stops");
}

static void test_has_next(void)
{
    playlist_t pl;
    const uint16_t idx[3] = { 1, 2, 3 };

    memset(&pl, 0, sizeof pl);
    playlist_set_repeat(&pl, PLAYLIST_REPEAT_OFF);
    playlist_load(&pl, idx, 3, 0);
    CHECK(playlist_has_next(&pl) == 1, "has_next: mid-list");
    playlist_advance(&pl);
    CHECK(playlist_has_next(&pl) == 1, "has_next: second track");
    playlist_advance(&pl);               /* now on the last track */
    CHECK(playlist_current(&pl) == 3, "has_next: on the last track");
    CHECK(playlist_has_next(&pl) == 0, "has_next: last, repeat off");

    playlist_set_repeat(&pl, PLAYLIST_REPEAT_ALL);
    CHECK(playlist_has_next(&pl) == 1, "has_next: repeat all wraps");

    playlist_set_repeat(&pl, PLAYLIST_REPEAT_ONE);
    CHECK(playlist_has_next(&pl) == 1, "has_next: repeat one replays");

    playlist_reset(&pl);
    CHECK(playlist_has_next(&pl) == 0, "has_next: empty list");
}

static void test_peek_next(void)
{
    playlist_t pl;
    const uint16_t idx[3] = { 10, 20, 30 };
    uint16_t n = 0;

    memset(&pl, 0, sizeof pl);
    playlist_set_repeat(&pl, PLAYLIST_REPEAT_OFF);
    playlist_load(&pl, idx, 3, 0);
    CHECK(playlist_peek_next(&pl, &n) == 1 && n == 20, "peek: next is 20");
    CHECK(playlist_position(&pl) == 1, "peek does not move the list");
    playlist_advance(&pl);
    CHECK(playlist_peek_next(&pl, &n) == 1 && n == 30, "peek: next is 30");
    playlist_advance(&pl);               /* last track, repeat off */
    CHECK(playlist_peek_next(&pl, &n) == 0, "peek: end of list");

    playlist_set_repeat(&pl, PLAYLIST_REPEAT_ALL);
    CHECK(playlist_peek_next(&pl, &n) == 1 && n == 10,
          "peek: repeat all wraps");
    playlist_set_repeat(&pl, PLAYLIST_REPEAT_ONE);
    CHECK(playlist_peek_next(&pl, &n) == 1 && n == 30, "peek: repeat one");

    playlist_reset(&pl);
    CHECK(playlist_peek_next(&pl, &n) == 0, "peek: empty list");
}

int main(void)
{
    test_order_and_advance();
    test_shuffle();
    test_empty();
    test_has_next();
    test_peek_next();

    if (fails != 0) {
        printf("test_playlist: %d FAILURES\n", fails);
        return 1;
    }
    printf("test_playlist: OK\n");
    return 0;
}
