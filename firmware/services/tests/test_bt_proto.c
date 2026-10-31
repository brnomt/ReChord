/*
 * test_bt_proto.c — Bluetooth module protocol state machine.
 *
 * Scripted module streams (ready, scan, connect, link, buttons, disconnect)
 * and the timer paths (ready timeout, scan/connect retries and limits, the
 * 100 ms notice delay). No hardware: bytes in, frames out, milliseconds.
 */
#include <stdio.h>
#include <string.h>

#include "bt_proto.h"

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

static bt_proto_t bt;
static bt_frame_t frame;

static void reset_at(unsigned long now)
{
    bt_proto_init(&bt);
    bt_proto_power_on(&bt, now);
    frame.len = 0;
}

static void feed(const char *text, unsigned long now)
{
    const unsigned char *p = (const unsigned char *)text;

    while (*p != '\0')
        bt_proto_receive(&bt, *p++, now);
}

/* Feed a text frame then let the 2 ms silence parser accept short frames. */
static int poll_text(const char *text, unsigned long now)
{
    int got;

    feed(text, now);
    got = bt_proto_poll(&bt, now + 2, &frame);
    return got;
}

static void test_ready(void)
{
    TEST("bt_proto: power-on and ready");

    reset_at(100);
    CHECK_EQI(bt_proto_poll(&bt, 200, &frame), 0, "no reply yet");
    CHECK_EQI(poll_text("CV1", 300), 0, "CV1 is not a command");
    CHECK_EQI(bt.state, BT_STATE_READY, "CV1 -> ready");
    CHECK_EQI(bt.version, '1', "module version");

    reset_at(100);
    bt_proto_poll(&bt, 100 + 5001, &frame);
    CHECK_EQI(bt.state, BT_STATE_NO_REPLY, "no CV1 in 5 s -> no reply");
}

static void test_scan(void)
{
    TEST("bt_proto: scan finds devices");

    reset_at(0);
    poll_text("CV1", 10);
    CHECK_EQI(bt_proto_scan(&bt, 20, &frame), 1, "scan accepted");
    CHECK_EQI(frame.len, 5, "command is 5 bytes");
    CHECK(frame.bytes[0] == 'C' && frame.bytes[1] == 'S' &&
          frame.bytes[2] == '0' && frame.bytes[3] == '0', "CS00 sent");
    CHECK_EQI(bt.state, BT_STATE_SCANNING, "scanning");

    /* CS<NN><6 addr><name> */
    poll_text("CS10\xAA\xBB\xCC\xDD\xEE\xFF" "Buds", 30);
    CHECK_EQI(bt_proto_device_count(&bt), 1, "one device found");
    CHECK_EQI(bt_proto_is_saved(&bt, 0), 0, "scan result is not saved");
    {
        const bt_device_t *d = bt_proto_device(&bt, 0);

        CHECK(d != 0 && d->addr[0] == 0xAA && strcmp(d->name, "Buds") == 0,
              "address and name parsed");
    }
    poll_text("CS10\xAA\xBB\xCC\xDD\xEE\xFF" "Buds", 40);
    CHECK_EQI(bt_proto_device_count(&bt), 1, "same device not duplicated");

    poll_text("CSEN", 50);
    CHECK_EQI(bt.state, BT_STATE_READY, "CSEN -> ready");

    /* Scan retry after 4 s without an answer, stop after 30 s. */
    bt_proto_scan(&bt, 100, &frame);
    CHECK_EQI(bt_proto_poll(&bt, 100 + 4001, &frame), 1, "scan retried");
    CHECK(frame.bytes[1] == 'S', "CS00 retry");
    CHECK_EQI(bt_proto_poll(&bt, 100 + 30001, &frame), 1, "scan limit");
    CHECK(frame.bytes[1] == 'P', "CP00 ends the scan");
    CHECK_EQI(bt.state, BT_STATE_READY, "back to ready");
}

static void test_connect_and_stream(void)
{
    static const char addr[] = "\x01\x02\x03\x04\x05\x06";
    char frame_text[32];

    TEST("bt_proto: connect, new-link stream-off, stream on/off");

    reset_at(0);
    poll_text("CV1", 10);
    bt_proto_scan(&bt, 20, &frame);
    poll_text("CS10\x01\x02\x03\x04\x05\x06" "Buds", 30);
    poll_text("CSEN", 40);

    CHECK_EQI(bt_proto_connect(&bt, 0, 50, &frame), 1, "connect accepted");
    CHECK_EQI(frame.len, 10, "CL06 + address");
    CHECK(frame.bytes[0] == 'C' && frame.bytes[1] == 'L' &&
          frame.bytes[2] == '0' && frame.bytes[3] == '6', "CL06 header");
    CHECK(memcmp(frame.bytes + 4, addr, 6) == 0, "target address sent");

    /* No answer for 4 s: CH06 retry; 30 s: give up with CT00. */
    CHECK_EQI(bt_proto_poll(&bt, 50 + 4001, &frame), 1, "connect retry");
    CHECK(frame.bytes[1] == 'H', "CH06 retry");
    poll_text("CLER", 50 + 5000);
    CHECK_EQI(bt.state, BT_STATE_READY, "CLER -> ready");
    CHECK_EQI(bt.pairing, BT_PAIR_FAILED, "pairing failed reported");

    /* A paired reply waits for the link; CQO brings it up. */
    bt_proto_connect(&bt, 0, 6000, &frame);
    poll_text("CLOK", 6010);
    CHECK_EQI(bt.state, BT_STATE_READY, "CLOK stays ready until linked");
    CHECK_EQI(bt.pairing, BT_PAIR_OK, "paired");
    sprintf(frame_text, "CQO10%sBuds", addr);
    CHECK_EQI(poll_text(frame_text, 6020), 1, "new-link answer in the same poll");
    CHECK(frame.bytes[1] == 'Y' && frame.bytes[3] == 0, "CY0 00 sent");
    CHECK_EQI(bt.state, BT_STATE_CONNECTED, "CQO -> connected");
    CHECK(bt_proto_connected(&bt) != 0, "connected device resolved");

    /* A new link is answered exactly once. */
    CHECK_EQI(bt_proto_poll(&bt, 6030, &frame), 0, "new-link answer once");

    /* Stream on from a paused module: notice first, stream after 100 ms. */
    bt.play_state = 1;                 /* paused */
    CHECK_EQI(bt_proto_set_stream(&bt, 1, 6100, &frame), 1, "stream on");
    CHECK(frame.bytes[1] == 0x86, "play notice C 86 first");
    CHECK_EQI(bt_proto_poll(&bt, 6150, &frame), 0, "stream waits for 100 ms");
    CHECK_EQI(bt_proto_poll(&bt, 6200, &frame), 1, "stream frame after delay");
    CHECK(frame.bytes[1] == 'Y' && frame.bytes[3] == 0x1E, "CY0 1E");

    bt.play_state = 2;                 /* playing */
    CHECK_EQI(bt_proto_set_stream(&bt, 0, 6300, &frame), 1, "stream off");
    CHECK(frame.bytes[1] == 0x87, "pause notice C 87 first");
    bt_proto_poll(&bt, 6401, &frame);
    CHECK(frame.bytes[1] == 'Y' && frame.bytes[3] == 0, "CY0 00 after delay");

    /* Headphone buttons and a dropped link. */
    poll_text("CC", 6500);
    CHECK_EQI(bt.remote_plays, 1, "play button counted");
    poll_text("CA", 6510);
    poll_text("CB", 6520);
    CHECK_EQI(bt.remote_prevs, 1, "previous counted");
    CHECK_EQI(bt.remote_nexts, 1, "next counted");
    poll_text("CTOK", 6600);
    CHECK_EQI(bt.state, BT_STATE_READY, "CTOK -> ready");
    CHECK_EQI(bt.streaming, 0, "stream cleared with the link");

    /* Disconnect command. */
    bt_proto_connect(&bt, 0, 7000, &frame);
    poll_text("CHOK", 7010);
    CHECK_EQI(bt.state, BT_STATE_CONNECTED, "CHOK -> connected");
    CHECK_EQI(bt_proto_disconnect(&bt, 7100, &frame), 1, "disconnect accepted");
    CHECK(frame.bytes[1] == 'T', "CT00 sent");
}

static void test_query_periods(void)
{
    static const char addr[] = "\x10\x20\x30\x40\x50\x60";
    char connected[32];

    TEST("bt_proto: poll periods and the CP/CC pause");

    reset_at(0);
    poll_text("CV1", 10);
    bt_proto_scan(&bt, 20, &frame);
    poll_text("CS10\x10\x20\x30\x40\x50\x60" "Buds", 30);
    poll_text("CSEN", 40);
    bt_proto_connect(&bt, 0, 50, &frame);
    poll_text("CLOK", 60);
    sprintf(connected, "CQO10%sBuds", addr);
    CHECK_EQI(poll_text(connected, 70), 1, "link answer");
    CHECK_EQI(bt.state, BT_STATE_CONNECTED, "connected");

    CHECK_EQI(bt_proto_poll(&bt, 500, &frame), 0, "no query before 3 s");
    CHECK_EQI(bt_proto_poll(&bt, 3075, &frame), 1, "CQ00 after 3 s");
    CHECK(frame.bytes[1] == 'Q', "query frame");

    /* A play report pauses polling for 50 s (only while connected). */
    poll_text("CP", 3200);
    CHECK_EQI(bt_proto_poll(&bt, 10000, &frame), 0, "poll paused after CP");
    CHECK_EQI(bt_proto_poll(&bt, 3202 + 50001, &frame), 1,
              "poll resumes after 50 s");
}

static void test_saved_mask(void)
{
    bt_proto_t b;
    bt_device_t devices[3];

    TEST("bt_proto: seeded devices are saved, remove shifts the mask");

    memset(devices, 0, sizeof devices);
    devices[0].addr[0] = 1;
    devices[1].addr[0] = 2;
    devices[2].addr[0] = 3;
    bt_proto_init(&b);
    bt_proto_seed_devices(&b, devices, 3);
    CHECK_EQI(bt_proto_is_saved(&b, 0), 1, "seeded device saved");
    CHECK_EQI(bt_proto_is_saved(&b, 2), 1, "third seeded device saved");
    bt_proto_remove_device(&b, 0);
    CHECK_EQI(bt_proto_device_count(&b), 2, "one removed");
    CHECK_EQI(bt_proto_device(&b, 0)->addr[0], 2, "second moved to front");
    CHECK_EQI(bt_proto_is_saved(&b, 0), 1, "saved mask shifted");
}

int main(void)
{
    test_ready();
    test_scan();
    test_connect_and_stream();
    test_query_periods();
    test_saved_mask();
    if (s_failures == 0) {
        printf("test_bt_proto: OK\n");
        return 0;
    }
    printf("test_bt_proto: %d FAILURE(S)\n", s_failures);
    return s_failures;
}
