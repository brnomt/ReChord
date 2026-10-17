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
    fs_block_write_t wr;             /* NULL = read-only volume            */
    uint32_t base;                   /* volume boot sector, device LBA */
    int type;                        /* 0 = not mounted */
    uint32_t fat, fat_secs, data, spc, root, root_secs, nclus;
    uint32_t fsi;                    /* FAT32 FSInfo sector (0 = none)     */
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

/* Where the last dir_read() match keeps its 4-byte size field (the FAT short
 * entry +28) — the only field the in-place rewrite path patches. exFAT is
 * refused there: its size lives in the stream entry, covered by the
 * entry-set checksum. */
static uint32_t s_size_lba, s_size_off;

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
        v->fat_secs = fatsz;
        v->root_secs = (le16(b + 0x11) * 32u + 511u) / 512u;
        v->data = rsvd + nf * fatsz + v->root_secs;
        if (tot <= v->data)
            return FS_ERR_UNSUPPORTED;
        v->nclus = (tot - v->data) / v->spc;
        if (v->nclus < 4085u)                /* FAT12 */
            return FS_ERR_UNSUPPORTED;
        v->type = v->nclus < 65525u ? T_FAT16 : T_FAT32;
        v->root = v->type == T_FAT32 ? le32(b + 0x2C) : rsvd + nf * fatsz;
        if (v->type == T_FAT32)
            v->fsi = le16(b + 0x30) ? le16(b + 0x30) : 1u;
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
        s_size_lba = (d->clus == 0u) ? d->v->root + d->sec
                                     : clus_lba(d->v, d->clus) + d->sec;
        s_size_off = (d->off - 32u) + 28u;
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

        if (want < f->cidx || f->clus < 2u) {
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

long fs_seek(fs_file_t *f, long offset)
{
    if (f == NULL || !f->used || offset < 0)
        return FS_ERR_ARG;
    if (offset > (long)f->size)
        offset = (long)f->size;   /* legal; reads past end report EOF */
    f->pos = (uint32_t)offset;
    /* Reset the cluster walk to the file's FIRST cluster so the next fs_read
     * re-derives from (first, pos). Setting clus=0 here (old bug) made the
     * walk start at cluster 0 instead of first, so any read after a seek
     * pulled the wrong clusters (garbage) -> every FLAC failed with FORMAT
     * (its metadata walk seeks constantly). */
    f->cidx = 0;
    f->clus = f->first;
    return offset;
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

/* ---- creation (the allocation path) ------------------------------------
 *
 * The in-place paths above only touch data sectors of a file that already
 * exists. Creating a directory or a file needs allocation, and that means
 * writing FAT metadata: a free-cluster search, the chain in BOTH FAT copies,
 * and a directory entry. It stayed off while the project was bricking twice
 * -- which turned out to be the .bss ceiling at 0x0304A000, not this code
 * (DFIU fb41cb15; see fs_target.h).
 *
 * Ordering rule, same as the rewrite: data first, the directory entry LAST.
 * A crash or a power cut before the entry leaks clusters (fsck recovers
 * them) instead of leaving an entry pointing at nothing.
 *
 * FAT16/FAT32 only; exFAT is refused (its entry sets are checksummed).
 */

static void put_le32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t eoc(const vol_t *v)   /* end-of-chain marker for this FAT */
{
    return v->type == T_FAT16 ? 0xFFFFu : 0x0FFFFFFFu;
}

static int sec_write(const vol_t *v, uint32_t lba, const uint32_t *buf)
{
    if (v->wr == 0)
        return FS_ERR_UNSUPPORTED;
    if (v->wr(v->base + lba, 1u, buf) != 0)
        return FS_ERR_IO;
    s_sec_vol = 0;                   /* the cached sector is stale now */
    return FS_OK;
}

static int sec_zero(const vol_t *v, uint32_t lba)
{
    uint32_t sec[128];

    memset(sec, 0, sizeof sec);
    return sec_write(v, lba, sec);
}

/* One FAT entry into BOTH copies: a diverged mirror is silent corruption for
 * every other tool that touches this volume. */
static int fat_set(const vol_t *v, uint32_t c, uint32_t val)
{
    uint32_t sec[128];
    uint32_t lba, off;

    if (c < 2u || c >= v->nclus + 2u)
        return FS_ERR_ARG;
    if (v->type == T_FAT16) {
        uint8_t *b;

        lba = v->fat + c / 256u;
        off = (c % 256u) * 2u;
        if (v->rd(v->base + lba, 1u, sec) != 0)
            return FS_ERR_IO;
        b = (uint8_t *)sec;
        b[off] = (uint8_t)val;
        b[off + 1] = (uint8_t)(val >> 8);
    } else {
        lba = v->fat + c / 128u;
        off = (c % 128u) * 4u;
        if (v->rd(v->base + lba, 1u, sec) != 0)
            return FS_ERR_IO;
        sec[off / 4u] = (sec[off / 4u] & 0xF0000000u) | (val & 0x0FFFFFFFu);
    }
    if (sec_write(v, lba, sec) != FS_OK)
        return FS_ERR_IO;
    if (sec_write(v, lba + v->fat_secs, sec) != FS_OK)
        return FS_ERR_IO;
    return FS_OK;
}

/* Raw FAT entry value, WITHOUT fat_next()'s range check: that check reports
 * "end of chain" (0) for any value outside the cluster range, which in a
 * FAT16 volume is exactly an EOC marker (0xFFFF >= nclus + 2). For an
 * allocation scan that would read every allocated cluster as free and hand
 * out clusters that are already in use. Free is raw 0; on a read error report
 * "used" so a failure can never allocate something. */
static uint32_t fat_raw(const vol_t *v, uint32_t c)
{
    const uint8_t *b;

    if (v->type == T_FAT16) {
        if (!(b = sector(v, v->fat + c / 256u)))
            return 0xFFFFu;
        return le16(b + c % 256u * 2u);
    }
    if (!(b = sector(v, v->fat + c / 128u)))
        return 0x0FFFFFFFu;
    return le32(b + c % 128u * 4u) & 0x0FFFFFFFu;
}

/* Free cluster, or 0 when the volume is full. FAT32's FSInfo holds a hint
 * that Linux keeps valid; FAT16 has no FSInfo and a small FAT to scan. */
static uint32_t alloc_clus(const vol_t *v)
{
    uint32_t c, start = 2u;

    if (v->type == T_FAT32 && v->fsi != 0u &&
        v->rd(v->base + v->fsi, 1u, s_sec) == 0) {
        const uint8_t *b = (const uint8_t *)s_sec;
        uint32_t hint = le32(b + 0x1EC);

        s_sec_vol = 0;
        if (le32(b) == 0x41615252u && le32(b + 0x1E4) == 0x61417272u &&
            hint >= 2u && hint < v->nclus + 2u)
            start = hint;
    }
    for (c = start; c < v->nclus + 2u; c++)
        if (fat_raw(v, c) == 0u)
            return c;
    for (c = 2u; c < start; c++)
        if (fat_raw(v, c) == 0u)
            return c;
    return 0u;
}

/* FSInfo bookkeeping after taking `n` clusters from `first`. The free count
 * is set to "unknown" (0xFFFFFFFF, always valid) rather than decremented:
 * this code cannot know the true count without a full FAT scan, and a count
 * that drifts from reality is exactly what fsck reports ("Free cluster
 * summary wrong"). The allocation hint is still moved past what we handed
 * out, which is what keeps the next allocation fast. */
static void fsi_note(const vol_t *v, uint32_t first, uint32_t n)
{
    uint32_t sec[128];
    uint8_t *b = (uint8_t *)sec;

    if (v->type != T_FAT32 || v->fsi == 0u || n == 0u)
        return;
    if (v->rd(v->base + v->fsi, 1u, sec) != 0)
        return;
    if (le32(b) != 0x41615252u || le32(b + 0x1E4) != 0x61417272u)
        return;
    put_le32(b + 0x1E8, 0xFFFFFFFFu);
    put_le32(b + 0x1EC, first + n);
    (void)sec_write(v, v->fsi, sec);
}

/* Directory part of `path` resolved into s_ent, last component into `name`.
 * Rejects anything that is not "volume/dir.../name". */
static int resolve_parent(const char *path, const vol_t **pv, char *name,
                          size_t cap, size_t *nlen)
{
    const char *cur, *last = NULL;
    size_t n, ln = 0, cut;
    char dir[256];
    int r;

    if (path == NULL || cap == 0u)
        return FS_ERR_ARG;
    cur = path;
    while ((cur = component(cur, &n)) != NULL && n != 0u) {
        last = cur;
        ln = n;
        cur += n;
    }
    if (last == NULL || ln + 1u > cap)
        return FS_ERR_ARG;
    for (size_t i = 0; i < ln; i++)
        if ((unsigned char)last[i] >= 0x80u)
            return FS_ERR_UNSUPPORTED;  /* LFN writer is ASCII-only */
    memcpy(name, last, ln);
    name[ln] = '\0';
    *nlen = ln;
    cut = (size_t)(last - path);
    if (cut == 0u || cut >= sizeof dir)
        return FS_ERR_ARG;
    while (cut > 0u && path[cut - 1u] == '/')
        cut--;
    if (cut == 0u || cut >= sizeof dir)
        return FS_ERR_ARG;
    memcpy(dir, path, cut);
    dir[cut] = '\0';
    r = resolve(dir, pv);
    if (r != 0)
        return FS_ERR_OPEN;
    return 0;
}

/* A free slot in `dir` where `need` consecutive 32-byte entries fit,
 * extending the directory by a cluster when it is full (chained directories
 * only: the FAT16 root is fixed). Returns the first slot's volume-relative
 * LBA and byte offset, not `need` slots of coordinates. */
static int dir_slots(const vol_t *v, const ent_t *dir, unsigned need,
                     uint32_t *lba, uint32_t *off)
{
    cur_t d;
    const uint8_t *p;
    unsigned run = 0;
    uint32_t run_lba = 0, run_off = 0;

    cur_init(&d, v, dir);
    while ((p = cur_next(&d)) != NULL) {
        if (p[0] == 0x00 || p[0] == 0xE5) {
            uint32_t l = d.clus == 0u ? v->root + d.sec
                                      : clus_lba(v, d.clus) + d.sec;

            if (run == 0u) {
                run_lba = l;
                run_off = d.off - 32u;
            }
            if (++run >= need) {
                *lba = run_lba;
                *off = run_off;
                return FS_OK;
            }
            continue;
        }
        run = 0;
    }
    if (dir->clus < 2u || dir->contig)
        return FS_ERR_UNSUPPORTED;       /* cannot grow this directory */
    {
        uint32_t last = dir->clus, next = fat_next(v, last), fresh, s;

        while (next >= 2u) {
            last = next;
            next = fat_next(v, last);
        }
        fresh = alloc_clus(v);
        if (fresh < 2u)
            return FS_ERR_IO;            /* volume full */
        if (fat_set(v, fresh, eoc(v)) != FS_OK)
            return FS_ERR_IO;
        for (s = 0; s < v->spc; s++)
            if (sec_zero(v, clus_lba(v, fresh) + s) != FS_OK)
                return FS_ERR_IO;
        if (fat_set(v, last, fresh) != FS_OK)
            return FS_ERR_IO;
        fsi_note(v, fresh, 1u);
        *lba = clus_lba(v, fresh);
        *off = 0u;
        return FS_OK;
    }
}

static int slot_write(const vol_t *v, uint32_t lba, uint32_t off,
                      const uint8_t *ent)
{
    uint32_t sec[128];

    if (v->rd(v->base + lba, 1u, sec) != 0)
        return FS_ERR_IO;
    memcpy((uint8_t *)sec + off, ent, 32u);
    return sec_write(v, lba, sec);
}

static void short_entry(uint8_t *e, const char *base, const char *ext,
                        uint8_t attr, int lower_base, int lower_ext,
                        uint32_t clus, uint32_t size)
{
    memset(e, 0, 32);
    memset(e, ' ', 11);
    memcpy(e, base, strlen(base) > 8u ? 8u : strlen(base));
    memcpy(e + 8, ext, strlen(ext) > 3u ? 3u : strlen(ext));
    e[11] = attr;
    /* NT lower-case flags live in byte 12: bit 3 = base, bit 4 = extension
     * (byte 13 is the created-time tenths, NOT a flag byte). */
    e[12] = (uint8_t)((lower_base ? 0x08u : 0u) | (lower_ext ? 0x10u : 0u));
    e[13] = 0;
    /* Valid DOS timestamp (2024-01-01 00:00): the fields are cosmetic here,
     * but a month/day of 0 makes some tools complain. */
    e[14] = 0; e[15] = 0; e[16] = 0x21; e[17] = 0x58;
    e[22] = 0; e[23] = 0; e[24] = 0x21; e[25] = 0x58;
    e[20] = (uint8_t)(clus >> 16);
    e[21] = (uint8_t)(clus >> 24);
    e[26] = (uint8_t)clus;
    e[27] = (uint8_t)(clus >> 8);
    put_le32(e + 28, size);
}

/* The 8.3 dot: the reader's inverse. Names that fit 8.3 (base <= 8, ext <= 3,
 * safe characters) are stored as a short entry with the NT lower-case flags,
 * so the displayed name keeps its exact case; anything else gets a long-name
 * set plus a generated "BASE~1" short name. */
static int dirent_add(const vol_t *v, const ent_t *dir, const char *name,
                      int is_dir, uint32_t clus, uint32_t size)
{
    static const uint8_t k_lfn_off[13] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
    uint8_t sh[32], lfn[32];
    char base[9], ext[4];
    const char *dot = NULL;
    size_t nlen = strlen(name), blen, elen, units, i;
    uint16_t u[13];
    uint32_t sum = 0, lba, off;
    int fits = 1, lower_base = 0, lower_ext = 0, need_lfn;
    unsigned need;
    int r;

    for (i = 0; i < nlen; i++) {
        char ch = name[i];

        if (ch == '.') {
            if (i == 0u || dot != NULL)
                fits = 0;                /* leading / second dot */
            else
                dot = name + i;
            continue;
        }
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
              (ch >= '0' && ch <= '9') || ch == '_' || ch == '-' || ch == '$' ||
              ch == '~' || ch == '#' || ch == '%' || ch == '&' || ch == '\'' ||
              ch == '(' || ch == ')' || ch == '@' || ch == '^' || ch == '!' ||
              ch == '+' || ch == ';' || ch == '=' || ch == '[' || ch == ']' ||
              ch == '{' || ch == '}' || ch == ',' || ch == '`'))
            fits = 0;
    }
    blen = dot ? (size_t)(dot - name) : nlen;
    elen = dot ? nlen - blen - 1u : 0u;
    if (blen == 0u || blen > 8u || elen > 3u)
        fits = 0;
    need_lfn = !fits;

    if (!need_lfn) {
        for (i = 0; i < blen; i++) {
            if (name[i] >= 'a' && name[i] <= 'z')
                lower_base = 1;
            base[i] = (char)(name[i] >= 'a' && name[i] <= 'z'
                                 ? name[i] - 32 : name[i]);
        }
        base[blen] = '\0';
        for (i = 0; i < elen; i++) {
            if (dot[1 + i] >= 'a' && dot[1 + i] <= 'z')
                lower_ext = 1;
            ext[i] = (char)(dot[1 + i] >= 'a' && dot[1 + i] <= 'z'
                                ? dot[1 + i] - 32 : dot[1 + i]);
        }
        ext[elen] = '\0';
        need = 1u;
    } else {
        /* Short name: strip dots, uppercase, first 8 chars, "~1". */
        size_t k = 0;

        for (i = 0; i < nlen && k < 6u; i++) {
            char ch = name[i];

            if (ch == '.')
                continue;
            if (ch >= 'a' && ch <= 'z')
                ch = (char)(ch - 32);
            base[k++] = ch;
        }
        if (k == 0u)
            base[k++] = 'N';
        base[k++] = '~';
        base[k++] = '1';
        base[k] = '\0';
        if (blen > 8u)
            blen = 8u;
        ext[0] = '\0';
        elen = 0u;
        units = (nlen + 12u) / 13u;
        if (units == 0u || units > 19u)
            return FS_ERR_UNSUPPORTED;
        need = (unsigned)units + 1u;
    }

    r = dir_slots(v, dir, need, &lba, &off);
    if (r != FS_OK)
        return r;

    if (need_lfn) {
        /* The long-name checksum is computed over the short entry's 11 NAME
         * bytes, truncated to 8 bits at every step (a wider accumulator gives
         * a different low byte and the reader then rejects the whole set). */
        short_entry(sh, base, ext, (uint8_t)((is_dir ? 0x10u : 0u) |
                                             (name[0] == '.' ? 0x02u : 0u)),
                    0, 0, clus, size);
        for (i = 0; i < 11u; i++)
            sum = (uint8_t)(((sum & 1u) << 7) + (sum >> 1) + sh[i]);
        /* Entries are stored highest-ordinal-first, 0x40 on the first one. */
        for (i = 0; i < units; i++) {
            unsigned ord = (unsigned)(units - i);
            unsigned k;

            memset(lfn, 0xFF, 32);
            memset(u, 0xFFFF, sizeof u);
            for (k = 0; k < 13u; k++) {
                size_t idx = (ord - 1u) * 13u + k;

                if (idx < nlen)
                    u[k] = (uint16_t)(unsigned char)name[idx];
                else if (idx == nlen)
                    u[k] = 0;        /* the name is NUL-terminated, then
                                  * 0xFFFF padding (fsck and Windows both
                                  * expect the terminator, not raw padding) */
            }
            lfn[0] = (uint8_t)(ord | (i == 0u ? 0x40u : 0u));
            lfn[11] = 0x0F;
            lfn[12] = 0;                     /* long-entry type: always 0 */
            lfn[13] = (uint8_t)sum;
            lfn[26] = 0;                     /* first cluster: always 0 */
            lfn[27] = 0;
            for (k = 0; k < 13u; k++) {
                lfn[k_lfn_off[k]] = (uint8_t)u[k];
                lfn[k_lfn_off[k] + 1] = (uint8_t)(u[k] >> 8);
            }
            r = slot_write(v, lba, off + (uint32_t)i * 32u, lfn);
            if (r != FS_OK)
                return r;
        }
        return slot_write(v, lba, off + (uint32_t)units * 32u, sh);
    }

    short_entry(sh, base, ext, (uint8_t)(is_dir ? 0x10u : 0u),
                lower_base, lower_ext, clus, size);
    return slot_write(v, lba, off, sh);
}

/* Find `name` in `dir` (case-insensitive, as the reader sees it). Returns 1
 * and fills *out when it is there, 0 otherwise. */
static int dir_find(const vol_t *v, const ent_t *dir, const char *name,
                    size_t n, ent_t *out)
{
    cur_t d;
    ent_t e;

    cur_init(&d, v, dir);
    while (dir_read(&d, &e) == 1)
        if (name_eq(name, e.name, n)) {
            *out = e;
            return 1;
        }
    return 0;
}

int fs_target_mkdir(const char *path)
{
    const vol_t *v;
    ent_t parent;
    char name[FS_NAME_MAX];
    size_t nlen = 0;
    uint32_t clus, ssec;
    uint8_t e[32];
    int r;

    r = resolve_parent(path, &v, name, sizeof name, &nlen);
    if (r != 0)
        return r;
    if (!s_ent.is_dir)
        return FS_ERR_OPEN;
    if (v->type == T_EXFAT || v->wr == 0)
        return FS_ERR_UNSUPPORTED;
    parent = s_ent;                      /* dir_read() overwrites s_ent */
    if (dir_find(v, &parent, name, nlen, &s_ent))
        return s_ent.is_dir ? FS_OK : FS_ERR_OPEN;  /* exists: idempotent */
    if (name[0] == '.')                  /* no "." / ".." aliases */
        if (nlen == 1u || (nlen == 2u && name[1] == '.'))
            return FS_ERR_ARG;

    clus = alloc_clus(v);
    if (clus < 2u)
        return FS_ERR_IO;                /* volume full */
    if (fat_set(v, clus, eoc(v)) != FS_OK)
        return FS_ERR_IO;
    for (ssec = 0; ssec < v->spc; ssec++)
        if (sec_zero(v, clus_lba(v, clus) + ssec) != FS_OK)
            return FS_ERR_IO;
    /* "." points at the new directory; ".." points at the parent, and 0 when
     * that parent is the volume root (fsck: "Invalid '..' entry in the second
     * slot" otherwise on FAT32, whose root has a real cluster number). */
    short_entry(e, ".", "", 0x10u, 0, 0, clus, 0);
    if (slot_write(v, clus_lba(v, clus), 0u, e) != FS_OK)
        return FS_ERR_IO;
    {
        uint32_t up = (parent.clus == 0u || parent.clus == v->root)
                          ? 0u : parent.clus;

        short_entry(e, "..", "", 0x10u, 0, 0, up, 0);
    }
    if (slot_write(v, clus_lba(v, clus), 32u, e) != FS_OK)
        return FS_ERR_IO;

    fsi_note(v, clus, 1u);
    return dirent_add(v, &parent, name, 1, clus, 0);   /* commit */
}

int fs_target_file_create(const char *path, uint32_t bytes)
{
    const vol_t *v;
    ent_t parent;
    char name[FS_NAME_MAX];
    size_t nlen = 0;
    uint32_t first = 0, prev = 0, need, i, ssec;
    int r;

    r = resolve_parent(path, &v, name, sizeof name, &nlen);
    if (r != 0)
        return r;
    if (!s_ent.is_dir)
        return FS_ERR_OPEN;
    if (v->type == T_EXFAT || v->wr == 0)
        return FS_ERR_UNSUPPORTED;
    parent = s_ent;
    if (dir_find(v, &parent, name, nlen, &s_ent))
        return s_ent.is_dir ? FS_ERR_OPEN : FS_OK;  /* exists: idempotent */

    need = bytes == 0u ? 0u
                       : (bytes + v->spc * 512u - 1u) / (v->spc * 512u);
    for (i = 0; i < need; i++) {
        uint32_t c = alloc_clus(v);

        if (c < 2u)
            return FS_ERR_IO;            /* volume full (nothing committed) */
        if (fat_set(v, c, eoc(v)) != FS_OK)
            return FS_ERR_IO;
        if (prev >= 2u) {
            if (fat_set(v, prev, c) != FS_OK)
                return FS_ERR_IO;
        } else {
            first = c;
        }
        prev = c;
        for (ssec = 0; ssec < v->spc; ssec++)
            if (sec_zero(v, clus_lba(v, c) + ssec) != FS_OK)
                return FS_ERR_IO;
    }
    fsi_note(v, first, need);
    return dirent_add(v, &parent, name, 0, first, bytes);   /* commit */
}



int fs_target_set_writer(int idx, fs_block_write_t wr)
{
    if (idx < 0 || idx >= FS_VOLUMES)
        return FS_ERR_ARG;
    s_vol[idx].wr = wr;
    return FS_OK;
}

int fs_target_file_rewrite(const char *path, const void *data, uint32_t len)
{
    const vol_t *v;
    uint32_t sec[128];
    uint32_t c, sectors, done, i;
    int r;

    if (path == 0 || (data == 0 && len > 0))
        return FS_ERR_ARG;
    r = resolve(path, &v);
    if (r != 0 || s_ent.is_dir)
        return FS_ERR_OPEN;
    /* exFAT's file size lives in the stream entry, which is covered by the
     * entry-set checksum (SetChecksum in the 0x85 entry): patching the size
     * without recomputing that checksum corrupts the directory (fsck:
     * "checksum of a file is wrong"). The internal volume is FAT; refuse
     * exFAT here instead of corrupting it. */
    if (v->type == T_EXFAT)
        return FS_ERR_UNSUPPORTED;
    if (v->wr == 0)
        return FS_ERR_UNSUPPORTED;       /* read-only volume                */
    if (len > 0 && s_ent.clus == 0u)
        return FS_ERR_UNSUPPORTED;       /* no clusters owned: cannot grow  */

    /* Data sectors first (zero-padded), walking the file's own clusters. */
    sectors = (len + 511u) / 512u;

    /* Pre-flight: the file must ALREADY own enough clusters. A rejected
     * oversize rewrite must not touch the data at all. The cluster size
     * differs per volume, and exFAT's NoFatChain files have no chain to
     * probe — their run length follows the file size — so the two cases are
     * computed differently. */
    {
        uint32_t alloc_sectors;

        if (s_ent.contig) {
            uint32_t clus_bytes = v->spc * 512u;

            alloc_sectors = ((s_ent.size + clus_bytes - 1u) / clus_bytes) *
                            v->spc;
        } else {
            uint32_t have = 0, probe = s_ent.clus;

            while (have < sectors) {
                have += v->spc;
                if (have >= sectors)
                    break;
                probe = fat_next(v, probe);
                if (probe < 2u)
                    return FS_ERR_UNSUPPORTED;  /* allocation too small     */
            }
            alloc_sectors = have;
        }
        if (sectors > alloc_sectors)
            return FS_ERR_UNSUPPORTED;
    }

    c = s_ent.clus;
    done = 0;
    for (i = 0; i < sectors; i++) {
        uint32_t chunk = len - done;

        if (i > 0u && (i % v->spc) == 0u) {         /* next cluster */
            c = s_ent.contig ? c + 1u : fat_next(v, c);
            if (c < 2u)
                return FS_ERR_UNSUPPORTED;          /* shorter than `len`  */
        }
        if (chunk > 512u)
            chunk = 512u;
        memset(sec, 0, sizeof sec);
        memcpy(sec, (const uint8_t *)data + done, chunk);
        /* clus_lba() is volume-relative; the block I/O contract is absolute
         * device LBAs (the volume may sit behind an MBR / at s_user). */
        if (v->wr(v->base + clus_lba(v, c) + i % v->spc, 1u, sec) != 0)
            return FS_ERR_IO;
        done += chunk;
    }

    /* Commit: patch the 4-byte size field in the dirent. This is the only
     * FS metadata byte this path touches (same narrow pattern the updater's
     * retire step proves); the FAT table and the directory layout are
     * untouched and nothing is allocated. */
    if (v->rd(v->base + s_size_lba, 1u, sec) != 0)
        return FS_ERR_IO;
    sec[s_size_off / 4u] = len;
    if (v->wr(v->base + s_size_lba, 1u, sec) != 0)
        return FS_ERR_IO;
    s_sec_vol = 0;                       /* cached sectors are stale now   */
    return FS_OK;
}
