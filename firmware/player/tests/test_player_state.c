/*
 * test_player_state.c — host tests for the playback state machine
 * (player.c) with a mock codec and the mock B-core sink (ipc_b_mock.c).
 *
 * Covers: API guards, open->play->pause->seek->stop transitions, position
 * bookkeeping, end-of-track + next-track queue auto-advance, player_next,
 * decode-error -> PLAYER_ERROR recovery, and volume clamp/passthrough.
 */
#include <stdio.h>
#include <string.h>

#include "../player.h"
#include "mock_codec.h"
#include "test_util.h"

/* ---- event capture ---- */

static int ev_log[256];
static int ev_n;

static void on_event(int ev, void *user)
{
    (void)user;
    if (ev_n < (int)(sizeof(ev_log) / sizeof(ev_log[0])))
        ev_log[ev_n++] = ev;
}

static int ev_count(int ev)
{
    int i, n = 0;
    for (i = 0; i < ev_n; i++)
        if (ev_log[i] == ev)
            n++;
    return n;
}

static void ev_clear(void)
{
    ev_n = 0;
}

int main(void)
{
    mock_codec_cfg_t cfg;
    track_info_t ti;
    long pos, prev;
    int i, rc;

    player_init();
    player_set_callback(on_event, 0);

    TEST("API guards before any track");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "initial state STOPPED");
    CHECK_EQI(player_play(), PLAYER_ERR_EMPTY, "play without track");
    CHECK_EQI(player_pause(), PLAYER_ERR_STATE, "pause without play");
    CHECK_EQI(player_get_track(&ti), PLAYER_ERR_EMPTY, "no track info");
    CHECK_EQI(player_get_position_ms(), 0, "position 0 with no track");
    CHECK_EQI(player_seek(100), PLAYER_ERR_EMPTY, "seek without track");
    CHECK_EQI(player_stop(), PLAYER_OK, "stop is idempotent");

    TEST("codec dispatch by extension");
    CHECK_EQI(player_open("x.unknownext"), PLAYER_ERR_UNSUPPORTED,
              "no codec for extension");
    CHECK_EQI(player_open("x.mp3"), CODEC_ERR_UNSUPPORTED,
              "mp3 stub reports UNSUPPORTED (open failure)");
    CHECK_EQI(player_open(0), PLAYER_ERR, "NULL path rejected");

    /* From here on the mock codec drives the machine. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.total_frames = 44100;      /* 1 s @ 44100 Hz stereo */
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);
    CHECK_EQI(player_register_codec("mock", mock_codec_get()), PLAYER_OK,
              "mock codec registered");

    TEST("open -> track info");
    ev_clear();
    CHECK_EQI(player_open("track1.mock"), PLAYER_OK, "open track1");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "state STOPPED after open");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "track info available");
    CHECK(strcmp(ti.title, "track1") == 0, "title from filename");
    CHECK(strcmp(ti.path, "track1.mock") == 0, "path recorded");
    CHECK_EQI(ti.duration_ms, 1000, "duration 1000 ms");
    CHECK_EQI(ti.track_no, 1, "track_no starts at 1");
    CHECK_EQI(player_get_position_ms(), 0, "position 0 after open");
    CHECK(ev_count(PLAYER_EV_TRACK) == 1, "TRACK event emitted");

    TEST("play / pause / resume");
    ev_clear();
    CHECK_EQI(player_play(), PLAYER_OK, "play");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING, "state PLAYING");
    CHECK_EQI(player_play(), PLAYER_OK, "play is idempotent");
    CHECK_EQI(player_pause(), PLAYER_OK, "pause");
    CHECK_EQI(player_get_state(), PLAYER_PAUSED, "state PAUSED");
    CHECK_EQI(player_pause(), PLAYER_ERR_STATE, "double pause rejected");
    CHECK_EQI(player_play(), PLAYER_OK, "resume from pause");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING, "state PLAYING again");
    CHECK(ev_count(PLAYER_EV_STATE) >= 2, "STATE events emitted");

    TEST("seek + position advances while playing");
    CHECK_EQI(player_seek(250), PLAYER_OK, "seek(250 ms)");
    CHECK_EQI(player_get_position_ms(), 250, "position is exact at seek");
    prev = player_get_position_ms();
    for (i = 0; i < 20; i++) {
        player_process();
        pos = player_get_position_ms();
        if (pos < prev)
            break;               /* position must never go backwards */
        prev = pos;
    }
    CHECK(i == 20, "position monotonic across process() ticks");
    CHECK(prev > 250, "position advanced past the seek point");
    CHECK(prev >= 250 + 100, "advanced >= 100 ms in 20 ticks");
    CHECK(player_get_position_ms() < 1000, "still inside the 1 s track");

    TEST("stop rewinds and keeps the track queryable");
    CHECK_EQI(player_stop(), PLAYER_OK, "stop");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "state STOPPED");
    CHECK_EQI(player_get_position_ms(), 0, "position rewound to 0");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK,
              "track info survives stop");
    CHECK(strcmp(ti.title, "track1") == 0, "title still available");
    CHECK_EQI(player_seek(500), PLAYER_OK, "seek works in STOPPED");
    CHECK_EQI(player_get_position_ms(), 500, "STOPPED seek moves playhead");
    CHECK_EQI(player_play(), PLAYER_OK, "play restarts stopped track");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING, "PLAYING again");

    TEST("end of track: auto-advance through the queue");
    /* Short track + two queued successors. */
    cfg.total_frames = 100;
    mock_codec_configure(&cfg);
    CHECK_EQI(player_enqueue("next1.mock"), PLAYER_OK, "enqueue next1");
    CHECK_EQI(player_enqueue("next2.mock"), PLAYER_OK, "enqueue next2");
    ev_clear();
    CHECK_EQI(player_open("short.mock"), PLAYER_OK, "open short track");
    CHECK_EQI(player_play(), PLAYER_OK, "play short track");

    rc = PLAYER_OK;
    for (i = 0; i < 300 && rc == PLAYER_OK; i++) {
        rc = player_process();
        if (player_get_state() == PLAYER_STOPPED)
            break;
    }
    CHECK_EQI(rc, PLAYER_OK, "process clean through all tracks");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED,
              "stopped after the queue drains");
    CHECK_EQI(ev_count(PLAYER_EV_TRACK), 3,
              "3 TRACK events (short + next1 + next2)");
    CHECK_EQI(ev_count(PLAYER_EV_END_OF_TRACK), 1,
              "one END_OF_TRACK at the very end");
    CHECK_EQI(ev_count(PLAYER_EV_QUEUE_EMPTY), 0,
              "auto-advance never hits QUEUE_EMPTY");
    {
        /* Verify the auto-advanced track ids in event order. */
        CHECK_EQI(player_get_track(&ti), PLAYER_OK, "last track info");
        CHECK(strcmp(ti.title, "next2") == 0, "ended on next2");
        CHECK_EQI(ti.track_no, 4, "advanced twice (short=2, next1=3, next2=4)");
    }

    TEST("player_next with an empty queue");
    ev_clear();
    CHECK_EQI(player_open("track2.mock"), PLAYER_OK, "open track2");
    CHECK_EQI(player_next(), PLAYER_ERR_EMPTY, "next on empty queue fails");
    CHECK_EQI(ev_count(PLAYER_EV_QUEUE_EMPTY), 1, "QUEUE_EMPTY event");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "still STOPPED");

    TEST("player_next pops the queue and plays");
    CHECK_EQI(player_enqueue("q1.mock"), PLAYER_OK, "enqueue q1");
    CHECK_EQI(player_enqueue("q2.mock"), PLAYER_OK, "enqueue q2");
    ev_clear();
    CHECK_EQI(player_next(), PLAYER_OK, "next #1");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING, "playing q1");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "q1 track info");
    CHECK(strcmp(ti.title, "q1") == 0, "q1 opened");
    CHECK_EQI(player_next(), PLAYER_OK, "next #2");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "q2 track info");
    CHECK(strcmp(ti.title, "q2") == 0, "q2 opened");
    CHECK_EQI(player_get_state(), PLAYER_PLAYING, "playing q2");
    CHECK_EQI(ev_count(PLAYER_EV_TRACK), 2, "two TRACK events");

    TEST("decode error -> PLAYER_ERROR -> recovery");
    cfg.total_frames = 44100;
    cfg.fail_at_frame = 50;
    mock_codec_configure(&cfg);
    ev_clear();
    CHECK_EQI(player_open("bad.mock"), PLAYER_OK, "open failing track");
    CHECK_EQI(player_play(), PLAYER_OK, "play failing track");
    rc = PLAYER_OK;
    for (i = 0; i < 100 && player_get_state() == PLAYER_PLAYING; i++)
        rc = player_process();
    CHECK(rc < 0, "process reports the decode error");
    CHECK_EQI(player_get_state(), PLAYER_ERROR, "state PLAYER_ERROR");
    CHECK(ev_count(PLAYER_EV_ERROR) >= 1, "ERROR event emitted");
    CHECK_EQI(player_play(), PLAYER_ERR_STATE, "play refused in ERROR");
    CHECK_EQI(player_stop(), PLAYER_OK, "stop clears ERROR");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "recovered to STOPPED");
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);

    TEST("volume clamp + storage");
    CHECK_EQI(player_set_volume(-23), PLAYER_OK, "set -23 dB");
    CHECK_EQI(player_get_volume(), -23, "get -23 dB");
    CHECK_EQI(player_set_volume(-100), PLAYER_OK, "under-range clamps");
    CHECK_EQI(player_get_volume(), -60, "clamped to -60 dB");
    CHECK_EQI(player_set_volume(99), PLAYER_OK, "over-range clamps");
    CHECK_EQI(player_get_volume(), 20, "clamped to +20 dB");

    TEST("open failure leaves PLAYER_ERROR, next open recovers");
    CHECK_EQI(player_open("bad.wav"), CODEC_ERR_IO,
              "missing file -> codec IO error");
    CHECK_EQI(player_get_state(), PLAYER_ERROR, "state PLAYER_ERROR");
    mock_codec_configure(&cfg);
    CHECK_EQI(player_open("again.mock"), PLAYER_OK, "good open recovers");
    CHECK_EQI(player_get_state(), PLAYER_STOPPED, "back to STOPPED");

    if (failures == 0)
        printf("test_player_state: ALL PASS\n");
    else
        printf("test_player_state: %d FAILURES\n", failures);
    return failures;
}
