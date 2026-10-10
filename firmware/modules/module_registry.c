/*
 * module_registry.c — loaded-module table implementation.
 *
 * Fixed static table (no allocation): the registry must run before any heap
 * exists on the target and behave identically in host tests. Slots are
 * stable: remove() marks a slot EMPTY without compacting, so pointers from
 * rmf_registry_find()/rmf_registry_at() stay valid until that entry is
 * removed or unload_all() runs.
 */
#include "module_registry.h"

static rmf_registry_entry_t g_slots[RMF_REGISTRY_CAPACITY];

static void rmf_str_copy(char *dst, size_t dst_size, const char *src)
{
    size_t i;
    if (dst_size == 0) {
        return;
    }
    for (i = 0; i + 1 < dst_size && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

void rmf_registry_init(void)
{
    size_t i;
    for (i = 0; i < RMF_REGISTRY_CAPACITY; i++) {
        g_slots[i].state = RMF_MOD_STATE_EMPTY;
        g_slots[i].id = 0;
        g_slots[i].name[0] = '\0';
        g_slots[i].base = NULL;
        g_slots[i].size = 0;
    }
}

rmf_result_t rmf_registry_add(uint16_t id, const char *name,
                              void *base, uint32_t size)
{
    size_t i;
    rmf_registry_entry_t *slot = NULL;

    if (name == NULL || base == NULL) {
        return RMF_ERR_ARG;
    }
    for (i = 0; i < RMF_REGISTRY_CAPACITY; i++) {
        if (g_slots[i].state == RMF_MOD_STATE_LOADED) {
            if (g_slots[i].id == id) {
                return RMF_ERR_DUPLICATE;
            }
        } else if (slot == NULL) {
            slot = &g_slots[i];
        }
    }
    if (slot == NULL) {
        return RMF_ERR_FULL;
    }

    slot->id = id;
    rmf_str_copy(slot->name, RMF_REGISTRY_NAME_MAX, name);
    slot->base = base;
    slot->size = size;
    slot->state = RMF_MOD_STATE_LOADED;
    return RMF_OK;
}

const rmf_registry_entry_t *rmf_registry_find(uint16_t id)
{
    size_t i;
    for (i = 0; i < RMF_REGISTRY_CAPACITY; i++) {
        if (g_slots[i].state == RMF_MOD_STATE_LOADED && g_slots[i].id == id) {
            return &g_slots[i];
        }
    }
    return NULL;
}

const rmf_registry_entry_t *rmf_registry_at(size_t slot)
{
    if (slot >= RMF_REGISTRY_CAPACITY ||
        g_slots[slot].state != RMF_MOD_STATE_LOADED) {
        return NULL;
    }
    return &g_slots[slot];
}

size_t rmf_registry_capacity(void)
{
    return RMF_REGISTRY_CAPACITY;
}

size_t rmf_registry_count(void)
{
    size_t i, n = 0;
    for (i = 0; i < RMF_REGISTRY_CAPACITY; i++) {
        if (g_slots[i].state == RMF_MOD_STATE_LOADED) {
            n++;
        }
    }
    return n;
}

rmf_result_t rmf_registry_remove(uint16_t id)
{
    size_t i;
    for (i = 0; i < RMF_REGISTRY_CAPACITY; i++) {
        if (g_slots[i].state == RMF_MOD_STATE_LOADED && g_slots[i].id == id) {
            g_slots[i].state = RMF_MOD_STATE_EMPTY;
            g_slots[i].id = 0;
            g_slots[i].name[0] = '\0';
            g_slots[i].base = NULL;
            g_slots[i].size = 0;
            return RMF_OK;
        }
    }
    return RMF_ERR_NOT_FOUND;
}

void rmf_registry_unload_all(void)
{
    rmf_registry_init();
}
