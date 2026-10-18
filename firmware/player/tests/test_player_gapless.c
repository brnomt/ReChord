/*
 * test_player_gapless.c — gapless playback host tests.
 *
 * Covers: (1) leading encoder-delay trim, (2) trailing padding trim, and
 * (3) seamless same-format auto-advance WITHOUT a B-core restart.
 */
#include "player.h"
#include "mock_codec.h"
#include "ipc_b_mock.h"
#include "ipc_b.h"
#include "test_util.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int      g_count;
static int32_t *g_sink;
static int      g_sink_cap;

static void sink_cb(const int32_t *pcm, int frames, void *u)
{
    int i;
    (void)u;
    for (i = 0; i < frames * 2; i++) {
        if (g_count < g_sink_cap)
            g_sink[g_count++] = pcm[i];
    }
}

static void drain_to_stop(unsigned max_iters)
{
    unsigned it = 0;
    while (player_get_state() == PLAYER_PLAYING && it < max_iters) {
        player_process();
        it++;
    }
}

/* Trim test: delay=100, padding=100, total=44100 -> exactly 44000 emitted,
 * and the emitted samples equal mock_sample(100..44049) in order. */
static void test_gapless_trim(void)
{
    mock_codec_cfg_t cfg = {0};
    track_info_t ti;
    int i;

    cfg.total_frames = 44100;
    cfg.gapless_delay = 100;
    cfg.gapless_padding = 100;
    cfg.fail_at_frame = -1;            /* no decode failures */
    cfg.rg_track_gain_tenth_db = 0;
    mock_codec_configure(&cfg);

    ipc_b_mock_set_frames_per_tick(512);
    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());

    g_count = 0;
    g_sink_cap = 44000 * 2;
    g_sink = (int32_t *)malloc(sizeof(int32_t) * g_sink_cap);

    ipc_b_mock_set_sink(sink_cb, 0);

    if (player_open("trim.mock") != PLAYER_OK) {
        failures++; printf("  FAIL: trim: player_open\n"); goto out;
    }
    player_get_track(&ti);
    CHECK_EQI(ti.gapless_delay, 100, "trim: track reports delay");
    CHECK_EQI(ti.gapless_padding, 100, "trim: track reports padding");

    player_play();
    drain_to_stop(200000);

    /* 44100 total frames, 100 encoder-delay + 100 padding trimmed -> 43900 emitted. */
    CHECK_EQI(g_count, (44100 - 100 - 100) * 2,
              "trim: emitted total - delay - padding");
    for (i = 0; i < (44100 - 100 - 100) && i * 2 < g_count - 1; i++) {
        int32_t L = mock_sample(100 + i, 0);
        int32_t R = mock_sample(100 + i, 1);
        if (g_sink[i * 2] != L || g_sink[i * 2 + 1] != R) {
            printf("  FAIL: trim: frame %d = %d,%d want %d,%d\n",
                   i, g_sink[i*2], g_sink[i*2+1], L, R);
            failures++;
            break;
        }
    }
    if (i == (44100 - 100 - 100))
        CHECK(1, "trim: contiguous post-delay samples");

out:
    ipc_b_mock_set_sink(0, 0);
    free(g_sink);
    g_sink = 0;
}

/* Seamless advance: two same-format tracks (delay=0,pad=0,total=44100).
 * Auto-advance must NOT restart the B core (start_count unchanged) and the
 * sink must see 2*44100 frames with no gap between them. */
static void test_gapless_continuity(void)
{
    mock_codec_cfg_t cfg = {0};
    int start0, consumed, i;

    cfg.total_frames = 44100;
    cfg.gapless_delay = 0;
    cfg.gapless_padding = 0;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_mock_set_frames_per_tick(512);
    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());

    g_count = 0;
    g_sink_cap = 44100 * 2 * 2;
    g_sink = (int32_t *)malloc(sizeof(int32_t) * g_sink_cap);

    ipc_b_mock_set_sink(sink_cb, 0);

    if (player_open("a.mock") != PLAYER_OK) {
        failures++; printf("  FAIL: continuity: player_open\n"); goto out;
    }
    player_enqueue("b.mock");             /* enqueue both (only first plays) */
    player_play();
    start0 = ipc_b_start_count();

    drain_to_stop(400000);

    CHECK_EQI(ipc_b_start_count(), start0,
              "continuity: no B-core restart across boundary");

    consumed = g_count / 2;               /* frames, both channels */
    CHECK_EQI(consumed, 44100 * 2, "continuity: consumed both tracks");

    /* Track A's last frame must be immediately followed by B's first frame
     * (mock_sample(0)); no silence was inserted at the boundary. */
    CHECK(g_sink[(44100 - 1) * 2] == mock_sample(44100 - 1, 0),
          "continuity: A last frame intact");
    CHECK(g_sink[44100 * 2] == mock_sample(0, 0),
          "continuity: B first frame = mock_sample(0), no gap");

    /* Spot-check content continuity across the 44099 -> 0 boundary. */
    for (i = 0; i < 44100; i++) {
        if (g_sink[i * 2] != mock_sample(i, 0) ||
            g_sink[i * 2 + 1] != mock_sample(i, 1)) {
            printf("  FAIL: continuity: A frame %d mismatch\n", i);
            failures++;
            break;
        }
    }
    if (i == 44100)
        CHECK(1, "continuity: track A content byte-identical");

out:
    ipc_b_mock_set_sink(0, 0);
    free(g_sink);
    g_sink = 0;
}

/* Core B gapless (BRNOMT-73): at B's decoder EOF the player hands the next
 * track to B on the LIVE ring (ipc_b_next), NOT a core restart, and does not
 * wait for the ring to empty. */
static void test_gapless_core_b(void)
{
    mock_codec_cfg_t cfg = {0};
    track_info_t ti;
    int start0;

    cfg.total_frames = 44100;
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.gapless_delay = 0;
    cfg.gapless_padding = 0;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    ipc_b_mock_set_b_decodes(1);       /* pretend Core B is the decoder */
    ipc_b_mock_set_b_eof(0);

    CHECK_EQI(player_open("a.mock"), PLAYER_OK, "core-b: open A");
    player_enqueue("b.mock");
    CHECK_EQI(player_play(), PLAYER_OK, "core-b: play A");
    start0 = ipc_b_start_count();

    ipc_b_mock_set_b_eof(1);           /* "B's decoder drained" */
    CHECK_EQI(player_process(), PLAYER_OK, "core-b: process at B EOF");
    CHECK_EQI(ipc_b_next_count(), 1, "core-b: ipc_b_next called once");
    CHECK_EQI(ipc_b_start_count(), start0, "core-b: no B-core restart");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING, "core-b: still playing");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "core-b: track query");
    CHECK(strcmp(ti.path, "b.mock") == 0, "core-b: advanced to B");

    ipc_b_mock_set_b_decodes(0);
    ipc_b_mock_set_b_eof(0);
    player_stop();
}

/* Core B format change: the boundary must be hard, but the stop/start is
 * DEFERRED until the ring drains so the previous track's tail is not cut. */
static void test_gapless_core_b_format_change(void)
{
    mock_codec_cfg_t cfg = {0};
    int start0;

    cfg.total_frames = 44100;
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    ipc_b_mock_set_b_decodes(1);
    ipc_b_mock_set_b_eof(0);

    CHECK_EQI(player_open("a.mock"), PLAYER_OK, "core-b fmt: open A");
    player_enqueue("b.mock");
    player_play();
    start0 = ipc_b_start_count();

    /* The next track has a different rate. */
    cfg.sample_rate = 48000;
    mock_codec_configure(&cfg);

    ipc_b_mock_set_b_eof(1);
    CHECK_EQI(player_process(), PLAYER_OK, "core-b fmt: EOF hand-off");
    CHECK_EQI(ipc_b_start_count(), start0, "core-b fmt: restart deferred");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING, "core-b fmt: still playing");

    /* Ring is empty (B idle) -> the deferred restart now runs. */
    CHECK_EQI(player_process(), PLAYER_OK, "core-b fmt: deferred restart");
    CHECK_EQI(ipc_b_start_count(), start0 + 1, "core-b fmt: B restarted");

    ipc_b_mock_set_b_decodes(0);
    ipc_b_mock_set_b_eof(0);
    player_stop();
}

int main(void)
{
    TEST("player gapless");
    test_gapless_trim();
    test_gapless_continuity();
    test_gapless_core_b();
    test_gapless_core_b_format_change();
    if (failures == 0) {
        printf("test_player_gapless: OK\n");
        return 0;
    }
    printf("test_player_gapless: %d FAILURE(S)\n", failures);
    return failures;
}
