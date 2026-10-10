/*
 * test_ipc_mock.c — end-to-end host test: player -> codec -> ring ->
 * simulated B core (ipc_b_mock.c) consuming PCM.
 *
 * Verifies the documented streaming story: "play -> B consumes -> position
 * advances", PCM content integrity through the ring (frame-exact against
 * the mock generator), the /1 -> /4 clock-negotiation sequence, volume
 * passthrough, pause/resume, seek/flush semantics, and the "B is gone"
 * fault path.
 */
#include <stdio.h>
#include <string.h>

#include "../player.h"
#include "../ipc_b.h"
#include "../ipc_b_mock.h"
#include "mock_codec.h"
#include "test_util.h"

/* Everything the "B core" ever played, for frame-exact content checks.
 * Extra polls/ticks only reorder test steps, never the consumed stream. */
#define CAP_MAX 100000
static int16_t g_cap[CAP_MAX * 2];
static long g_cap_frames = 0;   /* frames the B core consumed in total  */

static void on_sink(const int16_t *pcm, int frames, void *user)
{
    int i;
    (void)user;
    for (i = 0; i < frames && g_cap_frames < CAP_MAX; i++, g_cap_frames++) {
        g_cap[g_cap_frames * 2]     = pcm[i * 2];
        g_cap[g_cap_frames * 2 + 1] = pcm[i * 2 + 1];
    }
}

static void cap_reset(void)
{
    g_cap_frames = 0;
}

static int content_ok(long from_frame, long count, long mock_offset)
{
    long i;
    for (i = 0; i < count; i++) {
        if (g_cap[(from_frame + i) * 2] != mock_sample(mock_offset + i, 0))
            return 0;
        if (g_cap[(from_frame + i) * 2 + 1] !=
            mock_sample(mock_offset + i, 1))
            return 0;
    }
    return 1;
}

int main(void)
{
    mock_codec_cfg_t cfg;
    ipc_b_status_t st;
    long pos, prev, max_pos;
    int i, rc;

    memset(&cfg, 0, sizeof(cfg));
    cfg.total_frames = 88200;      /* 2 s @ 44100 Hz stereo */
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    player_init();
    player_register_codec("mock", mock_codec_get());
    ipc_b_mock_set_sink(on_sink, 0);
    ipc_b_mock_set_frames_per_tick(256);
    cap_reset();

    TEST("streaming e2e: play -> B consumes -> position advances");
    CHECK_EQI(player_open("e2e.mock"), PLAYER_OK, "open e2e track");
    CHECK_EQI(player_play(), PLAYER_OK, "play");

    /* Documented startup: "player: B start 0 at 400 MHz" (div 1), then
     * the negotiation "B /1 -> /4 (100 MHz...)" on the next tick. */
    CHECK_EQI(ipc_b_poll_state(&st), IPC_B_RUNNING,
              "B core RUNNING after first poll");
    CHECK_EQI(st.clock_div, 1, "started at /1 (400 MHz)");
    CHECK_EQI(ipc_b_poll_state(&st), IPC_B_RUNNING, "still RUNNING");
    CHECK_EQI(st.clock_div, 4, "negotiated to /4 (100 MHz)");
    CHECK_EQI(st.volume_db, player_get_volume(), "volume visible to B core");

    CHECK_EQI(player_set_volume(-23), PLAYER_OK, "set volume -23 dB");
    ipc_b_poll_state(&st);
    CHECK_EQI(st.state, IPC_B_RUNNING, "poll after vol");
    CHECK_EQI(st.volume_db, -23, "volume passthrough (-23 dB)");

    prev = player_get_position_ms();
    max_pos = prev;
    rc = PLAYER_OK;
    for (i = 0; i < 3000 && rc == PLAYER_OK &&
                player_get_state() == PLAYER_PLAYING; i++) {
        rc = player_process();
        pos = player_get_position_ms();
        if (pos < prev)
            break;                    /* position must never rewind */
        prev = pos;
        if (pos > max_pos)
            max_pos = pos;
    }
    CHECK(rc == PLAYER_OK, "clean drain of the full 2 s track");
    CHECK(i < 3000, "finished well inside the iteration budget");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED,
              "stopped after end of track");
    CHECK_EQI(g_cap_frames, 88200, "B core consumed every decoded frame");
    CHECK(max_pos >= 1990 && max_pos <= 2000,
          "position walked up to the 2000 ms end");
    CHECK(content_ok(0, g_cap_frames, 0),
          "PCM through the ring is frame-exact vs the mock generator");
    CHECK_EQI(player_get_position_ms(), 0, "rewound after stop");

    TEST("ring counters advance (\"ring rd %u wr %u\")");
    /* Re-play briefly and watch both counters move. */
    CHECK_EQI(player_seek(0), PLAYER_OK, "rewind track");
    CHECK_EQI(player_play(), PLAYER_OK, "replay");
    {
        long rd0, wr0;
        player_process();
        ipc_b_poll_state(&st);
        rd0 = st.ring_rd;
        wr0 = st.ring_wr;
        for (i = 0; i < 10; i++)
            player_process();
        ipc_b_poll_state(&st);
        CHECK(st.ring_wr > wr0, "ring write counter advanced");
        CHECK(st.ring_rd > rd0, "ring read counter advanced");
    }
    CHECK_EQI(player_stop(), PLAYER_OK, "stop replay");

    TEST("pause freezes the stream, resume continues");
    cap_reset();
    CHECK_EQI(player_open("e2e2.mock"), PLAYER_OK, "open second track");
    CHECK_EQI(player_play(), PLAYER_OK, "play second track");
    for (i = 0; i < 5; i++)
        player_process();
    prev = player_get_position_ms();
    CHECK(prev > 0, "position advanced before pause");
    CHECK(g_cap_frames > 0, "B core consumed before pause");

    CHECK_EQI(player_pause(), PLAYER_OK, "pause");
    {
        long frozen_cap = g_cap_frames;
        for (i = 0; i < 5; i++)
            player_process();          /* no-ops while PAUSED */
        for (i = 0; i < 3; i++)
            ipc_b_poll_state(&st);     /* B idle: must not consume */
        CHECK_EQI(player_get_position_ms(), prev,
                  "position frozen while paused");
        CHECK_EQI(g_cap_frames, frozen_cap, "no PCM consumed while paused");
    }

    CHECK_EQI(player_play(), PLAYER_OK, "resume");
    for (i = 0; i < 5; i++)
        player_process();
    CHECK(player_get_position_ms() > prev, "position advances after resume");
    CHECK(g_cap_frames > 0, "consumption resumed");

    TEST("seek flushes the ring and repositions the stream");
    CHECK_EQI(player_seek(1000), PLAYER_OK, "seek(1000 ms)");
    CHECK_EQI(player_get_position_ms(), 1000, "playhead exactly at 1000 ms");
    CHECK_EQI(ipc_b_poll_state(&st), IPC_B_RUNNING, "B core still running");
    CHECK(st.ring_rd == st.ring_wr, "ring empty after seek flush");
    cap_reset();   /* stream restarts at frame 44100 (= 1000 ms) */
    for (i = 0; i < 5; i++)
        player_process();
    CHECK(g_cap_frames > 0, "streaming resumed after seek");
    CHECK(content_ok(0, (g_cap_frames < 128) ? g_cap_frames : 128, 44100),
          "post-seek PCM matches the mock generator at frame 44100");
    CHECK(player_get_position_ms() > 1000,
          "position counts up from the seek point");

    TEST("B-core fault -> PLAYER_ERROR -> stop recovery");
    ipc_b_mock_set_fault(1);
    CHECK_EQI(player_process(), PLAYER_ERR,
              "process reports the B-core fault");
    CHECK_EQI(player_get_state(), PLAYER_ERROR, "state PLAYER_ERROR");
    CHECK_EQI(player_stop(), PLAYER_OK, "stop recovers");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "back to STOPPED");
    CHECK_EQI(ipc_b_poll_state(&st), IPC_B_OFF, "B core off after stop");

    if (failures == 0)
        printf("test_ipc_mock: ALL PASS\n");
    else
        printf("test_ipc_mock: %d FAILURES\n", failures);
    return failures;
}
