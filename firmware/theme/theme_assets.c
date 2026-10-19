/*
 * theme_assets.c — bounded 1-bit asset cache for theme packages.
 *
 * No heap: one static cache (THEME_ASSET_CACHE_BYTES) holds the bitmaps of
 * the active package. Skin meters read them through theme_asset_get(); if a
 * package declares no assets the cache stays empty.
 */
#include <stdio.h>
#include <string.h>

#include "theme_assets.h"

#ifdef RECHORD_AP_BUILD
#include "../services/fs.h"
#endif

typedef struct {
    char name[THEME_ASSET_NAME_MAX];
    int  w, h, stride;
    const uint8_t *bits;
} loaded_asset_t;

static uint8_t        s_cache[THEME_ASSET_CACHE_BYTES];
static unsigned       s_used;
static loaded_asset_t s_loaded[THEME_ASSET_MAX];
static int            s_count;

static int read_asset(const char *path, uint8_t *dst, unsigned bytes)
{
#ifdef RECHORD_AP_BUILD
    fs_file_t *f = fs_open_read(path);
    long got;

    if (f == 0)
        return -1;
    got = fs_read(f, dst, bytes);
    fs_close(f);
    return got == (long)bytes ? 0 : -1;
#else
    FILE *f = fopen(path, "rb");
    size_t got;

    if (f == 0)
        return -1;
    got = fread(dst, 1, bytes, f);
    fclose(f);
    return got == bytes ? 0 : -1;
#endif
}

static int make_path(char *out, int cap, const char *root, const char *id,
                     const char *rel)
{
    int n = snprintf(out, (size_t)cap, "%s/%s/%s", root, id, rel);

    return (n > 0 && n < cap) ? 0 : -1;
}

void theme_assets_reset(void)
{
    s_used = 0;
    s_count = 0;
}

int theme_assets_load(const char *root, const char *id,
                      const theme_layout_t *l)
{
    int i;

    theme_assets_reset();
    if (root == 0 || id == 0 || l == 0)
        return -1;

    for (i = 0; i < l->asset_count; i++) {
        const theme_asset_decl_t *decl = &l->assets[i];
        char path[256];
        int stride = (decl->w + 7) / 8;
        unsigned bytes = (unsigned)(stride * decl->h);

        if (s_count >= THEME_ASSET_MAX ||
            s_used + bytes > THEME_ASSET_CACHE_BYTES)
            return -1;
        if (make_path(path, (int)sizeof path, root, id, decl->path) != 0)
            return -1;
        if (read_asset(path, s_cache + s_used, bytes) != 0) {
            theme_assets_reset();
            return -1;
        }
        memcpy(s_loaded[s_count].name, decl->name, sizeof decl->name);
        s_loaded[s_count].name[sizeof s_loaded[s_count].name - 1] = '\0';
        s_loaded[s_count].w = decl->w;
        s_loaded[s_count].h = decl->h;
        s_loaded[s_count].stride = stride;
        s_loaded[s_count].bits = s_cache + s_used;
        s_used += bytes;
        s_count++;
    }
    return 0;
}

const uint8_t *theme_asset_get(const char *name, int *w, int *h, int *stride)
{
    int i;

    if (name == 0)
        return 0;
    for (i = 0; i < s_count; i++) {
        if (strcmp(s_loaded[i].name, name) == 0) {
            if (w)
                *w = s_loaded[i].w;
            if (h)
                *h = s_loaded[i].h;
            if (stride)
                *stride = s_loaded[i].stride;
            return s_loaded[i].bits;
        }
    }
    return 0;
}
