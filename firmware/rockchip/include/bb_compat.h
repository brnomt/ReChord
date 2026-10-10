/* BB-only compatibility declarations for the original ReChord baseline.
 *
 * The complete AP SDK now supplies its own driver and filesystem definitions.
 * The BB build intentionally keeps the smaller integration headers that were
 * used before that import, so those compatibility constants must not leak into
 * the AP compilation.
 */
#ifndef RECHORD_BB_COMPAT_H
#define RECHORD_BB_COMPAT_H

#include "mailbox.h"
#include "service_globals.h"
#include "ModuleInfoTab.h"

/* DataDiskID: single source of truth is the MemDev.h macro (FW_IN_DEV).
 * The extern was removed 2026-10-09: it collided with the macro expansion.
 * No code assigns DataDiskID (read-only). */

#ifndef ATTR_DIRECTORY
#define ATTR_DIRECTORY 0x10
#endif
#ifndef DMA_CTLL_M2M_WORD
#define DMA_CTLL_M2M_WORD 0x00000010
#define DMA_CFGL_M2M_WORD 0x00000001
#define DMA_CFGH_M2M_WORD 0x00000000
#endif
#ifndef MEDIA_ID3_SAVE_CHAR_NUM
#define MEDIA_ID3_SAVE_CHAR_NUM 128
#endif
#ifndef MEDIA_FILE_TYPE_DELETED
/* MEDIA_FILE_TYPE_*: owned by the dir.h enum (do not #define). */

#endif
#ifndef RECORD_NULL
#define RECORD_NULL 0
#endif
#ifndef I2S_NORMAL_MODE
#endif
/* LogSecPerClus: VENDOR fat.h declares it as a variable (DRAM_FAT EXT uint8).
 * The old `#define LogSecPerClus 0` stub made assignments illegal. Removed
 * 2026-10-10. */
#ifndef FILE_NAME_SAVE_ADDR_OFFSET
#define FILE_NAME_SAVE_ADDR_OFFSET 0
#define DIR_CLUS_SAVE_ADDR_OFFSET 1
#define DIR_INDEX_SAVE_ADDR_OFFSET 2
#define ATTR_SAVE_ADDR_OFFSET 3
#endif

#endif /* RECHORD_BB_COMPAT_H */
