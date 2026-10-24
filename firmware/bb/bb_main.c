/*
 * bb_main.c — Core B audio DECODER (BRNOMT-68 phase 3c).
 *
 * Phase 2 had B render (the AP decoded). Phase 3c moves the DECODE here: the
 * AP owns the filesystem and streams the open track into the A->B file ring
 * (firmware/player/bfs.c), B's codec_io reads from it (firmware/bb/bcio.c),
 * B decodes with our normal codecs, applies the PCM post-processing the AP
 * used to do (gapless leading/trailing trim + ReplayGain) and becomes the
 * PCM-ring PRODUCER. The AP keeps the DAC/I2S bring-up and drains the ring in
 * its TX ISR (consumer), so B never touches the I2S and needs no IRQ of its
 * own.
 *
 * B memory (bb.ld): code in HIGHRAM0; .data/.bss (the codec decode pool from
 * services/workbuf + the codec state) and the stack in the B data region
 * 0x01040000..0x01060000.
 *
 * Commands come through the control block (ipc_shared.h): OPEN/NEXT/SEEK/CLOSE
 * via IPC_C_* (each carries a sequence number in IPC_C_SEQ; B echoes it in
 * IPC_C_ACK so a repeated command is not dropped), play/pause/stop via
 * IPC_SH_STATE. NEXT is a gapless track change (BRNOMT-73): it reopens the
 * codec exactly like OPEN but the PCM ring keeps flowing. ReplayGain (Q16) and
 * the gapless trim (delay/padding/total) come in IPC_C_RG / IPC_C_GAP_*.
 */
#include <stdint.h>
#include <string.h>

#include "ipc_shared.h"
#include "codec.h"
#include "rg_math.h"
#include "dsp/eq_bank.h"
#include "eq_ipc.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

/* I2S1 control (shared peripheral): bit0 = TX-empty IRQ enable. The AP's TX
 * ISR masks this when it finds the ring dry; B re-arms it after pushing data
 * (B is the producer, so the AP cannot know when to kick). */
#define REG32(a)   (*(volatile uint32_t *)(uintptr_t)(a))
#define I2S_INTCR  REG32(0x40030014u)
#define INTCR_TX_EMPTY 1u

/* Codec descriptors (the adapters own dr_flac/minimp3). */
extern const codec_t codec_wav;
extern const codec_t codec_mp3;
extern const codec_t codec_flac;

extern uint32_t __bb_stack_top;
void bb_reset(void);

/* Data/BSS live in the B data region (bb.ld); the flat image only carries the
 * .data LMA, so the reset entry copies it down and zeroes .bss first. */
extern uint32_t _bb_data_start[], _bb_data_end[], _bb_data_load[];
extern uint32_t _bb_bss_start[], _bb_bss_end[];

/* Vector table: SP, Reset. Must be the first bytes of the image. */
__attribute__((section(".vectors_bb"), used))
static const void *const bb_vectors[2] = {
    (void *)&__bb_stack_top,
    (void *)bb_reset
};

#define CHUNK_FRAMES 1024u

static uint8_t       s_cstate[PLAYER_CODEC_STATE_MAX];
static int32_t       s_pcm[CHUNK_FRAMES * 2];
static const codec_t *s_codec;
static int           s_channels = 2;
static int           s_rate = 48000;

/* BRNOMT-76: EQ bank applied to the decoded block before it enters the ring.
 * The curve itself arrives over the shared block (eq_ipc.h seqlock). */
static eq_bank_state_t s_eq;
static uint32_t        s_eq_gen;

/* PCM post-processing state (mirrors player.c's gapless cursors). */
static long s_emitted;              /* frames emitted after leading trim */
static int  s_leading_trimmed = 1;  /* 0 until the encoder delay is dropped */
static int  s_delay, s_pad;
static long s_total;                /* exact decoder frame count (0 unknown) */

static void dmb(void)
{
    __asm__ volatile("dmb" ::: "memory");
}

/* (Re)load the per-track gapless trim and reset its cursors. */
static void trim_reset(void)
{
    s_delay = (int)CTRL[IPC_C_GAP_DELAY];
    s_pad = (int)CTRL[IPC_C_GAP_PAD];
    s_total = (long)CTRL[IPC_C_GAP_TOTAL];
    s_emitted = 0;
    s_leading_trimmed = (s_delay == 0) ? 1 : 0;
}

static void ring_write(uint32_t off, const void *src, uint32_t n)
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

static const codec_t *codec_by_id(uint32_t id)
{
    if (id == 1u)
        return &codec_mp3;
    if (id == 2u)
        return &codec_flac;
    return &codec_wav;
}

static void handle_command(void)
{
    uint32_t seq = CTRL[IPC_C_SEQ];
    uint32_t cmd;

    /* Sequence-based handshake: a repeated command (two NEXTs, two SEEKs to
     * the same ms) still has a fresh sequence, so it is not dropped. */
    if (seq == CTRL[IPC_C_ACK])
        return;

    cmd = CTRL[IPC_C_CMD];

    switch (cmd) {
    case IPC_C_OPEN:
    case IPC_C_NEXT: {         /* NEXT = OPEN-style reopen on the live ring */
        unsigned i;
        codec_format_t f;

        if (s_codec != 0) {
            s_codec->close(s_cstate);
            s_codec = 0;
        }
        for (i = 0; i < sizeof s_cstate; i++)
            ((uint8_t *)s_cstate)[i] = 0;
        s_codec = codec_by_id(CTRL[IPC_C_ARG]);
        if (s_codec->open(s_cstate, "") == 0) {
            s_channels = 2;
            if (s_codec->format != 0) {
                (void)s_codec->format(s_cstate, &f);
                s_channels = (f.channels == 1) ? 1 : 2;
                if (f.sample_rate > 0)
                    s_rate = f.sample_rate;
            }
            trim_reset();
            (void)eq_bank_set_rate(&s_eq, s_rate);
            /* OPEN starts a new stream: stale history from another position
             * would click. NEXT is gapless, so it keeps the history (the rate
             * change, if any, already reset it inside set_rate). */
            if (cmd == IPC_C_OPEN)
                eq_bank_reset(&s_eq);
            CTRL[IPC_C_BEOF] = 0;
            CTRL[IPC_C_BERR] = 0;
            CTRL[IPC_C_BSTATE] = IPC_BS_OPEN;
        } else {
            s_codec = 0;
            CTRL[IPC_C_BERR] = 1;
            CTRL[IPC_C_BSTATE] = IPC_BS_ERROR;
        }
        break;
    }
    case IPC_C_SEEK:
        if (s_codec != 0) {
            (void)s_codec->seek(s_cstate, (long)CTRL[IPC_C_ARG]);
            trim_reset();               /* audible stream repositioned */
            eq_bank_reset(&s_eq);
            CTRL[IPC_C_BEOF] = 0;
        }
        break;
    case IPC_C_CLOSE:
        if (s_codec != 0) {
            s_codec->close(s_cstate);
            s_codec = 0;
        }
        CTRL[IPC_C_BSTATE] = IPC_BS_IDLE;
        break;
    default:
        break;
    }
    dmb();
    CTRL[IPC_C_ACK] = seq;        /* acknowledge the command sequence */
    dmb();
}

static void decode_round(void)
{
    uint32_t rd, wr, freeb;
    int got, gain;

    if (s_codec == 0 || CTRL[IPC_C_BEOF])
        return;

    rd = CTRL[IPC_SH_RD];
    wr = CTRL[IPC_SH_WR];
    freeb = IPC_RING_BYTES - (wr - rd);
    if (freeb < CHUNK_FRAMES * 8u)
        return;                       /* ring full; the AP's ISR is draining */

    got = s_codec->decode(s_cstate, s_pcm, (int)CHUNK_FRAMES);
    if (got < 0) {
        CTRL[IPC_C_BERR] = (uint32_t)got;
        CTRL[IPC_C_BSTATE] = IPC_BS_ERROR;
        dmb();
        return;
    }
    if (got == 0) {
        CTRL[IPC_C_BEOF] = 1;         /* decoder drained */
        CTRL[IPC_C_BSTATE] = IPC_BS_EOF;
        dmb();
        return;
    }

    /* Gapless: drop the encoder-delay samples at the very start. */
    {
        int lead = s_leading_trimmed ? 0 : s_delay;

        if (lead > 0) {
            if (lead >= got)
                return;               /* whole batch was delay: drop it */
            memmove(s_pcm, s_pcm + (long)lead * s_channels,
                    (size_t)((long)(got - lead) * s_channels) *
                        sizeof(int32_t));
            got -= lead;
            s_leading_trimmed = 1;
        }
    }

    s_emitted += got;

    /* Gapless: stop at total - delay - padding (the last audible sample). */
    if (s_total > 0) {
        long tail = s_total - s_delay - s_pad;
        long emit;

        if (tail < 0)
            tail = 0;
        if (s_emitted >= tail) {
            emit = tail - (s_emitted - got);
            got = (emit > 0) ? (int)emit : 0;
            CTRL[IPC_C_BEOF] = 1;
            CTRL[IPC_C_BSTATE] = IPC_BS_EOF;
        }
    }
    if (got <= 0)
        return;                       /* nothing audible left in this batch */

    /* ReplayGain: fixed gain (Q16) on the trimmed PCM. */
    gain = (int)CTRL[IPC_C_RG];
    if (gain != 32768)
        rg_scale_pcm(s_pcm, (long)got * s_channels, gain);

    if (s_channels == 1) {            /* upmix mono -> stereo on the wire */
        int i;

        for (i = got - 1; i >= 0; i--) {
            s_pcm[2 * i] = s_pcm[i];
            s_pcm[2 * i + 1] = s_pcm[i];
        }
    }

    /* BRNOMT-76: install a newly published curve, then run the EQ on the
     * full block. The buffer is interleaved STEREO here (mono was upmixed
     * above), so the sample count is frames * 2. */
    {
        eq_curve_t curve;

        if (eq_ipc_read(CTRL, &s_eq_gen, &curve) == 1)
            (void)eq_bank_apply(&s_eq, &curve);
    }
    eq_bank_process(&s_eq, s_pcm, got * 2);

    ring_write(wr, s_pcm, (uint32_t)got * 8u);
    wr += (uint32_t)got * 8u;
    dmb();
    CTRL[IPC_SH_WR] = wr;             /* publish after the data */
    dmb();
    I2S_INTCR |= INTCR_TX_EMPTY;      /* re-arm the AP's TX-empty feeder */
}

/* Stream geometry the AP prescanned while pre-opening the file (BRNOMT-73):
 * lets the FLAC adapter skip its own header walk — that walk is a seek
 * round-trip per metadata block and sits inside the gapless boundary. */
static int b_prescan_read(unsigned *max_block, unsigned *channels,
                          unsigned long *seektable)
{
    if (CTRL[IPC_C_PRE_VALID] != 1u)
        return 0;
    *max_block = CTRL[IPC_C_PRE_MAXBLOCK];
    *channels = CTRL[IPC_C_PRE_CHANNELS];
    *seektable = (unsigned long)CTRL[IPC_C_PRE_SEEKTABLE];
    return 1;
}

void bb_reset(void)
{
    uint32_t *d, *s;

    for (d = _bb_data_start, s = _bb_data_load; d < _bb_data_end; )
        *d++ = *s++;
    for (d = _bb_bss_start; d < _bb_bss_end; )
        *d++ = 0u;
    dmb();

    /* Register AFTER the .data/.bss copy (the hook lives in the codec's
     * statics, which that copy just zeroed). */
    flac_set_prescan_source(b_prescan_read);
    eq_bank_init(&s_eq, s_rate);

    CTRL[IPC_SH_MAGIC] = IPC_SH_MAGIC_VAL;   /* phase-1 gate / AP poll */
    CTRL[IPC_C_BSTATE] = IPC_BS_IDLE;

    for (;;) {
        handle_command();

        if (CTRL[IPC_SH_STATE] == IPC_SH_RUN)
            decode_round();
    }
}
