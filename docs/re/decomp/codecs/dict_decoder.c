#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/codecs/dict_decoder.c
 * Reconstructed from Ghidra decompilation of:
 *   DICTDECODER_InitStream  @ 0x0304fb44  (header/stream parser)
 *   dict_stream_read        @ 0x0304eec4  (buffered stream reader)
 *   dict_build_frame_index  @ 0x0304f90a  (seek/frame-offset table builder)
 *
 * The "DICTDECODER" is the container/stream layer that sits in front of the
 * individual codecs. Files use a 4-byte container magic:
 *   'VaT ' (0x20546156 LE)  -> g_dwDictMagicVaT @ 0x0304fd4c
 *   'VAT ' (0x20544156 LE)  -> g_dwDictMagicVAT @ 0x0304fd50
 *
 * NOTE (audit): the functions originally labeled DICTDECODER_OpenFile /
 * DICTDECODER_DispatchFile @ 0x0301020c/0x0301022c were misnamed. They are the
 * main-UI keypad handler and the GOODEF DSP reload path respectively, and
 * have been renamed in Ghidra to MainUI_KeyHandler / DSP_GOODEF_Reload.
 * This file covers the *real* DICTDECODER stream parser at 0x0304fb44.
 */

#include "../firmware.h"
#include "../firmware/os/os_api.h"
#include "codec_api.h"

/* Container magic values (little-endian 4CC) */
#define DICT_MAGIC_VAT_LOWER 0x20546156u   /* 'VaT ' */
#define DICT_MAGIC_VAT_UPPER 0x20544156u   /* 'VAT ' */

/* Format codes seen in the header (pHeader[0], pHeader[1] = subtype) */
#define DICT_FMT_70 0x70
#define DICT_FMT_71 0x71
#define DICT_FMT_F1 0xf1
#define DICT_FMT_75 0x75   /* index skipped for these two */
#define DICT_FMT_76 0x76

/*
 * DICTDECODER header (0x28 bytes / 20 shorts), field offsets in `short` units:
 *   [0]      format code (0x70/0x71/0xf1/0x75/0x76)
 *   [1]      format subtype (0x62/0x42/0x12 ...)
 *   [2](lo)  stream storage mode 1..4 (used by dict_stream_read)
 *   [4..5]   total data size (int @ byte 0x10)
 *   [6],[7]  block dims (validated: [7] == 0x2A4 for non-0x70/0x62 files)
 *   [8]      frames-per-block multiplier
 *   [9],[10] channel / alignment params
 *   [0xb]    codec sub-id (0x32 / 0x2e checks)
 *   [0xc..0xf] 64-bit sizes
 *   [0x10]   fixed marker (=4)
 *   [0x12]   computed data length (from frame index)
 */
typedef struct {
    uint16_t format;          /* +0x00 */
    uint16_t subtype;         /* +0x02 */
    uint16_t storage_mode;    /* +0x04 (low byte used) */
    uint16_t _pad06;          /* +0x06 */
    uint32_t total_size;      /* +0x08 (shorts [4],[5]) */
    uint16_t block_w;         /* +0x0c (short [6]) */
    uint16_t block_h;         /* +0x0e (short [7]) */
    uint16_t frames_per_blk;  /* +0x10 (short [8]) */
    uint16_t channels;        /* +0x12 (short [9]) */
    uint16_t align;           /* +0x14 (short [10]) */
    uint16_t sub_id;          /* +0x16 (short [0xb]) */
    uint32_t size_lo;         /* +0x18 (shorts [0xc],[0xd]) */
    uint32_t size_hi;         /* +0x1c (shorts [0xe],[0xf]) */
    uint16_t marker;          /* +0x20 (short [0x10]) = 4 */
    uint16_t _pad22;
    uint32_t data_len;        /* +0x24 (shorts [0x12],[0x13]) */
} DictHeader;

/* Low-level stream primitives (boot-ROM / fs helpers, kept as raw refs) */
extern int  dict_seek(int hStream, int off, int whence);   /* FUN_030198e4 */
extern int  dict_raw_read(int hStream, void *buf, int n);  /* FUN_030198b6 */
extern void dict_mem_read(void *dst, int src, int n);      /* FUN_0301a30e */
extern int  dict_stream_read(void *hdr, int n, void *out); /* @0x0304eec4 */
extern int  dict_build_frame_index(uint16_t *hdr, int hStream, void *idx); /* @0x0304f90a */
extern void debug_log(const char *fmt, ...);               /* FUN_03033618 */

/*
 * DICTDECODER_InitStream @ 0x0304fb44
 *
 * Zeroes the 0x28-byte header, validates the container magic, then reads and
 * range-checks the format fields. On success (return 1) the seek/frame index is
 * built via dict_build_frame_index and header->data_len is populated.
 *
 * Returns 1 on success, 0 on any validation/read failure.
 */
int DICTDECODER_InitStream(DictHeader *pHeader, int hStream)
{
    uint32_t magic;
    uint16_t *h = (uint16_t *)pHeader;

    if (pHeader == NULL || hStream == -1) {
        debug_log("DICTDECODER_InitDataStream 0\n");
        return 0;
    }

    /* Clear all 20 header shorts (0x28 bytes) */
    for (int i = 0; i < 20; i++)
        h[i] = 0;

    /* Open/prime the stream (mode probe) */
    if (dict_open_probe(hStream) == 0) {
        debug_log("DICTDECODER_InitDataStream 1\n");
        return 0;
    }
    if (dict_seek(hStream, 0, 0 /*SEEK_SET*/) != 0) {
        debug_log("DICTDECODER_InitDataStream 2\n");
        return 0;
    }

    /* --- Container magic --- */
    magic = 0;
    if (dict_stream_read((void *)hStream, 4, &magic) != 4)
        return 0;
    if (magic != g_dwDictMagicVaT && magic != g_dwDictMagicVAT)
        return 0;

    /* --- Header fields (widths from decompiler) --- */
    if (dict_stream_read((void *)hStream, 2, &h[0]) != 2) return 0;  /* format   */
    if (dict_stream_read((void *)hStream, 2, &h[1]) != 2) return 0;  /* subtype  */
    if (dict_stream_read((void *)hStream, 1, &h[2]) != 1) return 0;  /* storage  */
    if (dict_stream_read((void *)hStream, 1, &h[2]) != 1) return 0;  /* reserved */
    pHeader->storage_mode = (pHeader->storage_mode & 0xff00) | 2;

    /* three 32-bit reserved dwords + total size */
    uint32_t tmp = 0;
    if (dict_stream_read((void *)hStream, 4, &tmp) != 4) return 0;
    if (dict_stream_read((void *)hStream, 4, &tmp) != 4) return 0;
    if (dict_stream_read((void *)hStream, 4, &tmp) != 4) return 0;
    tmp = 0;
    if (dict_stream_read((void *)hStream, 4, &tmp) != 4) return 0;
    pHeader->total_size = tmp;

    /* --- Block dimensions --- */
    if (pHeader->format == DICT_FMT_70 || pHeader->format == DICT_FMT_71) {
        if (pHeader->subtype == 0x62) {
            pHeader->block_w = 0;
            pHeader->block_h = 0;
        } else {
            if (!read_block_dims(hStream, pHeader)) return 0;
        }
    } else {
        if (!read_block_dims(hStream, pHeader)) return 0;
    }

    /* frames/channels/align + reserved dword + sub-id */
    if (dict_stream_read((void *)hStream, 2, &h[8])  != 2) return 0;
    if (dict_stream_read((void *)hStream, 2, &h[9])  != 2) return 0;
    if (dict_stream_read((void *)hStream, 2, &h[10]) != 2) return 0;
    if (dict_stream_read((void *)hStream, 4, &tmp)   != 4) return 0;
    if (dict_stream_read((void *)hStream, 2, &h[0xb]) != 2) return 0;

    /* --- Per-format sub-id validation --- */
    if (pHeader->format == DICT_FMT_70) {
        if (pHeader->subtype == 0x42 && pHeader->sub_id != 0x32) return 0;
    } else if (pHeader->format == DICT_FMT_F1) {
        if (pHeader->subtype == 0x12 && pHeader->sub_id != 0x32) return 0;
    }
    if (pHeader->subtype == 0x62 && pHeader->sub_id != 0x2e) return 0;

    /* --- 64-bit sizes --- */
    if (dict_stream_read((void *)hStream, 4, &h[0xc]) != 4) return 0;
    if (dict_stream_read((void *)hStream, 4, &h[0xe]) != 4) return 0;
    pHeader->marker = 4;

    /* --- Build seek/frame index (skipped for raw 0x75/0x76) --- */
    if (pHeader->format != DICT_FMT_75 && pHeader->format != DICT_FMT_76) {
        uint32_t last_off = 0;
        struct { /* mirrors the 0x30-byte local index descriptor */
            uint32_t mode, r1, r2, r3, r4;
            uint32_t *p_off;
            uint32_t r6, r7, r8, r9, r10, r11;
        } idx = {0};
        idx.mode  = 1;
        idx.p_off = &last_off;

        if (dict_build_frame_index((uint16_t *)pHeader, hStream, &idx) == 0)
            return 0;

        pHeader->data_len = last_off - pHeader->align;
    }

    return 1;
}

/* Read block_w/block_h and enforce block_h == 0x2A4 (helper split out for clarity). */
static int read_block_dims(int hStream, DictHeader *pHeader)
{
    uint16_t *h = (uint16_t *)pHeader;
    if (dict_stream_read((void *)hStream, 2, &h[6]) != 2) return 0;
    if (dict_stream_read((void *)hStream, 2, &h[7]) != 2) return 0;
    if (pHeader->block_h != 0x2a4) return 0;
    return 1;
}
