/*
 * workbuf.c — the shared heavy-work region (see workbuf.h).
 *
 * Same single-owner claim model as scratch.c, but a SECOND region on purpose:
 * the media index/query use scratch.c and CAN run during playback, so the
 * codec/update buffers must not share that one.
 *
 * Alignment: 8 bytes, which covers the uint32_t/int32_t/uint16_t buffers the
 * users cast it to (the FLAC pool, fw_update's uint32_t blocks, mp3's int16
 * residual).
 */
#include "workbuf.h"

#include <stddef.h>

static unsigned s_owner;
static unsigned char s_buf[WORKBUF_BYTES] __attribute__((aligned(8)));

void *workbuf_claim(unsigned owner)
{
    if (owner == WORKBUF_OWNER_NONE ||
        (s_owner != WORKBUF_OWNER_NONE && s_owner != owner))
        return NULL;
    s_owner = owner;
    return s_buf;
}

void workbuf_release(unsigned owner)
{
    if (owner != WORKBUF_OWNER_NONE && s_owner == owner)
        s_owner = WORKBUF_OWNER_NONE;
}

unsigned workbuf_owner(void)
{
    return s_owner;
}

void *workbuf_for(unsigned owner)
{
    if (owner == WORKBUF_OWNER_NONE || s_owner != owner)
        return NULL;
    return s_buf;
}
