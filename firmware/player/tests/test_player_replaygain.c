/*
 * test_player_replaygain.c — ReplayGain through the player state machine.
 *
 * Covers: PCM scaling under TRACK, the off/track/album mode selection,
 * ALBUM->track tag fallback, AUTO (album queue vs shuffle vs same-album
 * continuity), and the pre-open rule: opening the next codec ahead of the
 * gapless boundary must NOT publish its gain while the current track plays.
 */
#include "player.h"
#include "mock_codec.h"
#include "ipc_b_mock.h"
#include "ipc_b.h"
#include "rg_math.h"
#include "../../services/media_query.h"
#include "test_util.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int       g_count;
static int32_t  *g_sink;
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

/* Track: -20 dB track, -10 dB album, album tag "Album". */
static void configure_rg_track(void)
{
    mock_codec_cfg_t cfg = {0};

    cfg.total_frames = 1024;
    cfg.channels = 2;
    cfg.sample_rate = 44100;
    cfg.fail_at_frame = -1;
    cfg.rg_track_gain_tenth_db = -200;
    cfg.rg_album_gain_tenth_db = -100;
    cfg.rg_track_peak_q15 = 0;
    cfg.rg_album_peak_q15 = 0;
    strcpy(cfg.meta_album, "Album");
    mock_codec_configure(&cfg);
}

static void fresh_player(void)
{
    ipc_b_mock_set_frames_per_tick(512);
    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());
}

/* Existing integration check: TRACK scales the emitted PCM exactly. */
static void test_rg_applied(void)
{
    long total_frames = 1024;
    int32_t *ref;
    int i;
    int32_t gain;

    configure_rg_track();
    fresh_player();

    g_count = 0;
    g_sink_cap = (int)(total_frames * 2);
    g_sink = (int32_t *)malloc(sizeof(int32_t) * g_sink_cap);
    ref = (int32_t *)malloc(sizeof(int32_t) * g_sink_cap);
    ipc_b_mock_set_sink(sink_cb, 0);

    player_set_replaygain_mode(PLAYER_RG_TRACK);   /* before open: cached at open */

    if (player_open("rg.mock") != PLAYER_OK) {
        failures++; printf("  FAIL: rg: player_open\n"); goto out;
    }
    player_play();
    drain_to_stop(65536);

    CHECK_EQI(g_count, (int)(total_frames * 2), "rg: all frames emitted");

    gain = rg_effective_gain_q16(-200, 0);         /* same fn the player uses */
    for (i = 0; i < total_frames; i++) {
        ref[i * 2]     = mock_sample(i, 0);
        ref[i * 2 + 1] = mock_sample(i, 1);
    }
    rg_scale_pcm(ref, total_frames * 2, gain);

    for (i = 0; i < total_frames && i * 2 < g_count; i++) {
        if (g_sink[i * 2] != ref[i * 2] ||
            g_sink[i * 2 + 1] != ref[i * 2 + 1]) {
            printf("  FAIL: rg: frame %d = %d,%d want %d,%d (gain %d)\n",
                   i, g_sink[i*2], g_sink[i*2+1],
                   ref[i*2], ref[i*2+1], (int)gain);
            failures++;
            break;
        }
    }
    if (i == total_frames)
        CHECK(1, "rg: PCM scaled exactly by track gain");

out:
    ipc_b_mock_set_sink(0, 0);
    free(g_sink);
    free(ref);
    g_sink = 0;
}

/* Mode selection on one open track (no playback needed). */
static void test_modes(void)
{
    configure_rg_track();
    fresh_player();

    CHECK_EQI(player_open("a.mock"), PLAYER_OK, "modes: open");

    player_set_replaygain_mode(PLAYER_RG_TRACK);
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -200, "modes: track -20 dB");

    player_set_replaygain_mode(PLAYER_RG_ALBUM);
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -100, "modes: album -10 dB");

    player_set_replaygain_mode(PLAYER_RG_OFF);
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), 0, "modes: off -> neutral");

    /* AUTO, no context -> track gain. */
    player_set_replaygain_mode(PLAYER_RG_AUTO);
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -200,
              "auto: no album context -> track");

    /* Album queue, unshuffled -> album gain. The source is set first, as the
     * UI now does before player_play_list(). */
    player_set_queue_source(PLAYER_QUEUE_CAT, "Album", MQ_ALBUM);
    player_set_replaygain_mode(PLAYER_RG_AUTO);
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -100,
              "auto: album queue -> album");

    /* Shuffle flips AUTO back to track gain immediately. */
    player_set_shuffle(1);
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -200,
              "auto: shuffle -> track");
    player_set_shuffle(0);
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -100,
              "auto: unshuffle -> album");
}

/* ALBUM falls back to the track tags when the album ones are absent. */
static void test_album_fallback(void)
{
    mock_codec_cfg_t cfg = {0};

    cfg.total_frames = 1024;
    cfg.fail_at_frame = -1;
    cfg.rg_track_gain_tenth_db = -200;
    cfg.rg_album_gain_tenth_db = 0;
    cfg.rg_album_peak_q15 = 0;
    mock_codec_configure(&cfg);
    fresh_player();

    CHECK_EQI(player_open("a.mock"), PLAYER_OK, "fallback: open");
    player_set_replaygain_mode(PLAYER_RG_ALBUM);
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -200,
              "fallback: album missing -> track gain");
}

/* AUTO continuity: an unshuffled advance to a track with the same album tag
 * stays on album gain. */
static void test_auto_continuity(void)
{
    track_info_t ti;

    configure_rg_track();
    fresh_player();
    ipc_b_mock_set_frames_per_tick(1024);

    player_set_replaygain_mode(PLAYER_RG_AUTO);
    CHECK_EQI(player_open("a.mock"), PLAYER_OK, "continuity: open A");
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -200,
              "continuity: first track uses track gain");

    player_enqueue("b.mock");
    CHECK_EQI(player_play(), PLAYER_OK, "continuity: play");
    drain_to_stop(65536);

    CHECK_EQI(player_get_track(&ti), PLAYER_OK, "continuity: track query");
    CHECK(strcmp(ti.path, "b.mock") == 0, "continuity: advanced to B");
    CHECK_EQI(player_get_replaygain_gain_tenth_db(), -100,
              "continuity: same album -> album gain");
}

/* Pre-open regression: opening the next codec must not publish its gain while
 * the current track is still playing. */
static void test_preopen_does_not_publish(void)
{
    int calls;

    configure_rg_track();
    fresh_player();

    player_set_replaygain_mode(PLAYER_RG_TRACK);
    CHECK_EQI(player_open("a.mock"), PLAYER_OK, "preopen: open A");
    CHECK_EQI(player_play(), PLAYER_OK, "preopen: play A");

    player_set_replaygain_mode(PLAYER_RG_TRACK);   /* explicit publish */
    calls = ipc_b_mock_replaygain_calls();
    CHECK_EQI(ipc_b_mock_last_replaygain(),
              rg_effective_gain_q16(-200, 0), "preopen: A gain published");

    CHECK_EQI(player_open("b.mock"), PLAYER_OK, "preopen: open B");
    CHECK_EQI(ipc_b_mock_replaygain_calls(), calls,
              "preopen: open does not publish the next track's gain");
}

int main(void)
{
    TEST("player replaygain");
    test_rg_applied();
    test_modes();
    test_album_fallback();
    test_auto_continuity();
    test_preopen_does_not_publish();
    if (failures == 0) {
        printf("test_player_replaygain: OK\n");
        return 0;
    }
    printf("test_player_replaygain: %d FAILURE(S)\n", failures);
    return failures;
}
