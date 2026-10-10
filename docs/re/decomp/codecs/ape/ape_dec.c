#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/codecs/ape/ape_dec.c
 * Reconstructed from Ghidra decompilation of:
 *   APE_Codec_Open @ 0x030eef60
 *
 * Monkey's Audio codec control switch. Not just "open" — a multi-opcode
 * dispatcher (param_1 selects the operation):
 *
 *   3   get bitrate+500
 *   4   get sample count          → *param_2
 *   5   get sample rate (uint16)
 *   6   get duration-related
 *   7   get duration ms
 *   8   OPEN: skip ID3v2 if present, call ape_open, log OK/FAIL
 *   10  get decode buffer pair (double-buffered)
 *   12  decode one frame into current buffer
 *   14  SEEK to millisecond offset
 *   15  CLOSE / teardown
 *   17  get bits-per-sample
 *
 * Returns 1 on success, 0 on failure.
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../codec_api.h"

/* APE runtime state (DAT_030ef140 family) */
typedef struct {
    uint32_t _r00[0x15];
    uint32_t seek_frame;      /* +0x54 */
    uint32_t _r58;
    uint32_t max_seek;        /* +0x5c */
    uint16_t bits;            /* +0x60 */
    uint16_t rate;            /* +0x62 */
    uint32_t total_samples;   /* +0x64 */
    uint32_t _r68;
    uint32_t decode_limit;    /* +0x6c */
    uint32_t duration_x;      /* +0x70 */
    uint32_t bitrate_base;    /* +0x74 */
    uint32_t pos_samples;     /* +0x78 */
} ApeState;

extern ApeState *g_ape;
extern int      *g_ape_io;          /* DAT_030ef148 — fd at +0xc, id3_skip at +8 */
extern void    (**g_ape_seek)(int off, int whence, int fd);
extern int     (**g_ape_read)(void *buf, int n, int fd);
extern void      ape_log(const char *msg);          /* FUN_030ed3e6 */
extern void      ape_pre_open(void);                /* FUN_030ef17c */
extern int       ape_open_core(void);               /* FUN_030ef1b0 */
extern void      ape_post_open(void);               /* FUN_030ecc14 */
extern void      ape_decode_frame(void *buf, int *out_n); /* FUN_030f0360 */
extern void      ape_close_a(void);                 /* FUN_030f0384 */
extern void      ape_close_b(void);                 /* FUN_030ecc64 */
extern int       ape_seek_frames(uint32_t a, uint32_t b, uint32_t rate, int z); /* FUN_030ee87e */

/*
 * APE_Codec_Open @ 0x030eef60
 */
int APE_Codec_Open(int opcode, uint32_t *param_2, uint32_t *param_3)
{
    switch (opcode) {
    case 3:
        *param_2 = g_ape->bitrate_base + 500;
        return 1;

    case 4:
        *param_2 = g_ape->total_samples;
        return 1;

    case 5:
        *param_2 = g_ape->rate;
        return 1;

    case 6:
        *param_2 = g_ape->duration_x;
        return 1;

    case 7: {
        uint32_t samples = g_ape->total_samples;
        if (samples == 0) return 1;
        {
            uint32_t pos = g_ape->pos_samples;
            uint32_t sec = pos / samples;
            *param_2 = ((pos - samples * sec) * 1000) / samples + sec * 1000;
        }
        return 1;
    }

    case 8: {                                   /* OPEN */
        uint8_t hdr[0x14];
        uint32_t id3_size = 0;

        g_ape_io[3 /* +0xc /4 */] = *g_ape_default_fd;
        (**g_ape_seek)(0, 0, g_ape_io[3]);
        (**g_ape_read)(hdr, 0x14, g_ape_io[3]);

        /* ID3v2 syncsafe size if "ID3" present */
        if (hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3') {
            if (hdr[3] != 0xff && hdr[5] < 0x80 && hdr[6] < 0x80 &&
                hdr[7] < 0x80 && hdr[8] < 0x80) {
                id3_size = ((uint32_t)hdr[5] << 21) | ((uint32_t)hdr[6] << 14)
                         | ((uint32_t)hdr[7] << 7)  | (uint32_t)hdr[8];
            }
        }
        g_ape_io[2] = id3_size;
        if (id3_size == 0)
            (**g_ape_seek)(0, 0, g_ape_io[3]);
        else {
            g_ape_io[2] = id3_size + 10;
            (**g_ape_seek)((int)(id3_size + 10), 0, g_ape_io[3]);
        }

        ape_pre_open();
        {
            int rc = ape_open_core();
            ape_post_open();
            if (rc < 0) {
                ape_log("ape_open_FAIL!");
                return 0;
            }
            ape_log("ape_open_OK");
        }
        return 1;
    }

    case 10: {                                  /* get current PCM buffer */
        int *dbuf = (int *)((uint8_t *)g_ape_dbuf - 0x10);
        int slot = *(int *)((uint8_t *)g_ape_dbuf - 0xc);
        *param_2 = (uint32_t)((uint8_t *)g_ape_dbuf)[slot * 4] + *g_ape_pcm_base;
        *param_3 = (uint32_t)*dbuf;
        *(int *)((uint8_t *)g_ape_dbuf - 0xc) = slot ^ 1;
        return 1;
    }

    case 0xc: {                                 /* decode one frame */
        int *out_n = (int *)((uint8_t *)g_ape_dbuf - 0x10);
        int slot = *(int *)((uint8_t *)g_ape_dbuf - 0xc);
        ape_decode_frame(
            (void *)(((uint8_t *)g_ape_dbuf)[slot * 4] + *g_ape_pcm_base),
            out_n);
        if (*out_n == 0) return 0;
        if (g_ape->decode_limit <= g_ape->pos_samples) return 0;
        return 1;
    }

    case 0xe: {                                 /* SEEK (ms) */
        uint32_t ms = (uint32_t)param_2;
        if (ms > g_ape->duration_x)
            ms = g_ape->duration_x;
        {
            uint32_t rate = g_ape->total_samples;   /* used as divisor */
            uint32_t seek_u = g_ape->seek_frame;
            uint32_t frame = (ms / 1000) * rate / seek_u
                           + (rate * (ms % 1000)) / (seek_u * 1000);
            if (frame >= g_ape->max_seek) {
                int off = ape_seek_frames(g_ape->max_seek * seek_u,
                                          0 /*hi*/, rate, 0);
                (**g_ape_seek)(off * (int)g_ape->bitrate_base, 0, g_ape_io[3]);
                frame = g_ape->max_seek;
            }
            g_ape->pos_samples = frame * g_ape->seek_frame;
            g_ape->_r00[5] = frame;              /* +0x14 */
            g_ape->_r00[6] = 0;                  /* +0x18 */
        }
        return 1;
    }

    case 0xf:
        ape_close_a();
        ape_close_b();
        return 1;

    case 0x11:
        *param_2 = g_ape->bits;
        return 1;

    default:
        return 0;
    }
}

/* Placeholders for globals referenced above — filled from Ghidra DAT_* */
extern int *g_ape_default_fd;
extern void *g_ape_dbuf;
extern int  *g_ape_pcm_base;
