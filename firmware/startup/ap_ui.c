/*
 * ap_ui.c -- milestone 2: key-driven UI loop on the AP (fw1), called by
 * Main() after the splash. Keys come from the board (keys_adc/keys_power),
 * are debounced by input_scan() and dispatched to the screen stack.
 */
#include <stdio.h>

#include "drivers/display.h"
#include "drivers/emmc.h"
#include "drivers/panel_io.h"
#include "drivers/usb_msc.h"
#include "services/fw_update.h"
#include "services/input.h"
#include "ui/ui.h"
#include "ui/screens/screens.h"

static int s_adc = 1023, s_power, s_cable;
static uint32_t s_user;              /* user volume start, 0 = unknown */
static char s_emmc[32], s_usb[32] = "plug in";

/* Read-only eMMC probe for the root menu: size and user volume start. */
static void emmc_probe(void)
{
    uint32_t user;
    int r = emmc_init();

    if (r) {
        snprintf(s_emmc, sizeof s_emmc, "init %d", r);
        return;
    }
    r = emmc_layout(&user);
    if (r)
        snprintf(s_emmc, sizeof s_emmc, "%uM lay %d", (unsigned)(emmc_sectors() >> 11), r);
    else
        snprintf(s_emmc, sizeof s_emmc, "%uM u%x", (unsigned)(emmc_sectors() >> 11),
                 (unsigned)(s_user = user));
}

static const char *emmc_text(void) { return s_emmc; }
static const char *usb_text(void)  { return s_usb; }

static const char *usbd_text(void)
{
    static char buf[16];
    snprintf(buf, sizeof buf, "%02x %02x g%x", (unsigned)(usb_msc_diag & 0xFFu),
             (unsigned)(usb_msc_diag >> 8 & 0xFFu), (unsigned)(usb_msc_diag >> 16));
    return buf;
}

static void line(int row, const char *s)
{
    display_draw_text(8, 8 + row * 14, s, 0xFFFF, 0x0000);
}

static void screen(const char *title)
{
    display_fill_rect(0, 0, DISPLAY_W - 1, DISPLAY_H - 1, 0x0000);
    line(0, title);
    display_flush_rows(0, DISPLAY_H - 1);
}

static void update_progress(const char *what, uint32_t done, uint32_t total)
{
    static unsigned last = 101;
    char buf[40];
    unsigned pct = (unsigned)(done / (total / 100u + 1u));

    if (pct == last)
        return;
    last = pct;
    snprintf(buf, sizeof buf, "%-10s %3u%%", what, pct);
    line(2, buf);
    display_flush_rows(8 + 2 * 14, 8 + 2 * 14 + 7);
}

/* KEY_SELECT = yes; BACK / power = no; any = 1 with any_key. */
static int ask(int any_key)
{
    input_event_t ev;

    while (keys_power())
        panel_io_delay_ms(1);
    input_mock_clear();
    for (;;) {
        input_scan(keys_adc(), keys_power());
        while (input_poll(&ev) > 0) {
            if (any_key || ev.type == KEY_SELECT)
                return 1;
            if (ev.type == KEY_BACK || ev.type == KEY_POWER)
                return 0;
        }
        panel_io_delay_ms(5);
    }
}

/* After a USB session: offer HIFIECnn.IMG / ECHOMINI.IMG if present. */
static void update_prompt(void)
{
    static fw_image_t img;
    char buf[40];
    int r;

    screen("Looking for a firmware update");
    r = fw_update_find(s_user, &img, update_progress);
    if (r == FWU_NONE || (r && !img.name[0]))
        return;
    if (r) {
        screen("Firmware update NOT usable");
        snprintf(buf, sizeof buf, "%s: error %d", img.name, r);
        line(2, buf);
        line(4, "press any key");
        display_flush_rows(0, DISPLAY_H - 1);
        ask(1);
        return;
    }
    screen("Firmware update detected");
    snprintf(buf, sizeof buf, "%s  %u KiB", img.name, (unsigned)(img.size >> 10));
    line(2, buf);
    for (int half = 0; half < 2; half++) {
        char *p = buf + snprintf(buf, sizeof buf, half ? "    " : "SHA ");
        for (int i = 0; i < 16; i++)
            p += snprintf(p, 3, "%02x", img.sha256[half * 16 + i]);
        line(3 + half, buf);
    }
    line(6, "Update?  PLAY = yes   BACK = no");
    display_flush_rows(0, DISPLAY_H - 1);
    if (!ask(0))
        return;

    screen("Updating - do NOT power off");
    r = fw_update_install(&img, update_progress);
    if (r) {
        snprintf(buf, sizeof buf, "Update FAILED: error %d", r);
        line(4, buf);
        line(5, "keep power on: unplug/replug USB");
        line(6, "to retry, or debrick via maskrom");
        display_flush_rows(0, DISPLAY_H - 1);
        ask(1);
        return;
    }
    r = fw_update_retire(&img);
    snprintf(buf, sizeof buf, "Update done (rename %d)", r);
    line(4, buf);
    line(5, "power-cycle the player now");
    display_flush_rows(0, DISPLAY_H - 1);
    for (;;)                             /* no working warm reboot yet */
        ;
}

/* Cable plugged in: expose the user volume until eject/unplug/power key. */
static void usb_session(void)
{
    uint32_t rd, wr;
    int r;

    screen("USB mass storage");
    line(1, "eject on the PC or press power");
    display_flush_rows(0, DISPLAY_H - 1);
    r = usb_msc_run(s_user, emmc_sectors() - s_user, keys_power, &rd, &wr);
    snprintf(s_usb, sizeof s_usb, "x%d r%uM w%uM", r, (unsigned)(rd >> 11), (unsigned)(wr >> 11));
    while (keys_power())                 /* don't leak the exit press */
        panel_io_delay_ms(1);
    if (r == USB_EXIT_EJECT || r == USB_EXIT_UNPLUG)
        update_prompt();
    ui_invalidate_all();
}

static void open_files(void)    { browser_screen_set_root("/"); ui_push_screen(&browser_screen); }
static void open_player(void)   { ui_push_screen(&player_screen); }
static void open_settings(void) { settings_screen_reload(); ui_push_screen(&settings_screen); }

static const char *keys_text(void)
{
    static char buf[24];
    snprintf(buf, sizeof buf, "adc %d pwr %d", s_adc, s_power);
    return buf;
}

static const menu_item_t k_root_items[] = {
    { "Files",       NULL,      open_files },
    { "Now Playing", NULL,      open_player },
    { "Settings",    NULL,      open_settings },
    { "Keys",        keys_text, NULL },
    { "eMMC",        emmc_text, NULL },
    { "USB",         usb_text,  NULL },
    { "USB diag",    usbd_text, NULL },
};
#define RC_BUILD "m4c"
static const menu_model_t k_root = { "ReChord " RC_BUILD, k_root_items, 7 };

void ap_ui_loop(void)
{
    input_event_t ev;

    emmc_probe();
    ui_init();
    menu_screen_bind(&k_root);
    ui_push_screen(&menu_screen);
    for (;;) {
        int adc = keys_adc(), power = keys_power(), cable = usb_cable();

        if (cable && !s_cable && s_user)
            usb_session();
        s_cable = cable;

        /* live "Keys" row on the root menu: hardware check without a log */
        if ((adc - s_adc > 8 || s_adc - adc > 8 || power != s_power) &&
            ui_stack_depth() == 1) {
            ui_invalidate_all();
        }
        s_adc = adc;
        s_power = power;
        input_scan(adc, power);

        while (input_poll(&ev) > 0) {
            if (ev.type == KEY_MENU) {
                while (ui_stack_depth() > 1)
                    ui_pop_screen();
            } else {
                ui_handle_event(&ev);
            }
        }
        if (ui_stack_depth() == 0)       /* BACK on the root menu */
            ui_push_screen(&menu_screen);
        ui_render();
        /* ponytail: busy-wait scan period (~5 ms at the 24 MHz boot clock);
         * move to SysTick once the clock tree is set up. */
        panel_io_delay_ms(1);
    }
}
