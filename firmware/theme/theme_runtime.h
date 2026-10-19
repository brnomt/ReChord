/*
 * theme_runtime.h — package discovery, atomic application and persistence
 * (BRNOMT-69).
 *
 * A package is any directory under <root> that contains a readable
 * `theme.cfg`. The palette and the layout are parsed from that same text and
 * committed together only after BOTH pass validation, so a broken package can
 * never leave half of the UI on its values.
 *
 * Fixed-capacity and heap-free: the scan writes at most THEME_PKG_MAX entries
 * into the caller's array and reads one manifest into a static buffer.
 */
#ifndef RECHORD_THEME_RUNTIME_H
#define RECHORD_THEME_RUNTIME_H

#include "theme.h"

#define THEME_PKG_ID_MAX   32
#define THEME_PKG_MAX      16
#define THEME_PKG_TEXT_MAX 4096

typedef struct theme_pkg_info {
    char id[THEME_PKG_ID_MAX];      /* directory name                    */
    char name[THEME_NAME_MAX];      /* `name=` from the manifest         */
} theme_pkg_info_t;

/* Enumerate valid packages under `root` (directories with a readable
 * manifest), sorted by id. Returns the number written (0..max) or a negative
 * value when the root cannot be listed. */
int theme_pkg_scan(const char *root, theme_pkg_info_t *out, int max);

/* Read <root>/<id>/theme.cfg and apply palette + layout atomically.
 * Returns 0 on success; < 0 when the package is missing or invalid (the
 * active theme/layout are then untouched). */
int theme_pkg_apply(const char *root, const char *id);

/* Persist the selection into an existing, pre-created file (the device only
 * rewrites files it created; see fs_target_file_rewrite()). */
int theme_pkg_store(const char *active_path, const char *id);

/* Boot entry: read the active id, apply its package, fall back to
 * `fallback_id`, and finally to the built-in Mono theme. Returns 0 when a
 * package was applied. */
int theme_pkg_boot(const char *root, const char *active_path,
                   const char *fallback_id);

/* id currently applied (empty when the built-in fallback is active). */
const char *theme_pkg_active_id(void);

#endif /* RECHORD_THEME_RUNTIME_H */
