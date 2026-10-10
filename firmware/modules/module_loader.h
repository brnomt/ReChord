/*
 * module_loader.h — RMF1 module loader (public API).
 *
 * Validates, verifies and deploys a ReChord Module Format v1 blob into a
 * caller-supplied RAM window. The loader is ADDRESS-WINDOW AGNOSTIC: it never
 * assumes any fixed RAM address, so the exact same code runs on the Cortex-M3
 * target (window = e.g. HIGHRAM0 at 0x0307A000) and in host tests (window =
 * a malloc'd buffer). See docs/rewrite/modules.md for the state machine.
 */
#ifndef RECHORD_MODULE_LOADER_H
#define RECHORD_MODULE_LOADER_H

#include <stddef.h>
#include <stdint.h>

#include "module_format.h"

/*
 * The device's documented checksum primitive (docs/re/route-b-minimum.md §4b):
 * CRC-32, non-reflected, MSB-first, polynomial 0x04C10DB7 (note: NOT the
 * common 0x04C11DB7 — bit 0x1000 is missing, as byte-verified on device),
 * init 0, xorout 0. Implemented bitwise from the definition (no table, no
 * libc) so the same routine is trivially portable and provably matches the
 * format spec. Known-answer vector: rmf_crc32("123456789", 9) == 0x889A9615.
 */
uint32_t rmf_crc32(const void *data, size_t len);

/* Where a successfully loaded module ended up. */
typedef struct {
    uint16_t id;
    uint16_t flags;
    void *base;         /* window base the code was deployed at           */
    uint32_t code_len;
    uint32_t bss_start; /* absolute bss range in the window ==            */
    uint32_t bss_len;   /*   [base + bss_start, base + bss_start + bss_len) */
    uintptr_t entry;    /* callable Thumb entry = base + entry_offset | 1 */
} rmf_loaded_t;

/*
 * Load one RMF1 blob into [window, window + window_size).
 *
 * Steps (any failure returns the matching rmf_result_t and leaves the window
 * untouched):
 *   1. structural checks  -> RMF_ERR_ARG / RMF_ERR_LENGTH
 *   2. header checks      -> RMF_ERR_MAGIC / RMF_ERR_VERSION / RMF_ERR_FLAGS
 *   3. fit checks         -> RMF_ERR_WINDOW
 *   4. checksum           -> RMF_ERR_CRC
 *   5. deploy: copy code to the window base, zero [bss_start, +bss_len)
 *
 * On RMF_OK, *out describes the deployed module. `out` may be NULL if the
 * caller only wants the side effect, but the deploy still happens.
 */
rmf_result_t rmf_load_module(const void *blob, size_t blob_len,
                             void *window, size_t window_size,
                             rmf_loaded_t *out);

/* Human-readable name of a result code, for logs. Never returns NULL. */
const char *rmf_strerror(rmf_result_t r);

#endif /* RECHORD_MODULE_LOADER_H */
