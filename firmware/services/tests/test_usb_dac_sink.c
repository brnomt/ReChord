/*
 * test_usb_dac_sink.c — UAC isochronous packet -> PCM ring policy.
 */
#include <stdio.h>
#include <string.h>

#include "usb_dac_sink.h"

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

static uint8_t s_ring[32768];
static uint32_t s_read[2 * 4096];

int main(void)
{
    ringbuf_t rb;
    usb_dac_sink_t sink;
    uint8_t packet[196];
    uint32_t frames;

    TEST("usb_dac_sink: 16-bit packet is left-justified and repeats at start");
    rb_init(&rb, s_ring, sizeof s_ring);
    usb_dac_sink_init(&sink, &rb);
    usb_dac_sink_start(&sink, 48000, 2);
    CHECK_EQI(sink.target, 960, "target is 20 ms");
    CHECK_EQI(sink.guard, 192, "guard is 4 ms");
    memset(packet, 0, sizeof packet);
    for (int i = 0; i < 48; i++) {
        packet[i * 4 + 0] = 0x34; packet[i * 4 + 1] = 0x12;   /* left  0x1234 */
        packet[i * 4 + 2] = 0xCD; packet[i * 4 + 3] = 0xAB;   /* right 0xABCD */
    }
    frames = usb_dac_sink_write(&sink, packet, 48 * 4);
    CHECK_EQI(frames, 49, "one drift repeat when the ring is empty");
    CHECK_EQI(sink.repeated, 1, "repeat counted");
    CHECK_EQI(rb_read(&rb, s_read, 49 * 8), 49 * 8, "all frames readable");
    CHECK_EQI(s_read[0], 0x12340000u, "left sample left-justified");
    CHECK_EQI(s_read[1], 0xABCD0000u, "right sample left-justified");
    CHECK_EQI(s_read[2 * 48], 0x12340000u, "repeated frame is the last one");

    TEST("usb_dac_sink: prefill gives 20 ms of silence");
    rb_reset(&rb);
    usb_dac_sink_start(&sink, 48000, 2);
    usb_dac_sink_prefill(&sink);
    CHECK_EQI(usb_dac_sink_fill(&sink), 960, "target frames of silence");
    CHECK_EQI(rb_read(&rb, s_read, 8), 8, "silent frame readable");
    CHECK_EQI(s_read[0], 0, "left silence");
    CHECK_EQI(s_read[1], 0, "right silence");

    TEST("usb_dac_sink: host ahead drops one frame");
    rb_reset(&rb);
    usb_dac_sink_start(&sink, 48000, 2);
    usb_dac_sink_prefill(&sink);
    memset(packet, 0, sizeof packet);
    for (int i = 0; i < 5; i++)
        usb_dac_sink_write(&sink, packet, 48 * 4);
    CHECK_EQI(usb_dac_sink_fill(&sink), 1200, "buffer above target+guard");
    CHECK_EQI(usb_dac_sink_write(&sink, packet, 48 * 4), 47,
              "one frame dropped");
    CHECK_EQI(sink.dropped, 1, "drop counted");

    TEST("usb_dac_sink: host behind repeats one frame");
    rb_reset(&rb);
    usb_dac_sink_start(&sink, 48000, 2);
    usb_dac_sink_prefill(&sink);
    (void)rb_read(&rb, s_read, 600 * 8);
    CHECK_EQI(usb_dac_sink_write(&sink, packet, 48 * 4), 49,
              "one frame repeated");
    CHECK_EQI(sink.repeated, 1, "repeat counted");

    TEST("usb_dac_sink: 24-bit samples");
    rb_reset(&rb);
    usb_dac_sink_start(&sink, 48000, 3);
    memset(packet, 0, sizeof packet);
    packet[0] = 0x56; packet[1] = 0x34; packet[2] = 0x12; /* 0x123456 */
    packet[3] = 0xFF; packet[4] = 0xFF; packet[5] = 0xFF; /* -1       */
    frames = usb_dac_sink_write(&sink, packet, 32 * 6);
    CHECK_EQI(frames, 33, "repeat in 24-bit mode");
    CHECK_EQI(rb_read(&rb, s_read, 33 * 8), 33 * 8, "24-bit frames readable");
    CHECK_EQI(s_read[0], 0x12345600u, "24-bit left left-justified");
    CHECK_EQI(s_read[1], 0xFFFFFF00u, "24-bit -1 left-justified");

    TEST("usb_dac_sink: malformed length is ignored");
    rb_reset(&rb);
    usb_dac_sink_start(&sink, 48000, 2);
    CHECK_EQI(usb_dac_sink_write(&sink, packet, 3), 0, "partial frame ignored");
    usb_dac_sink_start(&sink, 48000, 5);
    CHECK_EQI(sink.started, 0, "unsupported depth refused");

    if (s_failures)
    {
        printf("%d FAILURE(S)\n", s_failures);
        return 1;
    }
    printf("ALL USB DAC SINK TESTS PASSED\n");
    return 0;
}
