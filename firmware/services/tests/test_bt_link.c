/*
 * test_bt_link.c — Bluetooth policy + store.
 *
 * Fake UART (bytes in/out), fake routing, fake player. Covers the on/off
 * choice, auto-connect, connect/stream, remote buttons, forgetting a device
 * and the persisted file round-trip.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "bt_link.h"
#include "bt_store.h"
#include "fs_host.h"
#include "../../drivers/bt_hw.h"
#include "../../player/player.h"

static int s_failures;

#define TEST(name) printf("== %s ==\n", name)

#define CHECK(cond, name) do {                                             \
    if (cond) printf("  PASS: %s\n", name);                                \
    else { printf("  FAIL: %s\n", name); s_failures++; }                    \
} while (0)

#define CHECK_EQI(got, want, name) do {                                    \
    long g_ = (long)(got), w_ = (long)(want);                              \
    if (g_ == w_) printf("  PASS: %s\n", name);                            \
    else { printf("  FAIL: %s (got %ld, want %ld)\n", name, g_, w_);        \
           s_failures++; }                                                \
} while (0)

/* ---- fake hardware ------------------------------------------------------ */

static int s_hw_on;
static int s_routed;
static char s_sent[512];
static unsigned s_sent_len;
static char s_rx[512];
static unsigned s_rx_len, s_rx_pos;

void bt_hw_power_on(void) { s_hw_on = 1; }
void bt_hw_power_off(void) { s_hw_on = 0; }
int  bt_hw_powered(void) { return s_hw_on; }

int bt_hw_write(const uint8_t *bytes, unsigned len)
{
    if (s_sent_len + len <= sizeof s_sent) {
        memcpy(s_sent + s_sent_len, bytes, len);
        s_sent_len += len;
    }
    return 0;
}

int bt_hw_read(void)
{
    if (s_rx_pos >= s_rx_len)
        return -1;
    return (unsigned char)s_rx[s_rx_pos++];
}

void bt_hw_route(int to_module) { s_routed = to_module ? 1 : 0; }
int  bt_hw_routed(void) { return s_routed; }
int  bt_hw_settling(void) { return 0; }
int  bt_audio_pin_override(int port, int pin, int level)
{
    (void)port; (void)pin;
    return level;
}

/* ---- fake player -------------------------------------------------------- */

static int s_player_state = PLAYER_STOPPED;
static int s_plays, s_pauses, s_nexts, s_prevs;

int player_get_state(void) { return s_player_state; }
int player_play(void) { s_plays++; s_player_state = PLAYER_PLAYING; return 0; }
int player_pause(void) { s_pauses++; s_player_state = PLAYER_PAUSED; return 0; }
int player_next(void) { s_nexts++; return 0; }
int player_prev(void) { s_prevs++; return 0; }

/* ---- helpers ------------------------------------------------------------ */

static void reset_io(void)
{
    s_sent_len = 0;
    s_rx_len = s_rx_pos = 0;
    s_sent[0] = '\0';
}

static void inject(const char *text)
{
    while (*text != '\0') {
        if (s_rx_len < sizeof s_rx)
            s_rx[s_rx_len++] = *text;
        text++;
    }
}

/* The policy reads then polls in the same pass, so short frames (CV1) are
 * accepted on a following pass once the line is quiet for 2 ms — exactly how
 * the device service runs. */
static void feed(const char *text, unsigned long now)
{
    inject(text);
    bt_link_service(now);
    bt_link_service(now + 3);
}

static int sent_has(const char *needle, unsigned len)
{
    unsigned i;

    if (len > s_sent_len)
        return 0;
    for (i = 0; i + len <= s_sent_len; i++)
        if (memcmp(s_sent + i, needle, len) == 0)
            return 1;
    return 0;
}

static void fresh(void)
{
    reset_io();
    s_hw_on = 0;
    s_routed = 0;
    s_player_state = PLAYER_STOPPED;
    s_plays = s_pauses = s_nexts = s_prevs = 0;
}

/* ---- tests -------------------------------------------------------------- */

static void test_disabled_by_default(void)
{
    fresh();
    bt_link_init(0);
    CHECK_EQI(bt_link_enabled(), 0, "off by default");
    CHECK_EQI(s_hw_on, 0, "module not powered while off");
    CHECK(strcmp(bt_link_status_text(), "Off") == 0, "status Off");

    bt_link_set_enabled(1, 1000);
    CHECK_EQI(s_hw_on, 1, "enabling powers the module");
    CHECK(strcmp(bt_link_status_text(), "Starting...") == 0,
          "status Starting");
    feed("CV1", 1002);
    CHECK(strcmp(bt_link_status_text(), "Not connected") == 0,
          "CV1 -> Not connected");
}

static void test_persisted_auto_connect(void)
{
    static const char addr[] = "\xAA\xBB\xCC\xDD\xEE\xFF";
    bt_store_t st;
    const bt_device_t *d;

    fresh();
    memset(&st, 0, sizeof st);
    st.on = 1;
    st.count = 1;
    memcpy(st.devices[0].addr, addr, 6);
    strcpy(st.devices[0].name, "Buds");
    CHECK_EQI(bt_store_save(&st), 0, "store saved");

    bt_link_init(0);
    CHECK_EQI(bt_link_enabled(), 1, "on restored from the file");
    CHECK_EQI(s_hw_on, 1, "module powered at init");
    CHECK_EQI(bt_link_state()->count, 1, "device restored");

    reset_io();
    feed("CV1", 100);                  /* module answers quickly */
    reset_io();
    bt_link_service(6000);             /* retry window elapsed -> connect */
    CHECK(sent_has("CL06", 4), "auto-connect sent CL06");
    CHECK(sent_has(addr, 6), "target address sent");

    reset_io();
    feed("CLOK", 6010);
    reset_io();
    feed("CQO10\xAA\xBB\xCC\xDD\xEE\xFF""Buds", 6020);
    CHECK_EQI(bt_link_state()->state, BT_STATE_CONNECTED, "connected");
    CHECK(strcmp(bt_link_status_text(), "Buds") == 0, "status shows name");
    CHECK_EQI(bt_link_state()->count, 1, "device list kept");
    d = &bt_link_state()->devices[0];   /* promoted to front */
    CHECK(strcmp(d->name, "Buds") == 0, "connected device kept in the list");

    /* Stream follows the player. */
    s_player_state = PLAYER_PLAYING;
    reset_io();
    bt_link_service(7000);
    CHECK(sent_has("\x43\x59\x30\x1E", 4), "stream on when playing");
    CHECK_EQI(s_routed, 1, "audio routed to the module");

    s_player_state = PLAYER_PAUSED;
    reset_io();
    bt_link_service(7100);
    CHECK(sent_has("\x43\x59\x30\x00", 4), "stream off when paused");

    /* Remote buttons after the guard window (connect at 6020). */
    reset_io();
    s_player_state = PLAYER_PAUSED;
    feed("CB", 12000);
    CHECK_EQI(s_nexts, 1, "headphone next skips");
    CHECK_EQI(s_pauses, 0, "next did not pause");

    reset_io();
    feed("CC", 16000);
    CHECK_EQI(s_plays, 1, "headphone play resumes");

    /* Forget the connected device. */
    bt_link_forget(0, 17000);
    CHECK_EQI(bt_link_state()->count, 0, "device forgotten");

    /* Persisted again (on=1, no devices). */
    CHECK_EQI(bt_store_load(&st), 0, "store reloads");
    CHECK_EQI(st.on, 1, "on persisted");
    CHECK_EQI(st.count, 0, "forget persisted");
}

static void test_link_lost_pauses(void)
{
    fresh();
    bt_link_init(0);
    bt_link_set_enabled(1, 1000);
    feed("CV1", 1002);
    feed("CQO10\x01\x02\x03\x04\x05\x06""Buds", 2000);
    s_player_state = PLAYER_PLAYING;
    bt_link_service(3000);
    CHECK_EQI(s_pauses, 0, "no pause while linked");

    reset_io();
    feed("CTOK", 4000);
    CHECK_EQI(bt_link_state()->state, BT_STATE_READY, "link dropped");
    CHECK_EQI(s_pauses, 1, "a dropped link pauses playback");
    CHECK_EQI(s_routed, 0, "audio unrouted");
}

static void test_scan_results_are_not_saved(void)
{
    static const char addr[] = "\x11\x22\x33\x44\x55\x66";

    fresh();
    remove(".work/bt/Internal/.rechord/BT");   /* start with no store */
    bt_link_init(0);
    bt_link_set_enabled(1, 1000);
    feed("CV1", 1002);
    CHECK_EQI(bt_link_state()->state, BT_STATE_READY, "ready");

    /* A device found by a scan is listed, but is not remembered yet. */
    feed("CS10" "\x11\x22\x33\x44\x55\x66" "Buds", 2000);
    CHECK_EQI(bt_link_state()->count, 1, "scan result listed");
    CHECK_EQI(bt_proto_is_saved(bt_link_state(), 0), 0,
              "scan result not saved");

    reset_io();
    bt_link_service(8000);             /* retry window, no saved device */
    CHECK(!sent_has("CL06", 4), "no auto-connect to an unknown device");

    /* Choosing it saves it and connects. */
    reset_io();
    bt_link_connect(0, 9000);
    CHECK(sent_has("CL06", 4), "user choice connects");
    CHECK_EQI(bt_proto_is_saved(bt_link_state(), 0), 1, "user choice saved");

    feed("CLOK", 9010);
    feed("CHOK", 9020);
    bt_link_service(9100);
    /* The persisted file now holds the device. */
    {
        bt_store_t st;

        CHECK_EQI(bt_store_load(&st), 0, "store saved");
        CHECK_EQI(st.count, 1, "chosen device persisted");
    }
    (void)addr;
}

static void test_idle_off_with_display(void)
{
    unsigned long t;

    TEST("bt_link: idle-off ignores a Bluetooth screen behind a dark display");

    fresh();
    remove(".work/bt/Internal/.rechord/BT");
    bt_link_init(0);
    bt_link_set_enabled(1, 1000);
    feed("CV1", 1002);
    CHECK_EQI(bt_link_state()->state, BT_STATE_READY, "ready");

    bt_link_screen_opened(2000);
    t = 2000 + BT_IDLE_OFF_MS + 1;
    bt_link_service(t);
    CHECK_EQI(s_hw_on, 1, "screen open + display on: module stays on");

    bt_link_set_display(0, t);
    t += BT_IDLE_OFF_MS + 1;
    bt_link_service(t);
    CHECK_EQI(s_hw_on, 0, "display off + 5 min: module off");
    CHECK_EQI(bt_link_enabled(), 1, "the on/off choice is kept");

    bt_link_set_display(1, t + 1000);
    bt_link_screen_opened(t + 1000);
    CHECK_EQI(s_hw_on, 1, "opening the screen wakes the module");
}


static void seed_two(void)
{
    static const char a1[] = "\xAA\xBB\xCC\xDD\xEE\xFF";
    static const char a2[] = "\x11\x22\x33\x44\x55\x66";
    bt_store_t st;

    fresh();
    memset(&st, 0, sizeof st);
    st.on = 1;
    st.count = 2;
    memcpy(st.devices[0].addr, a1, 6);
    strcpy(st.devices[0].name, "Buds");
    memcpy(st.devices[1].addr, a2, 6);
    strcpy(st.devices[1].name, "Speaker");
    bt_store_save(&st);
    bt_link_init(0);
    reset_io();
    feed("CV1", 100);
}

static void test_wired_headphones_block(void)
{
    static const char a1[] = "\xAA\xBB\xCC\xDD\xEE\xFF";

    seed_two();
    bt_link_set_wired(1, 150);                 /* headphones in the jack */
    reset_io();
    bt_link_service(6000);                     /* auto-connect window elapsed */
    CHECK(!sent_has("CL06", 4), "no auto-connect with wired headphones");
    bt_link_connect(0, 6100);
    CHECK(!sent_has("CL06", 4), "no manual connect with wired headphones");
    CHECK(strcmp(bt_link_status_text(), "Headphones in") == 0,
          "the screen says why");

    bt_link_set_wired(0, 6200);                /* unplugged */
    reset_io();
    bt_link_service(12000);
    CHECK(sent_has("CL06", 4), "auto-connect resumes once unplugged");
    CHECK(sent_has(a1, 6), "to the most recent device");

    /* Plugging in mid-attempt abandons it. */
    reset_io();
    bt_link_set_wired(1, 12100);
    CHECK_EQI(bt_link_state()->state, BT_STATE_READY, "attempt abandoned");
    CHECK(sent_has("CT00", 4), "module told to drop the attempt");
    bt_link_set_wired(0, 12200);
}

static void test_forget_drops_attempt_and_relents(void)
{
    seed_two();
    reset_io();
    bt_link_service(6000);                     /* auto-connecting to Buds */
    CHECK_EQI(bt_link_state()->state, BT_STATE_CONNECTING, "connecting");
    reset_io();
    bt_link_forget(0, 6100);                   /* forget the one in flight */
    CHECK(sent_has("CT00", 4), "forget drops the attempt in flight");
    CHECK_EQI(bt_link_state()->count, 1, "entry removed");
    CHECK(strcmp(bt_link_state()->devices[0].name, "Speaker") == 0,
          "the other device remains");

    /* User-disconnect hold is released: the remaining device may connect. */
    reset_io();
    bt_link_service(12000);
    CHECK(sent_has("CL06", 4), "auto-connect to the remaining device");
}

int main(void)
{
    mkdir(".work", 0777);
    mkdir(".work/bt", 0777);
    mkdir(".work/bt/Internal", 0777);
    mkdir(".work/bt/Internal/.rechord", 0777);
    remove(".work/bt/Internal/.rechord/BT");
    fs_host_set_root(".work/bt");

    TEST("bt_link: off by default");
    test_disabled_by_default();
    TEST("bt_link: persisted device, auto-connect, stream, remote");
    test_persisted_auto_connect();
    TEST("bt_link: dropped link pauses");
    test_link_lost_pauses();
    TEST("bt_link: scan results are not auto-connected");
    test_scan_results_are_not_saved();
    test_idle_off_with_display();
    TEST("bt_link: wired headphones block Bluetooth");
    test_wired_headphones_block();
    TEST("bt_link: forget drops an attempt in flight");
    test_forget_drops_attempt_and_relents();

    if (s_failures == 0) {
        printf("test_bt_link: OK\n");
        return 0;
    }
    printf("test_bt_link: %d FAILURE(S)\n", s_failures);
    return s_failures;
}
