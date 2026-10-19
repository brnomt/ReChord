/*
 * theme_assets.h — 1-bit assets shipped inside a theme package (BRNOMT-69).
 *
 * A package declares assets in its manifest:
 *     asset_eye_open=assets/eye_open.bin,64,64
 * and the runtime reads the raw MSB-first bitmap rows (stride = (w+7)/8)
 * into a fixed cache. Loading is atomic per package: if any declared asset
 * is missing or does not fit, the package is rejected and the active theme
 * stays in place.
 */
#ifndef RECHORD_THEME_ASSETS_H
#define RECHORD_THEME_ASSETS_H

#include <stdint.h>

#include "theme_pkg.h"

#define THEME_ASSET_CACHE_BYTES 1024

/* Load every asset declared by `l` from <root>/<id>/<path>. Returns 0 when
 * all of them loaded (or none were declared), < 0 otherwise. */
int theme_assets_load(const char *root, const char *id,
                      const theme_layout_t *l);

/* Look up a loaded asset. Returns NULL when it was not declared/loaded;
 * otherwise fills w/h/stride and returns the MSB-first bitmap bytes. */
const uint8_t *theme_asset_get(const char *name, int *w, int *h, int *stride);

/* Drop the cache (theme switch / fallback). */
void theme_assets_reset(void);

#endif /* RECHORD_THEME_ASSETS_H */
