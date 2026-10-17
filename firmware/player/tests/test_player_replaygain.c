/*
 * test_player_replaygain.c — end-to-end ReplayGain host test through the
 * player state machine and the IPC mock sink.
 *
 * A mock codec reports a -20 dB track gain (no peak). With RG set to TRACK,
 * the player must scale the emitted PCM by the same Q16 gain rg_math would
 * apply, sample-for-sample. This pins the player <-> rg_math integration
 * without depending on a real MP3/FLAC file.
 */
#include "player.h"
#include "mock_codec.h"
#include "ipc_b_mock.h"
#include "ipc_b.h"
#include "rg_math.h"
#include "test_util.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int       g_count;
static int32_t  *g_sink;
static int       g_sink_cap;

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

static void test_rg_applied(void)
{
    mock_codec_cfg_t cfg = {0};
    long total_frames = 1024;
    int32_t *ref;
    int i;
    int32_t gain;

    cfg.total_frames = total_frames;
    cfg.channels = 2;
    cfg.sample_rate = 44100;
    cfg.fail_at_frame = -1;              /* no decode failures */
    cfg.rg_track_gain_tenth_db = -200;       /* -20 dB */
    cfg.rg_track_peak_q15 = 0;              /* no peak info -> no limiting */
    mock_codec_configure(&cfg);

    ipc_b_mock_set_frames_per_tick(512);
    ipc_b_init();
    player_init();
    player_register_codec("mock", mock_codec_get());

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

    /* Build the reference: raw mock samples, scaled by the player's gain. */
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

int main(void)
{
    TEST("player replaygain");
    test_rg_applied();
    if (failures == 0) {
        printf("test_player_replaygain: OK\n");
        return 0;
    }
    printf("test_player_replaygain: %d FAILURE(S)\n", failures);
    return failures;
}
