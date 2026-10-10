/*
 * ipc_b_target.c — target implementation of the A<->B audio IPC, mapped
 * onto the documented hardware protocol facts (clean-room; see ipc_b.h and
 * the facts table in docs/rewrite/player.md).
 *
 * MAPPING (fact -> our implementation):
 *   shared block ~0x01020000  -> ipc_b_shared_t at 0x01020000
 *   "ring rd %u wr %u"        -> shared ring_rd/ring_wr + local ringbuf_t
 *   "player: B start 0 at
 *    400 MHz"                 -> IPC_B_CMD_START, clock_div=1 (400 MHz)
 *   "B /1 -> /4 (100 MHz)"    -> clock negotiation handshake (below)
 *   "B drained" / "B is gone"
 *   / "B off" / "B FAULT"     -> ipc_b_state_t + fault_kind/pc/lr/cfsr
 *   30 s supervisor timeout   -> supervisor tick guard (below)
 *
 * ## STATUS: compile-ready stub — register-level bring-up is TODO ##
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
static ringbuf_t g_ring;

static ipc_b_state_t g_state = IPC_B_OFF;
static int  g_frame_bytes = IPC_B_FRAME_BYTES;
static int  g_paused = 0;
static long g_frames_consumed = 0;
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
    g_frames_consumed = 0;
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
    g_frames_consumed = 0;
    g_paused = 0;
    g_supervisor_ticks = 0;

    sh->ring_rd = 0;
    sh->ring_wr = 0;
    sh->frames_consumed = 0;
    sh->sample_rate = (uint32_t)track->sample_rate;
    sh->channels = (uint32_t)track->channels;
    sh->clock_div = 1;                    /* documented start clock       */
    sh->state = IPC_B_STARTING;
    sh->cmd = IPC_B_CMD_START;
    sh->cmd_arg0 = (track->duration_ms > 0) ? (uint32_t)track->duration_ms : 0;
    target_doorbell();

    g_state = IPC_B_STARTING;
    return 0;
}

int ipc_b_stop(void)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    sh->cmd = IPC_B_CMD_STOP;
    target_doorbell();
    sh->state = IPC_B_OFF;                /* "B off"                      */
    rb_reset(&g_ring);
    shared_publish_ring();
    g_frames_consumed = 0;
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
    sh->cmd = paused ? IPC_B_CMD_PAUSE : IPC_B_CMD_RESUME;
    target_doorbell();
    return 0;
}

int ipc_b_set_volume(int db)
{
    IPC_B_SHARED->volume_db = (uint32_t)db;
    IPC_B_SHARED->cmd = IPC_B_CMD_SET_VOL;
    target_doorbell();
    return 0;
}

int ipc_b_poll_state(ipc_b_status_t *out)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    g_supervisor_ticks++;

    if (sh->magic == IPC_B_MAGIC) {
        /* The B side publishes its state; mirror it and apply the
         * supervisor timeout ("B is gone" when start never completes). */
        g_state = (ipc_b_state_t)sh->state;
        g_frames_consumed = (long)sh->frames_consumed;
        if (g_state == IPC_B_STARTING &&
            g_supervisor_ticks > IPC_B_SUPERVISOR_TICKS) {
            g_state = IPC_B_FAULT;
            sh->state = IPC_B_FAULT;
            sh->fault_kind = 1;           /* timeout kind                 */
        }
    } else {
        /* Handshake never appeared: B firmware not loaded/started. */
        g_state = (g_state == IPC_B_OFF) ? IPC_B_OFF : IPC_B_FAULT;
    }

    if (out != 0) {
        out->state = g_state;
        out->frames_consumed = g_frames_consumed;
        out->ring_rd = (long)sh->ring_rd;
        out->ring_wr = (long)sh->ring_wr;
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
        g_state == IPC_B_FAULT)
        return 0;

    written = rb_write(&g_ring, pcm,
                       (size_t)frames * (size_t)g_frame_bytes);
    shared_publish_ring();
    if (written != 0) {
        target_doorbell();                /* TODO: batch/IRQ-coalesce     */
    }
    return (int)(written / (size_t)g_frame_bytes);
}

int ipc_b_flush(void)
{
    volatile ipc_b_shared_t *sh = IPC_B_SHARED;

    rb_reset(&g_ring);
    g_frames_consumed = 0;
    shared_publish_ring();
    sh->frames_consumed = 0;
    sh->cmd = IPC_B_CMD_FLUSH;
    target_doorbell();
    return 0;
}
