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

/* Block writer: same shape. Volumes are READ-ONLY until a writer is
 * registered with fs_target_set_writer(); the write path covers
 * single-component files in the volume ROOT on FAT16/FAT32 (exFAT
 * stays read-only). fs_target_mount() clears any registered writer. */
typedef int (*fs_block_write_t)(uint32_t lba, uint32_t count, const void *buf);
int fs_target_set_writer(int idx, fs_block_write_t wr);

/* Mounts FAT16 / FAT32 / exFAT found at `base` (superfloppy or MBR
 * partition 0) as volume `idx` named `name`. 0, or FS_ERR_* (unsupported
 * file system, I/O). */
int fs_target_mount(int idx, const char *name, fs_block_read_t rd, uint32_t base);
void fs_target_unmount(int idx);

/* "FAT32" / "exFAT" / "FAT16", or NULL when not mounted. */
const char *fs_target_type(int idx);

#endif
