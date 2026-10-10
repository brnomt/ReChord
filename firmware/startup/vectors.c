/*
 * vectors.c — Cortex-M3 core vector table for the ReChord core image.
 *
 * BOOT CONTRACT (docs/re/route-b-minimum.md §2, byte-verified): the vector
 * table sits at 0x03050000 in the RAM image; its first two words are
 *
 *   [0] initial SP = 0x03004000
 *   [1] Reset      = the startup entry (the reference chains a veneer at
 *       0x0306296E to its real entry at 0x030500E4; ours points straight
 *       at our clean-room entry).
 *
 * Our Reset target is rechord_startup_entry (firmware/startup/startup.c):
 * IRQs off -> boot param capture -> .data copy -> .bss zero -> DSB/ISB ->
 * rechord_main -> hang.
 *
 * Table contents: the 16 core Cortex-M3 entries (initial SP, Reset, NMI,
 * HardFault, MemManage, BusFault, UsageFault, reserved, SVCall, DebugMon,
 * reserved, PendSV, SysTick). The RKNanoC peripheral IRQ slots follow once
 * the IRQ map is integrated (TODO) — routing them all to
 * rechord_default_handler until then is safe: a spurious exception parks in
 * a visible loop instead of running garbage.
 *
 * The table is 256-byte aligned because VTOR's low bits are forced to 0.
 * Handler words must be ODD (Thumb bit). A Thumb function's address
 * already carries bit 0 (its ELF symbol value is odd, and a static
 * initializer only accepts an unadorned address constant — `(uintptr_t)&fn
 * | 1` is rejected by GCC), so the plain cast below IS the correct odd
 * handler address; the requirement is kept in this comment instead.
 *
 * The section is .vectors so the image link script can place/KEEP it (the
 * Route-B link puts the table at 0x03050000).
 */
#include <stdint.h>

extern void rechord_startup_entry(void);

/* Park-forever default for every not-yet-wired exception. */
void rechord_default_handler(void);
void rechord_default_handler(void)
{
    for (;;)
        __asm volatile("" ::: "memory");
}

const uint32_t vectors[16]
    __attribute__((section(".vectors"), used, aligned(256))) = {
    0x03004000u,                                        /* 0x00 initial MSP */
    (uint32_t)(uintptr_t)&rechord_startup_entry,        /* 0x04 Reset       */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x08 NMI         */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x0C HardFault   */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x10 MemManage   */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x14 BusFault    */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x18 UsageFault  */
    0u,                                                 /* 0x1C reserved    */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x20 SVCall      */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x24 DebugMon    */
    0u,                                                 /* 0x28 reserved    */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x2C PendSV      */
    (uint32_t)(uintptr_t)&rechord_default_handler,      /* 0x30 SysTick     */
    /* Peripheral IRQ slots: TODO once the RKNanoC IRQ map lands (the
     * remaining slots zero-fill, which is safe: they are never enabled). */
};
