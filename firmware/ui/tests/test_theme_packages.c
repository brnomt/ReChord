/*
 * test_theme_packages.c — the shipped instrument packages, end to end.
 *
 * Applies each package exactly as the firmware does (theme.cfg palette +
 * layout + assets + main skin) and loads every screen skin, so a package
 * that exceeds a limit or references a missing asset fails the host suite.
 */
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_services.h"
#include "../../theme/theme.h"
#include "../../theme/theme_pkg.h"
#include "../../theme/theme_runtime.h"
#include "../../theme/theme_skin.h"
#include "../../theme/theme_assets.h"

#define ROOT "../../../themes"

static const char *const k_skins[] = {
    "home.skin", "now_playing.skin", "library.skin",
    "library_cat.skin", "browser.skin", "settings.skin"
};

static void check_asset(const char *name, int w, int h)
{
    int aw = 0, ah = 0, stride = 0;
    const uint8_t *bits = theme_asset_get(name, &aw, &ah, &stride);

    CHECK(bits != 0);
    CHECK_EQ_INT(aw, w);
    CHECK_EQ_INT(ah, h);
    CHECK_EQ_INT(stride, (w + 7) / 8);
}

static void check_package(const char *id, const char *name,
                          const char *asset, int aw, int ah)
{
    int i;

    CHECK_EQ_INT(theme_pkg_apply(ROOT, id), 0);
    CHECK_STR_EQ(theme_pkg_active_id(), id);
    CHECK_STR_EQ(theme_get()->name, name);
    CHECK_EQ_INT(theme_layout()->home, THEME_HOME_SKIN);
    CHECK(theme_layout()->home_nav == THEME_NAV_HORIZONTAL);
    if (asset != 0)                  /* asset-free packages (live scope) */
        check_asset(asset, aw, ah);

    for (i = 0; i < (int)(sizeof k_skins / sizeof k_skins[0]); i++)
        CHECK_EQ_INT(theme_skin_load(ROOT, id, k_skins[i]), 0);
}

void test_theme_packages(void)
{
    ui_init();
    mock_services_reset();

    check_package("oscilloscope", "Oscilloscope", 0, 0, 0);
    check_package("oscilloscope-60", "Oscilloscope 60", 0, 0, 0);
    check_package("vu-meter", "VU Meter", "vu_a", 64, 40);
    check_package("808", "808", "ring", 32, 32);
}
