/*
 * module_registry.h — app-side registry of feature modules.
 *
 * WHY: bring-up must init/start every feature module in one deterministic
 * order, and the UI / settings screens must be able to enumerate what is
 * present in this firmware image (name + lifecycle state) without knowing
 * any module internals. Each module is a small const descriptor; the
 * registry keeps descriptors in a fixed-size static table — no dynamic
 * allocation anywhere (architecture.md).
 *
 * STORAGE POLICY: the registry stores POINTERS to descriptors. A descriptor
 * must have static lifetime (a const table in the module's own .c); it is
 * never copied and never freed.
 *
 * LIFECYCLE ORDER (documented in docs/rewrite/core.md):
 *   add() ... add()  — registration (bring-up, before any hook runs)
 *   init_all()       — every init() in registration order; HALTS at the
 *                      first failing init (a broken module must not be
 *                      built upon by later inits)
 *   start_all()      — every start() in registration order, only after ALL
 *                      inits succeeded (modules may rely on each other
 *                      being initialized)
 *   stop_all()       — every stop() in REVERSE registration order (tear
 *                      down dependents before their dependencies)
 *
 * Query API (count/get/find) is safe to call any time after registration.
 */
#ifndef RECHORD_APP_MODULE_REGISTRY_H
#define RECHORD_APP_MODULE_REGISTRY_H

/* Registry capacity (fixed static table; grow here if a build needs more). */
#define MODULE_REGISTRY_MAX 16

/* Return codes. */
#define MODULE_REG_OK        0
#define MODULE_REG_ERR_ARG (-1)  /* NULL descriptor, NULL name, bad index   */
#define MODULE_REG_ERR_FULL (-2) /* capacity reached                        */
#define MODULE_REG_ERR_DUP  (-3) /* module name already registered          */
#define MODULE_REG_ERR_RUN  (-4) /* an init/start hook returned nonzero     */

/* One feature module: name + lifecycle hooks. Any hook may be NULL
 * (a module that only needs init, or none at all, is legal). Hook contract:
 * init()/start() return 0 on success. */
typedef struct module_descriptor {
    const char *name;
    int  (*init)(void);
    int  (*start)(void);
    void (*stop)(void);
} module_descriptor_t;

/* Clear the registry (bring-up starts from a deterministic empty table;
 * also used between host test cases). */
void module_registry_reset(void);

/* Register a module. Returns MODULE_REG_OK, MODULE_REG_ERR_ARG (NULL
 * descriptor or NULL name), MODULE_REG_ERR_FULL, or MODULE_REG_ERR_DUP. */
int module_registry_add(const module_descriptor_t *m);

/* Number of registered modules (0 .. MODULE_REGISTRY_MAX). */
int module_registry_count(void);

/* Descriptor at index (0 .. count-1), NULL when out of range. */
const module_descriptor_t *module_registry_get(int index);

/* First descriptor with this exact (case-sensitive) name, NULL if absent. */
const module_descriptor_t *module_registry_find(const char *name);

/* Run lifecycle hooks in the documented order. init_all()/start_all()
 * return MODULE_REG_OK, MODULE_REG_ERR_ARG (nothing registered is OK, so
 * this is only for future use) or MODULE_REG_ERR_RUN when a hook failed —
 * iteration then stops at the failing module. stop_all() has no return. */
int  module_registry_init_all(void);
int  module_registry_start_all(void);
void module_registry_stop_all(void);

#endif /* RECHORD_APP_MODULE_REGISTRY_H */
