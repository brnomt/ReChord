#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/codecs/dsd/dsd_dec.c
 * Reconstructed from Ghidra decompilation of:
 *   DSD_IFF_Open     @ 0x030ff2e8  (DSF container parse)
 *   DSD_BufferReady  @ 0x030ff2d8
 *   DSD_DecodeBlock  @ 0x030ffa3c  (1-bit → PCM FIR expansion)
 *
 * DSF (DSD Stream File) layout walked by DSD_IFF_Open:
 *   'DSD '  — file header + size
 *   'fmt '  — format id, channel type, sample rate, bits/sample, sample count
 *   'data'  — raw DSD payload
 *
 * Bits/sample must be ≤ 7 (else "bit per sample not support!").
 * Supported rates advertised elsewhere: "DSD64,DSD128,DSD256 ".
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../codec_api.h"

typedef struct {
    void *fd;                 /* +0x00 */
    uint32_t format_id;       /* +0x04 */
    uint32_t channels;        /* +0x08  [2] */
    uint32_t bits;            /* +0x0c  [3] */
    uint32_t sample_rate;     /* +0x10  [4] */
    uint32_t channel_type;    /* +0x14  [5] */
    uint32_t data_pos;        /* +0x18  [6] */
    uint32_t data_size;       /* +0x1c  [7] */
    uint32_t file_size;       /* +0x20  [8] */
    uint32_t _r24;
    uint32_t block_bytes;     /* +0x28  [10] = rate * block_frames */
    uint32_t n_blocks;        /* +0x2c  [11] */
    uint32_t buf_pos;         /* +0x2c used by BufferReady */
    uint32_t buf_end;         /* +0x30 */
} DsdCtx;

extern int  (*g_dsd_read)(void *buf, int n, void *fd);
extern int  (*g_dsd_seek)(int off, int whence, void *fd);
extern int  (*g_dsd_tell)(void *fd);
extern void dsd_log(const char *fmt, ...);

extern DsdCtx *g_dsd;                     /* DAT_030ff6d4 */

/*
 * DSD_BufferReady @ 0x030ff2d8
 * Returns 1 when decode cursor has reached end of current buffer.
 */
int DSD_BufferReady(void)
{
    return (g_dsd->buf_pos >= g_dsd->buf_end) ? 1 : 0;
}

/*
 * DSD_IFF_Open @ 0x030ff2e8
 * Parse DSF header. Returns 1 on success, 0 on any chunk error.
 */
int DSD_IFF_Open(void)
{
    uint8_t hdr[12];
    uint32_t n, tmp32, tmp64_lo;
    int32_t  chunk_extra;

    n = (uint32_t)g_dsd_read(hdr, 12, g_dsd->fd);
    if (n != 12 || hdr[0] != 'D' || hdr[1] != 'S' || hdr[2] != 'D' || hdr[3] != ' ') {
        dsd_log("DSD err \n");
        return 0;
    }

    /* file size (8 bytes, we keep low dword) */
    if (g_dsd_read(&tmp64_lo, 8, g_dsd->fd) == 8) {
        g_dsd->file_size = tmp64_lo;
        dsd_log("fileSize=%d\n", tmp64_lo);
    }

    /* optional ID3v2 offset field */
    if (g_dsd_read(&tmp32, 8, g_dsd->fd) != 8 || tmp32 != 0)
        dsd_log("ID3V2 start=%d\n", tmp32);

    /* 'fmt ' chunk */
    n = (uint32_t)g_dsd_read(hdr, 12, g_dsd->fd);
    if (n != 12 || hdr[0] != 'f' || hdr[1] != 'm' || hdr[2] != 't' || hdr[3] != ' ') {
        dsd_log("fmt err\n");
        return 0;
    }

    if (g_dsd_read(&tmp32, 4, g_dsd->fd) == 4)
        g_dsd->format_id = tmp32;

    if (g_dsd_read(&tmp32, 4, g_dsd->fd) != 4 || tmp32 != 0) {
        dsd_log("formatid err\n");
        return 0;
    }

    if (g_dsd_read(&tmp32, 4, g_dsd->fd) == 4)
        g_dsd->channel_type = tmp32;
    if (g_dsd_read(&tmp32, 4, g_dsd->fd) == 4)
        g_dsd->sample_rate = tmp32;
    if (g_dsd_read(&tmp32, 4, g_dsd->fd) == 4)
        g_dsd->channels = tmp32;

    if (g_dsd_read(&tmp32, 4, g_dsd->fd) == 4) {
        g_dsd->bits = tmp32;
        if (tmp32 > 7) {
            dsd_log("bit per sample not support!\n");
            return 0;
        }
    }

    if (g_dsd_read(&tmp64_lo, 8, g_dsd->fd) == 8)
        dsd_log("sampleCount=%lu\n", (unsigned long)tmp64_lo);

    if (g_dsd_read(&tmp32, 4, g_dsd->fd) == 4)
        g_dsd->block_bytes = g_dsd->sample_rate * tmp32;

    g_dsd_seek(4, 1, g_dsd->fd);            /* skip reserved */

    /* 'data' chunk */
    n = (uint32_t)g_dsd_read(hdr, 12, g_dsd->fd);
    if (n != 12 || hdr[0] != 'd' || hdr[1] != 'a' || hdr[2] != 't' || hdr[3] != 'a') {
        dsd_log("dsf read ckid err\n");
        return 0;
    }

    /* chunk size is in the 12-byte header trailing fields (iStack_44 in decomp) */
    chunk_extra = *(int32_t *)(hdr + 4);    /* approximate — decomp used separate read */
    g_dsd->data_pos  = (uint32_t)g_dsd_tell(g_dsd->fd);
    g_dsd->data_size = (uint32_t)chunk_extra - 12;
    if (g_dsd->block_bytes != 0)
        g_dsd->n_blocks = g_dsd->data_size / g_dsd->block_bytes;

    dsd_log("data size = %d\n", g_dsd->data_size);
    return 1;
}

/*
 * DSD_DecodeBlock @ 0x030ffa3c
 *
 * Expands packed DSD bits through a 64-tap lookup table (stride 0xF0) into
 * 32-bit PCM samples. param_1 = per-channel ring state, param_3 = input
 * byte count, param_4/5 = output buffer + bytes-per-sample.
 */
uint32_t DSD_DecodeBlock(uint32_t *state, int in_base, uint32_t in_len,
                         int out_base, uint32_t out_bps)
{
    /* Faithful body is large — see Ghidra @ 0x030ffa3c.
       Outer loop over channels, inner over in_len/channels bytes,
       ring index & 0x3f, accumulate FIR taps from DAT_030ffca8. */
    (void)state; (void)in_base; (void)in_len; (void)out_base; (void)out_bps;
    return 0; /* placeholder — full FIR loop in Ghidra */
}
