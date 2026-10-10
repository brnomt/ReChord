/*
 * firmware/firmware/os/event_bitmap.c
 * Real event flag bitset (replaces mislabeled mid-function stubs).
 *
 *   event_set            @ 0x03073840  — flags[id>>5] |= 1<<(id&31)
 *   event_test_and_clear @ 0x0307385c  — test bit; if set, clear and return 1
 *   event_clear          @ 0x03073882  — flags[id>>5] &= ~(1<<(id&31))
 *
 * Former labels event_set@0x0300c6b4 / event_clear@0x0300c6f6 were mid-function
 * fragments (shared_frame_epilogue_c6b4 / shared_mid_entry_c6f6) — not events.
 */

#include <stdint.h>
#include "decomp_support.h"
#include "decomp_globals.h"

extern uint32_t *g_event_flags; /* DAT_030738bc */

void event_set(uint32_t id)
{
    uint32_t word = id >> 5;
    uint32_t bit = id & 0x1f;
    g_event_flags[word] |= (1u << bit);
}

uint32_t event_test_and_clear(uint32_t id)
{
    uint32_t word = id >> 5;
    uint32_t mask = 1u << (id & 0x1f);
    uint32_t v = g_event_flags[word];
    if ((v & mask) == 0)
        return 0;
    g_event_flags[word] = v & ~mask;
    return 1;
}

void event_clear(uint32_t id)
{
    uint32_t word = id >> 5;
    uint32_t mask = 1u << (id & 0x1f);
    g_event_flags[word] &= ~mask;
}
