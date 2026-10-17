/*
 * store.h — crash-safe record store over raw blocks (no filesystem).
 *
 * WHY: the AP must persist a few KB on the internal eMMC (settings now, the
 * media index later) without going through the FAT layer — our FAT *write*
 * path is the one that bricked devices (DFIU fa3968cd / 15e1c9f5), while
 * block writes above the user-volume floor are hardware-proven (USB MSC,
 * and fw_update_retire's FAT/dirent sector writes).
 *
 * CONTRACT: the caller owns a fixed region of `2 * slot_sectors` sectors
 * starting at `lba` and guarantees it is NOT part of a mounted FAT volume
 * (see the backend notes): the store never consults or updates any
 * filesystem. Two slots alternate so a power cut at any point leaves the
 * previous record authoritative:
 *
 *   - each slot = 32-byte header (magic, version, seq, len, payload CRC,
 *     header CRC) followed by the payload, zero-padded to a sector;
 *   - a save writes the payload sectors FIRST and the header sector LAST
 *     (the "commit"), then reads everything back and verifies it;
 *   - a load picks the valid slot with the newest sequence number, so the
 *     older record survives a torn or corrupt newer one; both invalid reads
 *     as "empty" (fresh region) instead of failing a boot.
 *
 * A record's payload is bounded by the slot (capacity = slot_sectors*512 -
 * 32). Records are loaded whole; streaming readers can use store_active()
 * to locate the live payload and read it in chunks.
 */
#ifndef RECHORD_STORE_H
#define RECHORD_STORE_H

#include <stdint.h>

#define STORE_OK          0
#define STORE_ERR        (-1)   /* block I/O failure                        */
#define STORE_ERR_ARG    (-2)   /* NULL argument / uninitialised store      */
#define STORE_ERR_EMPTY  (-3)   /* no valid slot (fresh region / corrupt)   */
#define STORE_ERR_BIG    (-4)   /* record does not fit the slot / caller buf */

/* Header facts (little-endian words): 32 bytes, versioned. */
#define STORE_MAGIC       0x53484352u   /* 'RCHS' */
#define STORE_VERSION     1u
#define STORE_HDR_BYTES   32u

/* Block device behind the store: sector-granular (512 B), 4-byte aligned
 * buffers (the same contract as emmc_read/emmc_write). 0 = success. */
typedef struct {
    int (*read)(void *user, uint32_t lba, uint32_t count, void *buf);
    int (*write)(void *user, uint32_t lba, uint32_t count, const void *buf);
    void *user;
} store_io_t;

typedef struct {
    const store_io_t *io;
    uint32_t lba;            /* first sector of slot A                     */
    uint16_t slot_sectors;   /* sectors per slot (>= 1; 8 = 4 KiB)         */
} store_t;

/* Bind `s` to a region. No I/O happens here. */
void store_init(store_t *s, const store_io_t *io, uint32_t lba,
                uint16_t slot_sectors);

/* Payload capacity of one slot in bytes (0 for an uninitialised store). */
uint32_t store_capacity(const store_t *s);

/* Load the newest valid record into `buf` (cap bytes). Returns STORE_OK and
 * *out_len, STORE_ERR_EMPTY when no slot is valid, STORE_ERR_BIG when the
 * record is larger than `cap` (the other slot is not tried then), or
 * STORE_ERR on I/O failure. */
int store_load(store_t *s, void *buf, uint32_t cap, uint32_t *out_len);

/* Write `data` as the new newest record. Returns STORE_OK after a verified
 * read-back, STORE_ERR_BIG when it does not fit, STORE_ERR on I/O failure
 * (the previous record stays authoritative in both failure cases). */
int store_save(store_t *s, const void *data, uint32_t len);

#endif /* RECHORD_STORE_H */
