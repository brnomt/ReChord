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
#include "ipc_b_mock.h"
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

/* Write a minimal ID3v2.3 file (one TIT2 frame + a few "audio" bytes) for
 * the listing reader, which must never need the audio part. */
static int write_tagged_mp3(const char *path, const char *title)
{
    unsigned char buf[128];
    unsigned int sz;
    long tlen = (long)strlen(title);
    long body = 1 + tlen;               /* encoding byte + text */
    long n;

    FILE *f = fopen(path, "wb");
    if (f == 0)
        return -1;

    buf[0] = 'I'; buf[1] = 'D'; buf[2] = '3';
    buf[3] = 3; buf[4] = 0; buf[5] = 0;         /* v2.3, no flags */
    buf[6] = buf[7] = buf[8] = buf[9] = 0;      /* size, patched below */
    n = 10;
    memcpy(buf + n, "TIT2", 4); n += 4;
    buf[n++] = (unsigned char)(body >> 24);
    buf[n++] = (unsigned char)(body >> 16);
    buf[n++] = (unsigned char)(body >> 8);
    buf[n++] = (unsigned char)(body);
    buf[n++] = 0; buf[n++] = 0;                 /* frame flags */
    buf[n++] = 0;                               /* encoding: Latin-1 */
    memcpy(buf + n, title, (size_t)tlen); n += tlen;
    sz = (unsigned int)(n - 10);
    buf[6] = (unsigned char)((sz >> 21) & 0x7F);
    buf[7] = (unsigned char)((sz >> 14) & 0x7F);
    buf[8] = (unsigned char)((sz >> 7) & 0x7F);
    buf[9] = (unsigned char)(sz & 0x7F);

    if (fwrite(buf, 1, (size_t)n, f) != (size_t)n ||
        fwrite("AUDIOBYTES", 1, 10, f) != 10) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

static int write_untagged_file(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (f == 0)
        return -1;
    if (fwrite("NOT A TAG AT ALL", 1, 16, f) != 16) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

int main(void)
{
    mock_codec_cfg_t cfg;
    track_info_t ti;
    long pos, prev;
    int i, rc, seq_before;

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
    CHECK_EQI(player_open("x.mp3"), CODEC_ERR_IO,
              "real mp3 codec: missing file -> IO error");
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

    TEST("playhead exact past the 32-bit wrap point (5-minute track)");
    /* BRNOMT-66: frames*1000 wrapped the target's 32-bit `long` after ~49 s,
     * so the playhead went negative (huge elapsed time, bar at full). The
     * playhead must stay exact for the whole track. */
    cfg.total_frames = 44100L * 300;   /* 5 min @ 44100 Hz stereo */
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);
    ipc_b_mock_set_frames_per_tick(8192);   /* drain fast (~185 ms/pass) */
    CHECK_EQI(player_open("long.mock"), PLAYER_OK, "open 5-minute track");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "long track info");
    CHECK_EQI(ti.duration_ms, 300000, "duration 300 s");
    CHECK_EQI(player_play(), PLAYER_OK, "play long track");

    for (i = 0; i < 4000 && player_get_position_ms() < 50000; i++)
        (void)player_process();
    pos = player_get_position_ms();
    CHECK(pos >= 50000, "playhead reached 50 s (was the wrap point)");
    CHECK(pos < 60000, "playhead is sane, not the wrapped value");
    prev = pos;
    for (i = 0; i < 10; i++)
        (void)player_process();
    CHECK(player_get_position_ms() > prev, "playhead keeps advancing");
    CHECK(player_get_state() == PLAYER_PLAYING, "still playing");
    ipc_b_mock_set_frames_per_tick(256);    /* restore the mock default */
    CHECK_EQI(player_stop(), PLAYER_OK, "stop long track");

    TEST("track tags: tag values win, the filename is the fallback");
    seq_before = ti.track_no;              /* the playhead test's track */
    memset(&cfg, 0, sizeof cfg);
    cfg.total_frames = 44100;
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    snprintf(cfg.meta_title, sizeof cfg.meta_title, "Tagged Title");
    snprintf(cfg.meta_artist, sizeof cfg.meta_artist, "Tagged Artist");
    snprintf(cfg.meta_album, sizeof cfg.meta_album, "Tagged Album");
    snprintf(cfg.meta_genre, sizeof cfg.meta_genre, "Tagged Genre");
    cfg.meta_track = 7;
    mock_codec_configure(&cfg);
    CHECK_EQI(player_open("filename-fallback.mock"), PLAYER_OK, "open tagged");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "track info available");
    CHECK(strcmp(ti.title, "Tagged Title") == 0, "title from the tag");
    CHECK(strcmp(ti.artist, "Tagged Artist") == 0, "artist from the tag");
    CHECK(strcmp(ti.album, "Tagged Album") == 0, "album from the tag");
    CHECK(strcmp(ti.genre, "Tagged Genre") == 0, "genre from the tag");
    CHECK_EQI(ti.tag_track_no, 7, "tag track number");
    CHECK_EQI(ti.track_no, seq_before + 1,
              "tag track number does not touch the open-sequence counter");

    memset(&cfg, 0, sizeof cfg);
    cfg.total_frames = 44100;
    cfg.sample_rate = 44100;
    cfg.channels = 2;
    cfg.fail_at_frame = -1;
    mock_codec_configure(&cfg);
    CHECK_EQI(player_open("filename-fallback.mock"), PLAYER_OK,
              "open untagged");
    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "track info available");
    CHECK(strcmp(ti.title, "filename-fallback") == 0,
          "no tags -> title from the filename");
    CHECK_EQI(ti.artist[0], 0, "no tags -> artist empty");
    CHECK_EQI(ti.album[0], 0, "no tags -> album empty");
    CHECK_EQI(ti.tag_track_no, 0, "no tags -> tag track number 0");

    TEST("player_read_meta: header-only read for listings");
    {
        codec_meta_t m;

        CHECK_EQI(write_tagged_mp3("/tmp/rechord_player_meta.mp3",
                                   "Listing Title"), 0, "fixture written");
        CHECK_EQI(write_untagged_file("/tmp/rechord_player_notag.mp3"), 0,
                  "untagged fixture written");
        memset(&m, 0x5A, sizeof m);
        CHECK_EQI(player_read_meta("/tmp/rechord_player_meta.mp3", &m),
                  PLAYER_OK, "tagged file -> PLAYER_OK");
        CHECK(strcmp(m.title, "Listing Title") == 0, "title read");
        CHECK_EQI(m.artist[0], 0, "absent tag stays empty");
        CHECK_EQI(m.genre[0], 0, "absent genre stays empty");
        CHECK_EQI(player_read_meta("/tmp/rechord_player_notag.mp3", &m),
                  PLAYER_ERR, "untagged file -> PLAYER_ERR");
        CHECK_EQI(m.title[0], 0, "no tags -> out zeroed");
        CHECK_EQI(player_read_meta("/tmp/rechord_player_meta_missing.mp3", &m),
                  PLAYER_ERR, "missing file -> PLAYER_ERR");
        CHECK_EQI(player_read_meta("/tmp/rechord_player_meta.mp3", 0),
                  PLAYER_ERR, "NULL out -> PLAYER_ERR");
        CHECK_EQI(player_read_meta(0, &m), PLAYER_ERR, "NULL path -> PLAYER_ERR");
        remove("/tmp/rechord_player_meta.mp3");
        remove("/tmp/rechord_player_notag.mp3");
    }

    if (failures == 0)
        printf("test_player_state: ALL PASS\n");
    else
        printf("test_player_state: %d FAILURES\n", failures);
    return failures;
}
