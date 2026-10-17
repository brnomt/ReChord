/*
 * scratch.c — the shared working region (see scratch.h).
 *
 * The region is one static, sized for the largest user (the USB mass-storage
 * chunk, which used to be a function-local static of its own). Freeing the
 * others and sharing this one is what keeps the AP under its .bss ceiling
 * while still giving the media index a 32 KiB working buffer to build with.
 *
 * Alignment: 8 bytes, which covers the uint32_t/uint64_t buffers the users
 * cast it to.
 */
#include "scratch.h"

#include <stddef.h>

static unsigned s_owner;
static unsigned char s_scratch[SCRATCH_BYTES] __attribute__((aligned(8)));

void *scratch_claim(unsigned owner)
{
    if (owner == SCRATCH_OWNER_NONE || (s_owner != SCRATCH_OWNER_NONE && s_owner != owner))
        return NULL;
    s_owner = owner;
    return s_scratch;
}

void scratch_release(unsigned owner)
{
    if (owner != SCRATCH_OWNER_NONE && s_owner == owner)
        s_owner = SCRATCH_OWNER_NONE;
}

unsigned scratch_owner(void)
{
    return s_owner;
}

void *scratch_for(unsigned owner)
{
    if (owner == SCRATCH_OWNER_NONE || s_owner != owner)
        return NULL;
    return s_scratch;
}
