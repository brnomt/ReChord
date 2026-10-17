/*
 * test_input.c — host tests for the key event queue and ADC mapping.
 *
 * Covers: FIFO queue semantics, empty/NULL poll, invalid types, queue
 * overflow drop, mock clear, ADC band mapping (including the documented
 * 1023 idle reading) and adc_value pass-through.
 */
#include "input.h"

#include <stdio.h>
#include <string.h>

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

static void test_queue_fifo(void)
{
    input_event_t ev;

    input_mock_clear();
    CHECK(input_pending() == 0);
    CHECK(input_poll(&ev) == 0);             /* empty */
    CHECK(input_poll(NULL) == INPUT_ERR_ARG);

    /* Invalid event types are rejected. */
    CHECK(input_push(KEY_NONE, 0) == INPUT_ERR_ARG);
    CHECK(input_push(KEY_MENU + 1, 0) == INPUT_ERR_ARG);
    CHECK(input_push(-1, 0) == INPUT_ERR_ARG);

    CHECK(input_push(KEY_UP, 100) == INPUT_OK);
    CHECK(input_push(KEY_SELECT, 406) == INPUT_OK);   /* scripted ADC value */
    CHECK(input_push(KEY_POWER, -1) == INPUT_OK);     /* GPIO power key */
    CHECK(input_pending() == 3);

    CHECK(input_poll(&ev) == 1);
    CHECK(ev.type == KEY_UP && ev.adc_value == 100);
    CHECK(input_poll(&ev) == 1);
    CHECK(ev.type == KEY_SELECT && ev.adc_value == 406);
    CHECK(input_poll(&ev) == 1);
    CHECK(ev.type == KEY_POWER && ev.adc_value == -1);
    CHECK(input_poll(&ev) == 0);
    CHECK(input_pending() == 0);
}

static void test_overflow(void)
{
    input_event_t ev;
    int i;

    input_mock_clear();
    for (i = 0; i < INPUT_QUEUE_LEN; i++) {
        CHECK(input_push(KEY_DOWN, i) == INPUT_OK);
    }
    CHECK(input_pending() == (unsigned)INPUT_QUEUE_LEN);
    CHECK(input_push(KEY_UP, 0) == INPUT_ERR_FULL);   /* dropped */
    CHECK(input_pending() == (unsigned)INPUT_QUEUE_LEN);

    /* Oldest events survive; order preserved. */
    CHECK(input_poll(&ev) == 1);
    CHECK(ev.type == KEY_DOWN && ev.adc_value == 0);
    for (i = 1; i < INPUT_QUEUE_LEN; i++) {
        CHECK(input_poll(&ev) == 1);
        CHECK(ev.type == KEY_DOWN && ev.adc_value == i);
    }
    CHECK(input_poll(&ev) == 0);

    input_mock_clear();
    CHECK(input_pending() == 0);
}

static void test_adc_map(void)
{
    /* Extremes of the clusters measured on hardware (input.c). */
    CHECK(input_map_adc(0) == KEY_DOWN);      /* vol- */
    CHECK(input_map_adc(121) == KEY_UP);      /* vol+ */
    CHECK(input_map_adc(177) == KEY_UP);
    CHECK(input_map_adc(245) == KEY_SELECT);  /* play */
    CHECK(input_map_adc(299) == KEY_SELECT);
    CHECK(input_map_adc(321) == KEY_MENU);
    CHECK(input_map_adc(395) == KEY_MENU);
    CHECK(input_map_adc(483) == KEY_BACK);
    CHECK(input_map_adc(549) == KEY_BACK);

    /* Gaps, idle (>= 1001 measured) and out-of-range map to no key. */
    CHECK(input_map_adc(190) == KEY_NONE);
    CHECK(input_map_adc(430) == KEY_NONE);
    CHECK(input_map_adc(1001) == KEY_NONE);
    CHECK(input_map_adc(1023) == KEY_NONE);
    CHECK(input_map_adc(2000) == KEY_NONE);
    CHECK(input_map_adc(-1) == KEY_NONE);
}

static void test_scan(void)
{
    input_event_t ev;
    int i;

    input_mock_clear();
    CHECK(input_scan(1023, 0) == 0);
    /* vol+ (130) needs INPUT_DEBOUNCE_SCANS_VOL identical samples. */
    for (i = 1; i < INPUT_DEBOUNCE_SCANS_VOL; i++)
        CHECK(input_scan(130, 0) == 0);      /* not stable yet */
    CHECK(input_scan(130, 0) == 1);          /* Nth equal sample: press */
    CHECK(input_scan(130, 0) == 0);          /* hold: no repeat */
    for (i = 0; i < INPUT_DEBOUNCE_SCANS_VOL; i++)
        CHECK(input_scan(1023, 0) == 0);     /* debounced release */
    /* A single-sample glitch (back band) is ignored. */
    CHECK(input_scan(500, 0) == 0);
    for (i = 0; i < INPUT_DEBOUNCE_SCANS_VOL; i++)
        CHECK(input_scan(1023, 0) == 0);
    /* A bounce sweeping between bands never reaches N equal samples. */
    for (i = 0; i < 40; i++)
        CHECK(input_scan((i & 1) ? 130 : 250, 0) == 0);
    /* play/select (250) needs only INPUT_DEBOUNCE_SCANS samples. */
    for (i = 1; i < INPUT_DEBOUNCE_SCANS; i++)
        CHECK(input_scan(250, 0) == 0);
    CHECK(input_scan(250, 0) == 1);          /* 3rd equal sample: press */
    CHECK(input_scan(250, 0) == 0);          /* hold: no repeat */
    for (i = 0; i < INPUT_DEBOUNCE_SCANS; i++)
        CHECK(input_scan(1023, 0) == 0);     /* release */
    /* power GPIO wins once stable. */
    for (i = 1; i < INPUT_DEBOUNCE_SCANS; i++)
        CHECK(input_scan(1023, 1) == 0);
    CHECK(input_scan(1023, 1) == 1);
    CHECK(input_pending() == 3);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_UP && ev.adc_value == 130);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_SELECT && ev.adc_value == 250);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_POWER && ev.adc_value == -1);
    input_mock_clear();
}

int main(void)
{
    test_queue_fifo();
    test_overflow();
    test_adc_map();
    test_scan();

    printf("test_input: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
