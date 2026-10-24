/*
 * art_pack.c — precomputed album art lookup + streaming (see art_pack.h).
 *
 * Read-only by design. The only storage traffic is:
 *   - at track change: one file-size stat, the pack header, the key-table CRC
 *     pass and a binary search (a handful of sectors; deferred by the app loop
 *     until after the PCM ring is filled);
 *   - per art draw: the header (1 sector) + the image stream (large 120x120 =
 *     57 sectors).
 * No full-image RAM buffer: one 512-byte sector buffer feeds the framebuffer
 * row segments through display_blit_span().
 *
 * The pack is opened and closed per use. Holding a handle would take one of
 * fs_target's four read slots, which the codec, the Core-B file service and
 * the gapless pre-open already share.
 */
#include <stddef.h>
#include <string.h>

#include "art_pack.h"
#include "fs.h"
#include "rechord_files.h"
#include "../drivers/display.h"

/* ---- pack header (little-endian; docs/ART_PACK.md) ---------------------- */
#define H_MAGIC        0u
#define H_VERSION      4u
#define H_HDR_BYTES    8u
#define H_FLAGS        12u
#define H_LARGE_PX     16u
#define H_SMALL_PX     18u
#define H_TRACKS       20u
#define H_IMAGES       24u
#define H_IMG_SLOTS    28u
#define H_FREE_SLOTS   32u
#define H_KEY_OFF      36u
#define H_KEY_BYTES    40u
#define H_IMG_OFF      44u
#define H_IMG_BYTES    48u
#define H_SMALL_OFF    52u
#define H_SMALL_BYTES  56u
#define H_LARGE_OFF    60u
#define H_LARGE_BYTES  64u
#define H_KEY_CRC      68u
#define H_IMG_CRC      72u
#define H_SMALL_CRC    76u
#define H_LARGE_CRC    80u
#define H_HDR_CRC      84u

#define PACK_MAGIC     0x50545241u   /* "ARTP" little-endian */
#define PACK_VERSION   2u
#define HDR_BYTES      512u
#define KEY_ENTRY      16u
#define IMAGE_ENTRY    32u

/* ---- state ------------------------------------------------------------- */

static struct {
    int      ok;
    uint32_t hdr_crc;
    uint32_t image;
    uint32_t small_rel, large_rel;
    uint32_t small_base, large_base;
    /* ART_C_* order (the image-entry palette order, docs/ART_PACK.md). */
    uint16_t pal[11];
    uint16_t large_px, small_px;
} s;

/* One sector: the pack header, key entries, image entries and the pixel
 * stream all go through it (no full-image buffer). 4-byte aligned: the draw
 * loop hands row segments to display_blit_span() as uint16_t*. */
static uint8_t s_buf[HDR_BYTES] __attribute__((aligned(4)));

/* ---- helpers ----------------------------------------------------------- */

static uint32_t le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint16_t le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint64_t le64(const uint8_t *p)
{
    return (uint64_t)le32(p) | ((uint64_t)le32(p + 4) << 32);
}

/* CRC-32/ISO-HDLC (poly 0xEDB88320, init/final 0xFFFFFFFF): zlib.crc32 and
 * the host tool use the same convention. Bitwise on purpose: a table would
 * cost .bss and this runs on a handful of sectors. */
static uint32_t crc_update(uint32_t crc, const uint8_t *p, uint32_t n)
{
    while (n--) {
        int i;

        crc ^= *p++;
        for (i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)-(int32_t)(crc & 1u));
    }
    return crc;
}

static uint32_t crc_final(uint32_t crc)
{
    return crc ^ 0xFFFFFFFFu;
}

static int read_at(fs_file_t *f, uint32_t off, void *buf, uint32_t n)
{
    if (fs_seek(f, (long)off) != (long)off)
        return -1;
    return fs_read(f, buf, n) == (long)n ? 0 : -1;
}

static int section_crc(fs_file_t *f, uint32_t off, uint32_t bytes,
                       uint32_t want)
{
    uint32_t crc = 0xFFFFFFFFu;

    while (bytes > 0u) {
        uint32_t n = bytes > HDR_BYTES ? HDR_BYTES : bytes;

        if (read_at(f, off, s_buf, n) != 0)
            return -1;
        crc = crc_update(crc, s_buf, n);
        off += n;
        bytes -= n;
    }
    return crc_final(crc) == want ? 0 : -1;
}

/* Header CRC over the 512-byte header with its own CRC field zeroed. */
static int header_crc_ok(void)
{
    uint32_t want = le32(s_buf + H_HDR_CRC);

    s_buf[H_HDR_CRC + 0u] = 0u;
    s_buf[H_HDR_CRC + 1u] = 0u;
    s_buf[H_HDR_CRC + 2u] = 0u;
    s_buf[H_HDR_CRC + 3u] = 0u;
    return crc_final(crc_update(0xFFFFFFFFu, s_buf, HDR_BYTES)) == want;
}

/* ---- key --------------------------------------------------------------- */

uint64_t art_pack_key(const char *device_path, uint32_t file_bytes)
{
    uint64_t h = 0xCBF29CE484222325ull;   /* FNV-1a 64 offset basis */
    const uint8_t *p = (const uint8_t *)device_path;
    int i;

    if (p == 0)
        return 0;
    if (*p == '/')                        /* "/<Volume>/..." -> "<Volume>/..." */
        p++;

    for (; *p != 0; p++)
        h = (h ^ *p) * 0x100000001B3ull;
    for (i = 0; i < 4; i++) {
        h = (h ^ (uint8_t)((file_bytes >> (8 * i)) & 0xFFu)) *
            0x100000001B3ull;
    }
    return h;
}

/* ---- lookup ------------------------------------------------------------ */

void art_pack_clear(void)
{
    s.ok = 0;
    s.image = 0;
    s.small_rel = s.large_rel = 0;
    s.small_base = s.large_base = 0;
    memset(s.pal, 0, sizeof s.pal);
    s.large_px = s.small_px = 0;
    s.hdr_crc = 0;
}

int art_pack_set_track(const char *device_path)
{
    fs_file_t *f;
    fs_file_t *tf;
    long fsize;
    long tsize;
    uint64_t key;
    uint32_t tracks, images, slots;
    uint32_t key_off, key_bytes, img_off, img_bytes;
    uint32_t small_off, small_bytes, large_off, large_bytes;
    uint32_t large_px, small_px;
    uint32_t hdr_crc;
    int32_t lo, hi;
    int i;
    int found = -1;

    art_pack_clear();
    if (device_path == 0 || device_path[0] == '\0')
        return -1;

    /* The key includes the file size: one stat (own handle, closed at once). */
    tf = fs_open_read(device_path);
    if (tf == 0)
        return -1;
    tsize = fs_file_size(tf);
    fs_close(tf);
    if (tsize <= 0)
        return -1;
    key = art_pack_key(device_path, (uint32_t)tsize);

    f = fs_open_read(ARTPACK_PATH);
    if (f == 0)
        return -1;
    fsize = fs_file_size(f);
    if (fsize < (long)HDR_BYTES)
        goto fail;
    if (read_at(f, 0, s_buf, HDR_BYTES) != 0)
        goto fail;

    if (le32(s_buf + H_MAGIC) != PACK_MAGIC ||
        le32(s_buf + H_VERSION) != PACK_VERSION ||
        le32(s_buf + H_HDR_BYTES) != HDR_BYTES)
        goto fail;
    hdr_crc = le32(s_buf + H_HDR_CRC);
    if (!header_crc_ok())
        goto fail;
    s.hdr_crc = hdr_crc;

    large_px = le16(s_buf + H_LARGE_PX);
    small_px = le16(s_buf + H_SMALL_PX);
    tracks = le32(s_buf + H_TRACKS);
    images = le32(s_buf + H_IMAGES);
    slots = le32(s_buf + H_IMG_SLOTS);
    key_off = le32(s_buf + H_KEY_OFF);
    key_bytes = le32(s_buf + H_KEY_BYTES);
    img_off = le32(s_buf + H_IMG_OFF);
    img_bytes = le32(s_buf + H_IMG_BYTES);
    small_off = le32(s_buf + H_SMALL_OFF);
    small_bytes = le32(s_buf + H_SMALL_BYTES);
    large_off = le32(s_buf + H_LARGE_OFF);
    large_bytes = le32(s_buf + H_LARGE_BYTES);

    if (large_px == 0u || small_px == 0u || large_px > 255u ||
        small_px > 255u || images > slots)
        goto fail;
    if ((key_off % HDR_BYTES) != 0u || (img_off % HDR_BYTES) != 0u ||
        (small_off % HDR_BYTES) != 0u || (large_off % HDR_BYTES) != 0u)
        goto fail;
    if (key_bytes < tracks * KEY_ENTRY ||
        img_bytes < slots * IMAGE_ENTRY)
        goto fail;
    if ((uint32_t)fsize < key_off || (uint32_t)fsize - key_off < key_bytes ||
        (uint32_t)fsize < img_off || (uint32_t)fsize - img_off < img_bytes ||
        (uint32_t)fsize < small_off ||
        (uint32_t)fsize - small_off < small_bytes ||
        (uint32_t)fsize < large_off ||
        (uint32_t)fsize - large_off < large_bytes)
        goto fail;
    if (section_crc(f, key_off, key_bytes, le32(s_buf + H_KEY_CRC)) != 0)
        goto fail;

    lo = 0;
    hi = (int32_t)tracks - 1;
    while (lo <= hi) {
        int32_t mid = lo + (hi - lo) / 2;

        if (read_at(f, key_off + (uint32_t)mid * KEY_ENTRY, s_buf,
                    KEY_ENTRY) != 0)
            goto fail;
        if (le64(s_buf) == key) {
            found = mid;
            break;
        }
        if (le64(s_buf) < key)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    if (found < 0)
        goto fail;

    s.image = le32(s_buf + 8);
    if (s.image >= slots)
        goto fail;
    if (read_at(f, img_off + s.image * IMAGE_ENTRY, s_buf, IMAGE_ENTRY) != 0)
        goto fail;
    s.small_rel = le32(s_buf + 0);
    s.large_rel = le32(s_buf + 4);
    for (i = 0; i < 11; i++)
        s.pal[i] = le16(s_buf + 8 + (uint32_t)i * 2u);

    if (s.small_rel > small_bytes ||
        small_bytes - s.small_rel < (uint32_t)small_px * small_px * 2u ||
        s.large_rel > large_bytes ||
        large_bytes - s.large_rel < (uint32_t)large_px * large_px * 2u)
        goto fail;

    s.small_base = small_off;
    s.large_base = large_off;
    s.large_px = (uint16_t)large_px;
    s.small_px = (uint16_t)small_px;
    s.ok = 1;
    fs_close(f);
    return 0;

fail:
    fs_close(f);
    art_pack_clear();
    return -1;
}

int art_pack_ok(void)
{
    return s.ok;
}

int art_pack_color(int which)
{
    if (which < 0 || which >= (int)(sizeof s.pal / sizeof s.pal[0]))
        return 0;
    return s.pal[which];
}

int art_pack_dim(int large)
{
    return large ? s.large_px : s.small_px;
}

/* ---- draw -------------------------------------------------------------- */

int art_pack_draw(int x, int y, int large)
{
    fs_file_t *f;
    uint32_t base, rel, bytes;
    uint32_t size;
    uint32_t left;
    int row, col;

    if (!s.ok)
        return -1;
    size = large ? s.large_px : s.small_px;
    base = large ? s.large_base : s.small_base;
    rel = large ? s.large_rel : s.small_rel;
    if (size == 0u)
        return -1;

    f = fs_open_read(ARTPACK_PATH);
    if (f == 0)
        return -1;
    /* Detect a pack replaced under us: the layout we cached no longer holds. */
    if (read_at(f, 0, s_buf, HDR_BYTES) != 0 ||
        le32(s_buf + H_MAGIC) != PACK_MAGIC ||
        le32(s_buf + H_HDR_CRC) != s.hdr_crc || !header_crc_ok()) {
        fs_close(f);
        art_pack_clear();
        return -1;
    }
    if (fs_seek(f, (long)(base + rel)) != (long)(base + rel)) {
        fs_close(f);
        return -1;
    }

    bytes = size * size * 2u;
    left = bytes;
    row = 0;
    col = 0;
    while (left > 0u) {
        uint32_t chunk = left > HDR_BYTES ? HDR_BYTES : left;
        long n = fs_read(f, s_buf, chunk);
        uint32_t i = 0;

        if (n <= 0)
            break;
        if (((uint32_t)n & 1u) != 0u)
            break;                    /* mid-pixel short read: corrupt pack */
        while (i + 1u < (uint32_t)n) {
            uint32_t avail = ((uint32_t)n - i) / 2u;
            int run = (int)size - col;

            if ((uint32_t)run > avail)
                run = (int)avail;
            display_blit_span(x + col, y + row, run,
                              (const uint16_t *)(const void *)(s_buf + i));
            col += run;
            i += (uint32_t)run * 2u;
            if ((uint32_t)col >= size) {
                col = 0;
                row++;
            }
        }
        left -= (uint32_t)n;
    }
    fs_close(f);
    return left == 0u ? 0 : -1;
}
