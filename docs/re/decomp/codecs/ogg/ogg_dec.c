#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/codecs/ogg/ogg_dec.c
 * Reconstructed from Ghidra decompilation of:
 *   VorbisOGG_Parser @ 0x03023be8
 *
 * Despite the name, this is a multi-format tag / cover-art scanner dispatched
 * by media type (return of FUN_0300f088). Cases observed:
 *   3   ID3v2 picture          → FUN_03023426 (ID3_Picture_Parser sibling)
 *   4/8/9  stream size probe
 *   5   RIFF/WAVE walk for "LIST"/embedded art
 *   6   APETAGEX footer scan for "Cover Art (front)"
 *   7   FLAC METADATA_BLOCK_PICTURE (type 6) / vorbiscomment
 *   10  Ogg page walk looking for "vorbis" identification packet
 *   14  DSD "DSD " chunk → nested picture parse
 *
 * Returns cover-art size / offset info via *param_2, or 0 / -1 on failure.
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../codec_api.h"

extern uint16_t media_type_of(void *path_buf, const void *table); /* FUN_0300f088 */
extern uint32_t ID3_Picture_Parser(int fd, int *out_type);        /* FUN_03023426 */
extern int  rom_file_size(int fd);           /* func_0x02ff80c0 */
extern int  rom_file_read(void *buf, int n, int fd); /* func_0x02ffc9d6 */
extern void rom_file_seek(int off, int whence, int fd); /* func_0x02ffcece */
extern int  rom_memcmp(const void *a, const void *b, int n); /* func_0x02ff89b4 */
extern int  rom_memmem(const void *h, const void *n, int hn); /* func_0x02ff89ce */
extern int  rom_strcmp(const char *a, const char *b);        /* func_0x02ff89ec */

/*
 * VorbisOGG_Parser @ 0x03023be8
 */
uint32_t VorbisOGG_Parser(int fd_or_path, int *out_pic_type)
{
    uint16_t kind;
    uint8_t  buf[0x200];
    uint32_t n, size, i;
    int fd = (int)(int8_t)fd_or_path;   /* decompiler treats as char-promoted fd */

    rom_file_seek(0, 0, fd);
    {
        uint8_t path_buf[4];
        FUN_0300c784(path_buf, fd_or_path);     /* build path key */
        kind = media_type_of(path_buf, g_fmt_table);
    }

    switch (kind) {
    case 3:                                     /* ID3 */
        *g_id3_flag = 0;
        return ID3_Picture_Parser(fd_or_path, out_pic_type);

    case 4: case 8: case 9: {
        int a, b, c;
        media_probe_open(fd_or_path);
        media_probe_reset();
        a = media_probe_a();
        b = media_probe_b();
        c = media_probe_c();
        if (a >= 0 && (int)b >= 0 && c >= 0) {
            *out_pic_type = c;
            rom_file_seek(a, 0, fd);
            return (uint32_t)b;
        }
        return 0;
    }

    case 5: {                                   /* RIFF / WAVE */
        int rem = rom_file_size(fd);
        if (rem <= 0) return 0;
        n = rom_file_read(buf, 12, fd);
        if (n < 12) return 0;
        if (buf[0] != 'R' || buf[1] != 'I' || buf[2] != 'F' || buf[3] != 'F')
            return 0;
        if (buf[8] != 'W' || buf[9] != 'A' || buf[10] != 'V' || buf[11] != 'E')
            return 0;
        rem -= 12;
        while (rem > 0) {
            uint32_t chunk_sz;
            uint8_t  id[4];
            if (rom_file_read(id, 4, fd) < 4) break;
            if (rom_file_read(&chunk_sz, 4, fd) < 4 || (int)chunk_sz < 0) break;
            if (rom_memcmp(id, "LIST", 4) == 0)  /* or art-bearing chunk */
                return ID3_Picture_Parser(fd, out_pic_type);
            rom_file_seek((int)chunk_sz, 1, fd);
            rem -= 8 + (int)chunk_sz;
        }
        return 0;
    }

    case 6: {                                   /* APEv2 tag at EOF */
        size = rom_file_size(fd);
        if (size == 0) return 0;
        rom_file_seek((int)size - 0x20, 0, fd);
        if (rom_file_read(buf, 8, fd) < 8) return 0;
        if (rom_memcmp(buf, "APETAGEX", 8) != 0) return 0;
        /* version, tag_size, item_count, flags — then walk items for
           "Cover Art (front)" and sniff JPEG (FF D8) / BMP (BM) */
        /* Full item loop omitted — see Ghidra @ 0x03023dxx */
        return 0;
    }

    case 7: {                                   /* FLAC / vorbiscomment picture */
        size = rom_file_size(fd);
        if (size == 0) return 0;
        if (rom_file_read(buf, 4, fd) < 4) return 0;
        if (rom_memcmp(buf, "fLaC", 4) == 0 || rom_memcmp(buf, "OggS", 4) == 0) {
            /* walk metadata blocks; type 6 = PICTURE */
            n = rom_file_read(buf, 0x200, fd);
            if (n >= 0x200)
                return (uint32_t)rom_memmem(buf, "image/", 6);
        }
        return 0;
    }

    case 10: {                                  /* Ogg pages → "vorbis" */
        for (;;) {
            uint8_t hdr[8], seg, packet_type, vorbis_id[6];
            yield_ui();                         /* func_0x03004d9c */
            if (rom_file_read(hdr, 4, fd) < 4) break;
            if (rom_memcmp(hdr, "OggS", 4) != 0) break;
            if (rom_file_read(&seg, 1, fd) == 0) return 0;
            if (seg != 0) break;
            if (rom_file_read(&packet_type, 1, fd) == 0) return 0;
            if (rom_file_read(hdr, 8, fd) < 8) break;
            rom_file_seek(0xc, 1, fd);
            {
                uint8_t seglen;
                if (rom_file_read(&seglen, 1, fd) == 0) return 0;
                if (rom_file_read(buf, seglen, fd) < seglen) break;
                if (packet_type != 1) {
                    uint8_t pt2;
                    if (rom_file_read(&pt2, 1, fd) == 0) return 0;
                    if (rom_file_read(vorbis_id, 6, fd) < 6) break;
                    if (pt2 == 3)
                        return (uint32_t)rom_memmem(vorbis_id, "vorbis", 6);
                    rom_file_seek(-7, 1, fd);
                }
                /* skip segment payload sum */
                {
                    int sum = 0, k;
                    for (k = 0; k < seglen; k++) sum += buf[k];
                    rom_file_seek(sum, 1, fd);
                }
            }
            if (packet_type == 4) break;
        }
        return 0;
    }

    case 14: {                                  /* DSD chunk */
        rom_file_seek(0, 0, fd);
        n = rom_file_read(buf, 0x14, fd);
        if (n < 0x14) return 0xffffffff;
        if (buf[0] != 'D' || buf[1] != 'S' || buf[2] != 'D') return 0xffffffff;
        if (rom_file_read(g_dsd_chunk_size, 4, fd) < 4) return 0xffffffff;
        rom_file_seek(*g_dsd_chunk_size, 0, fd);
        n = ID3_Picture_Parser(fd_or_path, out_pic_type);
        *g_dsd_chunk_size = 0;
        return n;
    }

    default:
        return 0;
    }
}
