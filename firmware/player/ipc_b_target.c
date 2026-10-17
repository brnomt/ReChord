/*
 * ipc_b_target.c — target A<->B audio IPC (BRNOMT-68 phase 3c).
 *
 * Phase 3c moves the DECODE to Core B. The AP keeps the DAC/I2S bring-up and
 * the I2S TX ISR feeder, but the PCM ring (firmware/player/ipc_shared.h) is now
 * PRODUCED by B and CONSUMED by the AP's ISR (roles swapped from phase 2). The
 * AP also serves the track bytes over the A->B file ring (bfs.c) while B's
 * codec_io reads them.
 *
 * Fallback: if B does not ACK the OPEN handshake (older B image / fault), the
 * AP decodes and produces the ring itself exactly as phase 2 — audio never
 * breaks because of the handoff.
 *
 * Single producer / single consumer; Cortex-M3 has no data cache, every
 * publication is followed by a DMB.
 */
#include <string.h>

#include "ipc_b.h"
#include "bfs.h"
#include "ipc_shared.h"
#include "audio/audio_output.h"
#include "rknano_i2s.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

#define PUMP_SCRATCH_FRAMES 128

static ipc_b_state_t g_state = IPC_B_OFF;
static int  g_paused;
static int  g_volume_db;
static int  s_b_decodes;            /* 1 = Core B is the ring producer */

static void dmb(void)
{
    __asm__ volatile("dmb" ::: "memory");
}

static void sh_set(unsigned i, uint32_t v)
{
    CTRL[i] = v;
    dmb();
}

/* Copy n bytes into the ring at byte offset `off` (wrap-aware). Frames are
 * 8 bytes and the ring is 64 KiB, so a frame never straddles the boundary. */
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

/* ISR feeder: drain one int32 stereo frame from the B-produced ring. */
static int shared_pull(uint32_t *left, uint32_t *right, void *user)
{
    uint32_t rd, wr;

    (void)user;
    rd = CTRL[IPC_SH_RD];
    wr = CTRL[IPC_SH_WR];
    if (rd == wr) {
        CTRL[IPC_SH_UNDERRUNS] = CTRL[IPC_SH_UNDERRUNS] + 1u;
        return 0;
    }
    {
        const volatile uint32_t *f =
            (const volatile uint32_t *)(IPC_RING_BASE + (rd & IPC_RING_MASK));
        *left = f[0];
        *right = f[1];
    }
    rd += 8u;
    CTRL[IPC_SH_RD] = rd;
    CTRL[IPC_SH_FRAMES] = CTRL[IPC_SH_FRAMES] + 1u;
    dmb();
    return 1;
}

static uint32_t codec_id_for(const char *path)
{
    const char *dot = 0, *p;

    if (path == 0)
        return 0;
    for (p = path; *p; p++)
        if (*p == '.')
            dot = p;
    if (dot == 0)
        return 0;                 /* wav */
    if (strcmp(dot, ".flac") == 0)
        return 2;
    if (strcmp(dot, ".mp3") == 0)
        return 1;
    return 0;
}

/* Ask Core B to open the track. A must have opened the file for it first. */
static int b_handoff(const char *path)
{
    int i;

    if (path == 0)
        return -1;
    if (bfs_open(path) != 0)
        return -1;

    CTRL[IPC_C_ARG] = codec_id_for(path);
    dmb();
    CTRL[IPC_C_CMD] = IPC_C_OPEN;
    dmb();

    for (i = 0; i < 200; i++) {         /* ~200 ms, B opens + reads headers */
        bfs_poll();
        if (CTRL[IPC_C_ACK] == IPC_C_OPEN)
            break;
        dac_delay_ms(1);
    }
    if (CTRL[IPC_C_ACK] != IPC_C_OPEN || CTRL[IPC_C_BSTATE] != IPC_BS_OPEN) {
        bfs_close();
        return -1;
    }
    return 0;
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
    s_b_decodes = 0;
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
    s_b_decodes = 0;

    /* Stop the feed, reset the ring indices, publish the format. */
    sh_set(IPC_SH_STATE, IPC_SH_IDLE);
    sh_set(IPC_SH_RD, 0);
    sh_set(IPC_SH_WR, 0);
    sh_set(IPC_SH_FRAMES, 0);
    sh_set(IPC_SH_UNDERRUNS, 0);
    sh_set(IPC_SH_RATE, (uint32_t)track->sample_rate);
    sh_set(IPC_SH_CHANNELS, (uint32_t)track->channels);
    sh_set(IPC_SH_BITS, (uint32_t)track->bits);

    /* Offload decode to Core B; on any failure the AP decodes (phase 2). */
    if (b_handoff(track->path) == 0)
        s_b_decodes = 1;

    /* The AP owns the DAC/I2S bring-up in both cases. */
    fmt.sample_rate = (uint32_t)track->sample_rate;
    fmt.bits = (uint8_t)track->bits;
    fmt.channels = (uint8_t)track->channels;
    if (audio_output_configure(&fmt) != 0) {
        g_state = IPC_B_FAULT;
        sh_set(IPC_SH_STATE, IPC_SH_IDLE);
        return -1;
    }
    audio_output_start(0);              /* serializer on (XFER) */
    rknano_i2s_attach(shared_pull, 0);  /* AP ISR drains the ring */
    rknano_i2s_kick();
    audio_output_fade_in();

    sh_set(IPC_SH_STATE, IPC_SH_RUN);   /* B starts decoding on RUN */
    g_state = IPC_B_RUNNING;
    return 0;
}

int ipc_b_stop(void)
{
    if (s_b_decodes) {
        CTRL[IPC_C_CMD] = IPC_C_CLOSE;
        dmb();
        s_b_decodes = 0;
    }
    rknano_i2s_detach();
    sh_set(IPC_SH_STATE, IPC_SH_IDLE);
    audio_output_fade_out();
    audio_output_stop();
    audio_output_clear_played();
    bfs_close();
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
        out->b_eof = (int)CTRL[IPC_C_BEOF];
        out->b_error = (int)CTRL[IPC_C_BSTATE] == IPC_BS_ERROR;
    }
    return (int)g_state;
}

int ipc_b_ring_fill(const int32_t *pcm, int frames)
{
    uint32_t rd, wr, free_bytes;
    int max_frames;

    if (pcm == 0 || frames <= 0 || g_state != IPC_B_RUNNING || g_paused)
        return 0;
    if (s_b_decodes)
        return 0;                 /* B is the producer */

    rd = CTRL[IPC_SH_RD];
    wr = CTRL[IPC_SH_WR];
    free_bytes = IPC_RING_BYTES - (wr - rd);
    max_frames = (int)(free_bytes / 8u);
    if (frames > max_frames)
        frames = max_frames;
    if (frames <= 0)
        return 0;

    if (CTRL[IPC_SH_CHANNELS] == 1u) {
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
    rknano_i2s_kick();          /* re-arm the feeder (it masks when dry) */
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

int ipc_b_b_decodes(void)
{
    return s_b_decodes;
}

int ipc_b_b_eof(void)
{
    return s_b_decodes ? (int)CTRL[IPC_C_BEOF] : 0;
}

int ipc_b_seek(long ms)
{
    int i;

    if (!s_b_decodes)
        return 0;

    sh_set(IPC_SH_STATE, IPC_SH_IDLE);   /* stop B's production */
    sh_set(IPC_SH_RD, 0);
    sh_set(IPC_SH_WR, 0);

    CTRL[IPC_C_ARG] = (uint32_t)ms;
    dmb();
    CTRL[IPC_C_CMD] = IPC_C_SEEK;
    dmb();
    for (i = 0; i < 500; i++) {          /* ~500 ms: file service repositions */
        bfs_poll();
        if (CTRL[IPC_C_ACK] == IPC_C_SEEK)
            break;
        dac_delay_ms(1);
    }
    sh_set(IPC_SH_FRAMES, 0);
    sh_set(IPC_SH_STATE, g_paused ? IPC_SH_PAUSE : IPC_SH_RUN);
    return 0;
}
