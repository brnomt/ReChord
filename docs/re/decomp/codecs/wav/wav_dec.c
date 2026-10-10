#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/codecs/wav/wav_dec.c
 * Reconstructed from Ghidra decompilation of:
 *   wav_lib_decode       @ 0x030a42ee
 *   wav_lib_parse_header @ 0x030a46c8  (sibling; not expanded here)
 *   Source: ..\..\Common\Codec\Audio\Wav\WAV_lib\...
 *
 * Validates a WAVE "fmt " chunk already buffered in the codec context and
 * copies accepted parameters into the runtime config slots.
 *
 * Accepted format tags (first byte of fmt):
 *   0x01  PCM          — 8/16/24/32-bit, ≤2 ch, blockAlign ≤0x3000
 *   0x02  ADPCM        — strict 0x32-byte fmt, 4-bit, blockAlign 0x20, cbSize 7
 *   0x11  IMA-ADPCM    — 2-bit nibbles, blockAlign ≤0x1000, bits 3 or 4
 *   0xFFFE (via -2/-1) WAVE_FORMAT_EXTENSIBLE — SubFormat GUID match required
 *
 * Returns 1 on success, 0 on reject / short read.
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../codec_api.h"

/* Large WAV codec context — only fields touched by wav_lib_decode */
typedef struct {
    uint32_t channels;        /* +0x00  (also mirrored to runtime) */
    uint32_t bits;            /* +0x04 */
    uint32_t fmt_dw[0x10];    /* +0x08  raw fmt payload starts at [2] */
    /* ... */
    uint32_t file_pos;        /* +0x8214  param_1[0x2085] */
    uint32_t rate;            /* +0x8228  [0x208a] */
    uint32_t avg_bps;         /* +0x822c  [0x208b] */
    uint8_t  ch_rt;           /* +0x8230  [0x208c] */
    uint16_t bits_rt;         /* +0x8234  [0x208d] */
    uint16_t block_rt;        /* +0x8236 */
    uint16_t tag_rt;          /* +0x8238  [0x208e] */
} WavCtx;

extern void *(*g_wav_seek)(int fd, int off, int whence);  /* DAT_030a4538 */
extern int   (*g_wav_read)(void *dst, uint32_t n, int fd); /* DAT_030a453c */
extern int  *g_wav_fd_slot;                                /* DAT_030a4534 */
extern void  wav_log(const char *fmt, const char *file, int line, ...);
extern void  wav_memcpy(void *dst, const void *src, uint32_t n); /* FUN_0309f692 */
extern int   wav_guid_eq(const void *a, const void *b, int n);   /* FUN_0309f6e8 */
extern const uint8_t *g_wav_ext_subfmt_guid;                     /* DAT_030a45f0 */

/*
 * wav_lib_decode @ 0x030a42ee
 *
 * param_1 = WavCtx*, param_2 = fmt-chunk byte length to validate.
 */
int wav_lib_decode(uint32_t *param_1, uint32_t param_2)
{
    uint32_t fmt0, rate, avg, block_bits, ba_bits;
    uint16_t channels, block_align, bits, tag;
    int fd = *g_wav_fd_slot;

    (*g_wav_seek)((int)param_1[0x2085], 0, fd);
    if ((*g_wav_read)(param_1 + 2, param_2, fd) != (int)param_2) {
        wav_log("FILE: %s, LINE: %d,",
                "..\\..\\Common\\Codec\\Audio\\Wav\\...", 0xff);
        return 0;
    }
    param_1[0x2085] += param_2;

    wav_log("FILE: %s, LINE: %d, wav_formatTa",
            "..\\..\\Common\\Codec\\Audio\\Wav\\...", 0x106, (char)param_1[2]);

    tag = (uint16_t)param_1[2];          /* low 16 of first dword = format tag */
    channels = (uint16_t)(param_1[2] >> 16);

    if ((char)param_1[2] == 0x02) {
        /* Microsoft ADPCM — exact 0x32-byte fmt */
        if (param_2 != 0x32) return 0;
        rate  = param_1[3];
        avg   = param_1[4];
        block_bits = param_1[5];
        ba_bits    = param_1[6];
        if (channels > 2) return 0;
        if ((block_bits & 0xffff) > 0x1000) return 0;
        if ((block_bits >> 16) != 4) return 0;          /* 4-bit ADPCM */
        if ((ba_bits & 0xffff) != 0x20) return 0;
        if ((uint16_t)(ba_bits >> 16) > 0x800) return 0;
        if ((param_1[7] & 0xffff) != 7) return 0;       /* cbSize */
        goto accept;
    }

    if ((char)param_1[2] == 0x11) {
        /* IMA ADPCM */
        rate  = param_1[3];
        avg   = param_1[4];
        block_bits = param_1[5];
        ba_bits    = param_1[6];
        if (channels > 2) return 0;
        if ((block_bits & 0xffff) > 0x1000) return 0;
        if ((ba_bits & 0xffff) != 2) return 0;
        if ((uint16_t)(ba_bits >> 16) > 0x1000) return 0;
        {
            int nb = (int16_t)(block_bits >> 16);
            if (nb != 3 && nb != 4) return 0;
        }
        goto accept;
    }

    if ((char)param_1[2] == 0x01) {
        /* PCM */
        if (param_2 < 0x35) {
            wav_memcpy(&fmt0, param_1 + 2, param_2);  /* local copy path */
            (void)fmt0;
            rate = param_1[3]; avg = param_1[4];
            block_bits = param_1[5]; ba_bits = param_1[6];
        } else {
            rate = param_1[3]; avg = param_1[4];
            block_bits = param_1[5]; ba_bits = param_1[6];
        }
        if (channels > 2) return 0;
        if ((block_bits & 0xffff) > 0x3000) return 0;
        bits = (uint16_t)(block_bits >> 16);
        if (bits != 8 && bits != 16 && bits != 24 && bits != 32) {
            wav_log("FILE: %s, LINE: %d, nBlockAlign=",
                    "..\\..\\Common\\Codec\\Audio\\Wav\\...", 0x17a,
                    block_bits & 0xffff);
            return 0;
        }
        goto accept;
    }

    /* WAVE_FORMAT_EXTENSIBLE: tag bytes look like 0xFFFE (stored as -2 / -1) */
    if (*(int8_t *)((uint8_t *)param_1 + 9) == -1 && (char)param_1[2] == -2) {
        uint32_t sub[10];
        int i;
        for (i = 0; i < 10; i++)
            sub[i] = param_1[2 + i];
        if (wav_guid_eq(&sub[6], g_wav_ext_subfmt_guid, 0xc) != 0)
            return 0;
        rate = sub[1]; avg = sub[2];
        block_bits = sub[3]; ba_bits = sub[4];
        channels = (uint16_t)(sub[0] >> 16);
        tag = (uint16_t)sub[6];                 /* from extensible header */
        if ((sub[6] & 0xffff) != 1) return 0;   /* PCM SubFormat */
        if (channels > 2) return 0;
        if ((block_bits & 0xffff) > 0x1000) return 0;
        bits = (uint16_t)(block_bits >> 16);
        if (bits != 8 && bits != 16 && bits != 24 && bits != 32)
            return 0;
        goto accept;
    }

    return 0;

accept:
    param_1[0x208a] = avg;
    param_1[0x208b] = rate;
    *(uint16_t *)((uint8_t *)param_1 + 0x8232) = (uint16_t)block_bits;
    *(uint8_t *)(param_1 + 0x208c) = (uint8_t)(channels);
    *param_1 = channels;
    *(uint16_t *)(param_1 + 0x208d) = (uint16_t)(ba_bits >> 16);
    *(uint16_t *)(param_1 + 1) = (uint16_t)(ba_bits >> 16);
    *(uint16_t *)((uint8_t *)param_1 + 0x8236) = (uint16_t)(block_bits >> 16);
    *(uint16_t *)((uint8_t *)param_1 + 6) = (uint16_t)(block_bits >> 16);
    *(uint16_t *)(param_1 + 0x208e) = tag;
    (void)bits;
    return 1;
}
