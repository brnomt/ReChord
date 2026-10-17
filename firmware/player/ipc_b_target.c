/*
 * ipc_b_target.c — target implementation of the A<->B audio IPC.
 *
 * CURRENT MAPPING (2026-10-16): the B core bring-up is still TODO (the
 * mailbox/shared-block wiring below are placeholders), so the "B core"
 * sink is a DIRECT DAC renderer via the audio_output HAL: decoded PCM
 * lands in g_ring and the I2S1 TX-empty IRQ (rknano_i2s_tx_isr, vector
 * slot 48) drains it into the CS43131 path at the sample clock.  Decode
 * and playback therefore overlap — the same streaming shape the stock
 * uses (its DMA ring; docs/HANDOVER "route A").  When the real B core
 * lands, this file switches back to the mailbox protocol (the shared
 * block/doorbell code below are kept and unused).
 *
 * What is NOT yet real (to be settled on hardware, tracked in
 * docs/rewrite/player.md open items):
 *   1. Doorbell: which mailbox interrupt tells the B core a command or new
 *      ring data is ready. Candidate per firmware/ipc.h: mailbox channel 1
 *      (MB_CH_DECODE) at 0x40110000. TODO: confirm channel + IRQ wiring.
 *   2. Exact word offsets of the shared block at 0x01020000 — the reference
 *      documents the region, not the layout. Our struct is clean-room; TODO
 *      verify against the ROM/loader handoff ("invalid loader slot/geometry
 *      handoff").
 *   3. Clock negotiation registers (CRU): how the AP asks the B core to
 *      step /1 (400 MHz) -> /4 (100 MHz). TODO: CRU register sequence.
 *   4. Where the PCM ring physically lives (AP RAM seen by the B core's
 *      DMA, or a dedicated shared window). TODO: memory-map confirmation.
 *   5. B-core fault capture (pc/lr/cfsr) transport — the reference logs
 *      "B FAULT kind %u pc %08x lr %08x cfsr %08x"; TODO: dump word order.
 *
 * The state machine, counters and command set below are complete and
 * compile for arm-none-eabi-gcc; only the MMIO doorbells are placeholders
 * (they write our clean-room shared block, which no one consumes yet).
 */
#include "ipc_b.h"
#include "ringbuf.h"
#include "audio/audio_output.h"

/* Documented shared IPC block region (~0x01020xxx in the reference facts). */
#define IPC_B_SHARED_BASE   0x01020000u

/* Candidate doorbell: mailbox channel 1 (MB_CH_DECODE) at 0x40110000
 * (firmware/ipc.h). TODO(bring-up): confirm and implement the ring/IRQ
 * handshake; the register offsets below are placeholders. */
#define IPC_B_MAILBOX_BASE  0x40110000u
#define IPC_B_MBOX_CMD      (*(volatile uint32_t *)(IPC_B_MAILBOX_BASE + 0x00u))
#define IPC_B_MBOX_INT_SET  (*(volatile uint32_t *)(IPC_B_MAILBOX_BASE + 0x10u))
#define IPC_B_MBOX_CH_DECODE_INT (1u << 1)   /* MAILBOX_INT_1            */

/* Supervisor budget: the reference uses 30 s timeouts. TODO(bring-up):
 * convert poll ticks to ms once the poll cadence is fixed; 512 polls at a
 * ~16 ms UI tick is ~8 s — deliberately stricter than 30 s during
 * bring-up so a hung B core trips early. */
#define IPC_B_SUPERVISOR_TICKS 512

#define IPC_B_SHARED ((volatile ipc_b_shared_t *)IPC_B_SHARED_BASE)
#define IPC_B_MAGIC  0x42434252u   /* 'RBCB' little-endian               */
#define IPC_B_PROTO_VERSION 1u

/* Producer-side PCM ring (TODO(bring-up) item 4: place into the window the
 * B core reads via DMA; for now it is plain AP RAM behind our contract). */
static uint8_t   g_ring_storage[IPC_B_RING_BYTES];
/* Mono->stereo upmix scratch for the direct DAC sink: sized to the player's
 * decode staging buffer (PLAYER_PUMP_FRAMES=512 frames, stereo). */
#define IPC_B_PUMP_SCRATCH (512 * 2)
static ringbuf_t g_ring;

static ipc_b_state_t g_state = IPC_B_OFF;
static int  g_frame_bytes = IPC_B_FRAME_BYTES;
static int  g_paused = 0;
static long g_supervisor_ticks = 0;

/* TODO(bring-up): real doorbell — poke the mailbox IRQ the B core vectors
 * on (candidate: MAILBOX1/IRQ 10 per firmware/ipc.h). */
static void target_doorbell(void)
{
    IPC_B_MBOX_CMD = IPC_B_CMD_NONE;      /* placeholder write            */
    IPC_B_MBOX_INT_SET = IPC_B_MBOX_CH_DECODE_INT;
}

static void shared_publish_ring(void)
{
    IPC_B_SHARED->ring_rd = (uint32_t)g_ring.rd;
    IPC_B_SHARED->ring_wr = (uint32_t)g_ring.wr;
}

int ipc_b_init(void)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    if (rb_init(&g_ring, g_ring_storage, sizeof(g_ring_storage)) != 0)
        return -1;

    sh->magic = 0;
    sh->version = IPC_B_PROTO_VERSION;
    sh->cmd = IPC_B_CMD_NONE;
    sh->cmd_arg0 = 0;
    sh->cmd_arg1 = 0;
    sh->ack = 0;
    sh->state = IPC_B_OFF;
    sh->fault_kind = 0;
    sh->ring_base = (uint32_t)(uintptr_t)g_ring_storage;
    sh->ring_bytes = (uint32_t)IPC_B_RING_BYTES;
    sh->ring_rd = 0;
    sh->ring_wr = 0;
    sh->sample_rate = 0;
    sh->channels = 0;
    sh->volume_db = 0;
    sh->frames_consumed = 0;
    sh->clock_div = 1;                    /* "B start 0 at 400 MHz"       */
    sh->busy_pct = 0;
    sh->fault_pc = 0;
    sh->fault_lr = 0;
    sh->fault_cfsr = 0;
    sh->magic = IPC_B_MAGIC;              /* publish handshake last       */

    g_state = IPC_B_OFF;
    audio_output_clear_played();
    g_supervisor_ticks = 0;
    return 0;
}

int ipc_b_start(const ipc_b_track_t *track)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    if (track == NULL || track->channels < 1 || track->channels > 2 ||
        track->sample_rate <= 0)
        return -1;

    g_frame_bytes = track->channels * 2;
    rb_reset(&g_ring);
    audio_output_clear_played();
    g_paused = 0;
    g_supervisor_ticks = 0;

    sh->ring_rd = 0;
    sh->ring_wr = 0;
    sh->frames_consumed = 0;
    sh->sample_rate = (uint32_t)track->sample_rate;
    sh->channels = (uint32_t)track->channels;
    sh->clock_div = 1;                    /* documented start clock       */
    sh->state = IPC_B_RUNNING;
    sh->cmd = IPC_B_CMD_START;
    sh->cmd_arg0 = (track->duration_ms > 0) ? (uint32_t)track->duration_ms : 0;

    /* Direct-DAC renderer: configure the audio HAL for the track's format
     * (full verified bring-up at the sample rate), then stream the ring
     * through the TX IRQ. No B core. */
    {
        audio_format_t fmt;

        fmt.sample_rate = (uint32_t)track->sample_rate;
        fmt.bits = 16;                   /* pipeline standard (codec.h)   */
        fmt.channels = (uint8_t)track->channels;
        if (audio_output_configure(&fmt) != 0) {
            g_state = IPC_B_FAULT;
            sh->state = IPC_B_FAULT;
            return -1;
        }
    }
    audio_output_start(&g_ring);

    g_state = IPC_B_RUNNING;
    return 0;
}

int ipc_b_stop(void)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    sh->cmd = IPC_B_CMD_STOP;
    target_doorbell();
    sh->state = IPC_B_OFF;                /* "B off"                      */
    audio_output_hold(1);                 /* stop the feeder before reset */
    audio_output_detach();
    rb_reset(&g_ring);
    shared_publish_ring();
    audio_output_stop();
    audio_output_clear_played();
    g_paused = 0;
    g_state = IPC_B_OFF;
    return 0;
}

int ipc_b_pause(int paused)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    if (g_state != IPC_B_RUNNING && g_state != IPC_B_STARTING &&
        g_state != IPC_B_DRAINED)
        return -1;

    g_paused = paused ? 1 : 0;
    /* Held: the TX IRQ stops draining (FIFO runs out -> silence). Resume
     * re-arms it; any ring data that arrived while paused plays then. */
    audio_output_hold(g_paused);
    sh->cmd = paused ? IPC_B_CMD_PAUSE : IPC_B_CMD_RESUME;
    target_doorbell();
    return 0;
}

int ipc_b_set_volume(int db)
{
    /* The sink side owns the DAC volume (shared-block contract: "volume
     * currently applied by the B side"). Apply it to the CS43131
     * PCM_VOLUME regs through the HAL (click-free ramp) and publish. */
    audio_output_set_volume_db(db);
    IPC_B_SHARED->volume_db = (uint32_t)db;
    IPC_B_SHARED->cmd = IPC_B_CMD_SET_VOL;
    target_doorbell();
    return 0;
}

int ipc_b_poll_state(ipc_b_status_t *out)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    /* Direct-DAC renderer: no B core to handshake with. The TX IRQ drains
     * the ring at the sample clock; audio_output counts what left it. */
    if (g_state == IPC_B_STARTING)
        g_state = IPC_B_RUNNING;

    if (out != 0) {
        out->state = g_state;
        out->frames_consumed = audio_output_frames_played();
        /* Byte counters (free-running, like the mock's g_ring.rd/wr): the
         * player derives free space as RING_BYTES - (wr - rd) and its EOS
         * check is ring_wr == ring_rd (fully drained). */
        out->ring_rd = (long)g_ring.rd;
        out->ring_wr = (long)g_ring.wr;
        out->clock_div = (int)sh->clock_div;
        out->busy_pct = (int)sh->busy_pct;
        out->volume_db = (int)sh->volume_db;
    }
    return (int)g_state;
}

int ipc_b_ring_fill(const int16_t *pcm, int frames)
{
    size_t written;

    if (pcm == 0 || frames <= 0 || g_state == IPC_B_OFF ||
        g_state == IPC_B_FAULT || g_paused)
        return 0;

    /* Producer half: PCM goes into the ring, the TX IRQ drains it. The ring
     * always carries STEREO frames (one 32-bit word each, stock HWT_Bit32
     * order); upmix mono to L=R on the way in. Writes are clamped to free
     * space (returns < frames when the ring is full). */
    if (g_frame_bytes == 2) {
        /* mono source: duplicate each sample into both channels */
        static int16_t st[IPC_B_PUMP_SCRATCH];
        int n = frames, i;
        if (n > IPC_B_PUMP_SCRATCH / 2)
            n = IPC_B_PUMP_SCRATCH / 2;
        for (i = 0; i < n; i++)
            st[2 * i] = st[2 * i + 1] = pcm[i];
        written = rb_write(&g_ring, st, (size_t)n * 4u);
    } else {
        written = rb_write(&g_ring, pcm, (size_t)frames * 4u);
    }
    if (written == 0)
        return 0;
    shared_publish_ring();
    audio_output_kick();                  /* re-arm the TX IRQ if it went dry  */
    return (int)(written / 4u);
}

int ipc_b_flush(void)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    /* rb_reset is only legal with the consumer stopped: hold the feeder
     * across the reset (an ISR either ran already or sees hold and bails). */
    audio_output_hold(1);
    rb_reset(&g_ring);
    audio_output_clear_played();
    audio_output_hold(0);
    shared_publish_ring();
    sh->frames_consumed = 0;
    sh->cmd = IPC_B_CMD_FLUSH;
    target_doorbell();
    return 0;
}

int ipc_b_track_change(long duration_ms)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    /* Gapless boundary on the live stream: do NOT reset the ring or the
     * consumed counter. Tell the B core the new track's length and let it
     * keep running (DAC stays locked to the same sample rate). */
    sh->cmd = IPC_B_CMD_TRACK;
    sh->cmd_arg0 = (duration_ms > 0) ? (uint32_t)duration_ms : 0u;
    target_doorbell();
    return 0;
}
