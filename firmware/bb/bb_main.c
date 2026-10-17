/*
 * bb_main.c — Core B audio renderer (BRNOMT-68 phase 2).
 *
 * Phase 1 proved our code runs on the second Cortex-M3 (heartbeat). Phase 2
 * makes B render: the AP decodes and produces PCM into the shared SPSC ring
 * in HIGHRAM0 (firmware/player/ipc_shared.h); B polls the ring and pushes the
 * 32-bit left-justified slot words into the I2S1 TX FIFO. The AP still owns
 * the DAC/I2S bring-up (phase 3 moves the decode to B).
 *
 * No libc, no RTOS, no interrupts: B is dedicated to the feed, so a tight
 * poll loop is the simplest and most predictable consumer. The I2S bit clock
 * runs independently (master on the SoC), so pitch/speed are unaffected by
 * the poll cadence.
 */
#include <stdint.h>

#include "ipc_shared.h"

#define REG32(a)   (*(volatile uint32_t *)(uintptr_t)(a))
#define I2S_FIFOLR REG32(0x4003000Cu)
#define I2S_TXDR   REG32(0x40030024u)

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

extern uint32_t __bb_stack_top;
void bb_reset(void);

/* Data/BSS live in SYSRAM1 (bb.ld); the flat image only carries the .data
 * LMA, so the reset entry copies it down and zeroes .bss first. */
extern uint32_t _bb_data_start[], _bb_data_end[], _bb_data_load[];
extern uint32_t _bb_bss_start[], _bb_bss_end[];

/* Vector table: SP, Reset. Must be the first bytes of the image. */
__attribute__((section(".vectors_bb"), used))
static const void *const bb_vectors[2] = {
    (void *)&__bb_stack_top,
    (void *)bb_reset
};

void bb_reset(void)
{
    uint32_t *d, *s;

    for (d = _bb_data_start, s = _bb_data_load; d < _bb_data_end; )
        *d++ = *s++;
    for (d = _bb_bss_start; d < _bb_bss_end; )
        *d++ = 0u;
    __asm__ volatile("dsb" ::: "memory");

    CTRL[IPC_SH_MAGIC] = IPC_SH_MAGIC_VAL;      /* phase-1 gate / AP poll */

    for (;;) {
        uint32_t rd, wr;

        if (CTRL[IPC_SH_STATE] != IPC_SH_RUN)
            continue;

        /* Top the TX FIFO up past the IRQ threshold (same 0x1D as the AP
         * feeder). Each ring frame is two int32 left-justified slot words. */
        while ((I2S_FIFOLR & 0x3Fu) < 0x1Du) {
            rd = CTRL[IPC_SH_RD];
            wr = CTRL[IPC_SH_WR];
            if (rd == wr) {
                CTRL[IPC_SH_UNDERRUNS] = CTRL[IPC_SH_UNDERRUNS] + 1u;
                break;                          /* ring dry: wait */
            }
            {
                const volatile uint32_t *f =
                    (const volatile uint32_t *)(IPC_RING_BASE +
                                                (rd & IPC_RING_MASK));
                I2S_TXDR = f[0];
                I2S_TXDR = f[1];
            }
            rd += 8u;
            CTRL[IPC_SH_RD] = rd;
            CTRL[IPC_SH_FRAMES] = CTRL[IPC_SH_FRAMES] + 1u;
            __asm__ volatile("dmb" ::: "memory");
        }
    }
}
