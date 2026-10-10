/*
 * ipc_b_mock.c — host-side simulated B core (see ipc_b.h for the contract
 * and docs/rewrite/player.md for the protocol-facts mapping).
 *
 * The mock plays the role of the B core in the host tests:
 *   - consumes interleaved int16 PCM from the SPSC ring (ringbuf.h) at a
 *     fixed frames-per-poll rate (a stand-in for the real DAC cadence),
 *   - runs the documented supervisor states (STARTING -> RUNNING ->
 *     DRAINED, FAULT injection for "B is gone"),
 *   - simulates the documented clock negotiation: start at /1 (400 MHz),
 *     then step to /4 (100 MHz) like "B /1 -> /4 (100 MHz; ...)".
 *
 * Freestanding-safe: no libc, no allocation — this file also compiles for
 * the target (handy for on-device A/B bring-up tests before the real B
 * firmware exists), though it is host-test code.
 */
#include "ipc_b.h"
#include "ipc_b_mock.h"
#include "ringbuf.h"

#define MOCK_TICKS_TO_NEGOTIATE 2   /* polls before the /1 -> /4 step    */
#define MOCK_SINK_CHUNK_FRAMES  64  /* sink callback granularity         */

static uint8_t  g_ring_storage[IPC_B_RING_BYTES];
static ringbuf_t g_ring;

static ipc_b_state_t g_state = IPC_B_OFF;
static ipc_b_track_t g_track;
static long g_frames_consumed;
static int  g_frame_bytes = IPC_B_FRAME_BYTES;
static int  g_clock_div = 1;          /* 1 = 400 MHz start                */
static int  g_volume_db = 0;
static int  g_paused = 0;
static int  g_fault = 0;
static int  g_started = 0;
static long g_tick = 0;
static long g_total_frames = 0;

static int g_frames_per_tick = 256;
static ipc_b_mock_sink_fn g_sink_fn = 0;
static void *g_sink_user = 0;

void ipc_b_mock_set_frames_per_tick(int frames)
{
    g_frames_per_tick = (frames > 0) ? frames : 0;
}

void ipc_b_mock_set_sink(ipc_b_mock_sink_fn fn, void *user)
{
    g_sink_fn = fn;
    g_sink_user = user;
}

void ipc_b_mock_set_fault(int on)
{
    g_fault = on ? 1 : 0;
}

int ipc_b_init(void)
{
    if (rb_init(&g_ring, g_ring_storage, sizeof(g_ring_storage)) != 0)
        return -1;
    g_state = IPC_B_OFF;
    g_frames_consumed = 0;
    g_frame_bytes = IPC_B_FRAME_BYTES;
    g_clock_div = 1;
    g_volume_db = 0;
    g_paused = 0;
    g_fault = 0;
    g_started = 0;
    g_tick = 0;
    g_total_frames = 0;
    return 0;
}

int ipc_b_start(const ipc_b_track_t *track)
{
    if (track == NULL || track->channels < 1 || track->channels > 2 ||
        track->sample_rate <= 0)
        return -1;

    g_track = *track;
    g_frame_bytes = track->channels * 2;
    g_total_frames = (track->duration_ms > 0 && track->sample_rate > 0)
        ? (track->duration_ms * track->sample_rate) / 1000
        : 0;

    rb_reset(&g_ring);
    g_frames_consumed = 0;
    g_clock_div = 1;                 /* "B start 0 at 400 MHz"            */
    g_tick = 0;
    g_paused = 0;
    g_started = 1;
    g_state = IPC_B_STARTING;
    return 0;
}

int ipc_b_stop(void)
{
    g_started = 0;
    g_paused = 0;
    g_state = IPC_B_OFF;             /* "B off"                           */
    rb_reset(&g_ring);
    g_frames_consumed = 0;
    return 0;
}

int ipc_b_pause(int paused)
{
    if (!g_started)
        return -1;
    g_paused = paused ? 1 : 0;
    return 0;
}

int ipc_b_set_volume(int db)
{
    g_volume_db = db;
    return 0;
}

static void mock_consume(void)
{
    int16_t chunk[MOCK_SINK_CHUNK_FRAMES * 2];
    int want = g_frames_per_tick;

    while (want > 0) {
        int take = (want > MOCK_SINK_CHUNK_FRAMES)
                   ? MOCK_SINK_CHUNK_FRAMES : want;
        size_t got = rb_read(&g_ring, chunk,
                             (size_t)take * (size_t)g_frame_bytes);
        int frames = (int)(got / (size_t)g_frame_bytes);

        if (frames <= 0)
            break;
        if (g_sink_fn != 0)
            g_sink_fn(chunk, frames, g_sink_user);
        g_frames_consumed += frames;
        want -= frames;
    }
}

int ipc_b_poll_state(ipc_b_status_t *out)
{
    if (g_fault) {
        g_state = IPC_B_FAULT;       /* "B is gone" / "B FAULT kind ..."  */
        g_fault = 0;
    } else if (g_started) {
        g_tick++;
        if (g_state == IPC_B_STARTING) {
            g_state = IPC_B_RUNNING;
        } else if (g_state == IPC_B_RUNNING) {
            /* Documented negotiation: "B /1 -> /4 (100 MHz)". */
            if (g_tick >= MOCK_TICKS_TO_NEGOTIATE)
                g_clock_div = 4;
            if (!g_paused)
                mock_consume();
            if (g_total_frames > 0 && rb_used(&g_ring) == 0 &&
                g_frames_consumed >= g_total_frames)
                g_state = IPC_B_DRAINED;   /* "B drained"                 */
        }
    }

    if (out != 0) {
        out->state = g_state;
        out->frames_consumed = g_frames_consumed;
        out->ring_rd = (long)g_ring.rd;
        out->ring_wr = (long)g_ring.wr;
        out->clock_div = g_clock_div;
        /* "busy 10%" — pretend proportional to ring occupancy. */
        out->busy_pct = (int)((rb_used(&g_ring) * 100) /
                              (rb_capacity(&g_ring) + 1));
        out->volume_db = g_volume_db;
    }
    return (int)g_state;
}

int ipc_b_ring_fill(const int16_t *pcm, int frames)
{
    size_t written;

    if (pcm == 0 || frames <= 0 || !g_started || g_state == IPC_B_FAULT)
        return 0;

    written = rb_write(&g_ring, pcm,
                       (size_t)frames * (size_t)g_frame_bytes);
    return (int)(written / (size_t)g_frame_bytes);
}

int ipc_b_flush(void)
{
    rb_reset(&g_ring);
    g_frames_consumed = 0;
    return 0;
}
