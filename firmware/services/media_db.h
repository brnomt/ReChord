/*
 * media_db.h — the device's media index (BRNOMT-74).
 *
 * WHY: browsing must not scan the FAT on every navigation. The index is built
 * once per scan and read in pages afterwards.
 *
 * Layout (two slots, A/B, one file each):
 *
 *   [sector 0: header] [record 0] [record 1] ...          (one file per slot)
 *
 * A build always targets the slot that is NOT current: it truncates it,
 * appends the records in chunks, and finally patches the header in with
 * fs_patch() — that patch is the commit. An interrupted build therefore leaves
 * the previous index intact, and the reader picks the valid header with the
 * highest sequence number. The header sits in its own sector, so the commit is
 * a single-sector write; its CRC covers the records (verified on open).
 *
 * Records are a fixed size (so record i lives at a computed offset, no parsing
 * needed): the path relative to its volume, the four display tags, the track
 * number and the duration. Field capacities are small on purpose: this is a
 * browse index, not a tag database.
 */
#ifndef RECHORD_SERVICES_MEDIA_DB_H
#define RECHORD_SERVICES_MEDIA_DB_H

#include <stdint.h>

#define MEDIA_DB_A      "/Internal/.rechord/INDEX.A"
#define MEDIA_DB_B      "/Internal/.rechord/INDEX.B"

#define MEDIA_MAGIC     0x4D444231u   /* "MDB1" */
#define MEDIA_VERSION   2u
#define MEDIA_HDR_BYTES 512u
#define MEDIA_PATH_MAX  80
#define MEDIA_TEXT_MAX  24

typedef struct media_rec {
    char     path[MEDIA_PATH_MAX];  /* "SD/Music/Album/01 Title.flac" */
    char     title[32];
    char     artist[MEDIA_TEXT_MAX];
    char     album[MEDIA_TEXT_MAX];
    char     album_artist[MEDIA_TEXT_MAX];
    char     composer[MEDIA_TEXT_MAX];
    char     genre[16];
    char     label[16];
    uint16_t track_no;
    uint16_t year;                  /* 0 = unknown -> the views show "Undefined" */
    uint32_t duration_ms;
} media_rec_t;

/* The record size is derived, never written by hand: the header stores it and
 * the reader rejects a mismatch (that is the format-version check in practice). */
#define MEDIA_REC_BYTES ((uint32_t)sizeof(media_rec_t))
_Static_assert(sizeof(media_rec_t) <= 256u, "media_rec_t grew too far");

typedef struct media_hdr {
    uint32_t magic;
    uint32_t version;
    uint32_t seq;                   /* higher = newer; 0 = never committed */
    uint32_t count;
    uint32_t rec_bytes;
    uint32_t crc;                   /* over the records */
} media_hdr_t;

/* Build side. media_db_begin() picks the slot that is not current, truncates
 * it and reserves the header sector; media_db_add() buffers records and
 * appends them in chunks through the shared scratch region; media_db_end()
 * flushes the last chunk and commits the header. A failure at any point leaves
 * the previous index untouched. */
int      media_db_begin(void);
int      media_db_add(const media_rec_t *r);
int      media_db_end(void);

/* Abandon a build: releases the scratch and leaves the slot uncommitted, so
 * the previous index stays current. Safe to call after any failure. */
void     media_db_abort(void);

/* Read side: opens the newest valid slot and keeps it open for media_db_get()
 * (which streams by offset). media_db_open() returns 0 when an index exists;
 * FS_ERR_OPEN when there is none (or both are corrupt). */
int      media_db_open(void);
uint32_t media_db_count(void);
int      media_db_get(uint32_t i, media_rec_t *out);
void     media_db_close(void);

/* The path of slot 0 (A) or 1 (B), for logs and diagnostics. */
const char *media_db_slot_path(int which);

#endif /* RECHORD_SERVICES_MEDIA_DB_H */
