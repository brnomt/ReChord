/*
 * firmware/codecs/ape/ape_cuesheet.c
 * ape_scan_cuesheet @ 0x0301dcb8 (137 xrefs)
 *
 * Walks APE/MAC frame table looking for type 0x6364 ('dc') frames;
 * skips 0x6277 ('wb'). Returns status codes 1/2/3/-1.
 */

#include <stdint.h>
#include "decomp_support.h"
#include "decomp_globals.h"

typedef struct {
    uint32_t *io;       /* [0] */
    uint32_t _1;
    uint32_t _2;
    uint32_t result;    /* [3] */
    uint32_t field_a;   /* [4] */
    uint32_t index;     /* [5] */
    uint32_t start;     /* [6] */
    uint32_t length;    /* [7] */
    uint32_t buf_pos;   /* [8] */
    uint32_t window[0x100]; /* [9+] scratch */
} ApeCueCtx;

extern uint32_t ape_rom_read_thunk(uint32_t *dst, uint32_t want, int8_t fd);
extern uint32_t ape_rom_seek_thunk(int32_t offset, uint8_t whence, int8_t fd);
extern uint32_t ape_tell(uint32_t *io);

uint32_t ape_scan_cuesheet(ApeCueCtx *ctx)
{
    extern uint32_t *g_ape_meta;  /* DAT_0301df68 */
    extern uint32_t *g_ape_hdr;   /* DAT_0301df60 */
    ctx->result = 0;
    uint32_t want = 0x400;
    if (ctx->index >= g_ape_meta[3])
        return 2;

    do {
        uint32_t *fr;
        if (ctx->buf_pos < want) {
            fr = (uint32_t *)((uint8_t *)ctx + ctx->buf_pos + 0x24);
            ctx->buf_pos += 0x10;
        } else {
            want = ape_rom_read_thunk(ctx->window, 0x400, (int8_t)ctx->_1);
            if (want < 0x10)
                return 2;
            g_ape_meta[7] = want;
            fr = ctx->window;
            ctx->buf_pos = 0x10;
        }
        g_ape_meta[4]++;
        uint16_t type = *(uint16_t *)((uint8_t *)fr + 2);
        if (type == 0x6364) { /* 'dc' */
            ctx->index++;
            if (ctx->index > g_ape_meta[3])
                return 2;
            uint32_t base = g_ape_hdr[3];
            uint32_t pos = ape_tell(ctx->io);
            if (pos < base)
                pos = 0;
            else
                pos -= base;
            uint32_t start = *(uint8_t *)(fr + 2) | (*(int *)((uint8_t *)fr + 9) << 8);
            uint32_t len = *(uint8_t *)(fr + 3) | (*(int *)((uint8_t *)fr + 0xd) << 8);
            ctx->start = start;
            ctx->length = len;
            if (pos <= start) {
                ctx->result = (start - pos) + 8;
                ctx->field_a = len;
                if (len == 0)
                    return 3;
                if (*(char *)(fr + 1) != 0 || ((*(uint32_t *)((uint8_t *)fr + 5)) & 0xffffff) != 0)
                    return 1;
            }
        } else if (type != 0x6277) { /* not 'wb' */
            return 0xffffffffu;
        }
    } while (ctx->index != (int)g_ape_meta[3]);
    return 2;
}
