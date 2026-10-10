/*
 * boot_params.h — the 4-word ROM boot handoff captured at Reset.
 *
 * FACT (docs/re/route-b-minimum.md §2, byte-verified on the reference RAM
 * image): the C-runtime startup captures exactly FOUR 32-bit boot parameters
 * from the ROM handoff before .data/.bss init and stores them in a fixed
 * 4-word block (the reference stores it via the DAT_0305015c block).
 *
 * OPEN ITEM (TODO): the IDENTITY of the four words is not yet known (see
 * docs/rewrite/core.md, "Open items"). The struct therefore preserves the
 * words EXACTLY as handed over — no reinterpretation, no field packing — so
 * nothing is lost while the handoff ABI is being identified. Once the fields
 * are known, name them here (union over raw[4]) and keep raw[] as the
 * byte-exact fallback view.
 *
 * WHY header-only: the capture runs before the C runtime has initialized
 * .data/.bss (it is step 2 of the boot contract), so the capture code must
 * have ZERO link-time dependencies beyond the storage symbols. It also keeps
 * the legacy BB image linkable without new objects. Logic here is shared
 * verbatim by the target startup (firmware/startup/startup.c) and the host
 * tests (firmware/app/tests/), so what the tests exercise is what boots.
 *
 * STORAGE CONTRACT: exactly one translation unit defines
 *   rechord_boot_params, rechord_boot_params_captured
 * — firmware/startup/startup.c on target, the app test mock on host.
 *
 * Usage:
 *   boot_params_capture(w0, w1, w2, w3);        // startup.c at Reset
 *   const rechord_boot_params_t *bp = boot_params_get();
 *   uint32_t w2 = boot_params_word(bp, 2);      // 0 for NULL / out of range
 */
#ifndef RECHORD_APP_BOOT_PARAMS_H
#define RECHORD_APP_BOOT_PARAMS_H

#include <stdint.h>

/* Number of words in the ROM boot handoff (fixed by the boot contract). */
#define BOOT_PARAMS_WORDS 4u

/* The handoff block: 4 words, kept bit-exact (identity TODO, see header). */
typedef struct rechord_boot_params {
    uint32_t raw[BOOT_PARAMS_WORDS];
} rechord_boot_params_t;

/* Storage (see STORAGE CONTRACT above). */
extern rechord_boot_params_t rechord_boot_params;
extern uint32_t              rechord_boot_params_captured;

/*
 * Capture the 4 handoff words. Called by the startup sequence twice by
 * design: once immediately at Reset (boot-contract order: capture happens
 * before .data/.bss init) and once after .bss zeroing (our storage lives in
 * .bss, so the zero would otherwise erase the handoff — the words survive in
 * callee-saved registers/stack in between). See startup.c.
 */
static inline void boot_params_capture(uint32_t w0, uint32_t w1,
                                      uint32_t w2, uint32_t w3)
{
    rechord_boot_params.raw[0] = w0;
    rechord_boot_params.raw[1] = w1;
    rechord_boot_params.raw[2] = w2;
    rechord_boot_params.raw[3] = w3;
    rechord_boot_params_captured = 1u;
}

/* The captured block, or NULL when no capture has happened yet. */
static inline const rechord_boot_params_t *boot_params_get(void)
{
    return rechord_boot_params_captured ? &rechord_boot_params : 0;
}

/*
 * Raw word accessor, bounds-checked. Returns 0 for a NULL block or an index
 * >= BOOT_PARAMS_WORDS, so boot telemetry can print words without carrying
 * validity logic itself.
 */
static inline uint32_t boot_params_word(const rechord_boot_params_t *bp,
                                       unsigned index)
{
    return (bp != 0 && index < BOOT_PARAMS_WORDS) ? bp->raw[index] : 0u;
}

/* Nonzero once boot_params_capture() has run. */
static inline int boot_params_captured(void)
{
    return rechord_boot_params_captured != 0u;
}

#endif /* RECHORD_APP_BOOT_PARAMS_H */
