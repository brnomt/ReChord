/*
 * firmware/apps/audio/dac_gain_curve.c
 * Was mislabeled os_delay_ms @ 0x030098e4.
 *
 * Scales a source gain/time field into dest+0x0c using one of two curves
 * depending on mode (1 or 3 vs other). Used from MusicInit / MusicService.
 *
 * NOTE: Several call sites pass a bare immediate in r0 (e.g. 100, 15, 1)
 * which does not match the pointer signature below — those sites need a
 * follow-up pass (possible second entry / wrong BL target analysis).
 * Busy-wait delays used by FS timeouts are hifi_busy_delay @ 0x0306c2e8.
 */

#include <stdint.h>
#include "decomp_support.h"
#include "decomp_globals.h"

extern uint32_t g_gain_scale_k; /* DAT_03009ce0 */

typedef struct {
    uint32_t _pad[3];
    int32_t  value; /* +0x0c */
} GainSource;

typedef struct {
    uint32_t _pad[3];
    uint16_t scaled; /* +0x0c */
} GainDest;

void dac_gain_curve_apply(GainSource **psrc, int mode, GainDest *dst)
{
    int32_t v = (*psrc)->value;
    uint16_t out;
    if (mode != 1 && mode != 3) {
        /* (v - 0x1db0) * K / 0x578 */
        out = (uint16_t)((g_gain_scale_k * (v - 0x1db0)) / 0x578);
    } else {
        /* (v - 0x21fc) * 200 / 0x834 */
        out = (uint16_t)(((v - 0x21fc) * 200) / 0x834);
    }
    dst->scaled = out;
}
