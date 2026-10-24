/*
 * test_theme_runtime.c — package discovery, atomic apply and persistence
 * (BRNOMT-69). Host test: real temporary directories, real manifests.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../theme_pkg.h"
#include "../theme_runtime.h"
#include "../theme_skin.h"
#include "../drivers/display.h"

/* The runtime test never draws; the skin engine still references the display
 * primitives, so provide no-op implementations. */
void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t color)
{
    (void)x0; (void)y0; (void)x1; (void)y1; (void)color;
}

void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
    (void)x; (void)y; (void)s; (void)fg; (void)bg;
}

void display_bitmap1(int x, int y, int w, int h, int stride,
                     const uint8_t *bits, uint16_t fg, uint16_t bg)
{
    (void)x; (void)y; (void)w; (void)h; (void)stride;
    (void)bits; (void)fg; (void)bg;
}

void display_blit_span(int x, int y, int w, const uint16_t *src)
{
    (void)x; (void)y; (void)w; (void)src;
}

static int checks;
static int fails;

#define CHECK(cond) do {                                                   \
    checks++;                                                              \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        fails++;                                                           \
    }                                                                      \
} while (0)

static int write_file(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");

    if (f == 0)
        return -1;
    if (fputs(text, f) < 0) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

int main(void)
{
    char tmp[] = "/tmp/rechord_theme_XXXXXX";
    char root[256], path[320], active[256];
    theme_pkg_info_t infos[THEME_PKG_MAX];
    int n;

    printf("== theme package runtime ==\n");

    CHECK(mkdtemp(tmp) != 0);
    snprintf(root, sizeof root, "%s/themes", tmp);
    CHECK(mkdir(root, 0700) == 0);
    snprintf(active, sizeof active, "%s/active_theme", tmp);

    snprintf(path, sizeof path, "%s/mono", root);
    CHECK(mkdir(path, 0700) == 0);
    snprintf(path, sizeof path, "%s/mono/theme.cfg", root);
    CHECK(write_file(path,
        "format=1\nhome=menu\nhome_nav=vertical\nname=Mono\n"
        "background=0x0000\ntext=0xFFFF\naccent=0xFFFF\ndanger=0xC618\n"
        "success=0xFFFF\npad=4\nitem_h=12\nnp_bar_y=74\n") == 0);

    snprintf(path, sizeof path, "%s/companion-bot", root);
    CHECK(mkdir(path, 0700) == 0);
    snprintf(path, sizeof path, "%s/companion-bot/theme.cfg", root);
    CHECK(write_file(path,
        "format=1\nhome=skin\nhome_nav=horizontal\nskin=home.skin\n"
        "name=Companion Bot\n"
        "background=0x0000\ntext=0xFFFF\naccent=0xFFFF\ndanger=0xC618\n"
        "success=0xFFFF\nface_h=112\nhome_bar_y=150\nhome_bar_h=14\n"
        "pad=6\nitem_h=14\n") == 0);
    snprintf(path, sizeof path, "%s/companion-bot/home.skin", root);
    CHECK(write_file(path,
        "[skin]\nrefresh_ms=100\n"
        "[items]\ntype=list\naxis=horizontal\nprev=down\nnext=up\n"
        "item0=LIBRARY:library\nitem1=PLAYER:player\nitem2=SETTINGS:settings\n")
        == 0);

    snprintf(path, sizeof path, "%s/broken", root);
    CHECK(mkdir(path, 0700) == 0);
    snprintf(path, sizeof path, "%s/broken/theme.cfg", root);
    CHECK(write_file(path,
        "format=1\nhome=skin\nskin=missing.skin\npad=99\n") == 0);

    /* 1. scan sees every readable manifest, sorted by id. */
    n = theme_pkg_scan(root, infos, THEME_PKG_MAX);
    CHECK(n == 3);
    if (n == 3) {
        CHECK(strcmp(infos[0].id, "broken") == 0);
        CHECK(strcmp(infos[1].id, "companion-bot") == 0);
        CHECK(strcmp(infos[1].name, "Companion Bot") == 0);
        CHECK(strcmp(infos[2].id, "mono") == 0);
        CHECK(strcmp(infos[2].name, "Mono") == 0);
    }

    /* 2. apply commits palette + layout together. */
    CHECK(theme_pkg_apply(root, "mono") == 0);
    CHECK(strcmp(theme_pkg_active_id(), "mono") == 0);
    CHECK(theme_layout()->home == THEME_HOME_MENU);
    CHECK(theme_layout()->pad == 4);
    CHECK(theme_get()->background == 0x0000);

    CHECK(theme_pkg_apply(root, "companion-bot") == 0);
    CHECK(strcmp(theme_pkg_active_id(), "companion-bot") == 0);
    CHECK(theme_layout()->home == THEME_HOME_SKIN);
    CHECK(theme_layout()->home_nav == THEME_NAV_HORIZONTAL);
    CHECK(strcmp(theme_layout()->skin, "home.skin") == 0);
    CHECK(theme_skin_text() != NULL);
    CHECK(strstr(theme_skin_text(), "type=list") != NULL);
    CHECK(theme_skin_item_count() == 3);
    CHECK(theme_skin_refresh_ms() == 100);

    /* 3. an invalid package leaves the active theme untouched. */
    CHECK(theme_pkg_apply(root, "broken") != 0);
    CHECK(strcmp(theme_pkg_active_id(), "companion-bot") == 0);
    CHECK(theme_layout()->pad == 6);
    CHECK(theme_layout()->home == THEME_HOME_SKIN);

    /* 4. path/id hygiene: no traversal, no unknown directories. */
    CHECK(theme_pkg_apply(root, "../mono") != 0);
    CHECK(theme_pkg_apply(root, "missing") != 0);
    CHECK(theme_pkg_apply(root, "") != 0);

    /* 5. persistence + boot loads the stored package (newline trimmed). */
    CHECK(theme_pkg_store(active, "companion-bot") == 0);
    theme_layout_reset();
    CHECK(theme_pkg_boot(root, active, "mono") == 0);
    CHECK(theme_layout()->home == THEME_HOME_SKIN);
    CHECK(strcmp(theme_pkg_active_id(), "companion-bot") == 0);

    /* 6. a missing active file falls back to the requested default. */
    snprintf(path, sizeof path, "%s/does-not-exist", tmp);
    theme_layout_reset();
    CHECK(theme_pkg_boot(root, path, "mono") == 0);
    CHECK(theme_layout()->home == THEME_HOME_MENU);
    CHECK(strcmp(theme_pkg_active_id(), "mono") == 0);

    /* 7. an unknown stored id also falls back. */
    CHECK(theme_pkg_store(active, "mono") == 0);
    CHECK(write_file(active, "ghost\n") == 0);
    theme_layout_reset();
    CHECK(theme_pkg_boot(root, active, "mono") == 0);
    CHECK(strcmp(theme_pkg_active_id(), "mono") == 0);

    /* cleanup */
    snprintf(path, sizeof path, "%s/mono/theme.cfg", root); unlink(path);
    snprintf(path, sizeof path, "%s/mono", root); rmdir(path);
    snprintf(path, sizeof path, "%s/companion-bot/theme.cfg", root); unlink(path);
    snprintf(path, sizeof path, "%s/companion-bot/home.skin", root); unlink(path);
    snprintf(path, sizeof path, "%s/companion-bot", root); rmdir(path);
    snprintf(path, sizeof path, "%s/broken/theme.cfg", root); unlink(path);
    snprintf(path, sizeof path, "%s/broken", root); rmdir(path);
    rmdir(root);
    unlink(active);
    rmdir(tmp);

    printf("%d checks, %d failures\n", checks, fails);
    return fails == 0 ? 0 : 1;
}
