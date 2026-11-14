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

/* Boot entry: apply package `id` (the cfg `theme=` value; NULL or "" = none),
 * fall back to `fallback_id`, and finally to the built-in Mono theme. Returns
 * 0 when a package was applied. */
int theme_pkg_boot(const char *root, const char *id, const char *fallback_id);

/* id currently applied (empty when the built-in fallback is active). */
const char *theme_pkg_active_id(void);

/* Option selections are storage, which this module does not own. The app
 * installs a provider that fills `out[0..max-1]` with the digits persisted for
 * theme `id` (zeros when there is no entry). theme_pkg_apply() clamps them to
 * the declared choice counts, so a theme that shrinks an option list never
 * gets an out-of-range variant. Without a provider all options are 0. */
void theme_pkg_set_option_provider(void (*fn)(const char *id, int *out,
                                              int max));

#endif /* RECHORD_THEME_RUNTIME_H */
