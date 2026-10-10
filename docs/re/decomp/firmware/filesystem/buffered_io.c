/*
 * firmware/firmware/filesystem/buffered_io.c
 * 512-byte windowed fread/fseek used by several codec front-ends.
 *
 *   buffered_fread @ 0x030ae6b0  (85 xrefs)
 *   buffered_fseek @ 0x030ae61a  (76 xrefs)
 *   buffered_fread_ovl_0e65 @ 0x030e6560  (FLAC bank twin)
 *   buffered_fseek_ovl_0e64 @ 0x030e64ca
 */

#include <stdint.h>
#include "decomp_support.h"
#include "decomp_globals.h"

typedef struct {
    uint8_t  window[0x200];
    uint16_t pos;      /* +0x200 */
    uint16_t end;      /* +0x202 */
    uint32_t fd_ctx;   /* +0x204 */
} BufferedFile;

typedef int16_t (*buf_read_fn)(BufferedFile *bf, uint32_t want, uint32_t fd_ctx);
typedef void    (*buf_seek_fn)(int32_t off, uint8_t whence, uint32_t fd_ctx);

extern buf_read_fn *g_buf_read; /* DAT_030ae954 */
extern buf_seek_fn *g_buf_seek; /* DAT_030ae950 */
extern void hifi_memmove(void *dst, const void *src, uint32_t n);
/* Overlay uses FUN_030ac8a8 as memcpy twin */

/* buffered_fread @ 0x030ae6b0 — fread(ptr, size, nmemb, stream) semantics */
int buffered_fread(void *ptr, int size, int nmemb, BufferedFile *bf)
{
    if (ptr == 0)
        return -1;
    int left = size * nmemb;
    uint8_t *dst = (uint8_t *)ptr;
    while (1) {
        int avail = (int)bf->end - (int)bf->pos;
        int n = (left <= avail) ? left : avail;
        hifi_memmove(dst, bf->window + bf->pos, (uint32_t)n);
        dst += n;
        bf->pos = (uint16_t)(bf->pos + n);
        left -= n;
        if (left == 0)
            break;
        int16_t got = (*g_buf_read)(bf, 0x200, bf->fd_ctx);
        if (got == 0)
            break;
        bf->pos = 0;
        bf->end = (uint16_t)got;
    }
    return size * nmemb - left;
}

/* buffered_fseek @ 0x030ae61a */
uint32_t buffered_fseek(BufferedFile *bf, int offset, uint32_t whence)
{
    if (whence != 1) {
        if (whence == 0 || whence == 2) {
            (*g_buf_seek)(offset, (uint8_t)(whence & 0xff), bf->fd_ctx);
            bf->pos = 0;
            bf->end = 0;
            return 0;
        }
        return 0xffffffffu;
    }
    if (offset == 0)
        return 0;
    uint16_t pos = bf->pos;
    if (offset < 0) {
        if (-(int)pos != offset && (int)pos <= -offset)
            goto refill;
    } else {
        if ((int)(bf->end - pos) < offset)
            goto refill;
    }
    bf->pos = (uint16_t)(pos + (int16_t)offset);
    return 0;
refill:
    (*g_buf_seek)(offset - ((int)bf->end - (int)pos), 1, bf->fd_ctx);
    bf->pos = 0;
    bf->end = 0;
    return 0;
}

/*
 * Overlay twins in the FLAC codec bank — same 512-byte window protocol,
 * relocated g_buf_read / g_buf_seek pointers.
 */
int buffered_fread_ovl_0e65(void *ptr, int size, int nmemb, BufferedFile *bf)
{
    return buffered_fread(ptr, size, nmemb, bf);
}

uint32_t buffered_fseek_ovl_0e64(BufferedFile *bf, int offset, uint32_t whence)
{
    return buffered_fseek(bf, offset, whence);
}
