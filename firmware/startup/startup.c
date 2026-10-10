/*
 * startup.c — ReChord clean-room C-runtime startup (Echo Mini / RKnanoC).
 *
 * Two responsibilities (this file evolved from the original single startup;
 * the vector table moved to vectors.c — same boot contract, split for
 * clarity):
 *
 *   1. The 16-byte RKnanoFW image header used by the section_3 image
 *      (byte-identical to stock v3.7.0; pack_img.py only splices, it does
 *      not rewrite the magic).
 *
 *   2. The verified boot-contract C-runtime startup (docs/re/
 *      route-b-minimum.md §2), executed by our Reset entry:
 *        IRQs off -> capture 4 boot params -> .data copy -> .bss zero
 *        -> DSB/ISB -> call rechord_main -> hang.
 *
 * Memory model (byte-verified facts, same doc §2): the RAM image loads at
 * 0x0304F490; the vector table lives at 0x03050000 (SP 0x03004000) and its
 * Reset word points at our entry (the reference chains a veneer at
 * 0x0306296E to its real entry at 0x030500E4 — we point straight at ours).
 *
 * Linker contract (symbols the image link must provide):
 *   _data_start/_data_end  — .data VMA bounds (firmware.ld provides these)
 *   _bss_start/_bss_end    — .bss bounds
 *   _data_load_start       — OPTIONAL .data load address (LMA). Declared
 *                            weak: when the link does not provide it (RAM
 *                            image where .data already runs in place, e.g.
 *                            the legacy firmware.ld), the copy is skipped.
 *
 * Boot params: see firmware/app/boot_params.h — the 4 ROM handoff words are
 * stored bit-exact; their field identity is an open item (TODO).
 */
#include <stdint.h>

#include "app/boot_params.h"

/* ---- 16-byte RKnanoFW header (byte-exact with stock v3.7.0) ---- */
const uint8_t fw_image_header[16]
    __attribute__((section(".fw_header"), used, aligned(16))) = {
    'R', 'K', 'n', 'a', 'n', 'o', 'F', 'W',  /* magic */
    0x94, 0xE7, 0x01, 0x03,                  /* 0x0301E794 LE: initial SP */
    0x52, 0x00, 0x00, 0x00,                  /* 0x52: count/flags */
};

/* ---- linker contract symbols ---- */
extern uint32_t _data_start[], _data_end[];
extern uint32_t _bss_start[],  _bss_end[];
extern uint32_t _data_load_start[] __attribute__((weak));

/* ---- boot param storage (see boot_params.h STORAGE CONTRACT: exactly one
 *      TU defines these; on target it is this file) ---- */
rechord_boot_params_t rechord_boot_params;
uint32_t              rechord_boot_params_captured;

/* Declare main here so this TU never depends on app headers beyond
 * boot_params.h. Implemented by firmware/app/main.c (integrated image) or
 * firmware/rechord_app.c (legacy BB image). */
extern void rechord_main(void);

/* ---- tiny architecture helpers (host-compilable for future tests) ---- */

/* Step 1 of the contract: interrupts off before touching C runtime state —
 * an IRQ firing on a half-initialized .data/.bss is undefined. */
static void irq_disable(void)
{
#if defined(__arm__)
    __asm volatile("cpsid i" ::: "memory");
#else
    __asm volatile("" ::: "memory");
#endif
}

/* Step 5 of the contract: DSB + ISB so the memory init is architecturally
 * visible before the first instruction of main executes. */
static void sync_barriers(void)
{
#if defined(__arm__)
    __asm volatile("dsb" ::: "memory");
    __asm volatile("isb" ::: "memory");
#else
    __asm volatile("" ::: "memory");
#endif
}

/* Step 3: copy .data from its load image (when the link provides one) to
 * its run address. Skipped when _data_load_start is absent (0) or already
 * points at the run address (pure RAM image). */
static void data_copy(void)
{
    const uint32_t *src = _data_load_start;
    uint32_t *dst = _data_start;

    if ((uintptr_t)src == 0u || (uintptr_t)src == (uintptr_t)dst)
        return;
    while (dst < _data_end)
        *dst++ = *src++;
}

/* Step 4: zero .bss. Word-wise: the linker contract aligns both bounds to
 * words (architecture.md verification builds use the same link scripts). */
static void bss_zero(void)
{
    uint32_t *p = _bss_start;

    while (p < _bss_end)
        *p++ = 0u;
}

/*
 * The C-runtime startup itself (boot contract order, docs/re/
 * route-b-minimum.md §2). Entered by rechord_startup_entry with the 4 ROM
 * handoff words as arguments (r0-r3, AAPCS).
 *
 * NOTE on capture placement: the contract captures the boot params FIRST,
 * before .data/.bss init. Our storage lives in .bss, so the second capture
 * below re-stores the words after the zero — they survive the whole
 * sequence in callee-saved registers/stack (C argument semantics). Both
 * stores are intentional; do not "simplify" one away.
 */
void rechord_startup(uint32_t w0, uint32_t w1, uint32_t w2, uint32_t w3)
{
    irq_disable();                            /* 1. IRQs off              */
    boot_params_capture(w0, w1, w2, w3);      /* 2. capture boot params   */
    data_copy();                              /* 3. .data copy            */
    bss_zero();                               /* 4. .bss zero             */
    boot_params_capture(w0, w1, w2, w3);      /*    re-store past the zero*/
    sync_barriers();                          /* 5. DSB/ISB               */
    rechord_main();                           /* 6. call main             */
    for (;;)
        ;                                     /* 7. hang if main returns  */
}

/*
 * Reset entry (the Reset word in vectors.c points here).
 *
 * The 4 boot words arrive in r0-r3 (TODO: confirm against the ROM handoff
 * ABI — open item in docs/rewrite/core.md; if the ROM hands them over via a
 * memory block instead, this is the one place to change). r0-r3 is exactly
 * the AAPCS argument list of rechord_startup(), so a tail branch forwards
 * them untouched and the stack is already valid (vector table entry 0).
 */
#if defined(__arm__)
__attribute__((naked, noreturn)) void rechord_startup_entry(void)
{
    __asm volatile("b.w rechord_startup");
}
#else
__attribute__((noreturn)) void rechord_startup_entry(void)
{
    rechord_startup(0u, 0u, 0u, 0u);
}
#endif
