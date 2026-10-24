/*
 * overlay_demo.c -- copy and execute the tiny overlay at 0x0304AE64.
 *
 * The overlay binary is linked at 0x0304AE64 and embedded in the AP image
 * below the initial-row gate. At runtime it is copied into the free
 * loader-data window and called through a Thumb function pointer.
 */
#include "startup/overlay_demo.h"

#if AP_MEM_OVERLAY

#include <stdint.h>

#define OVERLAY_BASE 0x0304AE64u
#define OVERLAY_MAGIC 0xC0DE1234u

extern const uint8_t _binary_overlay_demo_bin_start[];
extern const uint8_t _binary_overlay_demo_bin_end[];

int ap_overlay_demo_run(void)
{
    const uint8_t *src = _binary_overlay_demo_bin_start;
    unsigned n = (unsigned)(_binary_overlay_demo_bin_end - src);
    volatile uint8_t *dst = (volatile uint8_t *)OVERLAY_BASE;
    uint32_t (*fn)(void) = (uint32_t (*)(void))(OVERLAY_BASE | 1u);
    unsigned i;

    for (i = 0; i < n; ++i)
        dst[i] = src[i];

    __asm__ volatile("dsb; isb" ::: "memory");
    return fn() == OVERLAY_MAGIC ? 0 : 1;
}

#endif
