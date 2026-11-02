/*
 * bt_link.c — Bluetooth policy.
 *
 * Owns the user's on/off choice, auto-connect, the audio route, the stream
 * on/off that follows the player, the headphone buttons, the idle power-off
 * and the persisted device list. All decisions are taken here so the protocol
 * module stays a pure state machine.
 *
 * Timings follow the field notes in docs/rewrite/bluetooth.md.
 */
#include <string.h>

#include "bt_link.h"
#include "bt_store.h"
#include "../drivers/bt_hw.h"
#include "log.h"
#include "../player/player.h"

/* The player is linked in the app image; the host tests provide their own.
 * Weak so this policy can be tested without the whole player stack. */
extern int player_get_state(void) __attribute__((weak));
extern int player_play(void) __attribute__((weak));
extern int player_pause(void) __attribute__((weak));
extern int player_next(void) __attribute__((weak));
extern int player_prev(void) __attribute__((weak));

#define BT_REMOTE_GUARD_MS 3000UL
#define BT_RX_PER_SERVICE  256
#define BT_FRAMES_PER_SERVICE 4
#define BT_CONNECT_LIMIT_MS 30000UL   /* give up a stuck device switch */

static bt_proto_t s_bt;
static unsigned long s_last_attempt;
static unsigned long s_ignore_remote_until;
static unsigned long s_idle_since;
static unsigned long s_pending_at;
static int8_t  s_pending_target;
static uint8_t s_init;
static uint8_t s_on;
static uint8_t s_user_disconnected;
static uint8_t s_screen_open;
static uint8_t s_display_on = 1;
static uint8_t s_dirty;
static uint8_t s_stream_wanted;
static uint8_t s_idle_off;   /* module asleep until the screen wakes it */
static bt_state_t s_last_state;
static uint8_t s_remote_plays;
static uint8_t s_remote_pauses;
static uint8_t s_remote_nexts;
static uint8_t s_remote_prevs;
static uint8_t s_known_count;

static void send_frame(const bt_frame_t *f)
{
    if (f != 0 && f->len > 0)
        (void)bt_hw_write(f->bytes, f->len);
}

static void save_devices(void)
{
    bt_store_t st;
    int i, n = 0;

    memset(&st, 0, sizeof st);
    st.on = s_on;
    /* Only user-chosen devices are remembered; scan results are ephemeral. */
    for (i = 0; i < s_bt.count && n < BT_DEV_MAX; i++) {
        if (bt_proto_is_saved(&s_bt, i))
            st.devices[n++] = s_bt.devices[i];
    }
    st.count = (uint8_t)n;
    if (bt_store_save(&st) != 0)
        log_printf("bt: device save failed");
    s_dirty = 0;                       /* never retry every service pass */
}

static int player_playing(void)
{
    return player_get_state != 0 && player_get_state() == PLAYER_PLAYING;
}

static void module_on(unsigned long now)
{
    bt_hw_power_on();
    bt_proto_power_on(&s_bt, now);
    s_last_state = BT_STATE_STARTING;
    s_idle_since = now;
    s_last_attempt = now;
    log_printf("bt: on");
}

static void module_off(void)
{
    bt_hw_route(0);
    bt_hw_power_off();
    bt_proto_power_off(&s_bt);
    s_last_state = BT_STATE_OFF;
    s_stream_wanted = 0;
    s_pending_target = -1;
}

/* ---- state changes ------------------------------------------------------ */

static void on_connected(unsigned long now)
{
    if (s_bt.connected >= 0) {
        bt_proto_mark_saved(&s_bt, s_bt.connected);
        bt_proto_promote_device(&s_bt, s_bt.connected);
    }
    {
        const bt_device_t *d = bt_proto_connected(&s_bt);
    s_dirty = 1;
    s_user_disconnected = 0;
    s_last_attempt = now;
    s_idle_since = now;
    s_ignore_remote_until = now + BT_REMOTE_GUARD_MS;
        log_printf("bt: connected %s", d != 0 ? d->name : "?");
    }
}

static void on_link_lost(unsigned long now)
{
    /* A dropped link pauses playback (the headphones are gone). */
    if (s_stream_wanted && player_pause != 0) {
        (void)player_pause();
        log_printf("bt: link lost, playback paused");
    }
    s_stream_wanted = 0;
    s_last_attempt = now;
    s_ignore_remote_until = now + BT_REMOTE_GUARD_MS;
    log_printf("bt: disconnected");
}

static void follow_state(unsigned long now)
{
    if (s_bt.state == s_last_state)
        return;
    if (s_bt.state == BT_STATE_CONNECTED)
        on_connected(now);
    else if (s_last_state == BT_STATE_CONNECTED)
        on_link_lost(now);
    if (s_bt.state == BT_STATE_READY)
        log_printf("bt: ready v%c", s_bt.version != 0 ? s_bt.version : '?');
    s_last_state = s_bt.state;
}

/* ---- remote buttons ----------------------------------------------------- */

static void handle_remote(unsigned long now)
{
    uint8_t plays = s_bt.remote_plays;
    uint8_t pauses = s_bt.remote_pauses;
    uint8_t nexts = s_bt.remote_nexts;
    uint8_t prevs = s_bt.remote_prevs;

    if (now < s_ignore_remote_until)
        return;
    if (nexts != s_remote_nexts) {
        if (player_next != 0)
            (void)player_next();
        log_printf("bt: remote next");
        s_ignore_remote_until = now + BT_REMOTE_GUARD_MS;
    } else if (prevs != s_remote_prevs) {
        if (player_prev != 0)
            (void)player_prev();
        log_printf("bt: remote prev");
        s_ignore_remote_until = now + BT_REMOTE_GUARD_MS;
    } else if (pauses != s_remote_pauses) {
        if (player_pause != 0)
            (void)player_pause();
        log_printf("bt: remote pause");
        s_ignore_remote_until = now + BT_REMOTE_GUARD_MS;
    } else if (plays != s_remote_plays) {
        if (player_play != 0)
            (void)player_play();
        log_printf("bt: remote play");
        s_ignore_remote_until = now + BT_REMOTE_GUARD_MS;
    }
    s_remote_plays = s_bt.remote_plays;
    s_remote_pauses = s_bt.remote_pauses;
    s_remote_nexts = s_bt.remote_nexts;
    s_remote_prevs = s_bt.remote_prevs;
}

/* ---- service ------------------------------------------------------------ */

static void attempt_connect(unsigned long now, int index)
{
    bt_frame_t f;

    if (index < 0 || index >= s_bt.count)
        return;
    if (bt_proto_connect(&s_bt, index, now, &f)) {
        send_frame(&f);
        s_last_attempt = now;
        log_printf("bt: connecting %s", s_bt.devices[index].name);
    }
}

void bt_link_service(unsigned long now)
{
    int n;

    if (!s_init || !s_on)
        return;
    if (!bt_hw_powered()) {
        /* Back from a USB park (not from an idle-off sleep). */
        if (!s_idle_off)
            module_on(now);
        return;
    }

    for (n = 0; n < BT_RX_PER_SERVICE; n++) {
        int c = bt_hw_read();

        if (c < 0)
            break;
        bt_proto_receive(&s_bt, (uint8_t)c, now);
    }
    for (n = 0; n < BT_FRAMES_PER_SERVICE; n++) {
        bt_frame_t f;

        if (!bt_proto_poll(&s_bt, now, &f))
            break;
        send_frame(&f);
    }

    follow_state(now);
    if (s_bt.count > s_known_count) {
        const bt_device_t *d = &s_bt.devices[s_bt.count - 1];

        log_printf("bt: found %s", d->name[0] != '\0' ? d->name : "?");
        s_known_count = s_bt.count;
    }

    /* Stream follows the player while a link is up. */
    {
        int want = s_bt.state == BT_STATE_CONNECTED && player_playing();

        if (want != s_stream_wanted) {
            bt_frame_t f;

            if (bt_proto_set_stream(&s_bt, want, now, &f))
                send_frame(&f);
            s_stream_wanted = want;
            s_ignore_remote_until = now + BT_REMOTE_GUARD_MS;
        }
    }
    bt_hw_route(s_bt.state == BT_STATE_CONNECTED && s_bt.streaming);

    handle_remote(now);

    /* Switch to a chosen device after the 2 s gap. */
    if (s_pending_target >= 0) {
        if (s_bt.state == BT_STATE_READY && now - s_pending_at >= BT_SWITCH_MS) {
            int target = s_pending_target;

            s_pending_target = -1;
            attempt_connect(now, target);
        } else if (now - s_pending_at >= BT_CONNECT_LIMIT_MS) {
            s_pending_target = -1;     /* the module never dropped the link */
        }
    }

    /* Auto-connect to the most recent device. */
    if (s_bt.state == BT_STATE_READY && s_bt.count > 0 &&
        !s_user_disconnected && s_pending_target < 0 &&
        now - s_last_attempt >= BT_RETRY_MS) {
        int i;

        for (i = 0; i < s_bt.count; i++) {
            if (bt_proto_is_saved(&s_bt, i)) {
                attempt_connect(now, i);
                break;
            }
        }
    }

    /* Idle off while nothing is connected and the Bluetooth screen is not
     * actually in front of the user (closed, or the panel is dark). */
    if (s_bt.state != BT_STATE_CONNECTED && (!s_screen_open || !s_display_on) &&
        now - s_idle_since >= BT_IDLE_OFF_MS) {
        module_off();
        s_idle_off = 1;
        log_printf("bt: idle off");
    }

    if (s_dirty)
        save_devices();
}

/* ---- public actions ----------------------------------------------------- */

void bt_link_init(unsigned long now)
{
    bt_store_t st;

    bt_store_load(&st);
    bt_proto_init(&s_bt);
    bt_proto_seed_devices(&s_bt, st.devices, st.count);
    s_on = st.on ? 1 : 0;
    s_init = 1;
    s_last_state = BT_STATE_OFF;
    s_idle_since = now;
    s_pending_target = -1;
    s_known_count = (uint8_t)s_bt.count;
    if (s_on)
        module_on(now);
}

int bt_link_enabled(void)
{
    return s_on;
}

void bt_link_set_enabled(int on, unsigned long now)
{
    s_on = on ? 1 : 0;
    s_idle_off = 0;
    s_dirty = 1;
    s_user_disconnected = 0;
    if (s_on) {
        if (!bt_hw_powered())
            module_on(now);
    } else {
        module_off();
        log_printf("bt: off");
    }
    save_devices();
}

const bt_proto_t *bt_link_state(void)
{
    return &s_bt;
}

void bt_link_screen_opened(unsigned long now)
{
    s_screen_open = 1;
    s_idle_off = 0;
    s_idle_since = now;
    if (s_on && !bt_hw_powered())
        module_on(now);
}

void bt_link_screen_closed(unsigned long now)
{
    s_screen_open = 0;
    s_idle_since = now;
}

void bt_link_set_display(int on, unsigned long now)
{
    on = on ? 1 : 0;
    if (on == s_display_on)
        return;
    s_display_on = (uint8_t)on;
    s_idle_since = now;            /* start a fresh idle window */
}

void bt_link_scan(unsigned long now)
{
    bt_frame_t f;

    if (!s_on)
        return;
    s_idle_off = 0;
    if (!bt_hw_powered())
        module_on(now);
    if (bt_proto_scan(&s_bt, now, &f)) {
        send_frame(&f);
        log_printf("bt: scan");
    }
}

void bt_link_connect(int device, unsigned long now)
{
    if (!s_on || device < 0 || device >= s_bt.count)
        return;
    s_user_disconnected = 0;
    bt_proto_mark_saved(&s_bt, device);
    s_dirty = 1;
    if (s_bt.state == BT_STATE_CONNECTED) {
        bt_frame_t f;

        if (bt_proto_disconnect(&s_bt, now, &f))
            send_frame(&f);
        s_pending_target = (int8_t)device;
        s_pending_at = now;
        return;
    }
    attempt_connect(now, device);
}

void bt_link_disconnect(unsigned long now)
{
    bt_frame_t f;

    s_user_disconnected = 1;
    s_pending_target = -1;
    if (bt_proto_disconnect(&s_bt, now, &f))
        send_frame(&f);
}

void bt_link_forget(int device, unsigned long now)
{
    if (device < 0 || device >= s_bt.count)
        return;
    /* Drop the link or the attempt in flight: removing only the entry would
     * leave the module still connecting to a device we no longer know. (The
     * module has no unpair command, so this is all ReChord can do.) */
    if (s_bt.connected == device || s_bt.target == device)
        bt_link_disconnect(now);
    bt_proto_remove_device(&s_bt, device);
    if (s_pending_target == device)
        s_pending_target = -1;
    else if (s_pending_target > device)
        s_pending_target--;
    /* bt_link_disconnect held auto-connect off for the device just dropped;
     * that device is gone, so the other saved devices may connect again. */
    s_user_disconnected = 0;
    s_dirty = 1;
    save_devices();                    /* a forget persists immediately */
}

void bt_link_power_off(void)
{
    module_off();
    s_idle_off = 0;              /* a park/wake restores Bluetooth */
}

const char *bt_link_status_text(void)
{
    switch (s_bt.state) {
    case BT_STATE_OFF:
        return s_on ? "On" : "Off";
    case BT_STATE_STARTING:
        return "Starting...";
    case BT_STATE_NO_REPLY:
        return "No module";
    case BT_STATE_SCANNING:
        return "Searching...";
    case BT_STATE_CONNECTING:
        return "Connecting...";
    case BT_STATE_CONNECTED: {
        const bt_device_t *d = bt_proto_connected(&s_bt);

        return d != 0 ? d->name : "Connected";
    }
    case BT_STATE_READY:
    default:
        return "Not connected";
    }
}
