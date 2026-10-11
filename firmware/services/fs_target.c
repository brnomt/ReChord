/*
 * fs_target.c -- read-only FAT16 / FAT32 (long names) / exFAT backend for
 * fs.h on the device. Volumes are registered by fs_target_mount(); paths are
 * "/<volume>/dir/file", "/" lists the volumes. Writes stay unsupported.
 * Names are reported as UTF-8; matching is ASCII case-insensitive.
 * Written from the FAT / exFAT on-disk format, no third-party code.
 */
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "fs_target.h"

#define T_FAT16 16
#define T_FAT32 32
#define T_EXFAT 64

typedef struct {
    char name[12];
    fs_block_read_t rd;
    uint32_t base;                   /* volume boot sector, device LBA */
    int type;                        /* 0 = not mounted */
    uint32_t fat, data, spc, root, root_secs, nclus;
} vol_t;

typedef struct {                     /* a directory entry or the root */
    char name[FS_NAME_MAX];
    int is_dir, contig;
    uint32_t clus, size;
} ent_t;

typedef struct {                     /* directory cursor */
    const vol_t *v;
    uint32_t clus, sec, off, left;   /* left: clusters, contiguous dirs only */
    int contig;
} cur_t;

struct fs_file {
    const vol_t *v;
    uint32_t first, clus, cidx, size, pos;
    int contig, used;
};

static vol_t s_vol[FS_VOLUMES];
static struct fs_file s_files[4];
static uint32_t s_sec[128], s_sec_lba;
static const vol_t *s_sec_vol;
static ent_t s_ent;
static uint16_t s_u16[256];

static uint32_t le16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
static uint32_t le32(const uint8_t *p) { return le16(p) | le16(p + 2) << 16; }

/* ponytail: one cached sector for everything; multi-sector reads when the
 * audio path needs throughput. lba is relative to the volume. */
static const uint8_t *sector(const vol_t *v, uint32_t lba)
{
    if (v != s_sec_vol || lba != s_sec_lba) {
        s_sec_vol = NULL;
        if (v->rd(v->base + lba, 1, s_sec))
            return NULL;
        s_sec_vol = v;
        s_sec_lba = lba;
    }
    return (const uint8_t *)s_sec;
}

/* ---- mount ---- */

static int is_boot(const uint8_t *b)
{
    return (b[0] == 0xEB || b[0] == 0xE9) &&
           (memcmp(b + 3, "EXFAT   ", 8) == 0 || le16(b + 0x0B) == 512u);
}

int fs_target_mount(int idx, const char *name, fs_block_read_t rd, uint32_t base)
{
    vol_t *v;
    const uint8_t *b;

    if (idx < 0 || idx >= FS_VOLUMES || !rd)
        return FS_ERR_ARG;
    v = &s_vol[idx];
    memset(v, 0, sizeof *v);
    v->rd = rd;
    v->base = base;
    s_sec_vol = NULL;
    if (!(b = sector(v, 0)))
        return FS_ERR_IO;
    if (!is_boot(b)) {
        if (b[0x1FE] != 0x55 || b[0x1FF] != 0xAA || le32(b + 0x1C6) == 0u)
            return FS_ERR_UNSUPPORTED;
        v->base += le32(b + 0x1C6);          /* MBR partition 0 */
        s_sec_vol = NULL;
        if (!(b = sector(v, 0)))
            return FS_ERR_IO;
        if (!is_boot(b))
            return FS_ERR_UNSUPPORTED;
    }
    if (memcmp(b + 3, "EXFAT   ", 8) == 0) {
        if (b[0x6C] != 9u || b[0x6D] > 16u)  /* 512-byte sectors only */
            return FS_ERR_UNSUPPORTED;
        v->fat = le32(b + 0x50);
        v->data = le32(b + 0x58);
        v->nclus = le32(b + 0x5C);
        v->root = le32(b + 0x60);
        v->spc = 1u << b[0x6D];
        v->type = T_EXFAT;
    } else {
        uint32_t rsvd = le16(b + 0x0E), nf = b[0x10], fatsz, tot;

        if (b[0x0D] == 0 || nf == 0)
            return FS_ERR_UNSUPPORTED;
        fatsz = le16(b + 0x16) ? le16(b + 0x16) : le32(b + 0x24);
        tot = le16(b + 0x13) ? le16(b + 0x13) : le32(b + 0x20);
        v->spc = b[0x0D];
        v->fat = rsvd;
        v->root_secs = (le16(b + 0x11) * 32u + 511u) / 512u;
        v->data = rsvd + nf * fatsz + v->root_secs;
        if (tot <= v->data)
            return FS_ERR_UNSUPPORTED;
        v->nclus = (tot - v->data) / v->spc;
        if (v->nclus < 4085u)                /* FAT12 */
            return FS_ERR_UNSUPPORTED;
        v->type = v->nclus < 65525u ? T_FAT16 : T_FAT32;
        v->root = v->type == T_FAT32 ? le32(b + 0x2C) : rsvd + nf * fatsz;
    }
    snprintf(v->name, sizeof v->name, "%s", name);
    return 0;
}

void fs_target_unmount(int idx)
{
    if (idx >= 0 && idx < FS_VOLUMES) {
        for (unsigned i = 0; i < sizeof s_files / sizeof s_files[0]; i++)
            if (s_files[i].v == &s_vol[idx])
                s_files[i].used = 0;
        s_vol[idx].type = 0;
        s_sec_vol = NULL;
    }
}

const char *fs_target_type(int idx)
{
    if (idx < 0 || idx >= FS_VOLUMES)
        return NULL;
    switch (s_vol[idx].type) {
    case T_FAT16: return "FAT16";
    case T_FAT32: return "FAT32";
    case T_EXFAT: return "exFAT";
    default:      return NULL;
    }
}

/* ---- clusters ---- */

/* Next cluster in the chain, 0 at the end or on error. */
static uint32_t fat_next(const vol_t *v, uint32_t c)
{
    const uint8_t *b;

    if (v->type == T_FAT16) {
        if (!(b = sector(v, v->fat + c / 256u)))
            return 0;
        c = le16(b + c % 256u * 2u);
    } else {
        if (!(b = sector(v, v->fat + c / 128u)))
            return 0;
        c = le32(b + c % 128u * 4u);
        if (v->type == T_FAT32)
            c &= 0x0FFFFFFFu;
    }
    return c >= 2u && c < v->nclus + 2u ? c : 0u;
}

static uint32_t clus_lba(const vol_t *v, uint32_t c)
{
    return v->data + (c - 2u) * v->spc;
}

/* ---- directories ---- */

static void cur_init(cur_t *d, const vol_t *v, const ent_t *dir)
{
    uint32_t cl = v->spc * 512u;

    d->v = v;
    d->clus = dir->clus;
    d->sec = d->off = 0;
    d->contig = dir->contig;
    d->left = dir->contig ? (dir->size + cl - 1u) / cl : 0u;
}

static const uint8_t *cur_next(cur_t *d)
{
    const uint8_t *b;

    if (d->off == 512u) {
        d->off = 0;
        d->sec++;
    }
    if (d->clus == 0u) {                     /* FAT16 fixed root */
        if (d->sec >= d->v->root_secs)
            return NULL;
        b = sector(d->v, d->v->root + d->sec);
    } else {
        if (d->sec == d->v->spc) {
            d->sec = 0;
            if (d->contig) {
                if (d->left <= 1u)
                    return NULL;
                d->left--;
                d->clus++;
            } else if (!(d->clus = fat_next(d->v, d->clus))) {
                return NULL;
            }
        }
        b = sector(d->v, clus_lba(d->v, d->clus) + d->sec);
    }
    if (!b)
        return NULL;
    b += d->off;
    d->off += 32u;
    return b;
}

static void put_utf8(char *out, const uint16_t *u, unsigned n)
{
    char *end = out + FS_NAME_MAX - 4;

    for (unsigned i = 0; i < n && u[i] && out < end; i++) {
        uint32_t c = u[i];

        if (c >= 0xD800u && c < 0xE000u)     /* surrogate: not decoded */
            c = '?';
        if (c < 0x80u) {
            *out++ = (char)c;
        } else if (c < 0x800u) {
            *out++ = (char)(0xC0u | c >> 6);
            *out++ = (char)(0x80u | (c & 0x3Fu));
        } else {
            *out++ = (char)(0xE0u | c >> 12);
            *out++ = (char)(0x80u | (c >> 6 & 0x3Fu));
            *out++ = (char)(0x80u | (c & 0x3Fu));
        }
    }
    *out = '\0';
}

static int fat_dir_read(cur_t *d, ent_t *e)
{
    static const uint8_t k_lfn_off[13] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
    int next_ord = -1;                       /* -1: no long name pending */
    uint8_t sum = 0;
    const uint8_t *p;

    while ((p = cur_next(d)) && p[0] != 0) {
        if (p[0] == 0xE5) {
            next_ord = -1;
            continue;
        }
        if (p[11] == 0x0F) {
            unsigned ord = p[0] & 0x3Fu;

            if (p[0] & 0x40u) {
                memset(s_u16, 0, sizeof s_u16);
                next_ord = (int)ord;
                sum = p[13];
            }
            if (next_ord != (int)ord || p[13] != sum || ord == 0u || ord > 19u) {
                next_ord = -1;
                continue;
            }
            for (int i = 0; i < 13; i++)
                s_u16[(ord - 1u) * 13u + (unsigned)i] = (uint16_t)le16(p + k_lfn_off[i]);
            next_ord--;
            continue;
        }
        if ((p[11] & 0x08) || p[0] == '.') {  /* volume label, "." / ".." */
            next_ord = -1;
            continue;
        }
        if (next_ord == 0) {
            uint8_t s = 0;
            for (int i = 0; i < 11; i++)
                s = (uint8_t)(((s & 1u) << 7) + (s >> 1) + p[i]);
            if (s != sum)
                next_ord = -1;
        }
        if (next_ord == 0) {
            for (unsigned i = 0; i < 256u; i++)
                if (s_u16[i] == 0xFFFFu)
                    s_u16[i] = 0;
            put_utf8(e->name, s_u16, 256);
        } else {                             /* 8.3, NT lower-case flags */
            char *o = e->name;
            for (int i = 0; i < 8 && p[i] != ' '; i++)
                *o++ = (char)((p[12] & 0x08) && p[i] >= 'A' && p[i] <= 'Z' ? p[i] + 32 : p[i]);
            if (p[0] == 0x05)
                e->name[0] = (char)0xE5;
            if (p[8] != ' ')
                *o++ = '.';
            for (int i = 8; i < 11 && p[i] != ' '; i++)
                *o++ = (char)((p[12] & 0x10) && p[i] >= 'A' && p[i] <= 'Z' ? p[i] + 32 : p[i]);
            *o = '\0';
        }
        e->is_dir = (p[11] & 0x10) != 0;
        e->contig = 0;
        e->clus = (d->v->type == T_FAT32 ? le16(p + 20) << 16 : 0u) | le16(p + 26);
        e->size = le32(p + 28);
        if (e->is_dir && e->clus == 0u)      /* ".." to root: not reported */
            continue;
        return 1;
    }
    return 0;
}

static int exfat_dir_read(cur_t *d, ent_t *e)
{
    const uint8_t *p;

    while ((p = cur_next(d)) && p[0] != 0) {
        unsigned n2, len, got = 0;
        uint32_t attr;

        if (p[0] != 0x85 || p[1] < 2u)
            continue;
        n2 = p[1];
        attr = le16(p + 4);
        if (!(p = cur_next(d)) || p[0] != 0xC0)
            continue;
        len = p[3];
        e->contig = (p[1] & 2u) != 0;
        e->clus = le32(p + 20);
        e->size = le32(p + 28) ? 0xFFFFFFFFu : le32(p + 24);   /* > 4 GiB: clamp */
        memset(s_u16, 0, sizeof s_u16);
        for (unsigned k = 1; k < n2; k++) {
            if (!(p = cur_next(d)))
                return 0;
            if (p[0] != 0xC1)
                break;
            for (unsigned i = 0; i < 15u && got < len; i++)
                s_u16[got++] = (uint16_t)le16(p + 2 + 2 * i);
        }
        put_utf8(e->name, s_u16, got);
        e->is_dir = (attr & 0x10u) != 0;
        return 1;
    }
    return 0;
}

static int dir_read(cur_t *d, ent_t *e)
{
    return d->v->type == T_EXFAT ? exfat_dir_read(d, e) : fat_dir_read(d, e);
}

/* ---- paths ---- */

static int name_eq(const char *a, const char *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        char x = a[i], y = b[i];
        if (x >= 'a' && x <= 'z') x -= 32;
        if (y >= 'a' && y <= 'z') y -= 32;
        if (x != y || !y)
            return 0;
    }
    return b[n] == '\0';
}

static const char *component(const char *p, size_t *n)
{
    while (*p == '/' || *p == '\\')
        p++;
    *n = 0;
    while (p[*n] && p[*n] != '/' && p[*n] != '\\')
        (*n)++;
    return p;
}

/* Resolves path into s_ent / *pv. 1 = the virtual root "/", 0 = found,
 * FS_ERR_* otherwise. */
static int resolve(const char *path, const vol_t **pv)
{
    const vol_t *v = NULL;
    size_t n;
    const char *c;
    cur_t d;

    if (!path)
        return FS_ERR_ARG;
    c = component(path, &n);
    if (n == 0u)
        return 1;
    for (int i = 0; i < FS_VOLUMES; i++)
        if (s_vol[i].type && name_eq(c, s_vol[i].name, n))
            v = &s_vol[i];
    if (!v)
        return FS_ERR_OPEN;
    s_ent.is_dir = 1;
    s_ent.contig = 0;
    s_ent.size = 0;
    s_ent.clus = v->type == T_FAT16 ? 0u : v->root;
    for (c = component(c + n, &n); n; c = component(c + n, &n)) {
        int found = 0;

        if (!s_ent.is_dir)
            return FS_ERR_OPEN;
        cur_init(&d, v, &s_ent);
        while (dir_read(&d, &s_ent) == 1)
            if ((found = name_eq(c, s_ent.name, n)))
                break;
        if (!found)
            return FS_ERR_OPEN;
    }
    *pv = v;
    return 0;
}

/* ---- fs.h ---- */

int fs_list_dir(const char *path, uint32_t index, fs_dirent_t *out,
                uint32_t max, uint32_t *out_count, uint32_t *out_total)
{
    const vol_t *v;
    uint32_t total = 0, count = 0;
    int r = resolve(path, &v);
    cur_t d;

    if (out_count) *out_count = 0;
    if (out_total) *out_total = 0;
    if (r < 0)
        return r;
    if (r == 1) {
        for (int i = 0; i < FS_VOLUMES; i++) {
            if (!s_vol[i].type)
                continue;
            if (total >= index && count < max && out) {
                snprintf(out[count].name, FS_NAME_MAX, "%s", s_vol[i].name);
                out[count++].is_dir = 1;
            }
            total++;
        }
    } else {
        if (!s_ent.is_dir)
            return FS_ERR_OPEN;
        cur_init(&d, v, &s_ent);
        while (dir_read(&d, &s_ent) == 1) {
            if (total >= index && count < max && out) {
                memcpy(out[count].name, s_ent.name, FS_NAME_MAX);
                out[count++].is_dir = s_ent.is_dir;
            }
            total++;
        }
    }
    if (out_count) *out_count = count;
    if (out_total) *out_total = total;
    return 0;
}

int fs_list_dir_each(const char *path, fs_dirent_cb cb)
{
    static fs_dirent_t one;
    uint32_t n, total, i = 0;

    if (!cb)
        return FS_ERR_ARG;
    do {
        int r = fs_list_dir(path, i, &one, 1, &n, &total);
        if (r)
            return r;
        if (n && cb(one.name, one.is_dir))
            return (int)i + 1;
        i += n;
    } while (n);
    return (int)i;
}

fs_file_t *fs_open_read(const char *path)
{
    const vol_t *v;

    if (resolve(path, &v) != 0 || s_ent.is_dir)
        return NULL;
    for (unsigned i = 0; i < sizeof s_files / sizeof s_files[0]; i++) {
        struct fs_file *f = &s_files[i];

        if (f->used)
            continue;
        memset(f, 0, sizeof *f);
        f->used = 1;
        f->v = v;
        f->first = f->clus = s_ent.clus;
        f->size = s_ent.size;
        f->contig = s_ent.contig;
        return f;
    }
    return NULL;
}

long fs_read(fs_file_t *f, void *buf, unsigned long cap)
{
    uint8_t *o = buf;
    unsigned long n = 0;

    if (!f || !f->used || (!buf && cap))
        return FS_ERR_ARG;
    while (n < cap && f->pos < f->size) {
        uint32_t cl = f->v->spc * 512u, want = f->pos / cl, off = f->pos % 512u, k;
        const uint8_t *b;

        if (want < f->cidx) {
            f->clus = f->first;
            f->cidx = 0;
        }
        while (f->cidx < want) {
            f->clus = f->contig ? f->clus + 1u : fat_next(f->v, f->clus);
            f->cidx++;
            if (!f->clus)
                return FS_ERR_IO;
        }
        if (f->clus < 2u ||
            !(b = sector(f->v, clus_lba(f->v, f->clus) + f->pos % cl / 512u)))
            return FS_ERR_IO;
        k = 512u - off;
        if (k > cap - n)
            k = (uint32_t)(cap - n);
        if (k > f->size - f->pos)
            k = f->size - f->pos;
        memcpy(o + n, b + off, k);
        n += k;
        f->pos += k;
    }
    return (long)n;
}

long fs_file_size(fs_file_t *f)
{
    return f && f->used ? (long)f->size : FS_ERR_ARG;
}

void fs_close(fs_file_t *f)
{
    if (f)
        f->used = 0;
}

fs_file_t *fs_open_write(const char *path)
{
    (void)path;
    return NULL;
}

fs_file_t *fs_open_append(const char *path)
{
    (void)path;
    return NULL;
}

long fs_write(fs_file_t *f, const void *buf, unsigned long len)
{
    (void)f; (void)buf; (void)len;
    return FS_ERR_UNSUPPORTED;
}
