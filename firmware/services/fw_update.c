/*
 * fw_update.c -- firmware update from the user volume (see fw_update.h).
 *
 * Image checks follow the reference updater: "RKnanoFW" at +0x1F8, fw size
 * at +0x1F4, a copy of the 512-byte header at offset fw size, and
 * fw_end = (size + 0x104000) & ~0xFFFFF bytes covered by the trailer CRC
 * stored at fw_end. Slot 1 = firmware-area sector 0, slot 2 = half way.
 */
#include <string.h>

#include "drivers/emmc.h"
#include "services/fw_update.h"

#define BLK 16u                              /* sectors per block (8 KiB) */

static uint32_t s_sec[128], s_fat[128], s_fat_lba;
static uint32_t s_blk[BLK * 128u], s_chk[BLK * 128u];
static struct { uint32_t fat, data, spc, root, nfats, fatsz; } s_fs;

static uint32_t le16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
static uint32_t le32(const uint8_t *p) { return le16(p) | le16(p + 2) << 16; }

/* ---- CRC-32 poly 0x04C10DB7, MSB first, init 0 (device trailer) ---- */

static uint32_t crc_update(uint32_t crc, const uint8_t *p, uint32_t n)
{
    static uint32_t tab[256];

    if (!tab[1]) {
        for (uint32_t i = 0; i < 256u; i++) {
            uint32_t r = i << 24;
            for (int k = 0; k < 8; k++)
                r = r & 0x80000000u ? r << 1 ^ 0x04C10DB7u : r << 1;
            tab[i] = r;
        }
    }
    while (n--)
        crc = crc << 8 ^ tab[(crc >> 24 ^ *p++) & 0xFFu];
    return crc;
}

/* ---- SHA-256 ---- */

typedef struct { uint32_t h[8], len; uint8_t buf[64]; uint32_t used; } sha256_t;

static const uint32_t k_sha[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

#define ROR(x, n) ((x) >> (n) | (x) << (32 - (n)))

static void sha_block(uint32_t *h, const uint8_t *p)
{
    uint32_t w[64], a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], k = h[7];

    for (int i = 0; i < 16; i++)
        w[i] = (uint32_t)p[4 * i] << 24 | (uint32_t)p[4 * i + 1] << 16 | (uint32_t)p[4 * i + 2] << 8 | p[4 * i + 3];
    for (int i = 16; i < 64; i++)
        w[i] = w[i - 16] + (ROR(w[i - 15], 7) ^ ROR(w[i - 15], 18) ^ w[i - 15] >> 3) +
               w[i - 7] + (ROR(w[i - 2], 17) ^ ROR(w[i - 2], 19) ^ w[i - 2] >> 10);
    for (int i = 0; i < 64; i++) {
        uint32_t t1 = k + (ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25)) + ((e & f) ^ (~e & g)) + k_sha[i] + w[i];
        uint32_t t2 = (ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        k = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += k;
}

static void sha_init(sha256_t *s)
{
    static const uint32_t iv[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
    memcpy(s->h, iv, sizeof iv);
    s->len = s->used = 0;
}

static void sha_update(sha256_t *s, const uint8_t *p, uint32_t n)
{
    s->len += n;
    while (n) {
        if (s->used == 0u && n >= 64u) {
            sha_block(s->h, p);
            p += 64; n -= 64;
            continue;
        }
        s->buf[s->used++] = *p++;
        n--;
        if (s->used == 64u) {
            sha_block(s->h, s->buf);
            s->used = 0;
        }
    }
}

/* Message length < 512 MiB (len counts bytes in 32 bits). */
static void sha_final(sha256_t *s, uint8_t out[32])
{
    uint32_t bits = s->len << 3, hi = s->len >> 29;
    uint8_t pad = 0x80;

    sha_update(s, &pad, 1);
    pad = 0;
    while (s->used != 56u)
        sha_update(s, &pad, 1);
    uint8_t l[8] = { 0, 0, 0, (uint8_t)hi, (uint8_t)(bits >> 24), (uint8_t)(bits >> 16),
                     (uint8_t)(bits >> 8), (uint8_t)bits };
    sha_update(s, l, 8);
    for (int i = 0; i < 32; i++)
        out[i] = (uint8_t)(s->h[i / 4] >> (24 - 8 * (i % 4)));
}

/* ---- FAT32 (MBR partition 0 or superfloppy) ---- */

static int fs_mount(uint32_t lba)
{
    const uint8_t *b = (const uint8_t *)s_sec;

    s_fat_lba = 0;
    if (emmc_read(lba, 1, s_sec))
        return FWU_EIO;
    if (!((b[0] == 0xEB || b[0] == 0xE9) && le16(b + 0x0B) == 512u)) {
        lba += le32(b + 0x1C6);                  /* MBR: first partition */
        if (emmc_read(lba, 1, s_sec))
            return FWU_EIO;
    }
    if (le16(b + 0x0B) != 512u || b[0x0D] == 0 || le16(b + 0x16) != 0 ||
        memcmp(b + 0x52, "FAT32", 5) != 0)
        return FWU_EFS;
    s_fs.spc = b[0x0D];
    s_fs.fat = lba + le16(b + 0x0E);
    s_fs.nfats = b[0x10];
    s_fs.fatsz = le32(b + 0x24);
    s_fs.data = s_fs.fat + s_fs.nfats * s_fs.fatsz;
    s_fs.root = le32(b + 0x2C);
    return 0;
}

static int fat_next(uint32_t c, uint32_t *next)
{
    uint32_t lba = s_fs.fat + c / 128u;

    if (lba != s_fat_lba) {
        s_fat_lba = 0;
        if (emmc_read(lba, 1, s_fat))
            return FWU_EIO;
        s_fat_lba = lba;
    }
    *next = s_fat[c % 128u] & 0x0FFFFFFFu;
    return 0;
}

static uint32_t clus_lba(uint32_t c)
{
    return s_fs.data + (c - 2u) * s_fs.spc;
}

static int is_update_name(const uint8_t *e)
{
    if (memcmp(e + 8, "IMG", 3) != 0)
        return 0;
    return memcmp(e, "ECHOMINI", 8) == 0 ||
           (memcmp(e, "HIFIEC", 6) == 0 && (unsigned)(e[6] - '0') < 10u &&
            (unsigned)(e[7] - '0') < 10u);
}

static int validate(fw_image_t *img, uint32_t first, fwu_progress_t progress)
{
    const uint8_t *h = (const uint8_t *)s_blk;
    uint32_t cl = s_fs.spc * 512u, c = first, next, fwsize, crc = 0, trailer = 0;
    sha256_t sha;

    if (img->size < 0x400u || first < 2u)
        return FWU_EFORMAT;
    for (uint32_t i = 1; i < (img->size + cl - 1u) / cl; i++, c = next) {
        if (fat_next(c, &next))
            return FWU_EIO;
        if (next != c + 1u)
            return FWU_EFRAG;
    }
    img->lba = clus_lba(first);

    if (emmc_read(img->lba, 1, s_blk))
        return FWU_EIO;
    fwsize = le32(h + 0x1F4);
    img->fw_end = (fwsize + 0x104000u) & ~0xFFFFFu;
    if (memcmp(h + 0x1F8, "RKnanoFW", 8) != 0 || fwsize < 0x200u || (fwsize & 511u) ||
        fwsize + 0x200u > img->size || img->fw_end + 4u > img->size)
        return FWU_EFORMAT;
    if ((img->fw_end >> 9) > emmc_fw_sectors() / 2u)
        return FWU_EFIT;
    if (emmc_read(img->lba + (fwsize >> 9), 1, s_chk))
        return FWU_EIO;
    if (memcmp(s_blk, s_chk, 512) != 0)
        return FWU_EFORMAT;

    sha_init(&sha);
    for (uint32_t off = 0; off < img->size; off += BLK * 512u) {
        uint32_t n = img->size - off < BLK * 512u ? img->size - off : BLK * 512u;

        if (emmc_read(img->lba + (off >> 9), (n + 511u) >> 9, s_blk))
            return FWU_EIO;
        if (off < img->fw_end)
            crc = crc_update(crc, h, img->fw_end - off < n ? img->fw_end - off : n);
        else if (off == img->fw_end)             /* fw_end is 1 MiB aligned */
            trailer = le32(h);
        sha_update(&sha, h, n);
        if (progress)
            progress("checking", off + n, img->size);
    }
    sha_final(&sha, img->sha256);
    return crc == trailer ? 0 : FWU_ECRC;
}

int fw_update_find(uint32_t vol_lba, fw_image_t *img, fwu_progress_t progress)
{
    const uint8_t *b = (const uint8_t *)s_sec;
    uint32_t c;
    int r;

    memset(img, 0, sizeof *img);
    if ((r = fs_mount(vol_lba)))
        return r;
    c = s_fs.root;
    for (int n = 0; n < 64 && c >= 2u && c < 0x0FFFFFF8u; n++) {
        for (uint32_t s = 0; s < s_fs.spc; s++) {
            if (emmc_read(clus_lba(c) + s, 1, s_sec))
                return FWU_EIO;
            for (uint32_t off = 0; off < 512u; off += 32u) {
                const uint8_t *e = b + off;
                char *p = img->name;

                if (e[0] == 0)
                    return FWU_NONE;
                if (e[0] == 0xE5 || e[11] == 0x0F || (e[11] & 0x18) || !is_update_name(e))
                    continue;
                memcpy(img->raw, e, 11);
                memcpy(p, e, 8);
                p[8] = '.';
                memcpy(p + 9, e + 8, 3);
                img->size = le32(e + 0x1C);
                img->dir_lba = clus_lba(c) + s;
                img->dir_off = off;
                return validate(img, le16(e + 0x14) << 16 | le16(e + 0x1A), progress);
            }
        }
        if (fat_next(c, &c))
            return FWU_EIO;
    }
    return FWU_NONE;
}

static int put(uint32_t rel, const uint32_t *src)
{
    if (emmc_fw_write(rel, BLK, src))
        return FWU_EWRITE;
    if (emmc_read(EMMC_FW_BASE + rel, BLK, s_chk))
        return FWU_EIO;
    return memcmp(src, s_chk, BLK * 512u) ? FWU_EVERIFY : 0;
}

/* Slot order and header-last writes are the reference's: at every point
 * one slot holds either the old or the new firmware, complete. */
int fw_update_install(const fw_image_t *img, fwu_progress_t progress)
{
    uint32_t n = img->fw_end >> 9, s2 = emmc_fw_sectors() / 2u, b;
    int r;

    if (n == 0u || n > s2)
        return FWU_EFIT;
    memset(s_blk, 0xFF, sizeof s_blk);
    if ((r = put(s2, s_blk)))
        return r;
    for (b = BLK; b < n; b += BLK) {
        if (emmc_read(img->lba + b, BLK, s_blk))
            return FWU_EIO;
        if ((r = put(s2 + b, s_blk)))
            return r;
        if (progress)
            progress("slot 2", b, n);
    }
    if (emmc_read(img->lba, BLK, s_blk))
        return FWU_EIO;
    if ((r = put(s2, s_blk)))
        return r;

    memset(s_blk, 0xFF, sizeof s_blk);
    if ((r = put(0, s_blk)))
        return r;
    for (b = BLK; b < n; b += BLK) {
        if (emmc_read(EMMC_FW_BASE + s2 + b, BLK, s_blk))
            return FWU_EIO;
        if ((r = put(b, s_blk)))
            return r;
        if (progress)
            progress("slot 1", b, n);
    }
    if (emmc_read(EMMC_FW_BASE + s2, BLK, s_blk))
        return FWU_EIO;
    return put(0, s_blk);
}

/* Deletes the image: its (contiguous) clusters freed in every FAT copy,
 * then the entry and its long-name entries in the same sector marked 0xE5.
 * ponytail: long-name entries spilling into the previous sector are left
 * for fsck. FSInfo free count is not touched (hosts treat it as a hint). */
int fw_update_retire(const fw_image_t *img)
{
    uint8_t *b = (uint8_t *)s_sec;
    uint32_t cl = s_fs.spc * 512u, first = 2u + (img->lba - s_fs.data) / s_fs.spc;
    uint32_t last = first + (img->size + cl - 1u) / cl - 1u;

    if (emmc_read(img->dir_lba, 1, s_sec))
        return FWU_EIO;
    if (memcmp(b + img->dir_off, img->raw, 11) != 0)
        return FWU_EFORMAT;
    for (uint32_t c = first; c <= last; ) {
        uint32_t sec = c / 128u, end = (sec + 1u) * 128u - 1u;

        if (end > last)
            end = last;
        for (uint32_t f = 0; f < s_fs.nfats; f++) {
            uint32_t lba = s_fs.fat + f * s_fs.fatsz + sec;

            if (emmc_read(lba, 1, s_fat))
                return FWU_EIO;
            for (uint32_t k = c; k <= end; k++)
                s_fat[k % 128u] &= 0xF0000000u;
            if (emmc_write(lba, 1, s_fat))
                return FWU_EWRITE;
        }
        c = end + 1u;
    }
    s_fat_lba = 0;
    b[img->dir_off] = 0xE5;
    for (int off = (int)img->dir_off - 32; off >= 0 && b[off + 11] == 0x0F && b[off] != 0xE5; off -= 32)
        b[off] = 0xE5;
    return emmc_write(img->dir_lba, 1, s_sec) ? FWU_EWRITE : 0;
}
