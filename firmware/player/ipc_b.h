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
 *   interleaved int16 PCM; the B core consumes PCM from the ring and drives
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
    int         channels;      /* 1 or 2, interleaved int16             */
    long        duration_ms;   /* for the drained/timeout supervisor    */
    const char *path;          /* logging aid on the B side; may be NULL*/
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

/* PCM ring geometry shared by both implementations (bytes, pow2). */
#define IPC_B_RING_BYTES     (64u * 1024u)
#define IPC_B_FRAME_BYTES    4u   /* stereo int16 is the worst case     */

/* ---- API (both implementations) ---- */

/* One-time init (ring + supervisor state). Returns 0. */
int ipc_b_init(void);

/* Start the B core for `track` (loads/starts the B audio path — the
 * documented "B start 0 at 400 MHz" step, then clock negotiation).
 * Returns 0, or < 0 on error. */
int ipc_b_start(const ipc_b_track_t *track);

/* Stop the B core ("B off") and reset streaming state. Returns 0. */
int ipc_b_stop(void);

/* Pause/resume consumption without discarding the ring (1 = paused).
 * Extension beyond the minimum API — pause keeps buffered PCM. */
int ipc_b_pause(int paused);

/* Set playback volume in dB (forwarded to the B core). Returns 0. */
int ipc_b_set_volume(int db);

/* Poll the supervisor state machine: advances the sim on the host mock,
 * samples the shared block on target. Fills `out` (may be NULL).
 * Returns the current ipc_b_state_t (>= 0) or < 0 on error. */
int ipc_b_poll_state(ipc_b_status_t *out);

/* Producer side: copy `frames` interleaved int16 frames into the ring.
 * Returns the number of frames actually accepted (clamped on overflow). */
int ipc_b_ring_fill(const int16_t *pcm, int frames);

/* Discard all buffered PCM and zero the per-stream consumed counter
 * (used on seek; only legal while paused/stopped or between fills). */
int ipc_b_flush(void);

#endif /* RECHORD_IPC_B_H */
