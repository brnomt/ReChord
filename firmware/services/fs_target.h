/*
 * fs_target.h -- volumes behind the target fs backend (fs_target.c).
 * Paths are "/<volume>/dir/file"; "/" lists the mounted volumes.
 */
#ifndef RECHORD_SERVICES_FS_TARGET_H
#define RECHORD_SERVICES_FS_TARGET_H

#include <stdint.h>

#define FS_VOLUMES 2

/* Block reader: absolute device sectors, 4-byte aligned buffer, 0 = ok. */
typedef int (*fs_block_read_t)(uint32_t lba, uint32_t count, void *buf);

/* Mounts FAT16 / FAT32 / exFAT found at `base` (superfloppy or MBR
 * partition 0) as volume `idx` named `name`. 0, or FS_ERR_* (unsupported
 * file system, I/O). */
int fs_target_mount(int idx, const char *name, fs_block_read_t rd, uint32_t base);
void fs_target_unmount(int idx);

/* "FAT32" / "exFAT" / "FAT16", or NULL when not mounted. */
const char *fs_target_type(int idx);

/* ---- in-place writes (no FAT-table writes, no allocation) --------------- */

/* Optional per-volume block writer (NULL = read-only volume), same contract
 * as emmc_write: absolute device LBAs, 512-byte sectors, 4-byte aligned
 * buffer. Set it AFTER fs_target_mount(). Returns 0 or FS_ERR_ARG. */
typedef int (*fs_block_write_t)(uint32_t lba, uint32_t count, const void *buf);
int fs_target_set_writer(int idx, fs_block_write_t wr);

/* Rewrite an existing file's contents IN PLACE: writes `len` bytes at offset
 * 0, zero-fills the rest of the sectors it touches, and patches the file's
 * 4-byte size field in its FAT short directory entry (so the file shows
 * exactly `len` bytes). The file keeps the clusters it already owns — `len`
 * must fit them (FS_ERR_UNSUPPORTED otherwise, checked BEFORE any byte is
 * written) — so the FAT table and the directory layout are never modified
 * and nothing is allocated. FAT16/FAT32 only: exFAT is refused
 * (FS_ERR_UNSUPPORTED) because its size lives in the stream entry, covered
 * by the entry-set checksum. Requires a writer on the file's volume. The
 * sector cache is dropped afterwards, so the next read sees the new
 * contents. Returns 0 or an FS_ERR_* code. */
int fs_target_file_rewrite(const char *path, const void *data, uint32_t len);

#endif
