/*
 * ap_main.c -- minimal AP (fw1) application main for milestone 1a.
 *
 * The clean-room AP startup (firmware/startup/ap_startup.c) ends its boot
 * recipe (bounded .bss zero -> VTOR = ap_vectors -> dsb/isb) by calling
 * Main().  This file provides that Main(): it brings up the display stack
 * and paints a first test screen, so the milestone-1a link exercises the
 * real drivers + theme code instead of being a bare stub.
 *
 * Test screen contract (firmware/drivers/display.h + firmware/ui/theme.h):
 *   - display_init()        one-shot panel bring-up + clear + flush
 *   - display_fill_rect()   inclusive rectangle fill in the framebuffer
 *   - display_draw_text()   8x8 glyph text, opaque cells
 *   - display_flush_rows()  push framebuffer rows to the panel (DCS 0x2A/
 *                           0x2B/0x2C path in panel_io_rknanoc.c)
 *   - all colors come from the live theme (theme_get()), never hardcoded.
 *
 * After painting (the splash), Main() runs the key-driven UI loop forever.
 */
#include <stdint.h>

#include "drivers/panel_io.h"
#include "drivers/display.h"
#include "ui/theme.h"       /* THEME_* geometry + palette (theme module) */

void ap_ui_loop(void);   /* ap_ui.c */

uint32_t Main(void)
{
    uint16_t bg = THEME_COLOR_BG;
    uint16_t fg = THEME_COLOR_FG;
    const theme_t *th = theme_get();

    display_init();

    /* Fill the whole screen (inclusive coordinates per display.h). */
    display_fill_rect(0, 0, DISPLAY_W - 1, DISPLAY_H - 1, bg);

    /* Accent band across the top, then the boot text line on it. */
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1, THEME_COLOR_ACCENT);
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y,
                      "ReChord AP alive", bg, THEME_COLOR_ACCENT);

    /* Second line: active theme name (proves the theme module is live). */
    display_draw_text(THEME_PAD, THEME_TITLE_H + THEME_PAD + THEME_TEXT_YOFF,
                      th->name, fg, bg);

    /* Make both lines visible on the panel. */
    display_flush_rows(0, THEME_TITLE_H + THEME_PAD + THEME_LINE_H);

#ifdef AP_TRIAL
    /* trial vehicle (firmware/hook/trial_entry.c): hand back to the host fw */
    panel_io_delay_ms(1000);   /* ~6 s at the 24 MHz boot clock */
    return 0;
#endif
    ap_ui_loop();
    return 0;
}

/* ==========================================================================
 * WEAK LINK STUBS -- present ONLY to prove the milestone-1a link.  Each one
 * is clearly reported in the milestone report; none of them is a runtime
 * feature.  Every stub is weak, so the real implementation replaces it
 * silently as soon as it lands.
 * ========================================================================== */

/*
 * rechord_startup_entry: reset target of the legacy Route-B vector table
 * (firmware/startup/vectors.c, table "vectors").  That table is kept in the
 * image for reference but is NOT the boot path of the clean-room fw1 AP
 * (the real entry is PowerOn_Reset via ap_vectors in ap_startup.c).  Stub:
 * park forever instead of running a boot recipe that does not belong here.
 */
__attribute__((weak)) void rechord_startup_entry(void)
{
    for (;;) {
    }
}

/*
 * panel_init_sequence: DCS power-on init for the panel (declared in
 * firmware/drivers/display.h).  NOT stubbed: the real implementation
 * (firmware/drivers/panel_init_rknanoc.c, the parallel drivers workstream)
 * landed in time and is part of the AP link (see firmware/ap.mk).  A weak
 * no-op fallback stood here while that file was still missing; it was
 * removed once the strong definition became linkable.
 */

/*
 * settings_view_load / settings_view_save: storage<->view adapters declared
 * in firmware/ui/settings_schema.h.  The UI host tests mock them
 * (firmware/ui/tests/mock_services.c); no target implementation exists yet
 * (it will adapt services/settings.c via settings_view_from_store/to_store
 * once the FAT backend lands).  STUBS: report "no storage" so the settings
 * screen falls back to defaults (see settings_screen_reload).
 */
__attribute__((weak)) int settings_view_load(void *s)
{
    (void)s;
    return -1;   /* load failed -> screen uses settings_view_defaults */
}

__attribute__((weak)) int settings_view_save(const void *s)
{
    (void)s;
    return -1;   /* not persisted yet */
}

/*
 * strchr / strrchr: needed by services/settings.c, theme/theme_file.c and
 * ui/screens/browser_screen.c.  These are REAL implementations (trivial),
 * provided weak here to close the milestone-1a link; per the policy in
 * firmware/libc/libc.h they belong in firmware/libc/str.c with a host test
 * and will drop out automatically once a strong definition lands there.
 */
__attribute__((weak)) char *strchr(const char *s, int c)
{
    char ch = (char)c;
    for (;; s++) {
        if (*s == ch)
            return (char *)s;
        if (*s == '\0')
            return (char *)0;
    }
}

__attribute__((weak)) char *strrchr(const char *s, int c)
{
    char ch = (char)c;
    const char *last = (const char *)0;
    for (;; s++) {
        if (*s == ch)
            last = s;
        if (*s == '\0')
            return (char *)last;
    }
}

/*
 * Host-stdio file calls used by the theme file loader
 * (firmware/theme/theme_file.c theme_file_load).  The AP links the
 * freestanding firmware/libc, which has no FILE I/O; the real target path
 * will route through firmware/services/fs.h once the FAT backend lands.
 * Stubs: no file, no load -- theme_file_parse() (pure text) is unaffected.
 */
__attribute__((weak)) void *fopen(const char *path, const char *mode)
{
    (void)path;
    (void)mode;
    return (void *)0;
}

__attribute__((weak)) char *fgets(char *s, int n, void *stream)
{
    (void)s;
    (void)n;
    (void)stream;
    return (char *)0;
}

__attribute__((weak)) int fgetc(void *stream)
{
    (void)stream;
    return -1;   /* EOF */
}

__attribute__((weak)) int feof(void *stream)
{
    (void)stream;
    return 1;    /* nothing to read */
}

__attribute__((weak)) int fclose(void *stream)
{
    (void)stream;
    return -1;
}
