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
#include "log.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

#define PUMP_SCRATCH_FRAMES 128

static ipc_b_state_t g_state = IPC_B_OFF;
static int  g_paused;
static int  g_volume_db;
static int  s_b_decodes;            /* 1 = Core B is the ring producer */
static int  s_eof_logged;           /* BEOF boundary already logged      */
/* Stream geometry for the next OPEN/NEXT (codec_t.prescan_info), pushed to
 * Core B so its re-open can skip the header walk (BRNOMT-73). */
static codec_prescan_t s_prescan;
static int  s_prescan_valid;

static void dmb(void)
{
    __asm__ volatile("dmb" ::: "memory");
}

static void sh_set(unsigned i, uint32_t v)
{
    CTRL[i] = v;
    dmb();
}

/* Log the file NAME (not the whole path) so a field log names the track being
 * started or handed off — without it a session cannot be read back. */
static void log_track(const char *path)
{
    char name[72];
    const char *b = path;
    const char *p;
    unsigned i = 0;

    if (path == 0 || path[0] == '\0')
        return;
    for (p = path; *p != '\0'; p++) {
        if (*p == '/')
            b = p + 1;
    }
    while (b[i] != '\0' && i < (unsigned)sizeof(name) - 1u) {
        name[i] = b[i];
        i++;
    }
    name[i] = '\0';
    log_printf("ipc: track %s", name);
}

/* The AP's own codec open failed for `path` (no B command is ever sent in that
 * case, so without this the log would show nothing at the boundary). */
void ipc_b_note_open_failed(const char *path)
{
    if (path != 0 && path[0] != '\0')
        log_track(path);
    log_printf("ipc: open failed (AP codec)");
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

/* Publish an AP->B command and return its sequence number. B acts when
 * IPC_C_SEQ != IPC_C_ACK, so a repeated command is not dropped. */
static uint32_t cmd_start(uint32_t cmd)
{
    uint32_t seq;

    CTRL[IPC_C_CMD] = cmd;
    dmb();
    seq = CTRL[IPC_C_SEQ] + 1u;
    CTRL[IPC_C_SEQ] = seq;
    dmb();
    return seq;
}

/* Wait up to ~1500 ms for Core B to finish the command published by
 * cmd_start(). The old 200 ms budget fell back to a hard restart (stop + DAC
 * relock) too eagerly, which is the ~1 s gap BRNOMT-73 is about. `bfs_poll`
 * keeps the file ring(s) fed while B opens a codec and reads its headers. */
/* Wait (bounded) for Core B to acknowledge `seq`, keeping its file ring fed —
 * B's commands read the file headers, and those reads are served right here.
 * Real time from the log clock, with an iteration guard so a frozen clock can
 * never spin forever. Returns 0 when B acknowledged. */
static int b_wait_ack(uint32_t seq, unsigned long budget_ms)
{
    unsigned long t0 = log_now_ms();
    unsigned long guard = 0;

    for (;;) {
        bfs_poll();
        if (CTRL[IPC_C_ACK] == seq)
            return 0;
        if (log_now_ms() - t0 >= budget_ms || ++guard > 40000000UL)
            return -1;
        delay_loops(2000);              /* ~10 us: brief, do not starve bus */
    }
}

static int b_cmd_wait(uint32_t seq)
{
    uint32_t cmd = CTRL[IPC_C_CMD];
    unsigned long t0 = log_now_ms();
    unsigned long ms;

    if (b_wait_ack(seq, 1500UL) != 0) {
        log_printf("ipc: cmd %u no ack after %lu ms (bstate %u berr %d "
                   "bstall %u)", (unsigned)cmd, log_now_ms() - t0,
                   (unsigned)CTRL[IPC_C_BSTATE], (int)CTRL[IPC_C_BERR],
                   (unsigned)CTRL[IPC_C_BSTALL]);
        return -1;
    }
    if (CTRL[IPC_C_BSTATE] != IPC_BS_OPEN) {
        /* B answered but did not open: its codec refused the file. */
        log_printf("ipc: cmd %u open failed after %lu ms (bstate %u berr %d "
                   "bstall %u)", (unsigned)cmd, log_now_ms() - t0,
                   (unsigned)CTRL[IPC_C_BSTATE], (int)CTRL[IPC_C_BERR],
                   (unsigned)CTRL[IPC_C_BSTALL]);
        return -1;
    }

    /* Boundary timing (BRNOMT-73): REAL milliseconds, same clock as the line
     * timestamps. The PCM ring holds ~162 ms @44.1k, so an ACK slower than
     * that is audible. Log both boundary commands so the log shows the path
     * (OPEN = hard restart, NEXT = gapless) and what Core B actually took. */
    ms = log_now_ms() - t0;
    if (cmd == IPC_C_OPEN || cmd == IPC_C_NEXT)
        log_printf("ipc: cmd %u ack %lu ms", (unsigned)cmd, ms);
    else if (ms >= 50UL)
        log_printf("ipc: cmd %u ack slow %lu ms", (unsigned)cmd, ms);
    return 0;
}

void ipc_b_set_prescan(const codec_prescan_t *p)
{
    if (p == 0) {
        s_prescan_valid = 0;
        return;
    }
    s_prescan = *p;
    s_prescan_valid = 1;
}

/* Push the stored geometry to Core B. Called right before OPEN/NEXT so it
 * always describes the command that consumes it; IPC_C_PRE_VALID is written
 * LAST so B can never read a half-published set. */
static void publish_prescan(void)
{
    if (s_prescan_valid) {
        sh_set(IPC_C_PRE_MAXBLOCK, s_prescan.max_block);
        sh_set(IPC_C_PRE_CHANNELS, s_prescan.channels);
        sh_set(IPC_C_PRE_SEEKTABLE, (uint32_t)s_prescan.seektable_bytes);
    }
    sh_set(IPC_C_PRE_VALID, s_prescan_valid ? 1u : 0u);
}

/* Hard start: A opens the track into ring 0, then asks B to open it. */
static int b_handoff(const char *path)
{
    uint32_t seq;

    if (path == 0)
        return -1;
    if (bfs_open(path) != 0)
        return -1;

    sh_set(IPC_C_RING, 0);
    publish_prescan();
    CTRL[IPC_C_ARG] = codec_id_for(path);
    dmb();
    seq = cmd_start(IPC_C_OPEN);
    if (b_cmd_wait(seq) != 0) {
        bfs_close();
        return -1;
    }
    return 0;
}

/* Gapless boundary: the AP has already promoted the pre-opened ring (next
 * file in RAM), so B only reopens its codec reading that ring. No bfs_open:
 * the SD open was paid at pre-open time. */
static int b_next_cmd(const char *path, uint32_t ring)
{
    uint32_t seq;

    if (path == 0)
        return -1;
    sh_set(IPC_C_RING, ring);
    publish_prescan();
    CTRL[IPC_C_ARG] = codec_id_for(path);
    dmb();
    seq = cmd_start(IPC_C_NEXT);
    return b_cmd_wait(seq);
}

/* Pre-open the NEXT track into the other file ring while the current plays. */
int ipc_b_preopen(const char *path)
{
    int rc;

    if (path == 0 || !s_b_decodes)
        return 0;                       /* AP decodes: nothing to pre-open */
    /* Timestamps come from log_printf: start/end bound the SD open, which
     * blocks the AP loop (BRNOMT-73 measurement). */
    log_printf("ipc: preopen start");
    rc = (bfs_preopen(path) == 0) ? 0 : -1;
    log_printf("ipc: preopen rc %d", rc);
    return rc;
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
    s_prescan_valid = 0;
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
    s_eof_logged = 0;

    /* One line per track start: the format is what decides gapless vs hard at
     * the NEXT boundary (BRNOMT-73). */
    log_printf("ipc: start %ld Hz %d ch %d b", track->sample_rate,
               track->channels, track->bits);
    log_track(track->path);

    /* Stop the feed, reset the ring indices, publish the format. */
    sh_set(IPC_SH_STATE, IPC_SH_IDLE);
    sh_set(IPC_SH_RD, 0);
    sh_set(IPC_SH_WR, 0);
    sh_set(IPC_SH_FRAMES, 0);
    sh_set(IPC_SH_UNDERRUNS, 0);
    sh_set(IPC_SH_RATE, (uint32_t)track->sample_rate);
    sh_set(IPC_SH_CHANNELS, (uint32_t)track->channels);
    sh_set(IPC_SH_BITS, (uint32_t)track->bits);
    /* PCM post-processing for Core B (it decodes): ReplayGain + gapless trim. */
    sh_set(IPC_C_RG, (uint32_t)track->rg_gain_q16);
    sh_set(IPC_C_GAP_DELAY, (uint32_t)track->gapless_delay);
    sh_set(IPC_C_GAP_PAD, (uint32_t)track->gapless_padding);
    sh_set(IPC_C_GAP_TOTAL, (uint32_t)track->total_samples);

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
        /* Quiesce Core B BEFORE the rings are torn down. Firing CLOSE off and
         * immediately calling bfs_close() (which pulls the file out from under
         * it) left B spinning in a ring read; the following OPEN paid for it —
         * field logs showed hard restarts at ~1000 ms where a clean start from
         * stopped took ~106 ms. The wait is bounded so a wedged core cannot
         * hang the stop itself. */
        uint32_t seq = cmd_start(IPC_C_CLOSE);

        (void)b_wait_ack(seq, 200UL);
        s_b_decodes = 0;
    }
    rknano_i2s_detach();
    sh_set(IPC_SH_STATE, IPC_SH_IDLE);
    audio_output_fade_out();
    audio_output_stop();
    audio_output_clear_played();
    bfs_close();
    g_paused = 0;
    s_eof_logged = 0;
    s_prescan_valid = 0;
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

    /* Boundary timing (BRNOMT-73): this timestamp vs the following
     * "ipc: next" line is the AP's reaction + metadata-open cost. */
    if (s_b_decodes && CTRL[IPC_C_BEOF] && !s_eof_logged) {
        s_eof_logged = 1;
        log_printf("ipc: B eof");
    }

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

int ipc_b_next(const ipc_b_track_t *track)
{
    int ring;

    if (track == NULL || !s_b_decodes)
        return -1;

    log_printf("ipc: next %ld Hz %d ch %d b", track->sample_rate,
               track->channels, track->bits);
    log_track(track->path);

    /* Gapless (BRNOMT-73): B reopens its codec reading the PRE-OPENED ring —
     * the next file is already in RAM, so the boundary pays no SD open.
     * Without a pre-open for exactly this track, return -1 and let the caller
     * take the hard path (never play the wrong file because the list changed).
     * Deliberately NO ipc_b_stop/flush and NO audio_output_configure: the PCM
     * ring cursors, the frames counter and the DAC/serializer stay live. */
    if (!bfs_preopen_ready(track->path)) {
        log_printf("ipc: next rejected (no matching preopen)");
        return -1;
    }

    ring = bfs_promote();
    if (ring < 0) {
        log_printf("ipc: next rejected (promote)");
        return -1;
    }

    sh_set(IPC_C_RG, (uint32_t)track->rg_gain_q16);
    sh_set(IPC_C_GAP_DELAY, (uint32_t)track->gapless_delay);
    sh_set(IPC_C_GAP_PAD, (uint32_t)track->gapless_padding);
    sh_set(IPC_C_GAP_TOTAL, (uint32_t)track->total_samples);
    if (b_next_cmd(track->path, (uint32_t)ring) != 0) {
        log_printf("ipc: next failed ack");
        return -1;              /* B did not ACK: caller falls back to hard */
    }
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

int ipc_b_set_replaygain(int gain_q16)
{
    if (s_b_decodes)
        sh_set(IPC_C_RG, (uint32_t)gain_q16);
    return 0;
}

int ipc_b_seek(long ms)
{
    uint32_t seq;
    int i;

    if (!s_b_decodes)
        return 0;

    sh_set(IPC_SH_STATE, IPC_SH_IDLE);   /* stop B's production */
    sh_set(IPC_SH_RD, 0);
    sh_set(IPC_SH_WR, 0);

    CTRL[IPC_C_ARG] = (uint32_t)ms;
    dmb();
    seq = cmd_start(IPC_C_SEEK);
    for (i = 0; i < 500; i++) {          /* ~500 ms: file service repositions */
        bfs_poll();
        if (CTRL[IPC_C_ACK] == seq)
            break;
        dac_delay_ms(1);
    }
    sh_set(IPC_SH_FRAMES, 0);
    sh_set(IPC_SH_STATE, g_paused ? IPC_SH_PAUSE : IPC_SH_RUN);
    return 0;
}
