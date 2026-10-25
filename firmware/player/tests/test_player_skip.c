/*
 * test_player_skip.c — an unplayable file must not kill playback.
 *
 * Two field regressions (BRNOMT-106, "format not supported" after 1-2 shuffled
 * tracks):
 *   1. A pre-open failure must be non-fatal: the current track keeps playing.
 *   2. At the boundary, advance_playlist() must skip the bad entry and play
 *      the next one instead of stopping with an error.
 */
#include "player.h"
#include "mock_codec.h"
#include "ipc_b_mock.h"
#include "ipc_b.h"
#include "test_util.h"

#include <stdio.h>
#include <string.h>

static const char *k_paths[] = { "a.mock", "b.mock", "c.mock" };

static int resolver(uint16_t idx, char *path, int cap)
{
    if (idx >= 3)
        return -1;
    snprintf(path, (size_t)cap, "%s", k_paths[idx]);
    return 0;
}

static void fresh_player(void)
{
    mock_codec_cfg_t cfg = {0};

    cfg.total_frames = 4096;         /* duration < 3 s: pre-open immediately */
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    strcpy(cfg.fail_open_match, "b.mock");   /* one bad file in the middle */
    mock_codec_configure(&cfg);

    ipc_b_mock_set_frames_per_tick(512);
    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    player_set_resolver(resolver);
}

static void drain_to_stop(unsigned max_iters)
{
    unsigned it = 0;

    while (player_get_state() == PLAYER_PLAYING && it < max_iters) {
        player_process();
        it++;
    }
}

/* AP-decodes path: the boundary opens b.mock, fails, skips to c.mock. */
static void test_skip_on_advance(void)
{
    uint16_t list[3] = { 0, 1, 2 };
    track_info_t ti;

    fresh_player();

    CHECK_EQI(player_play_list(list, 3, 0), PLAYER_OK, "skip: play list");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING, "skip: A playing");

    drain_to_stop(200000);

    CHECK_EQI(player_get_state(), PLAYER_STOPPED,
              "skip: list ended without an error");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "skip: track query");
    CHECK(strcmp(ti.path, "c.mock") == 0,
          "skip: unplayable B skipped, played C");
}

/* B-decodes path: pre-open B fails; A must keep playing (no ERROR), and the
 * B-EOF hand-off skips to C. */
static void test_preopen_failure_is_not_fatal(void)
{
    uint16_t list[3] = { 0, 1, 2 };
    track_info_t ti;

    fresh_player();
    ipc_b_mock_set_b_decodes(1);
    ipc_b_mock_set_b_eof(0);

    CHECK_EQI(player_play_list(list, 3, 0), PLAYER_OK, "preopen: play list");

    /* The mock reports a short track, so this pass pre-opens B and fails. */
    CHECK_EQI(player_process(), PLAYER_OK, "preopen: process");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING,
              "preopen: bad next track does not stop playback");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "preopen: still has A");
    CHECK(strcmp(ti.path, "a.mock") == 0, "preopen: A still current");

    /* B drains: the hand-off skips B and hands C to the live stream. */
    ipc_b_mock_set_b_eof(1);
    CHECK_EQI(player_process(), PLAYER_OK, "preopen: B EOF process");
    CHECK_EQI(ipc_b_next_count(), 1, "preopen: hand-off to C");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "preopen: track query");
    CHECK(strcmp(ti.path, "c.mock") == 0, "preopen: skipped to C");
}

int main(void)
{
    TEST("player skip unplayable");
    test_skip_on_advance();
    test_preopen_failure_is_not_fatal();
    if (failures == 0) {
        printf("test_player_skip: OK\n");
        return 0;
    }
    printf("test_player_skip: %d FAILURE(S)\n", failures);
    return failures;
}
