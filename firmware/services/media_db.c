/*
 * media_db.c — the media index (see media_db.h for the format and the
 * crash-safety story).
 *
 * The build buffers records in the shared scratch region and appends them in
 * chunks, so RAM is bounded no matter how big the library is. The header is
 * patched in LAST with fs_patch(): that single-sector write is the commit.
 */
#include <string.h>

#include "fs.h"
#include "media_db.h"
#include "scratch.h"

#define MEDIA_CHUNK_RECS (SCRATCH_BYTES / MEDIA_REC_BYTES)

static int      s_build_slot;         /* 0 = A, 1 = B (target of the build) */
static uint32_t s_build_count;
static uint32_t s_build_crc;
static uint32_t s_build_buf;          /* records buffered in the scratch */
static uint8_t *s_buf;

static fs_file_t *s_rd;               /* open slot for media_db_get() */
static uint32_t   s_rd_count;

/* CRC-32 (the same polynomial the store uses), bitwise on purpose: it runs on
 * a few hundred KB once per scan, not in any hot path. */
static uint32_t crc32(const void *data, uint32_t len, uint32_t crc)
{
    const uint8_t *p = (const uint8_t *)data;

    while (len--) {
        crc ^= *p++;
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)-(int32_t)(crc & 1u));
    }
    return crc;
}

const char *media_db_slot_path(int which)
{
    return which ? MEDIA_DB_B : MEDIA_DB_A;
}

/* Read the header of one slot. Returns 1 when it is a valid, committed index
 * whose records hash to its CRC. */
static int read_hdr(int which, media_hdr_t *h)
{
    fs_file_t *f = fs_open_read(media_db_slot_path(which));
    long n;

    if (f == 0)
        return 0;
    n = fs_read(f, h, sizeof *h);
    fs_close(f);
    if (n != (long)sizeof *h)
        return 0;
    if (h->magic != MEDIA_MAGIC || h->version != MEDIA_VERSION ||
        h->rec_bytes != MEDIA_REC_BYTES || h->seq == 0u)
        return 0;
    if (h->count > (0xFFFFFFFFu - MEDIA_HDR_BYTES) / MEDIA_REC_BYTES)
        return 0;
    return 1;
}

/* Verify a header's records against its CRC (one full pass). */
static int verify_hdr(int which, const media_hdr_t *h)
{
    uint8_t buf[512];
    fs_file_t *f = fs_open_read(media_db_slot_path(which));
    uint32_t left = h->count * MEDIA_REC_BYTES, crc = 0;
    long n;

    if (f == 0)
        return 0;
    if (fs_seek(f, (long)MEDIA_HDR_BYTES) < 0) {
        fs_close(f);
        return 0;
    }
    while (left) {
        unsigned want = left < sizeof buf ? left : (unsigned)sizeof buf;

        n = fs_read(f, buf, want);
        if (n <= 0 || (unsigned)n > want)
            break;
        crc = crc32(buf, (uint32_t)n, crc);
        left -= (uint32_t)n;
    }
    fs_close(f);
    return left == 0u && crc == h->crc;
}

int media_db_begin(void)
{
    media_hdr_t a, b;
    int have_a = read_hdr(0, &a), have_b = read_hdr(1, &b);
    media_hdr_t *cur = 0;

    /* Target the slot that is not current (an invalid header is never
     * current): rebuilding must never touch the index in use. */
    if (have_a && (!have_b || a.seq >= b.seq)) {
        s_build_slot = 1;
        cur = &a;
    } else {
        s_build_slot = 0;
        cur = have_b ? &b : 0;
    }
    if (cur != 0 && !verify_hdr(s_build_slot == 0 ? 1 : 0, cur))
        cur = 0;                       /* current slot is corrupt: no seq */

    if (fs_truncate(media_db_slot_path(s_build_slot)) != FS_OK)
        return FS_ERR_IO;
    /* Reserve the header sector; media_db_end() patches it in. */
    {
        static const uint8_t zero[MEDIA_HDR_BYTES] = { 0 };

        if (fs_append(media_db_slot_path(s_build_slot), zero,
                      MEDIA_HDR_BYTES) != FS_OK)
            return FS_ERR_IO;
    }
    s_build_count = 0;
    s_build_crc = 0;
    s_build_buf = 0;
    s_buf = scratch_claim(SCRATCH_OWNER_INDEX);
    if (s_buf == 0)
        return FS_ERR_IO;
    return FS_OK;
}

static int flush_chunk(void)
{
    uint32_t bytes = s_build_buf * MEDIA_REC_BYTES;

    if (bytes == 0u)
        return FS_OK;
    if (fs_append(media_db_slot_path(s_build_slot), s_buf, bytes) != FS_OK)
        return FS_ERR_IO;
    s_build_crc = crc32(s_buf, bytes, s_build_crc);
    s_build_buf = 0;
    return FS_OK;
}

int media_db_add(const media_rec_t *r)
{
    if (r == 0 || s_buf == 0)
        return FS_ERR_ARG;
    if (s_build_buf == MEDIA_CHUNK_RECS && flush_chunk() != FS_OK)
        return FS_ERR_IO;
    memcpy(s_buf + s_build_buf * MEDIA_REC_BYTES, r, MEDIA_REC_BYTES);
    s_build_buf++;
    s_build_count++;
    return FS_OK;
}

int media_db_end(void)
{
    media_hdr_t h;
    uint32_t seq = 0;
    media_hdr_t old;

    if (s_buf == 0)
        return FS_ERR_ARG;
    if (flush_chunk() != FS_OK)
        return FS_ERR_IO;
    scratch_release(SCRATCH_OWNER_INDEX);
    s_buf = 0;

    /* Sequence: one past the newest committed index, whichever slot it is. */
    if (read_hdr(0, &old) && old.seq > seq)
        seq = old.seq;
    if (read_hdr(1, &old) && old.seq > seq)
        seq = old.seq;

    memset(&h, 0, sizeof h);
    h.magic = MEDIA_MAGIC;
    h.version = MEDIA_VERSION;
    h.seq = seq + 1u;
    h.count = s_build_count;
    h.rec_bytes = MEDIA_REC_BYTES;
    h.crc = s_build_crc;
    /* Commit: one sector at offset 0, the file's size is left alone. */
    if (fs_patch(media_db_slot_path(s_build_slot), 0u, &h, sizeof h) != FS_OK)
        return FS_ERR_IO;
    return FS_OK;
}

int media_db_open(void)
{
    media_hdr_t a, b;
    int which;

    media_db_close();
    if (read_hdr(0, &a) && read_hdr(1, &b))
        which = a.seq >= b.seq ? 0 : 1;
    else if (read_hdr(0, &a))
        which = 0;
    else if (read_hdr(1, &b))
        which = 1;
    else
        return FS_ERR_OPEN;

    {
        media_hdr_t h;

        if (!read_hdr(which, &h) || !verify_hdr(which, &h))
            return FS_ERR_OPEN;
        s_rd = fs_open_read(media_db_slot_path(which));
        if (s_rd == 0)
            return FS_ERR_OPEN;
        s_rd_count = h.count;
    }
    return FS_OK;
}

uint32_t media_db_count(void)
{
    return s_rd ? s_rd_count : 0u;
}

int media_db_get(uint32_t i, media_rec_t *out)
{
    long n;

    if (s_rd == 0 || out == 0 || i >= s_rd_count)
        return FS_ERR_ARG;
    if (fs_seek(s_rd, (long)(MEDIA_HDR_BYTES + i * MEDIA_REC_BYTES)) < 0)
        return FS_ERR_IO;
    n = fs_read(s_rd, out, MEDIA_REC_BYTES);
    return n == (long)MEDIA_REC_BYTES ? FS_OK : FS_ERR_IO;
}

void media_db_close(void)
{
    if (s_rd != 0) {
        fs_close(s_rd);
        s_rd = 0;
    }
    s_rd_count = 0;
}
