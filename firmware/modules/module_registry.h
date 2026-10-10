/*
 * module_registry.h — loaded-module table (public API).
 *
 * Tracks which RMF1 modules are currently deployed: (id, name, base, size,
 * state). Pure bookkeeping over caller-owned windows — the registry never
 * allocates, copies code, or frees windows, so it is trivially host-testable
 * and safe to point at any RAM window on the target.
 *
 * The registry does NOT depend on the loader: wire them together at the app
 * level (load with rmf_load_module(), then rmf_registry_add()).
 */
#ifndef RECHORD_MODULE_REGISTRY_H
#define RECHORD_MODULE_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include "module_format.h"

#define RMF_REGISTRY_CAPACITY 16   /* simultaneous loaded modules          */
#define RMF_REGISTRY_NAME_MAX 16   /* incl. NUL; longer names are truncated */

typedef enum {
    RMF_MOD_STATE_EMPTY = 0,  /* slot free                                 */
    RMF_MOD_STATE_LOADED = 1  /* module deployed in [base, base + size)    */
} rmf_mod_state_t;

typedef struct {
    uint16_t id;
    char name[RMF_REGISTRY_NAME_MAX];
    void *base;
    uint32_t size;          /* resident footprint: bss_start + bss_len     */
    rmf_mod_state_t state;
} rmf_registry_entry_t;

/* Reset every slot to EMPTY. Call once at boot before any add(). */
void rmf_registry_init(void);

/*
 * Register a loaded module. `size` is the full resident footprint (code
 * window span including bss). The name is copied, NUL-terminated and
 * truncated to RMF_REGISTRY_NAME_MAX - 1 chars if longer.
 * Fails with RMF_ERR_DUPLICATE (id taken) or RMF_ERR_FULL (no slot).
 */
rmf_result_t rmf_registry_add(uint16_t id, const char *name,
                              void *base, uint32_t size);

/* Find by id; NULL if not registered. */
const rmf_registry_entry_t *rmf_registry_find(uint16_t id);

/*
 * Iteration: walk slots 0..RMF_REGISTRY_CAPACITY-1 with rmf_registry_at();
 * it returns NULL for empty/invalid slots. Use with rmf_registry_capacity().
 */
const rmf_registry_entry_t *rmf_registry_at(size_t slot);
size_t rmf_registry_capacity(void);

/* Number of LOADED entries. */
size_t rmf_registry_count(void);

/* Forget one module (slot freed; window memory is untouched). */
rmf_result_t rmf_registry_remove(uint16_t id);

/* Forget every module (slot state -> EMPTY; window memory untouched). */
void rmf_registry_unload_all(void);

#endif /* RECHORD_MODULE_REGISTRY_H */
