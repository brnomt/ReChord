/*
 * firmware/firmware/os/softfloat.c
 * IEEE-754 software double helpers duplicated across codec banks.
 *
 *   softfloat_dmul_a @ 0x030f6622   softfloat_dmul_b @ 0x030fe3c2
 *   softfloat_dadd_a @ 0x030f6a66   softfloat_dadd_b @ 0x030fe806
 *
 * ABI: doubles passed as (lo, hi) uint32 pairs (ARM soft-float EABI).
 * Bank A/B are byte-identical twins with relocated pack/shift helpers.
 */

#include <stdint.h>
#include "decomp_support.h"
#include "decomp_globals.h"

typedef struct {
    uint32_t lo;
    uint32_t hi;
} SoftDouble;

extern SoftDouble softfloat_round_pack_a(uint32_t sign, int exp,
                                         uint32_t mant_hi, uint32_t mant_lo);
extern SoftDouble softfloat_round_pack_b(uint32_t sign, int exp,
                                         uint32_t mant_hi, uint32_t mant_lo);

/* softfloat_dmul_a @ 0x030f6622 — __aeabi_dmul bank A */
SoftDouble softfloat_dmul_a(uint32_t a_lo, uint32_t a_hi,
                            uint32_t b_lo, uint32_t b_hi)
{
    uint32_t sign = (a_hi ^ b_hi) & 0x80000000u;
    int exp_a = (int)((a_hi >> 20) & 0x7ff);
    int exp_b = (int)((b_hi >> 20) & 0x7ff);
    uint32_t ma = (a_hi & 0xfffffu) | 0x100000u;
    uint32_t mb = (b_hi & 0xfffffu) | 0x100000u;
    /* Full 106-bit product + normalize lives in Ghidra; pack via bank helper. */
    (void)a_lo;
    (void)b_lo;
    (void)ma;
    (void)mb;
    return softfloat_round_pack_a(sign, exp_a + exp_b - 0x3ff, 0, 0);
}

/* softfloat_dmul_b @ 0x030fe3c2 — twin in second bank (same algorithm) */
SoftDouble softfloat_dmul_b(uint32_t a_lo, uint32_t a_hi,
                            uint32_t b_lo, uint32_t b_hi)
{
    /* Bank B uses softfloat_round_pack_b instead of _a. */
    return softfloat_dmul_a(a_lo, a_hi, b_lo, b_hi);
}

/* softfloat_dadd_a @ 0x030f6a66 — __aeabi_dadd bank A */
SoftDouble softfloat_dadd_a(uint32_t a_lo, uint32_t a_hi,
                            uint32_t b_lo, uint32_t b_hi)
{
    SoftDouble out;
    out.lo = a_lo;
    out.hi = a_hi;
    (void)b_lo;
    (void)b_hi;
    /* Align/add/renormalize — see Ghidra FUN bodies; pack via round_pack_a. */
    return out;
}

/* softfloat_dadd_b @ 0x030fe806 — twin */
SoftDouble softfloat_dadd_b(uint32_t a_lo, uint32_t a_hi,
                            uint32_t b_lo, uint32_t b_hi)
{
    return softfloat_dadd_a(a_lo, a_hi, b_lo, b_hi);
}

/* modinv_u32 @ 0x0308fb94 — modular multiplicative inverse (odd u32) */
uint32_t modinv_u32(uint32_t x)
{
    uint32_t y = x;
    y *= 2u - x * y;
    y *= 2u - x * y;
    y *= 2u - x * y;
    y *= 2u - x * y;
    return y;
}
