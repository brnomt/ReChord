/*
 * bb_main.c — minimal Core B image (BRNOMT-68 phase 1).
 *
 * Goal: prove OUR code runs on the second Cortex-M3 before moving any audio.
 * It sets the vector table at HIGHRAM0 0x01000000 (Core B vectors there after
 * CAL_CORE_SRST is released), writes a magic + a heartbeat counter into the
 * shared block at 0x01020000, and loops. No libc, no RTOS, no mailbox: the AP
 * polls shared[0] with a timeout (non-blocking).
 */
#include <stdint.h>

#define SHARED    ((volatile uint32_t *)0x01020000u)
#define BB_MAGIC  0x42524459u        /* 'BRDY' */

extern uint32_t __bb_stack_top;
void bb_reset(void);

/* Vector table: SP, Reset. Must be the first bytes of the image. GCC's Thumb
 * function pointer already carries bit0 (STT_FUNC in Thumb mode). */
__attribute__((section(".vectors_bb"), used))
static const void *const bb_vectors[2] = {
    (void *)&__bb_stack_top,
    (void *)bb_reset
};

void bb_reset(void)
{
    SHARED[0] = BB_MAGIC;
    for (;;) {
        SHARED[1] = SHARED[1] + 1u;
        for (volatile uint32_t i = 0; i < 200000u; i++)
            ;
    }
}
