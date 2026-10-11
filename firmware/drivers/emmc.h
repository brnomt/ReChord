/*
 * emmc.h -- eMMC block access (DW-MMC at 0x60030000), read-only for now.
 * Sector = 512 bytes; buffers must be 4-byte aligned.
 */
#ifndef RECHORD_EMMC_H
#define RECHORD_EMMC_H

#include <stdint.h>

/* 0 on success, negative = failing init step (see emmc_rknanoc.c). */
int emmc_init(void);
uint32_t emmc_sectors(void);
int emmc_read(uint32_t lba, uint32_t count, void *buf);

/* Loader layout from the IDB: firmware area = [0x2000, user_start),
 * user volume = [user_start, emmc_sectors()). 0 on success. */
int emmc_layout(uint32_t *user_start);

#endif
