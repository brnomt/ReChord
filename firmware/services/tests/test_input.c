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
    CHECK(input_scan(130, 0) == 0);          /* held below LONG: silent */
    /* RELEASE SWEEP through the back/menu/play bands must be invisible:
     * the pressed-bottom minimum classifies the press (ref 0x03057eb0). */
    CHECK(input_scan(200, 0) == 0);
    CHECK(input_scan(350, 0) == 0);
    CHECK(input_scan(500, 0) == 0);
    CHECK(input_scan(800, 0) == 0);
    for (i = 0; i < INPUT_DEBOUNCE_SCANS - 1; i++)
        CHECK(input_scan(1023, 0) == 0);
    CHECK(input_scan(1023, 0) == 1);         /* release: RELEASE(short)  */
    /* A press SWEEP from idle falls through back/menu/play bands without
     * registering them; only the settled bottom (the run minimum)
     * registers. */
    CHECK(input_scan(520, 0) == 0);
    CHECK(input_scan(380, 0) == 0);
    CHECK(input_scan(250, 0) == 0);
    CHECK(input_scan(150, 0) == 0);          /* vol+ band: sample #1 */
    for (i = 1; i < INPUT_DEBOUNCE_SCANS_VOL - 1; i++)
        CHECK(input_scan(130, 0) == 0);      /* settles at the min (130) */
    CHECK(input_scan(130, 0) == 1);          /* 8th UP sample: press */
    for (i = 0; i < INPUT_DEBOUNCE_SCANS - 1; i++)
        CHECK(input_scan(1023, 0) == 0);
    CHECK(input_scan(1023, 0) == 1);         /* release: RELEASE(short)  */
    /* Noise on a held key cannot ratchet the minimum into another band. */
    for (i = 1; i < INPUT_DEBOUNCE_SCANS; i++)
        CHECK(input_scan(250, 0) == 0);      /* play: 3-sample key */
    CHECK(input_scan(250, 0) == 1);          /* press play */
    for (i = 0; i < 12; i++)
        CHECK(input_scan((i & 1) ? 258 : 244, 0) == 0);   /* +-7 noise */
    CHECK(input_pending() == 5);            /* P,R,P,R,P (up,up,select)  */
    for (i = 0; i < INPUT_DEBOUNCE_SCANS - 1; i++)
        CHECK(input_scan(1023, 0) == 0);
    CHECK(input_scan(1023, 0) == 1);         /* release: RELEASE(short)  */
    /* power GPIO wins once stable. */
    for (i = 1; i < INPUT_DEBOUNCE_SCANS; i++)
        CHECK(input_scan(1023, 1) == 0);
    CHECK(input_scan(1023, 1) == 1);
    CHECK(input_pending() == 7);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_UP && ev.adc_value == 130
          && ev.kind == INPUT_PRESS);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_UP
          && ev.kind == INPUT_RELEASE && ev.long_press == 0);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_UP && ev.adc_value == 130
          && ev.kind == INPUT_PRESS);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_UP
          && ev.kind == INPUT_RELEASE && ev.long_press == 0);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_SELECT && ev.adc_value == 250
          && ev.kind == INPUT_PRESS);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_SELECT
          && ev.kind == INPUT_RELEASE && ev.long_press == 0);
    CHECK(input_poll(&ev) == 1 && ev.type == KEY_POWER && ev.adc_value == -1
          && ev.kind == INPUT_PRESS);
    input_mock_clear();
}

/* Short vs long press: a short press is exactly one PRESS (+ RELEASE with
 * long_press = 0); a held vol key produces PRESS, LONG, REPEATs and
 * RELEASE{long_press = 1}; a held non-repeating key produces LONG but no
 * REPEAT. */
static void test_scan_hold(void)
{
    input_event_t ev;
    int i;

    input_mock_clear();
    for (i = 0; i < INPUT_DEBOUNCE_SCANS_VOL; i++)
        (void)input_scan(130, 0);                    /* press registers  */
    CHECK(input_pending() == 1);
    for (i = 0; i < INPUT_LONG_SCANS - 1; i++)
        CHECK(input_scan(130, 0) == 0);              /* still short      */
    CHECK(input_scan(130, 0) == 1);                  /* LONG fires       */
    for (i = 1; i < INPUT_REPEAT_SCANS; i++)
        CHECK(input_scan(130, 0) == 0);
    CHECK(input_scan(130, 0) == 1);                  /* REPEAT #1        */
    for (i = 1; i < INPUT_REPEAT_SCANS; i++)
        CHECK(input_scan(130, 0) == 0);
    CHECK(input_scan(130, 0) == 1);                  /* REPEAT #2        */
    for (i = 0; i < INPUT_DEBOUNCE_SCANS; i++)
        (void)input_scan(1023, 0);                   /* release          */
    CHECK(input_pending() == 5);                     /* P,L,R,R,R        */
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_PRESS);
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_LONG && ev.long_press == 1);
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_REPEAT);
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_REPEAT);
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_RELEASE && ev.long_press == 1);

    input_mock_clear();
    for (i = 0; i < INPUT_DEBOUNCE_SCANS_VOL; i++)
        (void)input_scan(130, 0);                    /* press registers  */
    for (i = 0; i < 5; i++)
        (void)input_scan(130, 0);                    /* short hold       */
    for (i = 0; i < INPUT_DEBOUNCE_SCANS; i++)
        (void)input_scan(1023, 0);                   /* release          */
    CHECK(input_pending() == 2);                     /* P + RELEASE      */
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_PRESS);
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_RELEASE && ev.long_press == 0);

    input_mock_clear();
    for (i = 0; i < INPUT_DEBOUNCE_SCANS; i++)
        (void)input_scan(250, 0);                    /* play: press      */
    /* Held past the fast tier: still no REPEAT — SELECT never repeats. */
    for (i = 0; i < INPUT_LONG_SCANS + INPUT_FAST_HOLD_SCANS +
                    INPUT_REPEAT_SCANS; i++)
        (void)input_scan(250, 0);
    CHECK(input_pending() == 2);                     /* P + LONG only    */
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_PRESS);
    CHECK(input_poll(&ev) == 1 && ev.kind == INPUT_LONG);
    input_mock_clear();
}

/* BRNOMT-80: the repeat accelerates the longer a key is held. Feed `scans`
 * held samples of a vol+ key and count the REPEAT events, draining the queue
 * every pass exactly as the UI loop does (no backlog, no overflow). */
static int repeats_over_scans(int scans)
{
    input_event_t ev;
    int i, n = 0;

    for (i = 0; i < scans; i++) {
        (void)input_scan(130, 0);
        while (input_poll(&ev) == 1) {
            if (ev.kind == INPUT_REPEAT)
                n++;
        }
    }
    return n;
}

static void test_scan_hold_ladder(void)
{
    /* Two tier-1 intervals: fits in the tier-1 stretch and divides evenly,
     * so the same window length can be re-used on both tiers. */
    const int window = 2 * INPUT_REPEAT_SCANS;
    /* Hold up to one scan before the fast tier: the threshold scan itself
     * already repeats at the fast interval (held >= INPUT_FAST_HOLD_SCANS). */
    const int to_fast = INPUT_FAST_HOLD_SCANS - window - 1;
    int i;

    CHECK(to_fast > 0);
    input_mock_clear();
    for (i = 0; i < INPUT_DEBOUNCE_SCANS_VOL; i++)
        (void)input_scan(130, 0);                /* press registers      */
    /* Up to LONG: the marker only, no REPEATs yet. */
    CHECK(repeats_over_scans(INPUT_LONG_SCANS) == 0);
    /* Tier 1: one REPEAT every INPUT_REPEAT_SCANS (~0.3 s = ~3/s). */
    CHECK(repeats_over_scans(window) == window / INPUT_REPEAT_SCANS);
    /* Hold on to just below the fast threshold: still tier 1. */
    CHECK(repeats_over_scans(to_fast) == to_fast / INPUT_REPEAT_SCANS);
    /* Tier 2: the same window now yields the faster interval (~0.1 s). */
    CHECK(repeats_over_scans(window) == window / INPUT_FAST_REPEAT_SCANS);
    CHECK(window / INPUT_FAST_REPEAT_SCANS > window / INPUT_REPEAT_SCANS);
    /* A short tap right after the long hold starts from a clean phase. */
    for (i = 0; i < INPUT_DEBOUNCE_SCANS; i++)
        (void)input_scan(1023, 0);               /* release              */
    input_mock_clear();
    for (i = 0; i < INPUT_DEBOUNCE_SCANS_VOL; i++)
        (void)input_scan(130, 0);
    for (i = 0; i < 5; i++)
        (void)input_scan(130, 0);                /* still short          */
    for (i = 0; i < INPUT_DEBOUNCE_SCANS; i++)
        (void)input_scan(1023, 0);
    CHECK(input_pending() == 2);                 /* P + RELEASE only     */
    input_mock_clear();
}

int main(void)
{
    test_queue_fifo();
    test_overflow();
    test_adc_map();
    test_scan();
    test_scan_hold();
    test_scan_hold_ladder();

    printf("test_input: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
