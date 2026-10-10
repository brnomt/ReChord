/*
 * themes_builtin.c — built-in ReChord themes as .c data tables.
 *
 * WHY data in .c: the build-from-source rule (docs/rewrite/architecture.md §1)
 * wants every byte rebuildable from source; .c tables need no asset pipeline,
 * compile on host and target alike, and can be `const` (flash-resident on the
 * Cortex-M3). This file is the ONLY place in the firmware where color values
 * exist — everything else asks theme_get().
 *
 * Color values are RGB565 pixel words; the comment on each field shows the
 * approximate 8-bit RGB triple it decodes to ((r>>3)<<11 | (g>>2)<<5 | b>>3).
 *
 * Palette design notes:
 *   Classic — dark slate UI, the reference look; full-contrast text.
 *   Paper   — warm light/sepia, for bright environments and eye comfort
 *             (no pure white/pure black pair: reduces halation).
 *   Night   — very dim dark palette for bedside use: every channel kept low
 *             so the panel never blows out dark-adapted eyes.
 */
#include "theme.h"
#include "theme_priv.h"

static const theme_t builtin_themes[] =
{
    {   /* Classic — dark */
        "Classic",
        0x1083,   /* background  RGB( 18, 18, 24) */
        0x2146,   /* surface     RGB( 38, 40, 50) */
        0xEF7E,   /* text        RGB(236,238,242) */
        0x94B4,   /* text_dim    RGB(146,150,160) */
        0x44FF,   /* accent      RGB( 64,156,255) */
        0xFA8A,   /* danger      RGB(255, 82, 82) */
        0x56AF,   /* success     RGB( 82,214,120) */
        8,        /* spacing px                  */
        6,        /* radius px                   */
        0         /* font id (driver default)    */
    },
    {   /* Paper — light/sepia, eye-strain-friendly */
        "Paper",
        0xF79C,   /* background  RGB(246,240,228) */
        0xFFFE,   /* surface     RGB(255,252,244) */
        0x3144,   /* text        RGB( 48, 42, 32) */
        0x7B8C,   /* text_dim    RGB(122,112, 96) */
        0x9AC2,   /* accent      RGB(158, 90, 20) */
        0xB145,   /* danger      RGB(176, 42, 42) */
        0x2BE8,   /* success     RGB( 46,124, 70) */
        10,       /* spacing px                  */
        2,        /* radius px                   */
        0         /* font id                     */
    },
    {   /* Night — dim dark */
        "Night",
        0x0841,   /* background  RGB(  8,  8, 12) */
        0x10A3,   /* surface     RGB( 22, 22, 30) */
        0xAD56,   /* text        RGB(170,170,178) */
        0x630D,   /* text_dim    RGB( 98, 98,110) */
        0x23B6,   /* accent      RGB( 36,116,180) */
        0xB2AA,   /* danger      RGB(180, 86, 86) */
        0x44AC,   /* success     RGB( 70,150,100) */
        8,        /* spacing px                  */
        6,        /* radius px                   */
        0         /* font id                     */
    }
};

#define BUILTIN_COUNT ((int)(sizeof(builtin_themes) / sizeof(builtin_themes[0])))

int theme_builtin_count(void)
{
    return BUILTIN_COUNT;
}

const theme_t *theme_builtin_get(int i)
{
    if (i < 0 || i >= BUILTIN_COUNT)
        return 0;
    return &builtin_themes[i];
}
