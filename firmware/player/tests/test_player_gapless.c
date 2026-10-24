/*
 * test_player_gapless.c — gapless playback host tests.
 *
 * Covers: (1) leading encoder-delay trim, (2) trailing padding trim, and
 * (3) seamless same-format auto-advance WITHOUT a B-core restart.
 */
#include "player.h"
#include "playlist.h"
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

/* BRNOMT-73: the player pre-opens the NEXT file ~3 s before the end, exactly
 * once per track (a 2 s fixture is inside the window from the first pass). */
static void test_preopen_trigger(void)
{
    mock_codec_cfg_t cfg = {0};

    cfg.total_frames = 44100 * 2;      /* 2 s */
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    ipc_b_mock_set_b_decodes(1);
    ipc_b_mock_set_b_eof(0);

    CHECK_EQI(player_open("a.mock"), PLAYER_OK, "preopen: open A");
    player_enqueue("b.mock");
    CHECK_EQI(player_play(), PLAYER_OK, "preopen: play A");

    CHECK_EQI(player_process(), PLAYER_OK, "preopen: first process");
    CHECK_EQI(ipc_b_preopen_count(), 1, "preopen: asked for the next file");
    CHECK_EQI(player_process(), PLAYER_OK, "preopen: second process");
    CHECK_EQI(ipc_b_preopen_count(), 1, "preopen: only once per track");

    ipc_b_mock_set_b_decodes(0);
    player_stop();
}

/* BRNOMT-73 pending open: the next track's codec is opened during the pre-open
 * window, so the boundary only adopts its metadata (no AP-side re-open). A
 * user seek inside that window must still act on the PLAYING track. */
static void test_pending_open_adopt(void)
{
    mock_codec_cfg_t cfg = {0};
    track_info_t ti;
    int start0;

    cfg.total_frames = 44100 * 2;      /* 2 s -> inside the pre-open window */
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    ipc_b_mock_set_b_decodes(1);
    ipc_b_mock_set_b_eof(0);

    CHECK_EQI(player_open("a.mock"), PLAYER_OK, "pending: open A");
    player_enqueue("b.mock");
    CHECK_EQI(player_play(), PLAYER_OK, "pending: play A");
    start0 = ipc_b_start_count();

    CHECK_EQI(player_process(), PLAYER_OK, "pending: pre-open pass");
    CHECK_EQI(ipc_b_preopen_count(), 1, "pending: next file pre-opened");

    /* A seek during the window must re-target the PLAYING track, not the
     * pre-opened next one. */
    CHECK_EQI(player_seek(1000), PLAYER_OK, "pending: seek ok");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "pending: track after seek");
    CHECK(strcmp(ti.path, "a.mock") == 0, "pending: seek kept track A");

    /* Boundary: adopt the pre-opened codec, no storage access, no restart. */
    ipc_b_mock_set_b_eof(1);
    CHECK_EQI(player_process(), PLAYER_OK, "pending: boundary");
    CHECK_EQI(ipc_b_next_count(), 1, "pending: ipc_b_next once");
    CHECK_EQI(ipc_b_start_count(), start0, "pending: no B-core restart");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "pending: track query");
    CHECK(strcmp(ti.path, "b.mock") == 0, "pending: adopted track B");
    CHECK_EQI(ti.total_samples, 44100 * 2, "pending: adopted total samples");

    ipc_b_mock_set_b_decodes(0);
    ipc_b_mock_set_b_eof(0);
    player_stop();
}

/* BRNOMT-73: the playlist resolves the next record index during the pre-open
 * window (a media-DB lookup on the target); the boundary must REUSE that
 * resolution instead of repeating it inside the ring's cover. */
static int g_resolve_calls;

static int test_resolve(uint16_t idx, char *path, int cap)
{
    g_resolve_calls++;
    path[0] = 't';
    path[1] = (char)('0' + (int)(idx % 10));
    path[2] = '\0';
    strcpy(path + 2, ".mock");
    (void)cap;
    return 0;
}

static void test_playlist_resolve_cache(void)
{
    static const uint16_t idx[2] = {0, 1};
    mock_codec_cfg_t cfg = {0};
    int start0;

    cfg.total_frames = 44100 * 2;      /* 2 s -> inside the pre-open window */
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    ipc_b_mock_set_b_decodes(1);
    ipc_b_mock_set_b_eof(0);
    player_set_resolver(test_resolve);

    CHECK_EQI(player_play_list(idx, 2, 0), PLAYER_OK, "plcache: start list");
    start0 = ipc_b_start_count();
    g_resolve_calls = 0;               /* count only post-start resolutions */

    CHECK_EQI(player_process(), PLAYER_OK, "plcache: pre-open pass");
    CHECK_EQI(g_resolve_calls, 1, "plcache: resolved once for the pre-open");

    ipc_b_mock_set_b_eof(1);
    CHECK_EQI(player_process(), PLAYER_OK, "plcache: boundary");
    CHECK_EQI(ipc_b_next_count(), 1, "plcache: gapless hand-off");
    CHECK_EQI(ipc_b_start_count(), start0, "plcache: no B-core restart");
    CHECK_EQI(g_resolve_calls, 1, "plcache: boundary reused the resolution");

    ipc_b_mock_set_b_decodes(0);
    ipc_b_mock_set_b_eof(0);
    player_stop();
}

/* BRNOMT-90: the auto-advance must honour the repeat mode — ONE replays the
 * same track, ALL wraps from the last track to the first, OFF stops at the
 * end of the list. (Field report: repeat "one" still changed the track.) */
static void test_repeat_modes(void)
{
    static const uint16_t idx[3] = {0, 1, 2};
    mock_codec_cfg_t cfg = {0};
    track_info_t ti;

    cfg.total_frames = 44100 * 2;
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    ipc_b_mock_set_b_decodes(1);
    ipc_b_mock_set_b_eof(0);
    player_set_resolver(test_resolve);

    /* repeat ONE: the boundary replays t0, it does not move to t1. */
    CHECK_EQI(player_play_list(idx, 3, 0), PLAYER_OK, "repeat: start list");
    player_set_repeat(PLAYLIST_REPEAT_ONE);
    CHECK_EQI(player_process(), PLAYER_OK, "repeat: one pre-open");
    ipc_b_mock_set_b_eof(1);
    CHECK_EQI(player_process(), PLAYER_OK, "repeat: one boundary");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "repeat: one query");
    CHECK(strcmp(ti.path, "t0.mock") == 0, "repeat: one replays the same track");

    /* repeat ALL: start on the LAST track, the boundary wraps to the first. */
    ipc_b_mock_set_b_eof(0);
    CHECK_EQI(player_play_list(idx, 3, 2), PLAYER_OK, "repeat: play last");
    player_set_repeat(PLAYLIST_REPEAT_ALL);
    CHECK_EQI(player_process(), PLAYER_OK, "repeat: all pre-open");
    ipc_b_mock_set_b_eof(1);
    CHECK_EQI(player_process(), PLAYER_OK, "repeat: all boundary");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "repeat: all query");
    CHECK(strcmp(ti.path, "t0.mock") == 0, "repeat: all wraps to the first");

    /* repeat OFF: on the last track the list ends (no advance). */
    ipc_b_mock_set_b_eof(0);
    CHECK_EQI(player_play_list(idx, 3, 2), PLAYER_OK, "repeat: play last off");
    player_set_repeat(PLAYLIST_REPEAT_OFF);
    CHECK_EQI(player_process(), PLAYER_OK, "repeat: off pre-open");
    ipc_b_mock_set_b_eof(1);
    (void)player_process();
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "repeat: off query");
    CHECK(strcmp(ti.path, "t2.mock") == 0, "repeat: off stays on the last");

    ipc_b_mock_set_b_decodes(0);
    ipc_b_mock_set_b_eof(0);
    player_stop();
}

/* BRNOMT-87 resume: a seek made while STOPPED must reach the NEW stream — the
 * restored track plays from pos_ms, not from 0. */
static void test_stopped_seek_then_play(void)
{
    mock_codec_cfg_t cfg = {0};

    cfg.total_frames = 44100 * 4;      /* 4 s */
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    ipc_b_mock_set_b_decodes(0);
    ipc_b_mock_set_b_eof(0);

    CHECK_EQI(player_open("resume.mock"), PLAYER_OK, "resume: open");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "resume: opened stopped");
    CHECK_EQI(player_seek(2000), PLAYER_OK, "resume: seek while stopped");
    CHECK_EQI(player_play(), PLAYER_OK, "resume: play");
    CHECK_EQI(player_get_position_ms(), 2000,
              "resume: starts at the saved position");
    player_stop();
}

/* Resume queue: player_load_list opens the start track and leaves the player
 * STOPPED; the caller sets the origin and a single-track open clears it. */
static void test_load_list_paused(void)
{
    static const uint16_t idx[3] = { 0, 1, 2 };
    mock_codec_cfg_t cfg = {0};
    track_info_t ti;
    int kind = -1, cat = -1;
    char val[16];

    cfg.total_frames = 44100;
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
    player_set_resolver(test_resolve);
    ipc_b_mock_set_b_decodes(0);
    ipc_b_mock_set_b_eof(0);

    CHECK_EQI(player_load_list(idx, 3, 1), PLAYER_OK, "queue: load list");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "queue: leaves STOPPED");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "queue: track query");
    CHECK(strcmp(ti.path, "t1.mock") == 0, "queue: opens the start track");
    CHECK_EQI(player_get_position_ms(), 0, "queue: paused at zero");

    player_set_queue_source(PLAYER_QUEUE_CAT, "Blue", 3);
    player_get_queue_source(&kind, val, (int)sizeof val, &cat);
    CHECK_EQI(kind, PLAYER_QUEUE_CAT, "queue: source kind");
    CHECK_EQI(cat, 3, "queue: source cat");
    CHECK(strcmp(val, "Blue") == 0, "queue: source value");

    CHECK_EQI(player_open("t0.mock"), PLAYER_OK, "queue: single open");
    player_get_queue_source(&kind, val, (int)sizeof val, &cat);
    CHECK_EQI(kind, PLAYER_QUEUE_NONE, "queue: single open clears source");
}

int main(void)
{
    TEST("player gapless");
    test_gapless_trim();
    test_gapless_continuity();
    test_gapless_core_b();
    test_gapless_core_b_format_change();
    test_preopen_trigger();
    test_pending_open_adopt();
    test_playlist_resolve_cache();
    test_repeat_modes();
    test_stopped_seek_then_play();
    test_load_list_paused();
    if (failures == 0) {
        printf("test_player_gapless: OK\n");
        return 0;
    }
    printf("test_player_gapless: %d FAILURE(S)\n", failures);
    return failures;
}
