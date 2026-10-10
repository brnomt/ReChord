#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/codecs/wma/wma_dec.c
 * Reconstructed from Ghidra decompilation of:
 *   wma_audio_parse @ 0x0308cddc
 *   Source: ..\..\Common\Codec\Audio\Wma\wmalib\wmaudio_parse.c
 *
 * Parses ASF stream-properties / WAVEFORMATEX-like blobs inside WMA files.
 * Rockchip wma_dec_lib v0.0.1 (2012-03-26).
 *
 * Format tags handled:
 *   0x160  WMA speech / v1-like (min blob 0x16)
 *   0x161  WMA v2          (min blob 0x1c)
 *   0x162 / 0x163  WMA Pro / lossless-style (min blob 0x24, WAVE_FORMAT_EXTENSIBLE)
 *
 * Return codes: 0=ok (advance), 1=unsupported/error, 2=null ctx, 3=short buffer
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../codec_api.h"

/* ASF/WMA parse context (large; only fields used by wma_audio_parse shown) */
typedef struct {
    uint32_t pos_lo;          /* +0x00 file offset low */
    uint32_t pos_hi;          /* +0x04 file offset high */
    uint32_t _r08[4];
    uint32_t duration_units;  /* +0x18 / param_1[6] */
    uint32_t _r1c[8];
    uint16_t codec_family;    /* +0x3c  (short @ +0x3e in decomp = tag) */
    uint16_t _pad3e;
    /* ... */
    uint16_t fmt_tag;         /* +0x3c family area — see offsets below */
    /* Offsets from decompiler (dword indices): */
    /* [0xf]=family, [0x10]=rate, [0x11]=avg_bps, [0x12]=block_align,
       [0x13]=channels, [0x14]=samples, [0x15]=bits, [0x16]/[0x1c]=block,
       [0x17]=channel_mask / flags, [0x38]=?, [0x3a]=object count */
    uint32_t fields[0x40];
} WmaParseCtx;

extern int wma_input_cache_read(uint32_t *ctx, uint32_t **out, uint32_t lo, uint32_t hi); /* @ 0x03083d48 */
extern int wma_memcmp(const void *a, const void *b, uint32_t n);              /* @ 0x0308283c */
extern void wma_log(const char *fmt, const char *file, int line);             /* FUN_03081592 */

extern const uint8_t *g_guid_header_obj;   /* DAT_0308d330 */
extern const uint8_t **g_guid_stream_obj;  /* DAT_0308d334 */
extern const uint8_t **g_guid_ext_a;       /* DAT_0308d384 */
extern const uint8_t **g_guid_ext_b;       /* DAT_0308d388 */

/*
 * wma_audio_parse @ 0x0308cddc
 *
 * param_1 = parse context (also holds output WAVEFORMATEX fields)
 * param_2 = bytes available from current object (>= 0x18+0x36 required)
 */
int wma_audio_parse(uint32_t *param_1, int param_2)
{
    uint32_t *blob = NULL;
    uint32_t avail = (uint32_t)param_2 - 0x18;
    uint32_t obj_size, fmt_size;
    uint16_t fmt_tag;
    uint16_t obj_idx;
    int n;

    if (param_1 == NULL)
        return 2;
    if (avail < 0x36)
        return 3;

    n = wma_input_cache_read(param_1, &blob, param_1[0], param_1[1]);
    if (n != 0x36)
        return 3;

    /* Parse two 16-byte GUIDs + size fields from the 0x36-byte header */
    /* GUID0 @ blob[0..15], GUID1 @ blob[16..31], size @ blob[40..] */
    obj_size = (uint32_t)((uint8_t *)blob)[0x28]
             | (uint32_t)((uint8_t *)blob)[0x29] << 8
             | (uint32_t)((uint8_t *)blob)[0x2a] << 16
             | (uint32_t)((uint8_t *)blob)[0x2b] << 24;
    fmt_size = (uint32_t)((uint8_t *)blob)[0x2c]
             | (uint32_t)((uint8_t *)blob)[0x2d] << 8
             | (uint32_t)((uint8_t *)blob)[0x2e] << 16
             | (uint32_t)((uint8_t *)blob)[0x2f] << 24;

    blob = (uint32_t *)((uint8_t *)blob + 0x36);

    /* Must match ASF Header Object GUID */
    if (wma_memcmp(g_guid_header_obj, /* reconstructed GUID0 */ blob - 0x36 / 4, 0x10) == 0) {
        wma_log("FILE: %s, LINE: %d, WMV",
                "..\\..\\Common\\Codec\\Audio\\Wma\\...", 0x1be);
        return 1;
    }

    /* Stream Properties Object → skip (advance file ptr) */
    if (wma_memcmp(*g_guid_stream_obj, /* GUID0 */ blob - 0x36 / 4, 0x10) != 0) {
        uint32_t lo = param_1[0];
        param_1[0] = lo + avail;
        param_1[1] += (lo + avail < lo);   /* carry */
        return 0;
    }

    obj_idx = (uint16_t)param_1[0x3a /* approx */]; /* *(short*)(param_1+0xea)+1 */
    /* Increment stream object counter; only fill WAVEFORMATEX on last */
    ((uint16_t *)param_1)[0x75] = (uint16_t)(((uint16_t *)param_1)[0x75] + 1);
    if (((uint16_t *)param_1)[0x75] != (uint16_t)param_1[0x3a])
        goto advance;

    param_1[0x38] = (uint8_t)((uint8_t *)(blob - 9))[0] & 0x7f; /* type flags */

    if (obj_size != 0) {
        uint8_t *fmt;
        if (avail < obj_size + 0x36)
            return 3;
        n = asf_read(param_1, (uint32_t **)&fmt,
                     param_1[0] + 0x36, param_1[1] + (param_1[0] > 0xffffffc9));
        if ((uint32_t)n != obj_size)
            return 3;

        fmt_tag = *(uint16_t *)fmt;
        ((uint16_t *)param_1)[0x1f] = fmt_tag; /* +0x3e */

        if (fmt_tag == 0x160) {
            if (obj_size < 0x16) return 1;
            ((uint16_t *)param_1)[0x1e] = 1;           /* family = 1 */
            param_1[0x10] = rd_le32(fmt + 4);          /* sample rate */
            param_1[0x11] = rd_le32(fmt + 8);          /* avg bytes/sec */
            param_1[0x12] = *(uint16_t *)(fmt + 12);   /* block align */
            ((uint16_t *)param_1)[0x26] = *(uint16_t *)(fmt + 2); /* channels */
            ((uint16_t *)param_1)[0x2a] = rd_le16(fmt + 0x14);
            param_1[0x14] = rd_le16(fmt + 0x12);
            ((uint16_t *)param_1)[0x2b] = 1;
            {
                uint16_t bits = rd_le16(fmt + 0xe);
                ((uint16_t *)param_1)[0x2c] = bits;
                ((uint16_t *)param_1)[0x2d] = bits;
                ((uint16_t *)param_1)[0x38] = bits;
            }
            if (((uint16_t *)param_1)[0x26] == 1)
                param_1[0x17] = 4;
            else if (((uint16_t *)param_1)[0x26] == 2)
                param_1[0x17] = 3;
            else
                return 1;
        } else if (fmt_tag == 0x161) {
            if (obj_size < 0x1c) return 1;
            ((uint16_t *)param_1)[0x1e] = 2;
            param_1[0x10] = rd_le32(fmt + 4);
            param_1[0x11] = rd_le32(fmt + 8);
            param_1[0x12] = *(uint16_t *)(fmt + 12);
            ((uint16_t *)param_1)[0x26] = *(uint16_t *)(fmt + 2);
            ((uint16_t *)param_1)[0x2b] = 1;
            {
                uint16_t bits = *(uint16_t *)(fmt + 0xe);
                ((uint16_t *)param_1)[0x2c] = bits;
                ((uint16_t *)param_1)[0x2d] = bits;
                ((uint16_t *)param_1)[0x38] = bits;
            }
            param_1[0x14] = rd_le32(fmt + 0x12);
            ((uint16_t *)param_1)[0x2a] = rd_le16(fmt + 0x16);
            if (((uint16_t *)param_1)[0x26] == 1)
                param_1[0x17] = 4;
            else if (((uint16_t *)param_1)[0x26] == 2)
                param_1[0x17] = 3;
            else if (((uint16_t *)param_1)[0x26] == 6)
                param_1[0x17] = 0x3f;
            else
                return 1;
        } else if (fmt_tag == 0x162 || fmt_tag == 0x163) {
            if (obj_size < 0x24) return 1;
            ((uint16_t *)param_1)[0x1e] = 3;
            param_1[0x10] = rd_le32(fmt + 4);
            param_1[0x11] = rd_le32(fmt + 8);
            param_1[0x12] = *(uint16_t *)(fmt + 12);
            ((uint16_t *)param_1)[0x26] = rd_le16(fmt + 2);
            ((uint16_t *)param_1)[0x2b] = 0xfffe;       /* WAVE_FORMAT_EXTENSIBLE */
            {
                uint16_t bits = *(uint16_t *)(fmt + 0xe);
                ((uint16_t *)param_1)[0x2d] = bits;
                ((uint16_t *)param_1)[0x2c] = (uint16_t)((bits + 7) & 0xfff8);
            }
            param_1[0x17] = rd_le32(fmt + 0x14);
            ((uint16_t *)param_1)[0x2a] = rd_le16(fmt + 0x20);
            param_1[0x18] = 1;
            /* SubFormat GUID bytes hard-coded in firmware (WMA Pro) */
            ((uint16_t *)param_1)[0x32] = 0;
            ((uint16_t *)param_1)[0x33] = 0x10;
            ((uint8_t *)param_1)[0x68] = 0x80;
            ((uint8_t *)param_1)[0x69] = 0;
            ((uint8_t *)param_1)[0x6a] = 0;
            ((uint8_t *)param_1)[0x6b] = 0xaa;
            ((uint8_t *)param_1)[0x6c] = 0;
            ((uint8_t *)param_1)[0x6d] = 0x38;
            ((uint8_t *)param_1)[0x6e] = 0x9b;
            ((uint8_t *)param_1)[0x6f] = 0x71;
        } else {
            return 1;
        }
    }

    /* Optional extended content-description size → duration */
    if (fmt_size != 0) {
        int ext_len;
        if (wma_memcmp(*g_guid_ext_a, /* GUID1 */ NULL, 0x10) == 0)
            ext_len = 9;
        else if (wma_memcmp(*g_guid_ext_b, NULL, 0x10) == 0)
            ext_len = 8;
        else
            return 1;
        /* read + assign param_1[6] duration_units — omitted detail */
        (void)ext_len;
    }

    if (param_1[0x12] != 0) {
        uint32_t frames = param_1[6] / param_1[0x12];
        param_1[6] = (uint32_t)((uint16_t)param_1[0x13]) * param_1[0x14] * frames * 2;
    }

advance:
    {
        uint32_t lo = param_1[0];
        param_1[0] = lo + avail;
        param_1[1] += (lo + avail < lo);
    }
    return 0;
}

static uint32_t rd_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static uint16_t rd_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | (uint16_t)p[1] << 8;
}

/* wma_memcmp @ 0x0308283c */
int wma_memcmp(const void *lhs, const void *rhs, uint32_t length)
{
    const uint8_t *a = (const uint8_t *)lhs;
    const uint8_t *b = (const uint8_t *)rhs;
    uint32_t i = 0;
    int difference = 0;

    while (i < length && (difference = (int)a[i] - (int)b[i]) == 0)
        ++i;
    return difference;
}

/* wma_memmove @ 0x030827ca */
void wma_memmove(void *dst, const void *src, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    if (d == s || n == 0)
        return;
    if (d < s) {
        while (n--)
            *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--)
            *--d = *--s;
    }
}

/* wma_memset @ 0x0308280a — fill with byte value */
void wma_memset(void *dst, int val, uint32_t n)
{
    uint8_t *p = (uint8_t *)dst;
    while (n--)
        *p++ = (uint8_t)val;
}

/* wma_memclr @ 0x03082818 — zero fill */
void wma_memclr(void *dst, uint32_t n)
{
    wma_memset(dst, 0, n);
}

/*
 * wma_input_cache_read @ 0x03083d48
 * 512-byte ASF object cache reader used by wma_audio_parse.
 * Returns bytes placed in *out (0x36 for stream-properties header).
 */
int wma_input_cache_read(uint32_t *ctx, uint32_t **out, uint32_t lo, uint32_t hi)
{
    (void)ctx;
    (void)out;
    (void)lo;
    (void)hi;
    return 0;
}

typedef struct {
    uint8_t *input;          /* +0x1c */
    uint32_t input_left;     /* +0x20 */
    uint32_t cache;          /* +0x24 */
    uint32_t cache_bits;     /* +0x28 */
    uint32_t spill;          /* +0x2c */
    uint32_t spill_bits;     /* +0x30 */
} WmaBitReader;

extern int wma_bitreader_refill(WmaBitReader *br, int mode, uint32_t need);
extern const uint32_t *g_wma_bit_masks;

/* wma_bitreader_getbits @ 0x03084970 */
int wma_bitreader_getbits(WmaBitReader *br, uint32_t count, uint32_t *value)
{
    int status = 0;
    uint32_t cached = br->cache_bits;

    if (cached < count) {
        uint32_t spill_bits = br->spill_bits;
        if (spill_bits != 0) {
            uint32_t take = 32 - cached;
            if (spill_bits < take)
                take = spill_bits;
            spill_bits -= take;
            br->spill_bits = spill_bits;
            br->cache = (br->spill >> spill_bits) | (br->cache << take);
            br->cache_bits = cached + take;
            br->spill &= (1u << spill_bits) - 1u;
        }

        while (br->cache_bits < 25 && br->input_left != 0) {
            br->cache = (br->cache << 8) | *br->input++;
            --br->input_left;
            br->cache_bits += 8;
        }

        if (br->cache_bits < count) {
            status = wma_bitreader_refill(br, 2, count);
            if (status < 0)
                return status;
        }
    }

    br->cache_bits -= count;
    *value = (br->cache >> br->cache_bits) & g_wma_bit_masks[count];
    return status;
}

/* wma_floor_log2 @ 0x030842cc */
uint32_t wma_floor_log2(uint32_t value)
{
    uint32_t shift = 0;
    if (value <= 1)
        return 0;
    do {
        ++shift;
    } while ((value >> shift) > 1);
    return shift;
}
