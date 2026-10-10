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
    CHECK(input_push(KEY_POWER + 1, 0) == INPUT_ERR_ARG);
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
    /* Provisional band table boundaries (input.c). */
    CHECK(input_map_adc(0) == KEY_UP);
    CHECK(input_map_adc(200) == KEY_UP);
    CHECK(input_map_adc(201) == KEY_DOWN);
    CHECK(input_map_adc(400) == KEY_DOWN);
    CHECK(input_map_adc(401) == KEY_SELECT);
    CHECK(input_map_adc(406) == KEY_SELECT);
    CHECK(input_map_adc(600) == KEY_SELECT);
    CHECK(input_map_adc(601) == KEY_BACK);
    CHECK(input_map_adc(800) == KEY_BACK);
    CHECK(input_map_adc(801) == KEY_POWER);
    CHECK(input_map_adc(1022) == KEY_POWER);

    /* Documented idle reading and out-of-range values map to no key. */
    CHECK(input_map_adc(1023) == KEY_NONE);
    CHECK(input_map_adc(2000) == KEY_NONE);
    CHECK(input_map_adc(-1) == KEY_NONE);
}

int main(void)
{
    test_queue_fifo();
    test_overflow();
    test_adc_map();

    printf("test_input: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
