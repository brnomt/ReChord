/*
 * theme.c — theme registry + current-theme slot.
 *
 * The current theme lives in one writable slot and theme_get() hands out a
 * const pointer to it: consumers (UI, widgets) may cache the pointer for the
 * duration of one frame/redraw, and a theme switch is a plain struct copy —
 * no dynamic allocation anywhere (the DAP has no heap contract yet).
 */
#include <string.h>

#include "theme.h"
#include "theme_priv.h"

static theme_t g_current;
static int     g_have_current = 0;   /* lazy default to built-in 0 */

/* ASCII case-insensitive compare (theme names are ASCII by convention). */
static int name_eq(const char *a, const char *b)
{
    while (*a && *b) {
        int ca = (unsigned char)*a++;
        int cb = (unsigned char)*b++;
        if (ca >= 'a' && ca <= 'z') ca -= 'a' - 'A';
        if (cb >= 'a' && cb <= 'z') cb -= 'a' - 'A';
        if (ca != cb)
            return 0;
    }
    return *a == '\0' && *b == '\0';
}

const theme_t *theme_get(void)
{
    if (!g_have_current) {
        const theme_t *dflt = theme_builtin_get(0);
        if (dflt) {
            g_current = *dflt;
            g_have_current = 1;
        }
    }
    return &g_current;
}

void theme_set(const theme_t *t)
{
    if (t == 0)
        return;
    g_current = *t;
    g_have_current = 1;
}

int theme_count(void)
{
    return theme_builtin_count();
}

const char *theme_name(int i)
{
    const theme_t *t = theme_builtin_get(i);
    return t ? t->name : 0;
}

int theme_load_named(const char *name)
{
    int i, n;

    if (name == 0)
        return -1;

    n = theme_builtin_count();
    for (i = 0; i < n; i++) {
        const theme_t *t = theme_builtin_get(i);
        if (t && name_eq(t->name, name)) {
            g_current = *t;
            g_have_current = 1;
            return 0;
        }
    }
    return -2;   /* unknown theme name — current theme left untouched */
}

int theme_validate(const theme_t *t)
{
    int i;

    if (t == 0)
        return -1;

    /* Non-empty name: the settings UI lists themes by name. */
    if (t->name[0] == '\0')
        return -2;
    for (i = 0; i < THEME_NAME_MAX; i++)
        if (t->name[i] == '\0')
            break;
    if (i == THEME_NAME_MAX)
        return -2;   /* not NUL-terminated */

    /* Readability floor: text and accent must differ from the background. */
    if (t->text == t->background)
        return -3;
    if (t->accent == t->background)
        return -4;

    /* Semantic colors must be distinguishable from each other. */
    if (t->danger == t->success)
        return -5;

    if (t->spacing < 0 || t->spacing > 64)
        return -6;
    if (t->radius < 0 || t->radius > 64)
        return -7;
    if (t->font_id < 0)
        return -8;

    return 0;
}
