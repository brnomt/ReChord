/*
 * ap_main.c -- AP (fw1) application main.
 *
 * The clean-room AP startup (firmware/startup/ap_startup.c) ends its boot
 * recipe (bounded .bss zero -> VTOR = ap_vectors -> dsb/isb) by calling
 * Main().  This file provides that Main(): it raises the AP clock, brings up
 * the display stack, paints the boot card and then runs the key-driven UI
 * loop forever.
 *
 * The boot card is not theme-dependent: ap_boot_splash() / ap_boot_step()
 * (ap_ui.c) paint it with the same "system card" house look as the power,
 * USB and firmware-update screens. display_init() is the one-shot panel
 * bring-up + clear + flush; ap_ui owns the drawing.
 */
#include <stdint.h>

#include "drivers/panel_io.h"
#include "drivers/display.h"
#include "drivers/cru_pll.h"   /* AP core clock: 24 MHz -> 200 MHz */
#include "drivers/core_b.h"    /* BRNOMT-68 phase 1: Core B heartbeat */
#include "ui/theme.h"       /* THEME_* geometry + palette (theme module) */

#if AP_MEM_PROBE
#include "startup/mem_probe.h"
#endif
#if AP_MEM_OVERLAY
#include "startup/overlay_demo.h"
#endif
void ap_ui_loop(void);   /* ap_ui.c */

/* Boot card helpers (ap_ui.c): the same house style as every system card. */
void ap_boot_splash(void);
void ap_boot_step(int row, const char *k, const char *v, unsigned pct);

uint32_t Main(void)
{
    const theme_t *th = theme_get();
    int b_rc;
#if AP_MEM_PROBE
    int probe_rc;
#endif
#if AP_MEM_OVERLAY
    int overlay_rc;
#endif

    /* The stock/reference boot raises the AP from the 24 MHz oscillator to
     * the APLL at 200 MHz here (pclk 50 MHz). Everything below — and the
     * busy-wait delays in the drivers — is calibrated for that clock. */
    ap_clock_set_200mhz();

    display_init();

    /* Boot card: title bar + version, then one row per bring-up step. Row 1
     * keeps the splash subtitle; the steps start at row 2. */
    ap_boot_splash();
    ap_boot_step(2, "Theme", th->name, 33);

#if AP_MEM_PROBE
    /* 0x0304AE64 loader-data window probe result. */
    probe_rc = ap_mem_probe_run();
    ap_boot_step(3, "RAM 0304AE64", probe_rc == 0 ? "free" : "used", 66);
#endif
#if AP_MEM_OVERLAY
    /* Code overlay execution result. */
    overlay_rc = ap_overlay_demo_run();
    ap_boot_step(3, "Overlay", overlay_rc == 0 ? "OK" : "FAIL", 66);
#endif

    /* BRNOMT-68 phase 1: start Core B with our heartbeat image. Failure is
     * non-blocking (the AP keeps running; audio is unchanged). */
    b_rc = ap_core_b_start();
    ap_boot_step(4, "Core B", b_rc == 0 ? "ready" : "no", 100);

#if AP_MEM_PROBE || AP_MEM_OVERLAY
    panel_io_delay_ms(2000);   /* keep the probe/overlay row readable */
#endif

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
 * rechord_startup_entry (the legacy Route-B vector table's reset target) is
 * gone with firmware/startup/vectors.c: that table was never the boot path
 * (the real entry is PowerOn_Reset via ap_vectors in ap_startup.c) and the
 * AP code has to stay below LOADER_CODE_BASE 0x03088000.
 */

/*
 * panel_init_sequence: DCS power-on init for the panel (declared in
 * firmware/drivers/display.h).  NOT stubbed: the real implementation
 * (firmware/drivers/panel_init_rknanoc.c, the parallel drivers workstream)
 * landed in time and is part of the AP link (see firmware/ap.mk).  A weak
 * no-op fallback stood here while that file was still missing; it was
 * removed once the strong definition became linkable.
 */

/*
 * settings_view_load / settings_view_save: the real target adapters live in
 * firmware/ui/settings_view.c (linked via firmware/ap.mk); the UI host tests
 * substitute them in mock_services.c. They used to be weak stubs here.
 */

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
 * (firmware/theme/theme_file.c theme_file_load): the real target path is
 * firmware/libc/stdio_fs.c (stdio subset over the read-only FAT backend,
 * linked via firmware/ap.mk). They used to be weak stubs here, which linked
 * fine and silently made file themes inert.
 */
