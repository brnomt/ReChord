#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/firmware/media/id3_picture.c
 * Reconstructed from Ghidra decompilation of:
 *   ID3_Picture_Parser @ 0x03023526
 *
 * Scans a VorbisComment / FLAC METADATA_BLOCK_PICTURE style stream for
 * embedded cover art. On match of "METADATA_BLOCK_PICTURE":
 *   1. Opens a temp file "PICTURE_OGG"
 *   2. Base64-decodes the picture payload into that file
 *   3. Seeks back and probes for "image/" MIME
 *
 * Also walks Ogg pages when the picture lives in segmented packets.
 * Returns nonzero on success (MIME probe result), 0 on failure.
 *
 * Related strings:
 *   "METADATA_BLOCK_PICTURE", "PICTURE_OGG", "image/",
 *   "base64_decode_error: ret= %d", "FileWrite_error",
 *   "MetaBlockPicHandle Read Error"
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../media_api.h"

extern void rom_file_seek(int off, int whence, int fd);
extern int  rom_file_read(void *buf, int n, int fd);
extern int  rom_memcmp_n(const void *a, const void *b, int n);
extern int  rom_memmem(const void *hay, const void *needle, int n);
extern void rom_memcpy(void *d, const void *s, int n);
extern int  fs_open(void *path_key, const char *name);
extern int  fs_create(void *path_key, const char *name);
extern void fs_close(int fd);
extern int  fs_write(const void *buf, int off, int n, int fd);
extern void debug_printf(const char *fmt, ...);
extern void yield_ui(void);

extern uint8_t g_b64_table[];           /* DAT_03023ae8 */
extern int    *g_pic_open;              /* DAT_03023ad0 */
extern int8_t *g_pic_fd;                /* DAT_03023ad4 */

/*
 * ID3_Picture_Parser @ 0x03023526
 *
 * Calling convention in firmware is messy (unaff registers hold fd / remaining
 * length). This reconstruction uses explicit args matching the call sites from
 * VorbisOGG_Parser.
 */
uint32_t ID3_Picture_Parser(int fd, int *out_type)
{
    uint32_t remaining;
    uint32_t consumed = 0;
    int32_t  field_len;
    char     key[0x32];
    uint8_t  raw[0x200];
    uint8_t  decoded[0x200];
    int      pic_fd;
    int      out_off = 0;
    uint32_t have = 0;

    (void)out_type;
    rom_file_seek(4, 1, fd);            /* skip length prefix already consumed by caller */

    /* remaining = total - header; approximated from caller state */
    remaining = 0xffffffff;             /* walk until short read */

    while (remaining != 0) {
        if (rom_file_read(&field_len, 4, fd) < 4)
            return 0;
        consumed += 4;
        if (field_len < 1)
            return 0;

        /* Read key until '=' or 0x32 chars */
        {
            int i = 0;
            do {
                if (rom_file_read(&key[i], 1, fd) == 0)
                    return 0;
                consumed++;
                if (key[i] == '=' || i >= 0x31)
                    break;
                i++;
            } while (1);
            if (i == 0x32)
                return 0;

            {
                uint32_t payload = (uint32_t)field_len - (uint32_t)i - 1;
                if (rom_memcmp_n(key, "METADATA_BLOCK_PICTURE", i) != 0) {
                    rom_file_seek((int)payload, 1, fd);
                    consumed += payload;
                    continue;
                }

                /* Match — open PICTURE_OGG temp and base64-decode into it */
                if (*g_pic_open != 0) {
                    fs_close((int)*g_pic_fd);
                    *g_pic_fd = -1;
                    *g_pic_open = 0;
                }
                fs_create(&g_pic_path_key, "PICTURE_OGG");
                pic_fd = fs_open(&g_pic_path_key, "PICTURE_OGG");
                *g_pic_fd = (int8_t)pic_fd;
                if (pic_fd == -1)
                    return 0;
                *g_pic_open = 1;

                remaining -= consumed;
                /* Base64 decode loop (simplified from LAB_03023736) */
                while (remaining != 0 && payload != 0) {
                    uint32_t chunk = remaining > 0x200 ? 0x200 : remaining;
                    uint32_t n;
                    int b64_err = 0;
                    uint32_t out_n = 0;

                    yield_ui();
                    if (chunk > payload) chunk = payload;
                    n = (uint32_t)rom_file_read(raw, (int)chunk, fd);
                    if (n < chunk)
                        return 0;
                    remaining -= n;
                    payload -= n;

                    /* Validate + decode base64 (table lookup, 4→3) */
                    {
                        uint32_t j, acc = 0, nchars = 0, pads = 3;
                        uint8_t *dp = decoded;
                        for (j = 0; j < n; j++) {
                            uint8_t c = raw[j];
                            if (c == '\r' || c == '\n' || c == ' ')
                                continue;
                            if (c == '=') {
                                pads--;
                                if (pads > 2) { b64_err = -2; break; }
                            } else if (c >= 0x80 || g_b64_table[c] == 0x7f) {
                                b64_err = -3;
                                break;
                            }
                            acc = (acc << 6) | (g_b64_table[c] & 0x3f);
                            nchars++;
                            if (nchars == 4) {
                                nchars = 0;
                                if (pads) {
                                    *dp++ = (uint8_t)(acc >> 16);
                                    if (pads > 1) {
                                        *dp++ = (uint8_t)(acc >> 8);
                                        if (pads > 2)
                                            *dp++ = (uint8_t)acc;
                                    }
                                }
                                pads = 3;
                            }
                        }
                        out_n = (uint32_t)(dp - decoded);
                    }
                    if (b64_err) {
                        debug_printf("base64_decode_error: ret= %d\n", b64_err);
                        return 0;
                    }

                    {
                        uint32_t space = 0x200 - have;
                        uint32_t take = out_n < space ? out_n : space;
                        rom_memcpy(raw + have, decoded, (int)take); /* reuse raw as out buf */
                        have += take;
                        if (have == 0x200) {
                            if ((uint32_t)fs_write(raw, out_off, 0x200, pic_fd) < 0x200) {
                                debug_printf("FileWrite_error\n");
                                return 0;
                            }
                            out_off += 0x200;
                            have = out_n - take;
                            if (have)
                                rom_memcpy(raw, decoded + take, (int)have);
                        }
                    }
                }

                if (have) {
                    if ((uint32_t)fs_write(raw, out_off, (int)have, pic_fd) < have) {
                        debug_printf("FileWrite_error\n");
                        return 0;
                    }
                }

                rom_file_seek(0, 0, pic_fd);
                if (rom_file_read(raw, 0x200, pic_fd) < 0x200) {
                    debug_printf("MetaBlockPicHandle Read Error\n");
                    return 0;
                }
                return (uint32_t)rom_memmem(raw, "image/", 6);
            }
        }
    }
    return 0;
}
