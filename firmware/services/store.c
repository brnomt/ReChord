/*
 * store.c — crash-safe record store over raw sectors (see store.h).
 *
 * Freestanding-friendly: no allocation, one 512-byte sector of stack scratch
 * per call path (4-byte aligned, the block contract), and no static state —
 * so this module costs no .bss.
 *
 * Record layout per slot (little-endian u32 words):
 *   [0] magic 'RCHS'  [1] version:16 | header_bytes:16  [2] seq
 *   [3] payload len   [4] payload CRC  [5] header CRC (words 0..4)
 *   [6][7] reserved (0)
 * followed by the payload, zero-padded to a sector.
 */
#include <string.h>

#include "store.h"

#define STORE_IO_BYTES   512u
#define STORE_HDR_WORDS  (STORE_HDR_BYTES / 4u)
#define STORE_HDR_CRC_N  20u          /* bytes covered by the header CRC    */

enum {
    H_MAGIC = 0,
    H_VERSION = 1,
    H_SEQ = 2,
    H_LEN = 3,
    H_PAYCRC = 4,
    H_HDRCRC = 5,
    H_RSV0 = 6,
    H_RSV1 = 7
};

/* CRC-32, poly 0x04C10DB7, MSB-first, init 0, xorout 0 — the platform's
 * image-trailer primitive (fw_update.c). Bitwise on purpose: a 1 KiB table
 * would cost .bss, and a record is only a few KB. */
static uint32_t crc32_update(uint32_t crc, const uint8_t *p, uint32_t n)
{
    while (n--) {
        uint32_t i;

        crc ^= (uint32_t)*p++ << 24;
        for (i = 0; i < 8u; i++)
            crc = (crc & 0x80000000u) ? (crc << 1) ^ 0x04C10DB7u : crc << 1;
    }
    return crc;
}

static uint32_t slot_lba(const store_t *s, int slot)
{
    return s->lba + (uint32_t)slot * (uint32_t)s->slot_sectors;
}

static uint32_t payload_sectors(uint32_t len)
{
    return (len + STORE_IO_BYTES - 1u) / STORE_IO_BYTES;
}

/* Read one slot's header. Returns 1 when it is structurally valid (magic,
 * version, header size, CRCs, length), 0 otherwise; *seq_out gets the
 * sequence number when non-NULL. */
static int slot_header(store_t *s, int slot, uint32_t *seq_out)
{
    uint32_t sec[STORE_IO_BYTES / 4u];
    uint32_t *h = sec;

    if (s->io->read(s->io->user, slot_lba(s, slot), 1u, sec) != 0)
        return 0;
    if (h[H_MAGIC] != STORE_MAGIC)
        return 0;
    if ((h[H_VERSION] & 0xFFFFu) != STORE_VERSION ||
        (h[H_VERSION] >> 16) != STORE_HDR_BYTES)
        return 0;
    if (crc32_update(0, (const uint8_t *)h, STORE_HDR_CRC_N) != h[H_HDRCRC])
        return 0;
    if (h[H_LEN] > store_capacity(s))
        return 0;
    if (seq_out != 0)
        *seq_out = h[H_SEQ];
    return 1;
}

/* Newest-first comparison, wrap-safe. */
static int seq_newer(uint32_t a, uint32_t b)
{
    return (int32_t)(a - b) > 0;
}

/* Fully load one slot (header + payload + payload CRC). */
static int slot_load(store_t *s, int slot, void *buf, uint32_t cap,
                     uint32_t *out_len)
{
    uint32_t sec[STORE_IO_BYTES / 4u];
    uint32_t hdr[STORE_HDR_WORDS];
    uint32_t len, paycrc, off, i, nsec;

    if (s->io->read(s->io->user, slot_lba(s, slot), 1u, sec) != 0)
        return STORE_ERR;
    memcpy(hdr, sec, STORE_HDR_BYTES);      /* the payload loop reuses `sec` */
    if (hdr[H_MAGIC] != STORE_MAGIC)
        return STORE_ERR_EMPTY;
    if ((hdr[H_VERSION] & 0xFFFFu) != STORE_VERSION ||
        (hdr[H_VERSION] >> 16) != STORE_HDR_BYTES)
        return STORE_ERR_EMPTY;
    if (crc32_update(0, (const uint8_t *)hdr, STORE_HDR_CRC_N) !=
        hdr[H_HDRCRC])
        return STORE_ERR_EMPTY;
    len = hdr[H_LEN];
    paycrc = hdr[H_PAYCRC];
    if (len > store_capacity(s))
        return STORE_ERR_EMPTY;
    if (len > cap)
        return STORE_ERR_BIG;

    nsec = payload_sectors(len);
    for (i = 0, off = 0; i < nsec; i++) {
        uint32_t chunk = len - off;

        if (chunk > STORE_IO_BYTES)
            chunk = STORE_IO_BYTES;
        if (s->io->read(s->io->user, slot_lba(s, slot) + 1u + i, 1u, sec) != 0)
            return STORE_ERR;
        memcpy((uint8_t *)buf + off, sec, chunk);
        off += chunk;
    }
    if (crc32_update(0, (const uint8_t *)buf, len) != paycrc)
        return STORE_ERR_EMPTY;
    if (out_len != 0)
        *out_len = len;
    return STORE_OK;
}

void store_init(store_t *s, const store_io_t *io, uint32_t lba,
                uint16_t slot_sectors)
{
    if (s == 0)
        return;
    s->io = io;
    s->lba = lba;
    s->slot_sectors = slot_sectors;
}

uint32_t store_capacity(const store_t *s)
{
    if (s == 0 || s->io == 0 || s->slot_sectors == 0)
        return 0;
    return (uint32_t)s->slot_sectors * STORE_IO_BYTES - STORE_HDR_BYTES;
}

int store_load(store_t *s, void *buf, uint32_t cap, uint32_t *out_len)
{
    uint32_t seq0 = 0, seq1 = 0;
    int have0, have1, first, second, rc;

    if (s == 0 || s->io == 0 || s->io->read == 0 || buf == 0 ||
        out_len == 0 || s->slot_sectors == 0)
        return STORE_ERR_ARG;

    have0 = slot_header(s, 0, &seq0);
    have1 = slot_header(s, 1, &seq1);
    if (!have0 && !have1)
        return STORE_ERR_EMPTY;

    if (have0 && have1)
        first = seq_newer(seq0, seq1) ? 0 : 1;
    else
        first = have0 ? 0 : 1;
    second = 1 - first;

    rc = slot_load(s, first, buf, cap, out_len);
    if (rc != STORE_ERR_EMPTY && rc != STORE_ERR)
        return rc;                          /* OK, or STORE_ERR_BIG        */
    /* The newest slot is unreadable/corrupt: the older one still rules. */
    if (second == 0 ? !have0 : !have1)
        return STORE_ERR_EMPTY;
    return slot_load(s, second, buf, cap, out_len);
}

int store_save(store_t *s, const void *data, uint32_t len)
{
    uint32_t sec[STORE_IO_BYTES / 4u];
    uint32_t hdr[STORE_HDR_WORDS];
    uint32_t seq0 = 0, seq1 = 0, seq, pcrc, off, i, nsec;
    int have0, have1, target;

    if (s == 0 || s->io == 0 || s->io->read == 0 || s->io->write == 0 ||
        (data == 0 && len > 0) || s->slot_sectors == 0)
        return STORE_ERR_ARG;
    if (len > store_capacity(s))
        return STORE_ERR_BIG;

    have0 = slot_header(s, 0, &seq0);
    have1 = slot_header(s, 1, &seq1);

    /* Write into the invalid or OLDER slot so the newest record survives a
     * torn write (this is the whole point of the two slots). */
    if (have0 && have1) {
        target = seq_newer(seq0, seq1) ? 1 : 0;
        seq = (target == 0 ? seq1 : seq0) + 1u;
    } else if (have0) {
        target = 1;
        seq = seq0 + 1u;
    } else if (have1) {
        target = 0;
        seq = seq1 + 1u;
    } else {
        target = 0;
        seq = 1u;
    }

    pcrc = crc32_update(0, (const uint8_t *)data, len);
    memset(hdr, 0, sizeof hdr);
    hdr[H_MAGIC] = STORE_MAGIC;
    hdr[H_VERSION] = STORE_VERSION | (STORE_HDR_BYTES << 16);
    hdr[H_SEQ] = seq;
    hdr[H_LEN] = len;
    hdr[H_PAYCRC] = pcrc;
    hdr[H_HDRCRC] = crc32_update(0, (const uint8_t *)hdr, STORE_HDR_CRC_N);

    /* Payload first, zero-padded to whole sectors ... */
    nsec = payload_sectors(len);
    for (i = 0, off = 0; i < nsec; i++) {
        uint32_t chunk = len - off;

        if (chunk > STORE_IO_BYTES)
            chunk = STORE_IO_BYTES;
        memset(sec, 0, sizeof sec);
        memcpy(sec, (const uint8_t *)data + off, chunk);
        if (s->io->write(s->io->user, slot_lba(s, target) + 1u + i, 1u,
                         sec) != 0)
            return STORE_ERR;
        off += chunk;
    }
    /* ... then the header last: the commit point. */
    memset(sec, 0, sizeof sec);
    memcpy(sec, hdr, STORE_HDR_BYTES);
    if (s->io->write(s->io->user, slot_lba(s, target), 1u, sec) != 0)
        return STORE_ERR;

    /* Read-back verification (media errors are not hypothetical here). */
    if (s->io->read(s->io->user, slot_lba(s, target), 1u, sec) != 0)
        return STORE_ERR;
    if (memcmp(sec, hdr, STORE_HDR_BYTES) != 0)
        return STORE_ERR;
    {
        uint32_t crc = 0;

        for (i = 0, off = 0; i < nsec; i++) {
            uint32_t chunk = len - off;

            if (chunk > STORE_IO_BYTES)
                chunk = STORE_IO_BYTES;
            if (s->io->read(s->io->user, slot_lba(s, target) + 1u + i, 1u,
                            sec) != 0)
                return STORE_ERR;
            crc = crc32_update(crc, (const uint8_t *)sec, chunk);
            off += chunk;
        }
        if (crc != pcrc)
            return STORE_ERR;
    }
    return STORE_OK;
}
