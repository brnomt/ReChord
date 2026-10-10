/*
 * theme.h — ReChord UI theme bridge (colors come from the theme module).
 *
 * Integration decision (2026-10-09): the palette is NOT defined here any
 * more — every color reads `theme_get()` from firmware/theme/, so switching
 * themes at runtime restyles the whole UI without touching UI code (the
 * Rockbox-style goal). Geometry constants stay here: they derive from the
 * display/font contract and are not user-tunable data.
 *
 * Include resolution is deliberate: `../theme/theme.h` is a path relative to
 * THIS file, so it cannot collide with this header even though both are
 * named theme.h (quote-includes resolve the includer's directory first).
 */
#ifndef RECHORD_UI_THEME_H
#define RECHORD_UI_THEME_H

#include <stdint.h>
#include "../theme/theme.h"

/* ---- color helpers ---------------------------------------------------- */

/* Pack 8-bit RGB into RGB565 (used by theme data tables and UI effects). */
#define THEME_RGB565(r, g, b)                                   \
    ((uint16_t)(((((uint16_t)(r) & 0xF8u) << 8) |               \
                 (((uint16_t)(g) & 0xFCu) << 3) |               \
                 (((uint16_t)(b) & 0xF8u) >> 3))))

/* ---- palette: live values from the active theme ----------------------- */
#define THEME_COLOR_BG        (theme_get()->background)
#define THEME_COLOR_FG        (theme_get()->text)
#define THEME_COLOR_DIM       (theme_get()->text_dim)
#define THEME_COLOR_ACCENT    (theme_get()->accent)
#define THEME_COLOR_WARM      (theme_get()->success)  /* status emphasis */
#define THEME_COLOR_SELECT_BG (theme_get()->surface)  /* selected row    */
#define THEME_COLOR_TITLE_BG  (theme_get()->surface)  /* title bar       */
#define THEME_COLOR_BAR_BG    (theme_get()->surface)  /* bar trough      */
#define THEME_COLOR_BAR_FG    (theme_get()->accent)   /* bar fill        */
#define THEME_COLOR_WARN      (theme_get()->danger)   /* errors          */

/* ---- typography (font contract: 6x11 glyph cell, 12 px line pitch) ---- */
#define THEME_FONT_NAME        "builtin-6x11"
#define THEME_FONT_W           6   /* advance per character, px             */
#define THEME_FONT_H           11  /* glyph height, px                      */
#define THEME_LINE_H           12  /* line pitch, px (contract)             */
#define THEME_TEXT_YOFF        1   /* glyph top offset inside its line      */

/* ---- layout geometry (contract-derived, not user data) ---------------- */
#define THEME_PAD              4   /* outer padding, px                     */
#define THEME_TITLE_H          16  /* title bar height, px                  */
#define THEME_TITLE_TEXT_Y     2   /* title text baseline offset in bar     */
#define THEME_ITEM_H           THEME_LINE_H /* list row height, px          */
#define THEME_MENU_TOP         THEME_TITLE_H /* first list row y            */
#define THEME_SCROLLBAR_W      3   /* right-edge scrollbar width, px        */
#define THEME_TEXT_W(len)      ((int)(len) * THEME_FONT_W)

#endif /* RECHORD_UI_THEME_H */
