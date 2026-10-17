/*
 * bb_main.c — Core B audio DECODER (BRNOMT-68 phase 3c).
 *
 * Phase 2 had B render (the AP decoded). Phase 3c moves the DECODE here: the
 * AP owns the filesystem and streams the open track into the A->B file ring
 * (firmware/player/bfs.c), B's codec_io reads from it (firmware/bb/bcio.c),
 * B decodes with our normal codecs and becomes the PCM-ring PRODUCER. The AP
 * keeps the DAC/I2S bring-up and drains the ring in its TX ISR (consumer), so
 * B never touches the I2S and needs no IRQ of its own.
 *
 * B memory (bb.ld): code in HIGHRAM0; .data/.bss (the codec decode pool from
 * services/workbuf + the codec state) and the stack in the B data region
 * 0x01040000..0x01060000.
 *
 * Commands come through the control block (ipc_shared.h): OPEN/SEEK/CLOSE via
 * IPC_C_*, play/pause/stop via IPC_SH_STATE.
 */
#include <stdint.h>

#include "ipc_shared.h"
#include "codec.h"

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

static void dmb(void)
{
    __asm__ volatile("dmb" ::: "memory");
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
    uint32_t cmd = CTRL[IPC_C_CMD];

    if (cmd == CTRL[IPC_C_ACK])
        return;

    switch (cmd) {
    case IPC_C_OPEN: {
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
            if (s_codec->format != 0)
                (void)s_codec->format(s_cstate, &f),
                s_channels = (f.channels == 1) ? 1 : 2;
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
    CTRL[IPC_C_ACK] = cmd;
    dmb();
}

static void decode_round(void)
{
    uint32_t rd, wr, freeb;
    int got;

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

    if (s_channels == 1) {            /* upmix mono -> stereo on the wire */
        int i;

        for (i = got - 1; i >= 0; i--) {
            s_pcm[2 * i] = s_pcm[i];
            s_pcm[2 * i + 1] = s_pcm[i];
        }
    }

    ring_write(wr, s_pcm, (uint32_t)got * 8u);
    wr += (uint32_t)got * 8u;
    dmb();
    CTRL[IPC_SH_WR] = wr;             /* publish after the data */
    dmb();
    I2S_INTCR |= INTCR_TX_EMPTY;      /* re-arm the AP's TX-empty feeder */
}

void bb_reset(void)
{
    uint32_t *d, *s;

    for (d = _bb_data_start, s = _bb_data_load; d < _bb_data_end; )
        *d++ = *s++;
    for (d = _bb_bss_start; d < _bb_bss_end; )
        *d++ = 0u;
    dmb();

    CTRL[IPC_SH_MAGIC] = IPC_SH_MAGIC_VAL;   /* phase-1 gate / AP poll */
    CTRL[IPC_C_BSTATE] = IPC_BS_IDLE;

    for (;;) {
        handle_command();

        if (CTRL[IPC_SH_STATE] == IPC_SH_RUN)
            decode_round();
    }
}
