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
#if AP_CDC_OVERLAY
#include "startup/cdc_overlay.h"
#endif
#include "audio/audio_output.h"
#include "drivers/rknano_i2s.h"
#include "drivers/battery.h"
#include "theme/theme.h"
#include "theme/theme_pkg.h"
#include "theme/theme_runtime.h"
#include "theme/theme_skin.h"
#include "services/fs_target.h"
#include "services/fw_update.h"
#include "services/log.h"
#include "services/media_db.h"
#include "services/media_scan.h"
#include "services/media_query.h"
#include "services/rechord_files.h"
#include "services/resume.h"
#include "services/input.h"
#include "player.h"
#include "ui/ui.h"
#include "ui/screens/screens.h"
#include "ui/screens/theme_ui.h"
#include "ui/screens/skin_screen.h"
#include "player/bfs.h"     /* phase 3: A-side file service for Core B */

/* Debug-only System actions (DAC test + USB CDC console). Off by default:
 * the AP code must stay below LOADER_CODE_BASE 0x03088000 (DFIU 51cc212d). */
#ifndef AP_DEBUG_TOOLS
#define AP_DEBUG_TOOLS 0
#endif

/* BRNOMT-90: playlist record index -> track path (player_play_list). */
static int media_resolve(uint16_t idx, char *path, int cap)
{
    media_rec_t rec;

    /* Reuse an index that is already open (BRNOMT-98): the indices being
     * resolved come from the same query, and re-opening re-streams the whole
     * index just to re-verify its CRC -- twice per first Play otherwise. */
    if (!media_db_is_open() && media_db_open() != FS_OK)
        return -1;
    if (media_db_get((uint32_t)idx, &rec) != FS_OK)
        return -1;
    if ((int)strlen(rec.path) + 2 > cap)
        return -1;
    path[0] = '/';
    strcpy(path + 1, rec.path);
    return 0;
}

static int s_adc = 1023, s_power, s_cable;
static uint32_t s_user;              /* user volume start, 0 = unknown */
static char s_emmc[32], s_usb[32] = "plug in", s_sd[32];
static int s_sd_in = -1;
static int s_volume_seen;            /* last volume persisted (see loop) */
static unsigned s_log_ms;            /* last periodic log flush (see loop) */
static unsigned s_batt_ms;           /* last battery poll (see loop) */
static int      s_resume_done;       /* boot resume restore ran (BRNOMT-87) */
static int      s_resume_started;    /* first probe seen; settle timer     */
static unsigned s_resume_ms;         /* when the settle wait began         */
static unsigned s_face_ms;           /* skin animation refresh tick        */
static unsigned s_theme_ms;          /* per-screen skin animation tick     */
static uint32_t s_index_cache = 0xFFFFFFFFu;  /* index track count, cached */

/* BRNOMT-84: the UI loop is paced by the SysTick 1 ms tick and sleeps in WFI
 * between ticks. Key samples are rate-gated (the SARADC triple conversion is
 * ~600 us of busy-wait; doing it every pass was the main heat source), and
 * Now Playing is refreshed at 10 Hz instead of every pass. */
#define KEYS_SCAN_MS            5u
#define PLAYER_UI_REFRESH_MS  100u

static unsigned s_loop_seq;          /* last loop tick (WFI pacing)        */
static unsigned s_keys_ms;           /* last SARADC key sample             */
static int      s_key_adc = 1023;    /* last key-ladder sample             */
static int      s_key_power;         /* last power-key sample              */
static int      s_keys_fallback;     /* async sampler failed: poll instead */
static unsigned s_player_ms;         /* last Now Playing refresh           */

/* BRNOMT-75: display-off and Hold. Short power toggles the backlight; holding
 * power and pressing MENU toggles Hold (screen off + all keys locked except
 * the same combo). Playback/audio never stop. */
static int      s_screen_off;
static int      s_hold;
static int      s_power_combo;       /* this power press became a chord    */
static int      s_combo_latch;       /* swallow events until both released */
static int      s_combo_menu_scans;  /* stable MENU samples while power down */

/* BRNOMT-87 resume state: defined with the loop, used by the power paths. */
static void resume_poll(unsigned now, int force);

/* screen helpers defined further down (used by the USB/index screens) */
static void line(int row, const char *s);
static void screen(const char *title);

/*
 * Millisecond clock for the input hold ladder and the log. The DWT cycle
 * counter (what this used to read) does not tick on this SoC: the log showed
 * 0.000 timestamps and input.c silently dropped the clock and fell back to its
 * synthetic per-scan clock -- which is why the hold ladder never accelerated.
 *
 * BRNOMT-98 FIX: RVR used to be 0x00FFFFFF, i.e. a ~84 ms interrupt at
 * 200 MHz — NOT the 1 ms the loop was documented (and tuned) for. The UI loop
 * therefore ran at ~12 Hz and every timed constant (key scan, repeat ladder,
 * WFI pacing) was scaled by 84. RVR is now exactly one millisecond and the ms
 * counter is advanced in the handler, so the loop really runs at 1 kHz and the
 * millisecond clock no longer depends on a 24-bit free-running wrap.
 */
#define AP_CYCLES_PER_MS 200000u   /* SysTick counts core cycles @ 200 MHz */
#define AP_SYST_CSR      (*(volatile uint32_t *)0xE000E010u)
#define AP_SYST_RVR      (*(volatile uint32_t *)0xE000E014u)
#define AP_SYST_CVR      (*(volatile uint32_t *)0xE000E018u)

static volatile unsigned s_ms;       /* 1 ms counter, advanced by SysTick */
static volatile unsigned s_tick_seq; /* SysTick handler heartbeat         */

/* Wakes the WFI loop once per millisecond. Reading CSR clears COUNTFLAG. */
void SysTick_Handler(void)
{
    (void)AP_SYST_CSR;
    s_ms++;
    s_tick_seq++;
    /* BRNOMT-98: the SARADC is stepped here, one conversion per tick, so the
     * main loop never busy-waits on it. The completed scan is picked up by
     * keys_scan_take() in the loop. */
    keys_scan_tick();
}

static unsigned ap_ms(void)
{
    return s_ms;
}

static void ap_clock_init(void)
{
    AP_SYST_RVR = AP_CYCLES_PER_MS - 1u;   /* exactly 1 ms @ 200 MHz      */
    AP_SYST_CVR = 0;
    AP_SYST_CSR = 7u;                /* CLKSOURCE | TICKINT | ENABLE        */
    s_ms = 0;
    input_set_clock(ap_ms);
    /* BRNOMT-98: the DAC bring-up delays become real milliseconds instead of
     * the loop-count estimate that overshoots several-fold. */
    dac_delay_set_clock(ap_ms);
    log_init((log_clock_fn)ap_ms);
}

/* The log file lives in the device's own folder. Flushes APPEND, so history
 * survives until the file reaches LOG_FILE_MAX, at which point it is truncated
 * and starts over (a rotating log: a session's forensics must not be wiped by
 * the next flush, which is what a rewrite did). */
#define LOG_FILE_MAX (32u * 1024u)

/* Flush policy (why): the log must be readable WITHOUT plugging the USB cable
 * (field request) — but the internal volume is eMMC, so every write spends
 * endurance. Therefore: persist only when there is something NEW (an idle
 * player writes nothing at all), slowly while playing, and eagerly just before
 * the RAM ring would start dropping lines. The file rotates at LOG_FILE_MAX,
 * so the same small block range is reused instead of the file growing. */
#define LOG_FLUSH_MS      30000u   /* slow periodic persist while dirty      */
#define LOG_FLUSH_EAGER   32u      /* ...or immediately this many lines deep */

static int log_write_file(const char *path, const char *text, unsigned len)
{
    fs_file_t *f = fs_open_read(path);
    long size = f ? fs_file_size(f) : 0;

    if (f != 0)
        fs_close(f);
    if (size < 0)
        size = 0;
    if ((unsigned long)size + len > LOG_FILE_MAX) {
        /* Rotate: start over rather than growing without bound. */
        if (fs_truncate(path) != FS_OK)
            return -1;
    }
    return fs_append(path, text, len) == FS_OK ? 0 : -1;
}

/* USB storage exposes the volume to the PC: flush the log first so the user
 * can read what just happened. */
static void usb_storage_run(void);
static void usb_storage_entry(void)
{
    (void)log_flush("usb");
    usb_storage_run();
}

/* Index progress: coarse (every 8 files) so the scan is not slowed by the
 * panel, and the power key aborts. */
static void open_library(void)
{
    ui_push_screen(&library_screen);
}

static int index_progress(uint32_t files, const char *name)
{
    char b[48];

    snprintf(b, sizeof b, "%u files", (unsigned)files);
    line(1, b);
    line(2, name);
    display_flush_rows(0, DISPLAY_H - 1);
    return keys_power();
}

/* "Rebuild index": one full scan into the inactive slot, committed at the
 * end. An abort or a failure leaves the previous index in place. */
static void index_rebuild_run(void)
{
    char b[48];
    int r;

    screen("Rebuild index");
    line(1, "scanning...");
    display_flush_rows(0, DISPLAY_H - 1);
    r = media_scan_build(index_progress);
    s_index_cache = 0xFFFFFFFFu;          /* the rebuilt index is the truth */
    if (r == FS_OK) {
        uint32_t tracks = 0;

        line(1, "done");
        if (media_db_open() == FS_OK) {
            tracks = media_db_count();
            media_db_close();
            snprintf(b, sizeof b, "%u tracks", (unsigned)tracks);
        } else {
            snprintf(b, sizeof b, "index unreadable");
        }
        log_printf("index: rebuild ok, %lu tracks", (unsigned long)tracks);
    } else {
        /* The code says where it stopped: -1 begin (scratch/slot), -2 open,
         * -3 io, -4 unsupported; MEDIA_SCAN_ABORTED is its own value. */
        snprintf(b, sizeof b, r == MEDIA_SCAN_ABORTED ? "aborted" : "failed %d", r);
        log_printf("index: rebuild rc %d (previous index kept)", r);
        snprintf(b, sizeof b, r == MEDIA_SCAN_ABORTED ? "aborted" : "failed");
        line(1, b);
        snprintf(b, sizeof b, "previous index kept");
    }
    line(2, b);
    display_flush_rows(0, DISPLAY_H - 1);
    while (!keys_power() && keys_adc() >= 1000)   /* wait for a key */
        panel_io_delay_ms(1);
    while (keys_power() || keys_adc() < 1000)
        panel_io_delay_ms(1);
}

/* Root menu row: how many tracks the current index holds. Opening the index is
 * I/O, so the value is cached -- and invalidated after a rebuild, otherwise the
 * row keeps the count it read at boot (when there was no index yet). */
static const char *index_text(void)
{
    static char b[24];

    if (s_index_cache == 0xFFFFFFFFu) {
        s_index_cache = (media_db_open() == FS_OK) ? media_db_count() : 0u;
        media_db_close();
    }
    snprintf(b, sizeof b, "%u trk", (unsigned)s_index_cache);
    return b;
}

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
    /* In-place file rewrites (settings now, the media index later) are only
     * allowed on this volume, block-level, from the user-volume floor up —
     * see fs_target_file_rewrite() (BRNOMT-71). */
    if (s_user)
        (void)fs_target_set_writer(0, emmc_write);
    if (s_user)
        log_set_writer(log_write_file);
    /* First boot: create /.rechord (cfg + state) when missing. Idempotent, so
     * it runs on every boot and never touches what is already there. */
    if (s_user)
        (void)rechord_files_init();
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

/* ---- BRNOMT-75: display off / Hold ------------------------------------ */

static void screen_wake(void)
{
    if (!s_screen_off)
        return;
    s_screen_off = 0;
    s_player_ms = 0;             /* force a Now Playing sync on wake */
    backlight_on();
    ui_invalidate_all();
    log_printf("power: screen on");
}

static void screen_sleep(void)
{
    if (s_screen_off)
        return;
    s_screen_off = 1;
    backlight_off();
    log_printf("power: screen off");
}

static void hold_set(int on)
{
    on = on ? 1 : 0;
    if (s_hold == on)
        return;
    s_hold = on;
    if (s_hold) {
        screen_sleep();
    } else {
        screen_wake();
    }
    log_printf("power: hold %d", s_hold);
}

/* Power-hold tracking (shared by the idle and playing loops).
 * The power key is a mechanical switch on GPIO2.5; contact bounce (or one
 * glitched read while the UI is busy) must not restart the count. Debounce
 * the level first, then measure held TIME with ap_ms() so a slow render pass
 * (themes animate) cannot stretch "2 seconds" into tens of seconds.
 *
 * BRNOMT-75: a release before POWER_HOLD_MS is a SHORT press (screen off/on).
 * A Power+MENU chord sets s_power_combo, which suppresses both the short
 * action and the 2 s power-off for that press. */
#define POWER_HOLD_MS      2000u
#define POWER_DEBOUNCE     3
#define POWER_EV_NONE      0
#define POWER_EV_SHORT     1
#define POWER_EV_HOLD      2
static int s_power_level;         /* debounced power-key level (0/1) */
static int s_power_stable;
static int s_power_t0_set;        /* timestamp valid for this press    */
static unsigned s_power_down_ms;

static int power_hold_step(int power, unsigned now)
{
    int p = power ? 1 : 0;

    if (p != s_power_level) {
        if (++s_power_stable >= POWER_DEBOUNCE) {
            s_power_level = p;
            s_power_stable = 0;
            if (p) {
                s_power_down_ms = now;
                s_power_t0_set = 1;
            } else {
                int was = s_power_t0_set;
                unsigned held = now - s_power_down_ms;

                s_power_t0_set = 0;
                if (s_power_combo) {
                    s_power_combo = 0;
                    return POWER_EV_NONE;
                }
                if (was && !s_hold && held < POWER_HOLD_MS)
                    return POWER_EV_SHORT;
            }
        }
    } else {
        s_power_stable = 0;
    }
    if (s_power_level && s_power_t0_set && !s_power_combo && !s_hold &&
        (unsigned)(now - s_power_down_ms) >= POWER_HOLD_MS)
        return POWER_EV_HOLD;
    return POWER_EV_NONE;
}

/* After a USB session: offer HIFIECnn.IMG / ECHOMINI.IMG if present. */
static void update_prompt(void)
{
    static fw_image_t img;
    char buf[40];
    int r;

    screen("Looking for a firmware update");
    r = fw_update_find(s_user, &img, update_progress);
    log_printf("fw: scan rc %d '%s'", r, img.name);
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
    backlight_off();     /* else the panel keeps glowing over a black screen */
    audio_output_power_off();    /* silence the DAC path while we are "off" */

    if (usb_cable()) {
        /* On VBUS the rail cut cannot kill us (the host feeds the SoC) and,
         * with the rail held low, the power key does NOT restart the player —
         * stock firmware behaves the same way (BRNOMT-91). So with a cable in,
         * "off" means dark + silent + parked: the device keeps charging, and a
         * power press wakes the UI back up instead of leaving it unstartable
         * until the cable is pulled. */
        while (!keys_power())
            panel_io_delay_ms(1);
        while (keys_power())
            panel_io_delay_ms(1);
        s_power_t0_set = 0;          /* this press must not count as a hold */
        s_power_level = 0;
        s_power_combo = 0;
        s_combo_latch = 0;
        s_combo_menu_scans = 0;
        s_hold = 0;
        s_screen_off = 0;
        s_player_ms = 0;
        backlight_on();
        ui_invalidate_all();
        return;                      /* the main loop resumes the UI */
    }
    board_power_off();               /* battery: the rail cut is a real off */
}

/* USB-storage power key (BRNOMT-91). The MSC loop's exit predicate used to be
 * the raw key level, so a HOLD while plugged in only left the session and the
 * user could never switch the player off with the cable in. Now: a short press
 * exits the session (as the screen says), a ~2 s hold exits AND powers off.
 * Time-based, because the MSC loop's pass rate is not the UI's. */
static int s_usb_power_off;

static int usb_power_step(void)
{
    static unsigned long t0;
    static int down;

    if (keys_power()) {
        if (!down) {
            down = 1;
            t0 = ap_ms();
        }
        if ((unsigned long)(ap_ms() - t0) >= 2000UL) {
            s_usb_power_off = 1;      /* long hold: leave and turn off */
            return 1;
        }
        return 0;                     /* keep serving while it is held */
    }
    if (down) {
        down = 0;                     /* short press: just exit the session */
        return 1;
    }
    return 0;
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
    line(1, "eject on the PC, hold power to turn off");
    display_flush_rows(0, DISPLAY_H - 1);
    r = usb_msc_run(luns, 2, usb_power_step, &rd, &wr);
    fs_target_mount(0, "Internal", emmc_read, s_user);   /* host may have written */
    snprintf(s_usb, sizeof s_usb, "x%d r%uM w%uM", r, (unsigned)(rd >> 11), (unsigned)(wr >> 11));
    /* Session forensics: how it ended and which SCSI opcode/LUN was last on
     * the wire. When a host-side tool misbehaves (Nautilus freezing on
     * connect, say) this is the device's half of the story, and the flush
     * writes it to /.rechord/LOG for the PC to read. */
    log_printf("usb: exit %d diag %08lx rd %lu wr %lu",
               r, (unsigned long)usb_msc_diag, (unsigned long)rd,
               (unsigned long)wr);
    (void)log_flush("usb-exit");
    {
        int powered_off = s_usb_power_off;

        if (powered_off) {
            /* Held power: switch off now (USB storage is already stopped above,
             * so the PC sees a clean disconnect first). On VBUS the SoC itself
             * stays powered by the host — the rail cut only takes effect on
             * battery. */
            s_usb_power_off = 0;
            resume_poll(ap_ms(), 1);  /* persist the last position first */
            settings_screen_store();
            (void)log_flush("power");
            power_off();              /* may return on VBUS (park) */
        }
        while (keys_power())          /* don't leak the exit press */
            panel_io_delay_ms(1);

        /* Scan on ANY non-power-off exit. The old reason list (EJECT/UNPLUG/
         * USER) missed NO_HOST/SUSPEND/IO, so a session that ended without the
         * host resetting the bus skipped the scan, left the previous screen up
         * and made flashing impossible (field report). */
        if (!powered_off) {
            log_printf("usb: exit %d, scanning for an update", r);
            update_prompt();
        }
    }
    ui_invalidate_all();
}

static void open_files(void)
{
    /* Keep the folder restored by the resume boot (and the navigation memory);
     * only establish the BACK clamp. The first open starts at the root. */
    if (browser_screen_state()->path[0] == '\0')
        browser_screen_set_root("/");
    else
        browser_screen_set_root_keep_path("/");
    ui_push_screen(&browser_screen);
}
static void open_player(void)   { ui_push_screen(&player_screen); }
static void open_settings(void) { settings_screen_reload(); ui_push_screen(&settings_screen); }

static const char *keys_text(void)
{
    static char buf[24];
    snprintf(buf, sizeof buf, "adc %d pwr %d", s_adc, s_power);
    return buf;
}

#if AP_DEBUG_TOOLS
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
        if (rc == DAC_OK)
            audio_output_fade_in();  /* configure starts silent when fade is on */
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
#endif /* AP_DEBUG_TOOLS */

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

#if AP_CDC_OVERLAY
/* Menu action: load /Internal/.rechord/cdc.bin into 0x0304AE64 and run the
 * CDC console. The UI is already initialized and Internal is mounted. */
static void cdc_overlay_run(void)
{
    screen("CDC overlay");
    line(1, "loading...");
    display_flush_rows(0, DISPLAY_H - 1);
    (void)ap_cdc_overlay_run();
    ui_invalidate_all();
}
#endif

/* ---- System: the explicit diagnostic view (BRNOMT-72) -------------------- */
/* Everything that used to clutter the root menu (hardware probes, USB console,
 * index maintenance) lives here. Same list mechanics as every other menu, but
 * with its own view: the shared menu_screen holds ONE model/view, so pushing it
 * twice would clobber the root's cursor. */
static const menu_item_t k_sys_items[] = {
#if AP_DEBUG_TOOLS
    { "DAC test",      NULL,       dac_test_run },
    { "USB console",   NULL,       usb_console_run },
#endif
#if AP_CDC_OVERLAY
    { "CDC overlay",   NULL,       cdc_overlay_run },
#endif
    { "Rebuild index", NULL,       index_rebuild_run },
    { "Index",         index_text, NULL },
    { "Keys",          keys_text,  NULL },
    { "eMMC",          emmc_text,  NULL },
    { "SD",            sd_text,    NULL },
    { "USB",           usb_text,   NULL },
    { "USB diag",      usbd_text,  NULL },
};
#define SYS_ITEM_COUNT ((int)(sizeof k_sys_items / sizeof k_sys_items[0]))

static menu_view_t s_sys_view;
static const menu_model_t k_sys = { "System", k_sys_items, SYS_ITEM_COUNT, NULL };

static void system_screen_on_enter(void)
{
    menu_view_init(&s_sys_view);
    theme_ui_disarm();
}

static void system_screen_draw(void)
{
    menu_draw(&k_sys, &s_sys_view);
}

static void system_screen_handle_event(const input_event_t *ev)
{
    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;                       /* listings act on PRESS/REPEAT only */

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_sys_view, -1, SYS_ITEM_COUNT, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_sys_view, +1, SYS_ITEM_COUNT, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_sys_view.cursor >= 0 && s_sys_view.cursor < SYS_ITEM_COUNT &&
            k_sys_items[s_sys_view.cursor].on_select != 0)
            k_sys_items[s_sys_view.cursor].on_select();
        ui_invalidate_all();
        break;
    case KEY_BACK:
        ui_pop_screen();
        break;
    default:
        break;
    }
}

static const ui_screen_t system_screen_entry = {
    "system", system_screen_on_enter, NULL, system_screen_draw,
    system_screen_handle_event,
};

static void open_system(void) { ui_push_screen(&system_screen_entry); }

/* ---- root menu ---------------------------------------------------------- */
/* Title-row status (BRNOMT-72/83): volume, plus the battery approximation when
 * the gauge answers (5-step ladder -> 0/25/50/75/100 %). */
static const char *vol_status(void)
{
    static char buf[24];
    int pct = battery_percent();

    if (pct >= 0)
        snprintf(buf, sizeof buf, "%d dB %d%%", player_get_volume(), pct);
    else
        snprintf(buf, sizeof buf, "%d dB", player_get_volume());
    return buf;
}

static const menu_item_t k_root_items[] = {
    { "Now Playing", NULL, open_player },
    { "Library",     NULL, open_library },
    { "Files",       NULL, open_files },
    { "USB storage", NULL, usb_storage_entry },
    { "Settings",    NULL, open_settings },
    { "System",      NULL, open_system },
};
#define ROOT_ITEM_COUNT ((int)(sizeof k_root_items / sizeof k_root_items[0]))

#define RC_BUILD "m9i"
static const menu_model_t k_root = { "ReChord " RC_BUILD, k_root_items,
                                     ROOT_ITEM_COUNT, vol_status };

/* Keep the stack's root pair in sync with the active package: the classic menu
 * is always the root, and the skin home is inserted above it only while the
 * package declares home=skin. Called at boot and after a live theme change;
 * Settings (the top screen) is never disturbed. */
static void home_stack_sync(void)
{
    int want = (theme_layout()->home == THEME_HOME_SKIN);
    int i, found = -1;

    for (i = 0; i < ui_stack_depth(); i++)
        if (ui_stack_at(i) == &skin_home_screen) {
            found = i;
            break;
        }

    if (want && found < 0 && ui_stack_depth() >= 1)
        (void)ui_insert_screen(1, &skin_home_screen);
    else if (!want && found >= 0)
        (void)ui_remove_screen(found);
    ui_invalidate_all();
}

/* ---- resume state (BRNOMT-87) ------------------------------------------- */
/* Deferred write, same idea as the volume: a change arms a short quiet timer so
 * a burst of navigation is ONE save, and while playing the position is
 * refreshed at most every RESUME_SAVE_MS (a power cut loses at most that). */
#define RESUME_SAVE_MS  60000u
#define RESUME_QUIET_MS 2000u

static resume_state_t s_res;
static unsigned s_res_due;          /* 0 = nothing armed */
static unsigned s_res_saved_ms;

static int text_eq(const char *a, const char *b)
{
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

static void text_copy(char *dst, unsigned cap, const char *src)
{
    unsigned i = 0;

    if (cap == 0)
        return;
    while (src[i] != '\0' && i + 1u < cap) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/* Capture the player + browser state; write when it moved, or when `force`
 * (the power paths call it with force=1 so the last position is on disk). */
static void resume_poll(unsigned now, int force)
{
    track_info_t ti;
    const browser_state_t *bs;
    int moved = 0;

    if (player_get_track(&ti) == PLAYER_OK && ti.path[0] != '\0') {
        unsigned long pos = (unsigned long)player_get_position_ms();

        if (!text_eq(s_res.track, ti.path)) {
            text_copy(s_res.track, (unsigned)sizeof s_res.track, ti.path);
            s_res.pos_ms = pos;
            moved = 1;
        } else if (pos != s_res.pos_ms) {
            /* While playing: one refresh per RESUME_SAVE_MS. Paused/stopped:
             * capture the position the moment it changes. */
            if (player_get_state() != PLAYER_PLAYING ||
                (unsigned long)(now - s_res_saved_ms) >= RESUME_SAVE_MS) {
                s_res.pos_ms = pos;
                moved = 1;
            }
        }
    }

    bs = browser_screen_state();
    if (bs != 0 && bs->mounted) {
        if (!text_eq(s_res.folder, bs->path) || s_res.row != bs->cursor) {
            text_copy(s_res.folder, (unsigned)sizeof s_res.folder, bs->path);
            s_res.row = bs->cursor;
            moved = 1;
        }
    }

    /* Playback list origin (folder/category), so the queue can be rebuilt on
     * the next boot. The folder kind needs no value: the queue folder is the
     * parent of the restored track. */
    {
        int qkind = 0;
        int qcat = 0;
        char qval[RESUME_QUEUE_VALUE_MAX];

        player_get_queue_source(&qkind, qval, (int)sizeof qval, &qcat);
        if (qkind != s_res.queue_kind || qcat != s_res.queue_cat ||
            !text_eq(qval, s_res.queue_value)) {
            s_res.queue_kind = qkind;
            s_res.queue_cat = qcat;
            text_copy(s_res.queue_value, (unsigned)sizeof s_res.queue_value,
                      qval);
            moved = 1;
        }
    }

    if (moved || force)
        s_res_due = force ? now : now + RESUME_QUIET_MS;

    if (s_res_due != 0 && (int)(now - s_res_due) >= 0) {
        static int s_save_failed;   /* log once per failure run */

        if (resume_save(&s_res) != 0) {
            if (!s_save_failed)
                log_printf("resume: save failed");
            s_save_failed = 1;
        } else {
            s_save_failed = 0;
        }
        s_res_saved_ms = now;
        s_res_due = 0;
    }
}

/* Directory of a device path ("/SD/Music/t.flac" -> "/SD/Music"). */
static int path_parent(const char *path, char *out, int cap)
{
    int i = (int)strlen(path);

    if (path[0] != '/' || cap <= 1)
        return -1;
    while (i > 0 && path[i] != '/')
        i--;
    if (i <= 0) {
        out[0] = '/';
        out[1] = '\0';
        return 0;
    }
    if (i >= cap)
        i = cap - 1;
    memcpy(out, path, (size_t)i);
    out[i] = '\0';
    return 0;
}

/* Rebuild the saved playback queue from the media index and load it with the
 * current track at `start`, leaving the player STOPPED. Returns 1 on success.
 * The list is identified by its origin (folder/category + value), not by a
 * 2 KB dump of indices, so it survives index rebuilds. */
static int restore_saved_queue(const resume_state_t *st)
{
    char folder[RESUME_FOLDER_MAX];
    const uint16_t *list;
    unsigned n = 0;
    int start = -1;
    int rc;

    if (st->track[0] != '/')
        return 0;
    if (st->queue_kind == 1) {
        if (path_parent(st->track, folder, (int)sizeof folder) != 0)
            return 0;
        if (mq_folder_init_start(folder, st->track, &start) != 0)
            return 0;
    } else if (st->queue_kind == 2) {
        if (mq_filter_init_start(st->queue_cat,
                                 st->queue_value[0] ? st->queue_value : 0,
                                 st->track, &start) != 0)
            return 0;
    } else {
        return 0;
    }

    list = mq_filter_indices(&n);
    if (start < 0 || list == 0 || n == 0) {
        mq_done();
        return 0;
    }
    rc = player_load_list(list, (int)n, start);
    mq_done();                       /* the player copied the indices */
    return rc == PLAYER_OK;
}

/* BRNOMT-87: restore where the user left off, per `resume`:
 *   off    -> nothing
 *   folder -> the browser's saved folder/row (as state; boot stays on the
 *             Main UI, and Files opens there)
 *   track  -> that, plus the last track opened at pos_ms
 * Never autoplays: the restored track is opened + seeked and left STOPPED
 * (shown as "Paused"); SELECT/PLAY resumes from there.
 * BRNOMT-102: the old path pushed the browser here, so every boot landed in
 * the File Explorer instead of the active theme's home. */
static void resume_boot_restore(void)
{
    resume_state_t st;
    int mode = settings_screen_get()->resume_mode;
    int have_state = (resume_load(&st) == 0);
    uint32_t seed;

    if (!have_state)
        resume_default(&st);

    /* BRNOMT: the shuffle PRNG was never seeded, so every boot produced the
     * same order (xs32's fixed fallback). Advance a persistent boot counter
     * and seed from it, plus the boot ms/ADC noise as a second source. */
    st.boot++;
    seed = (uint32_t)(st.boot * 2654435761UL);
    seed ^= (uint32_t)(ap_ms() + 1u) * 2246822519u;
    seed ^= (uint32_t)(battery_mv() + 1) << 16;
    player_seed_shuffle(seed);

    /* Keep the counter even when resume is off; a later resume_poll save must
     * carry it forward (s_res.boot), never write it back to zero. */
    s_res.boot = st.boot;
    if (have_state) {
        if (resume_save(&st) != 0)
            log_printf("resume: boot counter save failed");
    } else {
        log_printf("resume: no state file");
    }

    if (mode <= 0)
        return;
    if (!have_state)
        return;
    /* Field diagnostics (numbers first so a long path cannot truncate them). */
    log_printf("resume: mode %d row %d pos %lu track '%s'",
               mode, st.row, st.pos_ms, st.track);
    (void)log_flush("resume-a");       /* breadcrumb: restore starts here   */

    if (st.folder[0] == '/') {
        browser_screen_set_root("/");
        browser_screen_open_at(st.folder, st.row);
        /* Do NOT push: boot lands on the Main UI (BRNOMT-102). The folder and
         * row stay loaded, so Files opens exactly where the user left off. */
        log_printf("resume: browser state '%s' row %d", st.folder, st.row);
    }
    if (mode >= 2 && st.track[0] == '/') {
        int restored = (st.queue_kind != 0) ? restore_saved_queue(&st) : 0;

        if (restored) {
            if (st.pos_ms > 0)
                (void)player_seek((long)st.pos_ms);
            player_set_queue_source(st.queue_kind,
                                    st.queue_kind == 2 ? st.queue_value : "",
                                    st.queue_cat);
            /* Deliberately NOT pushing Now Playing: the user lands on the
             * home and SELECT resumes from the saved position. */
            log_printf("resume: queue restored, paused");
        } else if (player_open(st.track) == PLAYER_OK) {
            if (st.pos_ms > 0)
                (void)player_seek((long)st.pos_ms);
            log_printf("resume: track loaded, paused");
        } else {
            log_printf("resume: open failed '%s'", st.track);
        }
    }
    (void)log_flush("resume-b");       /* breadcrumb: restore finished      */

    /* Seed the comparison state so the first poll does not re-save it. */
    text_copy(s_res.track, (unsigned)sizeof s_res.track, st.track);
    text_copy(s_res.folder, (unsigned)sizeof s_res.folder, st.folder);
    s_res.pos_ms = st.pos_ms;
    s_res.row = st.row;
    s_res.queue_kind = st.queue_kind;
    s_res.queue_cat = st.queue_cat;
    text_copy(s_res.queue_value, (unsigned)sizeof s_res.queue_value,
              st.queue_value);
    s_res_saved_ms = ap_ms();
}

void ap_ui_loop(void)
{
    input_event_t ev;

    emmc_probe();
    player_init();       /* register wav/mp3/flac codecs (registry) */
    player_set_resolver(media_resolve);   /* BRNOMT-90: list index -> path */
    mq_set_pump(player_process);  /* keep audio fed during library scans */
    ui_init();
    ap_clock_init();     /* ms clock for the input hold ladder */
    /* Active package: the cfg `theme=` id wins; older configs (no key) fall
     * back to /.rechord/active_theme, then to Mono. Settings reload first
     * because the id lives in rechord.cfg. */
    settings_screen_reload();
    {
        const char *theme_id = settings_screen_get()->theme;
        int applied = 0;

        if (theme_id[0] != '\0') {
            applied = (theme_pkg_apply(RECHORD_THEMES_DIR, theme_id) == 0);
            if (!applied)
                log_printf("theme: cfg id '%s' rejected", theme_id);
        }
        if (!applied)
            (void)theme_pkg_boot(RECHORD_THEMES_DIR, RECHORD_ACTIVE_THEME,
                                 "mono");
    }
    log_printf("boot: %s theme '%s' home %d nav %d skin v%d '%s'", RC_BUILD,
               theme_pkg_active_id(), theme_layout()->home,
               theme_layout()->home_nav, THEME_SKIN_ENGINE,
               theme_layout()->skin);
    /* Volume is runtime state: the player owns it while playing, so load the
     * saved value into the player here and let the loop below persist later
     * changes. The settings view was already reloaded for the theme id. */
    player_set_volume(settings_screen_get()->volume_db);
    player_set_shuffle(settings_screen_get()->shuffle);           /* BRNOMT-90 */
    player_set_repeat(settings_screen_get()->repeat_mode);
    player_set_fade_ms(settings_screen_get()->fade_ms);           /* BRNOMT-67 */
    player_screen_set_volume(player_get_volume());
    s_volume_seen = player_get_volume();
    battery_poll();      /* BRNOMT-83: seed the gauge before the first draw */
    menu_screen_bind(&k_root);
    ui_push_screen(&menu_screen);
    /* The package decides the home: a menu-only theme keeps the classic list;
     * one that declares home=skin gets the declarative home inserted above it
     * (BACK still reaches Files/USB/System). */
    skin_home_set_actions(open_library, open_player, open_settings);
    settings_screen_set_theme_applied(home_stack_sync);
    home_stack_sync();
    /* BRNOMT-87: the resume restore runs inside the loop, AFTER the first
     * sd_probe() — opening a /SD track before the volume is mounted fails
     * (field log: "resume: open failed" at 0.021 s). */
    for (;;) {
        unsigned now;
        int cable;
        int pev;
        int menu_now;
        int sampled = 0;

        /* BRNOMT-84: run the loop once per 1 ms SysTick and sleep in WFI.
         * The audio IRQ can still wake us; the heartbeat check avoids an
         * ap_ms() division on every audio interrupt. */
        if (s_tick_seq == s_loop_seq) {
            __asm__ volatile("wfi");
            continue;
        }
        s_loop_seq = s_tick_seq;
        now = ap_ms();

        /* BRNOMT-98: the SARADC is stepped from the SysTick hook, so this only
         * collects a finished scan at the same 5 ms cadence; it never waits on
         * the converter. If the async sampler ever stops completing, fall back
         * to the old blocking triple conversion (which also pauses the
         * sampler's use of the shared ADC). */
        if ((unsigned)(now - s_keys_ms) >= KEYS_SCAN_MS) {
            int adc = 0;
            int pw = 0;

            if (!s_keys_fallback && keys_scan_take(&adc, &pw)) {
                s_keys_ms = now;
                s_key_adc = adc;
                s_key_power = pw;
                sampled = 1;
                /* 5 ms samples: 3 samples = 15 ms (unchanged debounce). */
                input_set_vol_debounce(3);
                input_scan(s_key_adc, s_key_power);
            } else if (!s_keys_fallback && keys_scan_failed()) {
                s_keys_fallback = 1;
                log_printf("keys: async scan failed, polling fallback");
            }
            if (s_keys_fallback) {
                s_keys_ms = now;
                s_key_adc = keys_adc();
                s_key_power = keys_power();
                sampled = 1;
                input_set_vol_debounce(3);
                input_scan(s_key_adc, s_key_power);
            }
        }
        cable = usb_cable();

        if (sd_present() != s_sd_in) {   /* card inserted / removed */
            s_sd_in = sd_present();
            if (s_sd_in)
                panel_io_delay_ms(300);  /* let the contacts settle */
            sd_probe();
            if (!s_screen_off)
                ui_invalidate_all();
        }

        /* BRNOMT-87: restore once the first probe has settled. Time-based now
         * that the loop is 1 ms/tick (it used to be 3 passes). */
        if (!s_resume_done && s_sd_in >= 0) {
            if (!s_resume_started) {
                s_resume_started = 1;
                s_resume_ms = now;
            } else if ((unsigned)(now - s_resume_ms) >= 50u) {
                s_resume_done = 1;
                resume_boot_restore();
            }
        }

        /* VBUS edge: plugging the cable opens USB STORAGE right away. While
         * Hold is engaged the cable only charges; the combo must unlock. */
        if (cable && !s_cable) {
            if (!s_hold) {
                screen_wake();
                usb_storage_entry();
            }
        }
        s_cable = cable;

        /* BRNOMT-75: Power + MENU chord. input_scan() prioritizes the power
         * GPIO and would never report the MENU press, so detect it from the
         * raw ladder sample; two stable samples reject a transient. */
        menu_now = input_map_adc(s_key_adc) == KEY_MENU;
        if (sampled && s_key_power && menu_now) {
            if (s_combo_menu_scans < 2)
                s_combo_menu_scans++;
        } else if (!s_key_power || !menu_now) {
            s_combo_menu_scans = 0;
        }
        if (s_key_power && s_combo_menu_scans >= 2 && !s_combo_latch) {
            s_power_combo = 1;
            s_combo_latch = 1;
            s_combo_menu_scans = 0;
            hold_set(!s_hold);
            input_mock_clear();
        }
        if (s_combo_latch && !s_key_power && !menu_now) {
            s_combo_latch = 0;
            /* Keep s_power_combo set: power_hold_step() must see it on the
             * debounced release and suppress the short screen toggle. */
            input_mock_clear();
        }

        while (input_poll(&ev) > 0) {
            /* All four kinds reach the top screen (BRNOMT-93). Hold drops
             * everything except the chord handled above. */
            if (s_hold || s_combo_latch)
                continue;
            if (ev.type == KEY_POWER)
                continue;              /* global power handling below */
            /* Screen-off is display-only: keys keep working, but they must
             * not light the panel. Only a short Power press (or unlocking
             * Hold / USB insertion) wakes it. */
            if (ev.type == KEY_MENU) {
                /* BRNOMT-105: MENU opens Quick settings instead of jumping
                 * to the root. MENU again (or BACK) closes whichever quick
                 * overlay is on top. A dark panel wakes first. */
                if (s_screen_off)
                    screen_wake();
                if (ui_top_screen() == &quick_playlist_screen ||
                    ui_top_screen() == &quick_screen)
                    ui_pop_screen();
                else
                    (void)ui_push_screen(&quick_screen);
            } else {
                theme_ui_note_input();
                ui_handle_event(&ev);
            }
        }

        /* Short power = screen off/on; ~2 s = power off. Only on a fresh key
         * sample: the debounce counts samples, so calling this every 1 ms pass
         * would turn its 3-sample (15 ms) debounce into 3 ms. The HOLD check
         * is time-based, so a 5 ms cadence still fires it on time. */
        pev = sampled ? power_hold_step(s_key_power, now) : POWER_EV_NONE;
        if (pev == POWER_EV_HOLD) {
            resume_poll(now, 1);       /* persist the last position first */
            settings_screen_store();
            (void)log_flush("power");
            power_off();
        } else if (pev == POWER_EV_SHORT) {
            if (!s_hold) {
                if (s_screen_off)
                    screen_wake();
                else
                    screen_sleep();
            }
        }

        /* live "Keys" row on the root menu: hardware check without a log */
        if ((s_key_adc - s_adc > 8 || s_adc - s_key_adc > 8 ||
             s_key_power != s_power) &&
            !s_screen_off && ui_stack_depth() == 1) {
            ui_invalidate_all();
        }
        s_adc = s_key_adc;
        s_power = s_key_power;

        if (ui_stack_depth() == 0) {     /* BACK on the root menu */
            ui_push_screen(&menu_screen);
            home_stack_sync();
        }

        /* Volume is owned by the player module while playing, so watch it
         * here rather than in the player screen (which has no clock): adopt
         * a change into the settings view and let the screen write it once
         * the keys have been quiet (SETTINGS_FLUSH_QUIET_MS), so a volume
         * sweep is one save instead of one per step. */
        if (player_get_volume() != s_volume_seen) {
            s_volume_seen = player_get_volume();
            settings_screen_volume_changed(s_volume_seen, now);
        }
        settings_screen_flush_pending(now);
        resume_poll(now, 0);      /* BRNOMT-87: deferred position save */

        /* Battery gauge (BRNOMT-83): one slow poll (the SARADC is shared with
         * the keys); repaint the title row only when the step changed. */
        if ((unsigned)(now - s_batt_ms) >= 15000u) {
            int before = battery_percent();

            battery_poll();
            s_batt_ms = now;
            if (battery_percent() != before && !s_screen_off)
                ui_invalidate(0, 0, DISPLAY_W - 1, 16);
        }

        if (!s_screen_off) {
            /* Skin home: the clock feeds the theme's expressions; refresh
             * only the animated band the package declares. */
            skin_home_tick(now);
            theme_ui_tick(now);
            if (theme_layout()->home == THEME_HOME_SKIN &&
                ui_top_screen() == &skin_home_screen &&
                theme_skin_animated() &&
                (unsigned)(now - s_face_ms) >=
                    (unsigned)theme_skin_refresh_ms()) {
                int h = theme_skin_refresh_h();

                s_face_ms = now;
                if (h <= 0)
                    h = theme_layout()->face_h;
                if (h <= 0 || h > DISPLAY_H)
                    h = DISPLAY_H;
                ui_invalidate(0, 0, DISPLAY_W - 1, h - 1);
            }

            /* Per-screen skins: only animated ones wake the loop; a static
             * Now Playing/Library/Settings/Browser costs nothing per pass. */
            if (theme_ui_active() && ui_top_screen() != &skin_home_screen &&
                theme_skin_animated() &&
                (unsigned)(now - s_theme_ms) >=
                    (unsigned)theme_skin_refresh_ms()) {
                int h = theme_skin_refresh_h();

                s_theme_ms = now;
                if (h <= 0)
                    h = theme_layout()->face_h;
                if (h <= 0 || h > DISPLAY_H)
                    h = DISPLAY_H;
                ui_invalidate(0, 0, DISPLAY_W - 1, h - 1);
            }
        }

        /* Persist the RAM log without waiting for the USB cable (see the flush
         * policy above): only when there is something new, and eagerly before
         * the ring starts dropping lines. An idle player writes nothing. */
        if (log_pending() > 0 &&
            ((unsigned)(now - s_log_ms) >= LOG_FLUSH_MS ||
             log_pending() >= LOG_FLUSH_EAGER)) {
            (void)log_flush("auto");
            s_log_ms = now;              /* do not retry-storm on failure   */
        }

        /* audio pump: the ring/IRQ feeder is non-blocking once full. The loop
         * now runs at 1 kHz, so 8 rounds per tick is plenty. */
        bfs_poll();
        if (player_get_state() == PLAYER_PLAYING) {
            int n;

            for (n = 0; n < 8 && player_get_state() == PLAYER_PLAYING; n++)
                (void)player_process();
        } else {
            (void)player_process();
        }

        /* Only the visible screen needs a UI refresh; a dark panel skips all
         * drawing. Now Playing syncs at 10 Hz (its on_enter does the immediate
         * sync when it is pushed). */
        if (!s_screen_off) {
            if (ui_top_screen() == &player_screen &&
                (unsigned)(now - s_player_ms) >= PLAYER_UI_REFRESH_MS) {
                s_player_ms = now;
                player_screen_refresh();
            }
            ui_render();
        }
    }
}
