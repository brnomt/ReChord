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

/* ---- creation (allocation) ---------------------------------------------
 *
 * These DO write FAT metadata: a free-cluster search, the chain in both FAT
 * copies, and a directory entry. The data (a directory's cluster and entries,
 * a file's zeroed clusters) is written first and the entry that makes it
 * reachable is written LAST, so an interrupted create leaks clusters (which
 * fsck recovers) instead of leaving a dangling entry. The project kept this
 * path off while it was bricking twice: that turned out to be the .bss
 * ceiling at 0x0304A000, not this code (DFIU fb41cb15), and the ordering rule
 * above is the same one the device-proven rewrite uses.
 *
 * Both are idempotent: an existing directory (mkdir) or file (create) is left
 * alone and reported as FS_OK, so a first-boot initializer can just call them.
 * Names must be ASCII (the LFN writer is ASCII-only) and are written as a
 * plain 8.3 entry when they fit, otherwise as a long-name set. FAT16/FAT32
 * only; exFAT and read-only volumes return FS_ERR_UNSUPPORTED. A full volume
 * returns FS_ERR_IO. 0 on success. */

/* Create one directory (its parent must exist). */
int fs_target_mkdir(const char *path);

/* Create a file of `bytes` bytes, allocated (and zero-filled) up front. */
int fs_target_file_create(const char *path, uint32_t bytes);

#endif
