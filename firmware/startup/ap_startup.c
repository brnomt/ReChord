/*
 * ReChord AP (fw1) startup -- Cortex-M3 vector table + reset handler for the
 * A_CORE build.
 *
 * VERIFIED BOOT MODEL (docs/HANDOVER.md "PHASE A AUDIT RESULT"): the loader
 * scatter-loads the fw1 body into RAM contiguously (RAM = 0x0304F298 + IMG
 * offset) so the vector table lands at RAM 0x03050000 = IMG 0xD68 (the
 * RKnanoFW header base) and the image executes in place.  The ROM reads
 * word 0 (initial MSP) and word 1 (reset) from that table and enters at the
 * reset handler.  This file mirrors the reference image's recipe (its
 * startup at RAM 0x030500E4): bounded .data copy, bounded .bss zero, VTOR =
 * vector table, dsb/isb, Main.
 *
 * The stack is the 16 KiB BELOW 0x03004000 (the zone 0x03000000..0x03004000)
 * and nothing else may live there.  BSS is also zero-fillable by scatter
 * region rows, so the explicit clear here is defensive (idempotent).
 */
#include <stdint.h>

/* NOTE (milestone 1a compile fix): this file previously typedef'd uint32
 * locally and used ap_vectors before declaring it.  The local typedef
 * collided with the SDK typedef.h uint32 (unsigned long) that arrives via
 * the force-included armcc_compat.h, and the use-before-declaration of
 * ap_vectors is a hard error in every C mode.  Fixed by using stdint's
 * uint32_t throughout and declaring ap_vectors up front. */

extern const uint32_t ap_vectors[49];   /* defined at the bottom of this file */

extern uint32_t Main(void);
extern uint32_t _bss_start, _bss_end;
extern uint32_t _data_start, _data_end;

/* Initial MSP: byte-verified word 0 of the reference vector table.  The
 * stack zone is 0x03000000..0x03004000 ONLY (firmware_ap.ld __stack_top). */
#define AP_INITIAL_MSP 0x03004000u

void Default_Handler(void) __attribute__((naked));
void Default_Handler(void)
{
    for (;;)
        __asm volatile("");
}

/* Reset handler per the verified boot recipe (extracted from the reference
 * image's startup at 0x030500E4): zero ONLY the real .bss span, set VTOR to
 * our 256-aligned vector table, barriers, then main. The ROM does not set
 * VTOR - the reference writes it explicitly and so do we. */
#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08u)
void PowerOn_Reset(void) __attribute__((noreturn, used));
void PowerOn_Reset(void)
{
    /* Reference startup 0x030500E4: quiesce whatever the ROM left running
     * before VTOR moves to our table (any live IRQ would land in
     * Default_Handler). SysTick off, all NVIC IRQs disabled + unpended,
     * SHCSR cleared, AIRCR VECTCLRACTIVE. */
    __asm__ volatile("cpsid i");
    *(volatile uint32_t *)0xE000E010u = 0;
    *(volatile uint32_t *)0xE000E180u = 0xFFFFFFFFu;
    *(volatile uint32_t *)0xE000E184u = 0xFFFFFFFFu;
    *(volatile uint32_t *)0xE000E280u = 0xFFFFFFFFu;
    *(volatile uint32_t *)0xE000E284u = 0xFFFFFFFFu;
    *(volatile uint32_t *)0xE000ED24u = 0;
    *(volatile uint32_t *)0xE000ED0Cu = 0x05FA0002u;
    /* No .data copy here: the ROM already copied it (scatter row 1). Our
     * row 0 ends where the .data LMA begins, so that LMA is never loaded
     * (verdict #23: copying from it clobbered every initialized global). */
    uint32_t *p = &_bss_start;
    while (p < &_bss_end)
        *p++ = 0;
    SCB_VTOR = (uint32_t)&ap_vectors;   /* our table, 256-aligned */
    __asm__ volatile("dsb; isb" ::: "memory");
    Main();
#ifdef AP_CHAIN_REF
    /* ENTRY test (make entry-img): hand the ROM's r0-r3 to the reference's
     * reset (veneer -> its startup 0x030500E4, which redoes SP/IRQ/data/bss/
     * VTOR) so the device always ends in a firmware with USB + updater. */
    __asm__ volatile("ldr r4, =0x03003FF0\n ldm r4, {r0-r3}\n"
                     "ldr r4, =0x0306296F\n bx r4");
#endif
    for (;;)
        __asm volatile("");
}

#ifdef AP_CHAIN_REF
/* ROM entry: park r0-r3 (boot params; the ref startup saves them) at the
 * top of the stack zone, run below them. */
void ChainEntry(void) __attribute__((naked));
void ChainEntry(void)
{
    __asm__ volatile("ldr r4, =0x03003FF0\n stm r4, {r0-r3}\n mov sp, r4\n"
                     "b PowerOn_Reset");
}
#endif

/* ROM boot contract (verdict #19, matches reference + stock): the ROM copies
 * .data (scatter row 1, image right after row 0) to 0x03004000 and enters
 * through the vector table at its start - so .data must begin with one. */
const uint32_t ap_ram_vectors[2]
    __attribute__((section(".data.ram_vectors"), used)) = {
    AP_INITIAL_MSP,
    (uint32_t)&PowerOn_Reset,
};

/* Cortex-M3 vector table (aligned 256 for VTOR). [0]=initial MSP, [1]=reset.
 * Byte-identical word 0/word 1 roles to the reference image's table at
 * IMG 0xD68: word 0 = 0x03004000 (the stack TOP; the stack is the 16 KiB
 * below it), word 1 = the reset entry.
 * Peripheral slots: IRQn N sits at table index 16+N.  Only I2S1
 * (INT_ID_I2S1 = 48 in the SDK interrupt.h enum = NVIC IRQn 32 = index 48)
 * is wired so far — its TX-empty IRQ feeds the audio ring (route A,
 * docs/HANDOVER.md).  All other NVIC lines stay disabled (the reset
 * handler unpends/disables everything), so Default_Handler coverage is
 * belt-and-braces.  49 words = 196 bytes still fits the 256-byte VTOR
 * alignment window. */
extern void dac_i2s_tx_isr(void);

const uint32_t ap_vectors[49]
    __attribute__((section(".vectors"), used, aligned(256))) = {
    AP_INITIAL_MSP,                         /* 0x00 initial MSP = 0x03004000 */
    (uint32_t)&PowerOn_Reset,               /* 0x04 Reset_Handler             */
    (uint32_t)&Default_Handler,         /* 0x08 NMI                      */
    (uint32_t)&Default_Handler,         /* 0x0C HardFault                */
    (uint32_t)&Default_Handler,         /* 0x10 MemManage                */
    (uint32_t)&Default_Handler,         /* 0x14 BusFault                 */
    (uint32_t)&Default_Handler,         /* 0x18 UsageFault               */
    0, 0, 0, 0,                        /* 0x1C-0x28 reserved            */
    (uint32_t)&Default_Handler,         /* 0x2C SVCall                   */
    (uint32_t)&Default_Handler,         /* 0x30 DebugMon                 */
    0,                                  /* 0x34 reserved                 */
    (uint32_t)&Default_Handler,         /* 0x38 PendSV                   */
    (uint32_t)&Default_Handler,         /* 0x3C SysTick                  */
    /* IRQ0..IRQ31 (indices 16..47): unused, disabled at reset. */
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&Default_Handler, (uint32_t)&Default_Handler,
    (uint32_t)&dac_i2s_tx_isr,          /* IRQ32 = INT_ID_I2S1 (audio TX)  */
};
