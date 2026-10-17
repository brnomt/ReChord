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
#include "drivers/usb_cdc.h"
#include "audio/audio_output.h"
#include "drivers/rknano_i2s.h"
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
            if (ev.kind != INPUT_PRESS)     /* a short press answers once */
                continue;
            if (any_key || ev.type == KEY_SELECT)
                return 1;
            if (ev.type == KEY_BACK || ev.type == KEY_POWER)
                return 0;
        }
        panel_io_delay_ms(5);
    }
}

static void power_off(void);

/* Power-hold tracking (shared by the idle and playing loops).
 * The power key is a mechanical switch on GPIO2.5; the old counter reset
 * to zero on ANY misread sample, so contact bounce (or one glitched read
 * while the UI is busy — e.g. during playback) restarted the ~2 s count
 * and power-off never triggered. Debounce the level first (3 consecutive
 * samples), then count; `>=` also makes the threshold immune to skips. */
#define POWER_HOLD_COUNT   400u   /* ~2 s of loop passes (re-tuned after
                                   * the keys_adc latency fix; pass rate
                                   * is render-bound, not clock-based) */
#define POWER_DEBOUNCE     3
static int s_power_level;         /* debounced power-key level (0/1) */
static int s_power_stable;

static int power_hold_step(int power)
{
    int p = power ? 1 : 0;

    if (p != s_power_level) {
        if (++s_power_stable >= POWER_DEBOUNCE) {
            s_power_level = p;
            s_power_stable = 0;
        }
    } else {
        s_power_stable = 0;
    }
    s_power_held = s_power_level ? s_power_held + 1u : 0u;
    return s_power_held >= POWER_HOLD_COUNT;
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

/* MSC session: expose the user volume (LUN 0) and the SD card (LUN 1,
 * read-only) until eject/unplug/power key. Runs from the "USB storage"
 * menu entry AND automatically on the cable (VBUS) edge. */
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

/* Feed `seconds` of the 64-sample sine at 44.1 kHz through the polled I2S
 * FIFO. Returns dac_i2s_feed's rc (DAC_ERR_TIMEOUT = FIFO stopped
 * draining = no I2S clock); *frames_out = frames actually queued. */
static int tone_burst(const int16_t *sine64, int seconds, int16_t *chunk,
                      int *frames_out)
{
    int i, n = 0, rc = DAC_OK;

    audio_output_start(0);              /* ring NULL = polled path        */
    for (;;) {
        if (keys_power())
            break;
        for (i = 0; i < 256; i++) {
            int idx = (n + i) & 63;
            chunk[2 * i] = sine64[idx];
            chunk[2 * i + 1] = sine64[idx];
        }
        rc = audio_output_feed_blocking(chunk, 256);
        if (rc != DAC_OK)
            break;
        n += 256;
        if (n > 44100 * seconds)
            break;
    }
    audio_output_stop();
    *frames_out = n;
    return rc;
}

/* DAC bring-up test (Exp 1/2 of docs/rewrite/dac.md §7): run the CS43131
 * init (recovered from the reference fw1), show the breadcrumb + I2C scan
 * + HP status, then feed a 1 kHz tone over I2S1 until POWER (or 3 s). */
static void dac_test_run(void)
{
    /* sin(2*pi*i/64) * 9000, i = 0..63 */
    static const int16_t sine64[64] = {
           0,  785, 1567, 2340, 3096, 3827, 4530, 5200,
        5831, 6417, 6954, 7439, 7865, 8226, 8520, 8742,
        8889, 8960, 8952, 8867, 8703, 8462, 8148, 7762,
        7307, 6788, 6209, 5575, 4891, 4161, 3390, 2583,
        1746,  885,     0,  -885,-1746,-2583,-3390,-4161,
       -4891,-5575,-6209,-6788,-7307,-7762,-8148,-8462,
       -8703,-8867,-8952,-8960,-8889,-8742,-8520,-8226,
       -7865,-7439,-6954,-6417,-5831,-5200,-4530,-3827,
    };
    int16_t chunk[512];
    uint8_t hp[2];
    char buf[40];
    int rc, i, n = 0;

    screen("DAC test");
    line(1, "bring-up 44100 Hz...");
    display_flush_rows(0, DISPLAY_H - 1);
    {
        audio_format_t fmt;

        fmt.sample_rate = 44100;
        fmt.bits = 16;
        fmt.channels = 2;
        rc = audio_output_configure(&fmt);
    }

    dac_get_hp_status(hp);
    screen("DAC test result");
    snprintf(buf, sizeof buf, "rc %d step %d", rc, dac_last_step());
    line(1, buf);
    snprintf(buf, sizeof buf, "scan A%d%d B%d%d C%d%d",
             dac_scan_result(0, 0), dac_scan_result(0, 1),
             dac_scan_result(1, 0), dac_scan_result(1, 1),
             dac_scan_result(2, 0), dac_scan_result(2, 1));
    line(2, buf);
    snprintf(buf, sizeof buf, "hp %02x %02x", hp[0], hp[1]);
    line(3, buf);

    /* Register readback (chip 0) to diagnose "no sound": confirms which
     * CS43131 regs actually took vs our write sequence (POWER, unmute,
     * volume, output routing, FUN_03055ecc channel/gain). */
    {
        uint8_t v0, v1;
        struct { uint32_t map; const char *name; } regs[] = {
            { 0x000B0000, "PWR" }, { 0x00010009, "MUT" }, { 0x0001000B, "VOL" },
            { 0x0005000A, "CH0" }, { 0x0005000B, "CH1" }, { 0x00040010, "OUT" },
        };
        int ln = 4;
        for (i = 0; i < 6; i += 2) {
            v0 = v1 = 0xFF;
            dac_read_reg(0, regs[i].map, &v0);
            dac_read_reg(0, regs[i + 1].map, &v1);
            snprintf(buf, sizeof buf, "%s%02x %s%02x",
                     regs[i].name, v0, regs[i + 1].name, v1);
            line(ln++, buf);
        }
    }
    display_flush_rows(0, DISPLAY_H - 1);
    ask(1);                          /* read the register dump first */

    if (rc != DAC_OK) {
        screen("DAC test");
        line(1, "bring-up FAILED - any key");
        display_flush_rows(0, DISPLAY_H - 1);
        ask(1);
        return;
    }

    screen("DAC test tone");
    line(1, "tone: 1 kHz, POWER = stop");
    display_flush_rows(0, DISPLAY_H - 1);
    rc = tone_burst(sine64, 3, chunk, &n);

    /* The three observables that split the no-sound hypotheses:
     * fifo 00-00 = the TXDR writes vanish; fifo 00-1x = SCLK runs and the
     * fault is DAC-side (MCLK/PLL); clk/pdn = what the chip latched. */
    {
        unsigned lo, hi;
        uint8_t clk = 0xFF, pdn = 0xFF;

        rknano_i2s_fifo_range(&lo, &hi);
        dac_read_reg(0, 0x00010006, &clk);
        dac_read_reg(0, 0x00020000, &pdn);
        screen("DAC test tone result");
        snprintf(buf, sizeof buf, "tone rc %d f%d", rc, n);
        line(1, buf);
        snprintf(buf, sizeof buf, "fifo %02x-%02x", lo, hi);
        line(2, buf);
        snprintf(buf, sizeof buf, "clk %02x pdn %02x", clk, pdn);
        line(3, buf);
        line(4, "any key = end");
        display_flush_rows(0, DISPLAY_H - 1);
        ask(1);
    }

    /* Leave the hardware in the stock STOPPED state (FUN_03055F80 stop +
     * FUN_030564d8 close: hw mute + amp off + I2S1 reset held) — a live
     * amp path with I2S stopped is continuous noise (DFIU 60609d7f).
     * (The RCO-clock diagnostic tone is gone: MCLK_SRC_SEL=10 routes the
     * CS43131's uncalibrated RC standby oscillator into the audio path,
     * so it is BY DESIGN noisy/off-pitch — a probe, not a test mode.) */
    audio_output_power_off();
}

/* ---- USB CDC-ACM debug console ----------------------------------------- */
static int cdc_hex(const char *p, uint32_t *out)
{
    uint32_t v = 0; int n = 0;
    for (; *p == ' '; p++)
        ;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
        p += 2;
    for (; *p; p++) {
        int d;
        if (*p >= '0' && *p <= '9') d = *p - '0';
        else if (*p >= 'a' && *p <= 'f') d = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F') d = *p - 'A' + 10;
        else break;
        v = v * 16 + (uint32_t)d;
        n++;
    }
    *out = v;
    return n;
}

static void cdc_reply(const char *fmt, uint32_t a, uint32_t b, uint32_t c)
{
    char out[64];
    snprintf(out, sizeof out, fmt, a, b, c);
    usb_cdc_puts(out);
}

static void cdc_exec(char *ln)
{
    while (*ln == ' ')
        ln++;
    if (ln[0] == 'r') {                     /* r <map>  -> read chip 0     */
        uint32_t map; uint8_t v = 0xFF;
        cdc_hex(ln + 1, &map);
        if (dac_read_reg(0, map, &v) == DAC_OK)
            cdc_reply("0x%06x = 0x%02x\r\n", map, v, 0);
        else
            cdc_reply("0x%06x READ FAIL\r\n", map, 0, 0);
    } else if (ln[0] == 'w') {              /* w <map> <val> -> write both */
        uint32_t map, val;
        int n = cdc_hex(ln + 1, &map);
        cdc_hex(ln + 1 + n, &val);
        dac_write_reg(0, map, (uint8_t)val);
        cdc_reply("w 0x%06x <- 0x%02x\r\n", map, val, 0);
    } else if (ln[0] == 's') {              /* s -> I2C scan (3 candidates)*/
        uint32_t a, b, c;
        a = (dac_scan_result(0, 0) << 1) | dac_scan_result(0, 1);
        b = (dac_scan_result(1, 0) << 1) | dac_scan_result(1, 1);
        c = (dac_scan_result(2, 0) << 1) | dac_scan_result(2, 1);
        cdc_reply("scan A%x B%x C%x\r\n", (a << 4) | b, c, 0);
    } else if (ln[0] == 'q') {
        usb_cdc_request_exit();
    } else {
        usb_cdc_puts("cmds: r<map> w<map><val> s(scan) q(quit)\r\n");
    }
}

/* CDC-ACM diagnostic console, from the "USB console" menu entry (the
 * cable edge opens USB storage instead). Returns on POWER or unplug,
 * then tears the core down so the next mode starts from a host-visible
 * disconnect instead of stale endpoint state. */
static void usb_console_run(void)
{
    char ln[64];
    unsigned len = 0, frames = 0;

    screen("USB diag");
    line(1, "enumerating... POWER=exit");
    display_flush_rows(0, DISPLAY_H - 1);
    {
        /* I2C0 up so r/w cmds work (full bring-up at 44100). */
        audio_format_t fmt;

        fmt.sample_rate = 44100;
        fmt.bits = 16;
        fmt.channels = 2;
        (void)audio_output_configure(&fmt);
    }
    usb_cdc_init();
    for (;;) {
        int c;
        usb_cdc_poll();
        if (keys_power() || !usb_cable())
            break;
        /* Live enumeration state, refreshed every ~256 polls. */
        if ((frames++ & 0xFFu) == 0u) {
            const struct usb_cdc_diag_t *d = usb_cdc_diag();
            snprintf(ln, sizeof ln, "rst %u set %u cfg %u", d->resets, d->setups, d->configured ? 1u : 0u);
            line(2, ln);
            snprintf(ln, sizeof ln, "dev%u cfg%u str%u adr%u", d->desc_dev, d->desc_cfg, d->desc_str, d->set_addr);
            line(3, ln);
            snprintf(ln, sizeof ln, "cls%u oth%u xfer%u", d->class_req, d->other_req, d->xfer_err);
            line(4, ln);
            snprintf(ln, sizeof ln, "s %02x %02x %02x %02x %02x %02x %02x %02x", d->last_setup[0], d->last_setup[1], d->last_setup[2], d->last_setup[3], d->last_setup[4], d->last_setup[5], d->last_setup[6], d->last_setup[7]);
            line(5, ln);
            snprintf(ln, sizeof ln, "GINT %08x DSTS %03x", d->gintsts, d->dsts & 0xFFFu);
            line(6, ln);
            display_flush_rows(0, DISPLAY_H - 1);
        }
        c = usb_cdc_getc();
        if (c < 0)
            continue;
        if (c == '\r' || c == '\n') {
            ln[len] = 0;
            if (len) {
                cdc_exec(ln);
                len = 0;
            }
        } else if (len < sizeof ln - 1u) {
            ln[len++] = (char)c;
        }
    }
    usb_cdc_request_exit();
    usb_dwc2_stop();               /* host-visible disconnect, no stale EPs */
}

/* MSC on demand (was the automatic VBUS action; the console owns that now). */
static void usb_storage_run(void)
{
    if (!s_user) {
        screen("USB storage");
        line(1, "no eMMC user area");
        display_flush_rows(0, DISPLAY_H - 1);
        return;
    }
    usb_session();
}

static const menu_item_t k_root_items[] = {
    { "DAC test",    NULL,      dac_test_run },
    { "USB console", NULL,      usb_console_run },
    { "USB storage", NULL,      usb_storage_run },
    { "Files",       NULL,      open_files },
    { "Now Playing", NULL,      open_player },
    { "Settings",    NULL,      open_settings },
    { "Keys",        keys_text, NULL },
    { "eMMC",        emmc_text, NULL },
    { "SD",          sd_text,   NULL },
    { "USB",         usb_text,  NULL },
    { "USB diag",    usbd_text, NULL },
};
#define RC_BUILD "m7a"
static const menu_model_t k_root = { "ReChord " RC_BUILD, k_root_items, 11 };

void ap_ui_loop(void)
{
    input_event_t ev;

    emmc_probe();
    player_init();       /* register wav/mp3/flac codecs (registry) */
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

        /* VBUS edge: plugging the cable opens USB STORAGE right away (the
         * pre-CDC behaviour the user expects). The CDC console stays on
         * the "USB console" menu entry. */
        if (cable && !s_cable)
            usb_storage_run();
        s_cable = cable;

        /* power-hold: debounced (see power_hold_step); ~2 s of held power
         * turns the device off. */
        if (power_hold_step(power))
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
            /* Short vs long press: PRESS acts once; REPEAT continues the
             * action while held (volume / navigation). LONG and RELEASE
             * are markers for consumers that need the distinction. */
            if (ev.kind != INPUT_PRESS && ev.kind != INPUT_REPEAT)
                continue;
            if (ev.type == KEY_MENU) {
                while (ui_stack_depth() > 1)
                    ui_pop_screen();
            } else {
                ui_handle_event(&ev);
            }
        }
        if (ui_stack_depth() == 0)       /* BACK on the root menu */
            ui_push_screen(&menu_screen);

        /* audio pump + Now Playing sync (no-ops without a playing track).
         * While a track plays, decode ahead into the 32 KiB PCM ring: the
         * I2S1 TX IRQ (rknano_i2s_tx_isr) drains it at the sample clock,
         * so decode and playback overlap and the pump calls are
         * non-blocking once the ring is full. */
        if (player_get_state() == PLAYER_PLAYING) {
            int n;

            /* Pump only. Keys/power are sampled ONCE per pass above — the
             * ring+IRQ feeder made player_process() non-blocking, so the
             * old 8x rescan per pass was pure UI latency (8x the SARADC
             * work and 8x the debounce samples per pass). */
            for (n = 0; n < 8 && player_get_state() == PLAYER_PLAYING; n++)
                (void)player_process();      /* decodes into the ring */
            player_screen_refresh();
            ui_render();
            panel_io_delay_ms(1);   /* ring is ahead; don't hot-spin */
        } else {
            (void)player_process();
            player_screen_refresh();
            ui_render();
            /* ponytail: busy-wait scan period (~5 ms at the 24 MHz boot clock);
             * move to SysTick once the clock tree is set up. */
            panel_io_delay_ms(1);
        }
    }
}
