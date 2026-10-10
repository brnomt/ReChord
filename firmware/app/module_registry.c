/*
 * module_registry.c — see module_registry.h for the contract.
 *
 * Pure C99, no libc, no allocation: host-tested with plain `cc` and compiled
 * for the target unchanged (architecture.md §5).
 */
#include "app/module_registry.h"

/* The fixed table. Pointers only — descriptors live in the modules. */
static const module_descriptor_t *g_modules[MODULE_REGISTRY_MAX];
static int g_count;

void module_registry_reset(void)
{
    int i;

    for (i = 0; i < MODULE_REGISTRY_MAX; i++)
        g_modules[i] = 0;
    g_count = 0;
}

int module_registry_add(const module_descriptor_t *m)
{
    int i;

    if (m == 0 || m->name == 0)
        return MODULE_REG_ERR_ARG;
    if (g_count >= MODULE_REGISTRY_MAX)
        return MODULE_REG_ERR_FULL;

    /* Duplicate names would make find() ambiguous for the UI, so they are
     * rejected instead of silently shadowed. */
    for (i = 0; i < g_count; i++) {
        const char *a = g_modules[i]->name;
        const char *b = m->name;
        while (*a != '\0' && *a == *b) {
            a++;
            b++;
        }
        if (*a == *b)                 /* equal, including both NUL-terminated */
            return MODULE_REG_ERR_DUP;
    }

    g_modules[g_count++] = m;
    return MODULE_REG_OK;
}

int module_registry_count(void)
{
    return g_count;
}

const module_descriptor_t *module_registry_get(int index)
{
    if (index < 0 || index >= g_count)
        return 0;
    return g_modules[index];
}

const module_descriptor_t *module_registry_find(const char *name)
{
    int i;

    if (name == 0)
        return 0;

    for (i = 0; i < g_count; i++) {
        const char *a = g_modules[i]->name;
        const char *b = name;
        while (*a != '\0' && *a == *b) {
            a++;
            b++;
        }
        if (*a == *b)
            return g_modules[i];
    }
    return 0;
}

int module_registry_init_all(void)
{
    int i;

    for (i = 0; i < g_count; i++) {
        if (g_modules[i]->init != 0 && g_modules[i]->init() != 0)
            return MODULE_REG_ERR_RUN;   /* halt: later inits may depend on */
    }
    return MODULE_REG_OK;
}

int module_registry_start_all(void)
{
    int i;

    for (i = 0; i < g_count; i++) {
        if (g_modules[i]->start != 0 && g_modules[i]->start() != 0)
            return MODULE_REG_ERR_RUN;
    }
    return MODULE_REG_OK;
}

void module_registry_stop_all(void)
{
    int i;

    /* Reverse order: a module registered later may depend on an earlier
     * one, so it must be stopped first. */
    for (i = g_count - 1; i >= 0; i--) {
        if (g_modules[i]->stop != 0)
            g_modules[i]->stop();
    }
}
