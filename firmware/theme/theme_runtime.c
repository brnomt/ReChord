/*
 * theme_runtime.c — package discovery + atomic application (BRNOMT-69).
 *
 * Reads one manifest into a fixed static buffer, parses the palette and the
 * layout from the SAME text, validates both, and only then commits them. No
 * heap, no partial application: a malformed package leaves the active theme
 * exactly as it was.
 */
#include <stdio.h>
#include <string.h>

#include "theme_runtime.h"
#include "theme_pkg.h"
#include "theme_priv.h"

#ifdef RECHORD_AP_BUILD
#include "../services/fs.h"
#include "../services/fs_target.h"
#else
#include <dirent.h>
#endif

static char s_text[THEME_PKG_TEXT_MAX];
static char s_active[THEME_PKG_ID_MAX];

/* ---- small helpers ------------------------------------------------------ */

static int id_char(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.';
}

static int valid_id(const char *id)
{
    int i;

    if (id == 0 || id[0] == '\0' || id[0] == '.')
        return 0;
    for (i = 0; id[i] != '\0'; i++) {
        if (i >= THEME_PKG_ID_MAX - 1 || !id_char(id[i]))
            return 0;
    }
    return 1;
}

static int build_path(char *out, int cap, const char *root, const char *id)
{
    int n;

    if (root == 0 || id == 0)
        return -1;
    n = snprintf(out, (size_t)cap, "%s/%s/theme.cfg", root, id);
    return (n > 0 && n < cap) ? 0 : -1;
}

static void trim(char *s)
{
    int len = (int)strlen(s);
    int start = 0;

    while (start < len && (s[start] == ' ' || s[start] == '\t' ||
                           s[start] == '\r' || s[start] == '\n'))
        start++;
    while (len > start && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
                           s[len - 1] == '\r' || s[len - 1] == '\n'))
        len--;
    if (start > 0)
        memmove(s, s + start, (size_t)(len - start));
    s[len - start] = '\0';
}

static int read_text(const char *path, char *buf, unsigned cap)
{
    FILE *f;
    unsigned used = 0;

    if (path == 0 || buf == 0 || cap == 0)
        return -1;
    f = fopen(path, "r");
    if (f == 0)
        return -1;

    buf[0] = '\0';
    while (!feof(f)) {
        char line[256];
        unsigned n;

        if (fgets(line, (int)sizeof line, f) == 0)
            break;
        n = (unsigned)strlen(line);
        if (used + n + 1u > cap) {
            fclose(f);
            buf[0] = '\0';
            return -2;                   /* over the declared manifest cap */
        }
        memcpy(buf + used, line, n);
        used += n;
    }
    fclose(f);
    buf[used] = '\0';
    return (int)used;
}

static void copy_str(char *dst, int cap, const char *src)
{
    int i = 0;

    if (dst == 0 || cap <= 0)
        return;
    while (src != 0 && src[i] != '\0' && i + 1 < cap) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/* Display name from the manifest, falling back to the directory id. */
static void manifest_name(const char *text, const char *id, char *out, int cap)
{
    theme_t t;

    memset(&t, 0, sizeof t);
    (void)theme_file_parse(text, &t);
    if (t.name[0] != '\0')
        copy_str(out, cap, t.name);
    else
        copy_str(out, cap, id);
}

static void sort_infos(theme_pkg_info_t *out, int n)
{
    int i, j;

    for (i = 1; i < n; i++) {
        theme_pkg_info_t v = out[i];

        for (j = i - 1; j >= 0 && strcmp(out[j].id, v.id) > 0; j--)
            out[j + 1] = out[j];
        out[j + 1] = v;
    }
}

static void add_info(theme_pkg_info_t *out, int *n, int max,
                     const char *id, const char *text)
{
    if (*n >= max)
        return;
    copy_str(out[*n].id, (int)sizeof out[*n].id, id);
    manifest_name(text, id, out[*n].name, (int)sizeof out[*n].name);
    (*n)++;
}

/* ---- enumeration -------------------------------------------------------- */

#ifdef RECHORD_AP_BUILD

static const char        *s_scan_root;
static theme_pkg_info_t  *s_scan_out;
static int                s_scan_max;
static int                s_scan_n;

static int scan_cb(const char *name, int is_dir)
{
    char path[256];

    if (!is_dir || s_scan_n >= s_scan_max || !valid_id(name))
        return 0;
    if (build_path(path, (int)sizeof path, s_scan_root, name) != 0)
        return 0;
    if (read_text(path, s_text, sizeof s_text) < 0)
        return 0;
    add_info(s_scan_out, &s_scan_n, s_scan_max, name, s_text);
    return 0;
}

int theme_pkg_scan(const char *root, theme_pkg_info_t *out, int max)
{
    int r;

    if (root == 0 || out == 0 || max <= 0)
        return -1;
    s_scan_root = root;
    s_scan_out = out;
    s_scan_max = max;
    s_scan_n = 0;
    r = fs_list_dir_each(root, scan_cb);
    if (r < 0)
        return r;
    sort_infos(out, s_scan_n);
    return s_scan_n;
}

#else  /* host tests: iterate a real directory */

int theme_pkg_scan(const char *root, theme_pkg_info_t *out, int max)
{
    DIR *d;
    struct dirent *e;
    int n = 0;

    if (root == 0 || out == 0 || max <= 0)
        return -1;
    d = opendir(root);
    if (d == 0)
        return -1;
    while ((e = readdir(d)) != 0 && n < max) {
        char path[256];

        if (!valid_id(e->d_name))
            continue;
        if (build_path(path, (int)sizeof path, root, e->d_name) != 0)
            continue;
        if (read_text(path, s_text, sizeof s_text) < 0)
            continue;
        add_info(out, &n, max, e->d_name, s_text);
    }
    closedir(d);
    sort_infos(out, n);
    return n;
}

#endif

/* ---- application / persistence ----------------------------------------- */

int theme_pkg_apply(const char *root, const char *id)
{
    char path[256];
    theme_t t;
    theme_layout_t layout;

    if (!valid_id(id))
        return -1;
    if (build_path(path, (int)sizeof path, root, id) != 0)
        return -1;
    if (read_text(path, s_text, sizeof s_text) < 0)
        return -1;

    /* Deterministic base: a package that omits a color gets Mono's value, not
     * whatever theme happened to be active before. */
    t = *theme_builtin_get(0);
    if (theme_file_parse(s_text, &t) < 0)
        return -1;
    theme_layout_defaults(&layout);
    if (theme_layout_parse(s_text, &layout) != 0)
        return -1;
    if (theme_validate(&t) != 0)
        return -1;

    theme_set(&t);
    theme_layout_set(&layout);
    copy_str(s_active, (int)sizeof s_active, id);
    return 0;
}

int theme_pkg_store(const char *active_path, const char *id)
{
    if (active_path == 0 || !valid_id(id))
        return -1;

#ifdef RECHORD_AP_BUILD
    return fs_target_file_rewrite(active_path, id,
                                  (uint32_t)strlen(id)) == FS_OK ? 0 : -1;
#else
    {
        FILE *f = fopen(active_path, "w");
        unsigned n = (unsigned)strlen(id);

        if (f == 0)
            return -1;
        if (fwrite(id, 1, n, f) != n) {
            fclose(f);
            return -1;
        }
        fclose(f);
        return 0;
    }
#endif
}

int theme_pkg_boot(const char *root, const char *active_path,
                   const char *fallback_id)
{
    char id[THEME_PKG_ID_MAX];
    int applied = 0;

    id[0] = '\0';
    if (active_path != 0 && read_text(active_path, id, sizeof id) > 0) {
        trim(id);
        if (valid_id(id))
            applied = (theme_pkg_apply(root, id) == 0);
    }
    if (!applied && valid_id(fallback_id))
        applied = (theme_pkg_apply(root, fallback_id) == 0);
    if (!applied) {
        const theme_t *dflt = theme_builtin_get(0);

        if (dflt != 0)
            theme_set(dflt);
        theme_layout_reset();
        copy_str(s_active, (int)sizeof s_active,
                 fallback_id != 0 ? fallback_id : "");
        return -1;
    }
    return 0;
}

const char *theme_pkg_active_id(void)
{
    return s_active;
}
