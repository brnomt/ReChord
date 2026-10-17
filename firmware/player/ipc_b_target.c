/*
 * ipc_b_target.c — target A<->B audio IPC (BRNOMT-68 phase 2).
 *
 * The AP decodes and produces PCM into a shared SPSC ring in HIGHRAM0
 * (firmware/player/ipc_shared.h: ring at 0x01010000, control block at
 * 0x01020000); the B core (firmware/bb/) polls it and feeds the I2S1 TX
 * FIFO. The AP still runs the DAC/I2S bring-up (`audio_output_configure`);
 * phase 3 moves the decode itself to B.
 *
 * Single producer (AP) / single consumer (B). Cortex-M3 has no data cache;
 * each side publishes its index with a DMB. The control block is raw
 * uint32_t words (no struct layout guesswork), shared with bb_main.c.
 */
#include "ipc_b.h"
#include "ipc_shared.h"
#include "audio/audio_output.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

#define PUMP_SCRATCH_FRAMES 128

static ipc_b_state_t g_state = IPC_B_OFF;
static int  g_paused;
static int  g_volume_db;

static void sh_set(unsigned i, uint32_t v)
{
    CTRL[i] = v;
    __asm__ volatile("dmb" ::: "memory");
}

/* Copy n bytes into the ring at byte offset `off` (wrap-aware). Frames are
 * 8 bytes and the ring is 32 KiB, so a frame never straddles the boundary. */
static void ring_copy_in(uint32_t off, const void *src, uint32_t n)
{
    const uint8_t *s = (const uint8_t *)src;
    volatile uint8_t *base = (volatile uint8_t *)IPC_RING_BASE;
    uint32_t pos = off & IPC_RING_MASK;
    uint32_t first = IPC_RING_BYTES - pos;
    uint32_t i;

    if (n <= first) {
        for (i = 0; i < n; i++)
            base[pos + i] = s[i];
    } else {
        for (i = 0; i < first; i++)
            base[pos + i] = s[i];
        for (i = 0; i < n - first; i++)
            base[i] = s[first + i];
    }
}

int ipc_b_init(void)
{
    unsigned i;

    for (i = 0; i < IPC_SH_COUNT; i++)
        CTRL[i] = 0;
    sh_set(IPC_SH_STATE, IPC_SH_IDLE);

    g_state = IPC_B_OFF;
    g_paused = 0;
    g_volume_db = 0;
    audio_output_clear_played();
    return 0;
}

int ipc_b_start(const ipc_b_track_t *track)
{
    audio_format_t fmt;

    if (track == NULL || track->channels < 1 || track->channels > 2 ||
        track->sample_rate <= 0)
        return -1;

    g_paused = 0;

    /* Stop B's feed, reset the ring indices, publish the format. */
    sh_set(IPC_SH_STATE, IPC_SH_IDLE);
    sh_set(IPC_SH_RD, 0);
    sh_set(IPC_SH_WR, 0);
    sh_set(IPC_SH_FRAMES, 0);
    sh_set(IPC_SH_UNDERRUNS, 0);
    sh_set(IPC_SH_RATE, (uint32_t)track->sample_rate);
    sh_set(IPC_SH_CHANNELS, (uint32_t)track->channels);
    sh_set(IPC_SH_BITS, (uint32_t)track->bits);

    /* The AP owns the DAC/I2S bring-up for now (phase 3 moves it to B). */
    fmt.sample_rate = (uint32_t)track->sample_rate;
    fmt.bits = (uint8_t)track->bits;
    fmt.channels = (uint8_t)track->channels;
    if (audio_output_configure(&fmt) != 0) {
        g_state = IPC_B_FAULT;
        sh_set(IPC_SH_STATE, IPC_SH_IDLE);
        return -1;
    }
    audio_output_start(0);      /* serializer on (XFER); no AP-side feeder */
    audio_output_fade_in();

    sh_set(IPC_SH_STATE, IPC_SH_RUN);
    g_state = IPC_B_RUNNING;
    return 0;
}

int ipc_b_stop(void)
{
    sh_set(IPC_SH_STATE, IPC_SH_IDLE);
    audio_output_fade_out();
    audio_output_stop();
    audio_output_clear_played();
    g_paused = 0;
    g_state = IPC_B_OFF;
    return 0;
}

int ipc_b_pause(int paused)
{
    if (g_state != IPC_B_RUNNING && g_state != IPC_B_STARTING &&
        g_state != IPC_B_DRAINED)
        return -1;

    g_paused = paused ? 1 : 0;
    if (g_paused)
        audio_output_fade_out();
    sh_set(IPC_SH_STATE, g_paused ? IPC_SH_PAUSE : IPC_SH_RUN);
    if (!g_paused)
        audio_output_fade_in();
    return 0;
}

int ipc_b_set_volume(int db)
{
    audio_output_set_volume_db(db);
    g_volume_db = db;
    return 0;
}

int ipc_b_poll_state(ipc_b_status_t *out)
{
    if (g_state == IPC_B_STARTING)
        g_state = IPC_B_RUNNING;

    if (out != 0) {
        out->state = g_state;
        out->frames_consumed = (long)CTRL[IPC_SH_FRAMES];
        out->ring_rd = (long)CTRL[IPC_SH_RD];
        out->ring_wr = (long)CTRL[IPC_SH_WR];
        out->clock_div = 1;
        out->busy_pct = 0;
        out->volume_db = g_volume_db;
    }
    return (int)g_state;
}

int ipc_b_ring_fill(const int32_t *pcm, int frames)
{
    uint32_t rd, wr, free_bytes;
    int max_frames;

    if (pcm == 0 || frames <= 0 || g_state != IPC_B_RUNNING || g_paused)
        return 0;

    rd = CTRL[IPC_SH_RD];
    wr = CTRL[IPC_SH_WR];
    free_bytes = IPC_RING_BYTES - (wr - rd);
    max_frames = (int)(free_bytes / 8u);
    if (frames > max_frames)
        frames = max_frames;
    if (frames <= 0)
        return 0;

    if (CTRL[IPC_SH_CHANNELS] == 1u) {
        /* mono source: duplicate each sample into both channels, chunked. */
        static int32_t st[PUMP_SCRATCH_FRAMES * 2];
        int done = 0;

        while (done < frames) {
            int n = frames - done;
            int i;

            if (n > PUMP_SCRATCH_FRAMES)
                n = PUMP_SCRATCH_FRAMES;
            for (i = 0; i < n; i++)
                st[2 * i] = st[2 * i + 1] = pcm[done + i];
            ring_copy_in(wr + (uint32_t)done * 8u, st, (uint32_t)n * 8u);
            done += n;
        }
    } else {
        ring_copy_in(wr, pcm, (uint32_t)frames * 8u);
    }

    wr += (uint32_t)frames * 8u;
    sh_set(IPC_SH_WR, wr);      /* publish after the data (DMB inside) */
    return frames;
}

int ipc_b_flush(void)
{
    sh_set(IPC_SH_STATE, IPC_SH_IDLE);
    sh_set(IPC_SH_RD, 0);
    sh_set(IPC_SH_WR, 0);
    sh_set(IPC_SH_FRAMES, 0);
    audio_output_clear_played();
    if (g_state == IPC_B_RUNNING)
        sh_set(IPC_SH_STATE, g_paused ? IPC_SH_PAUSE : IPC_SH_RUN);
    return 0;
}

int ipc_b_track_change(long duration_ms)
{
    (void)duration_ms;          /* B keeps running on the live ring */
    return 0;
}
