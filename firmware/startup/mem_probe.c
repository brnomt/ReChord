/*
 * mem_probe.c -- runtime probe for 0x0304AE64..0x03050000.
 *
 * The first probe at 0x03088000 hung the AP after display_init: that region
 * is live loader/ROM memory at runtime. This probe targets the loader-data
 * window below the AP code instead (20,896 B). It runs after the loader,
 * reads three words, writes a pattern, reads it back and restores the
 * originals. A pass means the window is free for runtime overlays; a fail
 * means it is still owned after the loader.
 *
 * The initial fw1 scatter row stays below 0x03088000. This is a runtime test
 * only and is compiled out of the normal image (AP_MEM_PROBE=0).
 */
#include "startup/mem_probe.h"

#if AP_MEM_PROBE

#include <stdint.h>

#define PROBE_BASE 0x0304AE64u
#define PROBE_END  0x03050000u

int ap_mem_probe_run(void)
{
    volatile uint32_t *base = (volatile uint32_t *)PROBE_BASE;
    const uint32_t span = PROBE_END - PROBE_BASE;
    const uint32_t offsets[3] = { 0u, span / 8u, span / 4u - 1u };
    uint32_t original[3];
    uint32_t i;
    int ok = 1;

    for (i = 0; i < 3; ++i)
        original[i] = base[offsets[i]];

    for (i = 0; i < 3; ++i)
        base[offsets[i]] = 0x5A5A0000u | i;

    for (i = 0; i < 3; ++i) {
        if (base[offsets[i]] != (0x5A5A0000u | i))
            ok = 0;
    }

    for (i = 0; i < 3; ++i)
        base[offsets[i]] = original[i];

    return ok ? 0 : 1;
}

#endif
