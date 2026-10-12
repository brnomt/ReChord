/*
 * ap_ui.c -- milestone 2: key-driven UI loop on the AP (fw1), called by
 * Main() after the splash. Keys come from the board (keys_adc/keys_power),
 * are debounced by input_scan() and dispatched to the screen stack.
 */
#include <stdio.h>
#include <string.h>

#include "drivers/display.h"
#include "drivers/emmc.h"
#include "drivers/panel_io.h"
#include "drivers/usb_msc.h"
#include "services/fs_target.h"
#include "services/fw_update.h"
#include "services/input.h"
#include "player.h"
#include "ui/ui.h"
#include "ui/screens/screens.h"

static int s_adc = 1023, s_power, s_cable;
static unsigned s_power_held;
static uint32_t s_user;              /* user volume start, 0 = unknown */
static char s_emmc[32], s_usb[32] = "plug in", s_sd[32];
static int s_sd_in = -1;

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
    if (s_user)
        fs_target_mount(0, "Internal", emmc_read, s_user);
}

static const char *emmc_text(void) { return s_emmc; }

/* SD row: size, type, bus width and the first volume's file system. */
static void sd_probe(void)
{
    int r = sd_init();

    fs_target_unmount(1);
    if (r == -1) {
        snprintf(s_sd, sizeof s_sd, "no card");
        return;
    }
    if (r) {
        snprintf(s_sd, sizeof s_sd, "init %d", r);
        return;
    }
    r = fs_target_mount(1, "SD", sd_read, 0);
    snprintf(s_sd, sizeof s_sd, "%uM %s %db %s", (unsigned)(sd_sectors() >> 11),
             sd_is_hc() ? "HC" : "SC", sd_bus_width(),
             r ? (r == -3 ? "rd err" : "no fs") : fs_target_type(1));
}

static const char *sd_text(void) { return s_sd; }
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

static void power_off(void);

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
    snprintf(buf, sizeof buf, "Update done (cleanup %d)", r);
    line(4, buf);
    line(5, "press power to turn off, then on");
    display_flush_rows(0, DISPLAY_H - 1);
    ask(1);                              /* no working warm reboot yet */
    power_off();
}

static void power_off(void)
{
    screen("Powering off");
    while (keys_power())
        panel_io_delay_ms(1);
    display_clear(0x0000);
    display_flush_rows(0, DISPLAY_H - 1);
    board_power_off();
}

/* Cable plugged in: expose the user volume (LUN 0) and the SD card (LUN 1,
 * read-only) until eject/unplug/power key. */
static void usb_session(void)
{
    const usb_lun_t luns[2] = {
        { emmc_read, emmc_write, s_user, emmc_sectors() - s_user },
        { sd_read, NULL, 0, sd_sectors() },
    };
    uint32_t rd, wr;
    int r;

    screen("USB mass storage");
    line(1, "eject on the PC or press power");
    display_flush_rows(0, DISPLAY_H - 1);
    r = usb_msc_run(luns, 2, keys_power, &rd, &wr);
    fs_target_mount(0, "Internal", emmc_read, s_user);   /* host may have written */
    snprintf(s_usb, sizeof s_usb, "x%d r%uM w%uM", r, (unsigned)(rd >> 11), (unsigned)(wr >> 11));
    while (keys_power())                 /* don't leak the exit press */
        panel_io_delay_ms(1);
    /* Scan for an update on ANY session exit the screen advertises:
     * eject (LoEj), unplug (VBUS gone), AND the power key — the screen
     * says "eject on the PC or press power", and the reference updater
     * scans after "usb: exit key pressed" too (ECHOLOG). With no image
     * present fw_update_find returns FWU_NONE and we return seamlessly. */
    if (r == USB_EXIT_EJECT || r == USB_EXIT_UNPLUG || r == USB_EXIT_USER)
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
    { "SD",          sd_text,   NULL },
    { "USB",         usb_text,  NULL },
    { "USB diag",    usbd_text, NULL },
};
#define RC_BUILD "m6b"
static const menu_model_t k_root = { "ReChord " RC_BUILD, k_root_items, 8 };

void ap_ui_loop(void)
{
    input_event_t ev;

    emmc_probe();
    ui_init();
    menu_screen_bind(&k_root);
    ui_push_screen(&menu_screen);
    for (;;) {
        int adc = keys_adc(), power = keys_power(), cable = usb_cable();

        if (sd_present() != s_sd_in) {   /* card inserted / removed */
            s_sd_in = sd_present();
            if (s_sd_in)
                panel_io_delay_ms(300);  /* let the contacts settle */
            sd_probe();
            ui_invalidate_all();
        }

        if (cable && !s_cable && s_user)  /* after the SD probe: LUN 1 */
            usb_session();
        s_cable = cable;

        /* ponytail: hold time in loop passes, calibrated on hardware
         * (2000 passes = ~6 s, so 700 = ~2 s); use a timer once there is one */
        s_power_held = power ? s_power_held + 1u : 0u;
        if (s_power_held == 700u)
            power_off();

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

        /* audio pump + Now Playing sync (no-ops without a playing track) */
        (void)player_process();
        player_screen_refresh();

        ui_render();
        /* ponytail: busy-wait scan period (~5 ms at the 24 MHz boot clock);
         * move to SysTick once the clock tree is set up. */
        panel_io_delay_ms(1);
    }
}
