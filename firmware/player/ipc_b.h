/*
 * ipc_b.h — ReChord A<->B dual-core audio IPC (player module).
 *
 * HARD RULE: clean-room. The reference firmware's CODE is off-limits; the
 * PROTOCOL FACTS extracted in docs/re/refcfw-analysis.md are ours to
 * implement. This header is our own wire contract, shaped by those facts:
 *
 *   fact: dual Cortex-M3 (AP = UI/system core, B = audio core)
 *   fact: the B core runs the audio path, started at 400 MHz
 *         ("player: B start 0 at 400 MHz")
 *   fact: PCM streams through a ring buffer whose rd/wr counters are logged
 *         ("ring rd %u wr %u")
 *   fact: the B core clock is negotiated down after start
 *         ("player: B /1 -> /4 (100 MHz; busy 10%, out min 5576)")
 *   fact: a supervisor state machine watches the B core with 30 s timeouts
 *         and states including "B drained", "B is gone", "B off", and a
 *         fault dump ("B FAULT kind %u pc %08x lr %08x cfsr %08x")
 *   fact: a shared IPC block around 0x01020000 carries the handoff
 *
 * OUR design (clean-room choice, documented in docs/rewrite/player.md):
 *   the AP-side player decodes with our from-source codecs and produces
 *   interleaved int32 PCM (left-justified to 32 bits — one I2S slot word
 *   per sample); the B core consumes PCM from the ring and drives
 *   the DAC/I2S path (plus optional DSP). The ring is an SPSC ringbuf_t
 *   (ringbuf.h) whose rd/wr counters are exactly the logged pair.
 *
 * Implementations (link exactly one per image):
 *   ipc_b_mock.c   — host: simulated B core consuming the ring (tests)
 *   ipc_b_target.c — target: maps onto the shared block + clock
 *                    negotiation protocol (register-level bring-up TODO)
 */
#ifndef RECHORD_IPC_B_H
#define RECHORD_IPC_B_H

#include <stdint.h>

#include "codec.h"      /* codec_prescan_t (ipc_b_set_prescan) */

/* Supervisor states (naming follows the documented protocol facts). */
typedef enum
{
    IPC_B_OFF = 0,     /* "B off"     — core not started                */
    IPC_B_STARTING,    /* "B start"   — image loaded, core booting      */
    IPC_B_RUNNING,     /*             — consuming the ring              */
    IPC_B_DRAINED,     /* "B drained" — consumed everything we sent     */
    IPC_B_FAULT        /* "B is gone" / "B FAULT" — supervisor gave up  */
} ipc_b_state_t;

/* Track/format descriptor for ipc_b_start(). */
typedef struct
{
    long        sample_rate;   /* Hz (PCM format the ring will carry)   */
    int         channels;      /* 1 or 2, interleaved int32             */
    int         bits;          /* SOURCE depth (16/24/32); the ring words
                                * are always int32 left-justified        */
    long        duration_ms;   /* for the drained/timeout supervisor    */
    const char *path;          /* logging aid on the B side; may be NULL*/
    long        total_samples; /* exact decoder sample count (0 unknown)*/
    int         gapless_delay; /* leading trim samples                  */
    int         gapless_padding; /* trailing trim samples               */
    int         rg_gain_q16;   /* ReplayGain Q16 (32768 = 1.0x)         */
    /* Resume/start position: Core B seeks BEFORE its first decode round, so
     * the first PCM it produces is already at this position. 0 = start. */
    long        start_ms;
} ipc_b_track_t;

/* Snapshot returned by ipc_b_poll_state(). */
typedef struct
{
    ipc_b_state_t state;
    long          frames_consumed; /* frames played since last flush    */
    long          ring_rd;    /* ring read counter  ("ring rd %u ...")  */
    long          ring_wr;    /* ring write counter ("... wr %u")       */
    int           clock_div;  /* B clock divider: 1 = 400 MHz (start),
                                 4 = 100 MHz (negotiated steady state)  */
    int           busy_pct;   /* B core load estimate                   */
    int           volume_db;  /* volume currently applied by the B core */
    int           b_eof;      /* phase 3c: B's decoder drained          */
    int           b_error;    /* phase 3c: B reported a codec error     */
} ipc_b_status_t;

/*
 * Shared-block layout at the documented region ~0x01020000 (target).
 * Clean-room field set — exact word offsets are a TODO for hardware
 * bring-up (the reference only documents the region, not the layout).
 */
typedef struct
{
    volatile uint32_t magic;          /* handshake ('RBCB')             */
    volatile uint32_t version;        /* protocol version               */
    volatile uint32_t cmd;            /* AP -> B command                */
    volatile uint32_t cmd_arg0;
    volatile uint32_t cmd_arg1;
    volatile uint32_t ack;            /* B -> AP: last cmd completed    */
    volatile uint32_t state;          /* ipc_b_state_t on the B side    */
    volatile uint32_t fault_kind;     /* "B FAULT kind %u"              */
    volatile uint32_t ring_base;      /* PCM ring location              */
    volatile uint32_t ring_bytes;     /* power of two                   */
    volatile uint32_t ring_rd;        /* "ring rd %u"                   */
    volatile uint32_t ring_wr;        /* "wr %u"                        */
    volatile uint32_t sample_rate;
    volatile uint32_t channels;
    volatile uint32_t volume_db;
    volatile uint32_t frames_consumed;
    volatile uint32_t clock_div;      /* 1 @400 MHz start, 4 @100 MHz   */
    volatile uint32_t busy_pct;
    volatile uint32_t fault_pc;       /* "B FAULT ... pc %08x"          */
    volatile uint32_t fault_lr;
    volatile uint32_t fault_cfsr;
} ipc_b_shared_t;

/* Command words for the cmd/ack channel (our clean-room protocol). */
#define IPC_B_CMD_NONE       0u
#define IPC_B_CMD_START      1u
#define IPC_B_CMD_STOP       2u
#define IPC_B_CMD_PAUSE      3u
#define IPC_B_CMD_RESUME     4u
#define IPC_B_CMD_SET_VOL    5u
#define IPC_B_CMD_FLUSH      6u   /* drop ring contents (seek)           */
#define IPC_B_CMD_TRACK      7u   /* new track, same stream (gapless)    */
/* cmd_arg0 = duration_ms of the new track */

/* PCM ring geometry shared by both implementations (bytes, pow2). MUST match
 * IPC_RING_BYTES in ipc_shared.h (the target's ring lives in HIGHRAM0).
 * 64 KiB = 85.3 ms of 96 kHz stereo int32 (one 8192-sample FLAC block), so the
 * AP's atomic per-block decode has the whole block of slack. */
#define IPC_B_RING_BYTES     (64u * 1024u)
#define IPC_B_FRAME_BYTES    8u   /* stereo int32 is the worst case     */

/* ---- API (both implementations) ---- */

/* One-time init (ring + supervisor state). Returns 0. */
int ipc_b_init(void);

/* Start the B core for `track` (loads/starts the B audio path — the
 * documented "B start 0 at 400 MHz" step, then clock negotiation).
 * Returns 0, or < 0 on error. */
int ipc_b_start(const ipc_b_track_t *track);

/* Publish the stream geometry the AP discovered while opening a track
 * (codec_t.prescan_info) so Core B's re-open can size its decode pool without
 * repeating the header walk — that walk sits inside the gapless boundary.
 * Copied here and pushed to Core B with the next OPEN/NEXT command. Pass NULL
 * to clear (non-FLAC or an open that shares nothing). */
void ipc_b_set_prescan(const codec_prescan_t *p);

/* Diagnostic: the player's own codec open failed for `path`. No command ever
 * reaches Core B in that case, so without this the field log shows nothing at
 * the boundary. The target logs the file name; the host mock is a no-op. */
void ipc_b_note_open_failed(const char *path);

/* Stop the B core ("B off") and reset streaming state. Returns 0. */
int ipc_b_stop(void);

/* Pause/resume consumption without discarding the ring (1 = paused).
 * Extension beyond the minimum API — pause keeps buffered PCM. */
int ipc_b_pause(int paused);

/* Set playback volume in dB (forwarded to the B core). Returns 0. */
int ipc_b_set_volume(int db);

/* Announce a new track on an EXISTING stream (gapless continuation). The B
 * core is NOT restarted: the PCM ring keeps its read cursor and the stream
 * keeps flowing, so adjacent same-format tracks join sample-accurately with
 * no click/silence. `duration_ms` is the new track's length (used by the
 * supervisor's drained-timeout; 0 = unknown, keep ticking). Must only be
 * called while the B core is RUNNING. Returns 0, < 0 on error.
 *
 * This is the A<->B half of "B: boundary to track N: ... (gapless)"
 * (reference ECHOLOG). On the target it maps to a doorbell that advances
 * the B-side stream length without a core reset (register-level bring-up
 * TODO in docs/rewrite/player.md §7). The mock simply updates its length
 * bookkeeping. */
int ipc_b_track_change(long duration_ms);

/* Poll the supervisor state machine: advances the sim on the host mock,
 * samples the shared block on target. Fills `out` (may be NULL).
 * Returns the current ipc_b_state_t (>= 0) or < 0 on error. */
int ipc_b_poll_state(ipc_b_status_t *out);

/* Producer side: copy `frames` interleaved int32 frames into the ring.
 * Returns the number of frames actually accepted (clamped on overflow). */
int ipc_b_ring_fill(const int32_t *pcm, int frames);

/* Discard all buffered PCM and zero the per-stream consumed counter
 * (used on seek; only legal while paused/stopped or between fills). */
int ipc_b_flush(void);

/* Announce a NEW TRACK on the SAME live stream (gapless boundary).
 * The B core is NOT restarted (no ring flush, no clock relock); the
 * supervisor updates its expected length with `duration_ms`. cmd_arg0.
 * Returns non-negative on acceptance. */
int ipc_b_track_change(long duration_ms);

/* Gapless track change on Core B (BRNOMT-73). Hand the next track to Core B
 * ON THE LIVE STREAM: the AP opens the new file (bfs_open) and asks B to
 * reopen its codec (IPC_C_OPEN), while the PCM ring cursors and the
 * DAC/serializer stay untouched — same-format adjacent tracks join
 * sample-accurately with no click/silence. The AP must have set the new
 * track's trim/RG in the shared block first (this call does it from `track`).
 * Only legal while Core B is the decoder (ipc_b_b_decodes()); a format change
 * must NOT use this (the DAC has to re-lock — take the hard boundary).
 * Returns 0 on success, < 0 if Core B is not the decoder or did not ACK. */
int ipc_b_next(const ipc_b_track_t *track);

/* Pre-open the NEXT track while the current one still plays (BRNOMT-73): opens
 * the file and starts filling the second file ring, so ipc_b_next() at the
 * boundary does not pay the SD open. No-op (returns 0) when the AP is decoding.
 * Idempotent for the same path. Returns 0, or < 0 if the file cannot be
 * opened (the boundary then falls back to the hard path). */
int ipc_b_preopen(const char *path);

/* ---- phase 3c: decode on Core B ---- */

/* 1 when Core B is decoding (it is the PCM-ring producer; the AP must not
 * call ipc_b_ring_fill()). 0 on the host mock and when the handoff failed. */
int ipc_b_b_decodes(void);

/* 1 when Core B's decoder has drained the track (end of stream). */
int ipc_b_b_eof(void);

/* Ask Core B to reposition the (B-side) decoder. No-op when B is not the
 * producer. Only legal while running/paused. */
int ipc_b_seek(long ms);

/* Update the ReplayGain Q16 gain used by Core B (mode toggled mid-track).
 * No-op on the host mock (the AP applies its own gain there). */
int ipc_b_set_replaygain(int gain_q16);

#endif /* RECHORD_IPC_B_H */
