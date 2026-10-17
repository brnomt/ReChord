/*
 * input.c — key event queue + ADC threshold mapping (see input.h).
 *
 * The queue and mapping are ours; the hardware hookup is the only part that
 * touches the SDK and is marked TODO below. No third-party code is used.
 */
#include "input.h"

#include <stddef.h>

static input_event_t s_queue[INPUT_QUEUE_LEN];
static unsigned s_head;      /* index of the oldest queued event */
static unsigned s_count;     /* events currently queued */

/*
 * ADC ladder bands (10-bit, SAR-ADC channel 1, idle 1023). Measured on
 * hardware: vol- 0, vol+ 121..177, play 245..299, menu 321..395,
 * back 483..549, idle >= 1001. Bands sit between those clusters; readings
 * in the gaps map to no key. Power is a separate GPIO (input_scan).
 */
typedef struct {
    int type;
    int adc_min;
    int adc_max;
} adc_band_t;

static const adc_band_t s_adc_bands[] = {
    { KEY_DOWN,     0,   69 },   /* vol- */
    { KEY_UP,      70,  183 },   /* vol+ */
    { KEY_SELECT, 200,  319 },   /* play */
    { KEY_MENU,   320,  409 },   /* menu */
    { KEY_BACK,   450,  549 },   /* back */
};

#define ADC_IDLE 1023   /* documented idle reading: no key pressed */

static int s_scan_last = KEY_NONE;   /* previous mapped key            */
static int s_scan_held = KEY_NONE;   /* debounced key, if any */
static int s_scan_hold;              /* scans held past the press (LONG /
                                      * REPEAT timing)                     */
static int s_scan_stable;            /* consecutive samples agreeing with
                                      * s_scan_last (see input_scan)      */
static int s_scan_min = ADC_IDLE;    /* lowest ADC seen since idle — the
                                      * pressed-bottom classification the
                                      * stock scan uses (ref 0x03057eb0:
                                      * "if (v < min) min = v", classify
                                      * by min). Press/release transients
                                      * SWEEP the resistor ladder through
                                      * the Menu/Back bands; classifying
                                      * by the settled minimum makes those
                                      * sweeps unregistrable.            */

/* Min updates require a real drop (>= this many counts): ADC noise on a
 * held key must not ratchet the minimum into a lower key band. Bands are
 * >= 50 counts wide, so 16 is safely below half a band. */
#define INPUT_ADC_MIN_STEP 16

/* Debounce constants (INPUT_DEBOUNCE_SCANS*) live in input.h; they match
 * the stock key scan (ref 0x03057eb0): 3 consecutive samples for most
 * keys, 8 for the vol+- pair (the keys that phantom-fired under playback
 * when one sample was enough to "debounce"). */

int input_map_adc(int adc_value)
{
    unsigned i;

    if (adc_value < 0 || adc_value >= ADC_IDLE) {
        return KEY_NONE;
    }
    for (i = 0; i < sizeof(s_adc_bands) / sizeof(s_adc_bands[0]); i++) {
        if (adc_value >= s_adc_bands[i].adc_min &&
            adc_value <= s_adc_bands[i].adc_max) {
            return s_adc_bands[i].type;
        }
    }
    return KEY_NONE;
}

static int input_push_kind(int type, int adc_value, int kind, int long_press)
{
    input_event_t *slot;

    if (type < KEY_UP || type > KEY_MENU) {
        return INPUT_ERR_ARG;
    }
    if (s_count >= (unsigned)INPUT_QUEUE_LEN) {
        return INPUT_ERR_FULL;  /* dropped; the UI is not draining fast enough */
    }
    slot = &s_queue[(s_head + s_count) % (unsigned)INPUT_QUEUE_LEN];
    slot->type = type;
    slot->adc_value = adc_value;
    slot->kind = kind;
    slot->long_press = long_press;
    s_count++;
    return INPUT_OK;
}

int input_push(int type, int adc_value)
{
    return input_push_kind(type, adc_value, INPUT_PRESS, 0);
}

int input_poll(input_event_t *ev)
{
    if (ev == NULL) {
        return INPUT_ERR_ARG;
    }
    if (s_count == 0) {
        return 0;
    }
    *ev = s_queue[s_head];
    s_head = (s_head + 1) % (unsigned)INPUT_QUEUE_LEN;
    s_count--;
    return 1;
}

int input_scan(int adc_value, int power)
{
    int key, needed;

    if (power) {
        key = KEY_POWER;               /* GPIO power key wins            */
    } else if (adc_value >= ADC_IDLE) {
        key = KEY_NONE;                /* released (idle line)           */
    } else {
        /* Classify by the pressed-bottom ADC (stock scan, ref 0x03057eb0):
         * the minimum of the current press, not the passing sample — a
         * press/release transient sweeps through the Menu/Back bands and
         * must not register them. Noise needs INPUT_ADC_MIN_STEP counts
         * of real movement to move the bottom. */
        if (s_scan_min - adc_value >= INPUT_ADC_MIN_STEP)
            s_scan_min = adc_value;
        key = input_map_adc(s_scan_min);
    }

    needed = (key == KEY_UP || key == KEY_DOWN)
                 ? INPUT_DEBOUNCE_SCANS_VOL : INPUT_DEBOUNCE_SCANS;

    if (key != s_scan_last) {          /* changed: restart stability run */
        s_scan_last = key;
        s_scan_stable = 1;
        return 0;
    }
    s_scan_stable++;                   /* one more agreeing sample        */
    if (s_scan_stable < needed)        /* fires on the N-th equal sample  */
        return 0;
    if (key == KEY_NONE) {             /* debounced release/idle: re-arm */
        int rel = s_scan_held;
        int rel_adc = power ? -1 : s_scan_min;
        int rel_long = s_scan_hold >= INPUT_LONG_SCANS;

        s_scan_held = KEY_NONE;
        s_scan_hold = 0;
        s_scan_min = ADC_IDLE;         /* the pressed-bottom run         */
        if (rel == KEY_NONE)
            return 0;
        return input_push_kind(rel, rel_adc,
                               INPUT_RELEASE, rel_long) == INPUT_OK;
    }
    if (key == s_scan_held) {          /* held: LONG once, then REPEAT   */
        int adc = power ? -1 : s_scan_min;

        s_scan_hold++;
        if (s_scan_hold == INPUT_LONG_SCANS)
            return input_push_kind(key, adc, INPUT_LONG, 1) == INPUT_OK;
        if (s_scan_hold > INPUT_LONG_SCANS && INPUT_REPEATS(key) &&
            (s_scan_hold - INPUT_LONG_SCANS) % INPUT_REPEAT_SCANS == 0)
            return input_push_kind(key, adc, INPUT_REPEAT, 1) == INPUT_OK;
        return 0;
    }
    s_scan_held = key;
    s_scan_hold = 0;
    return input_push(key, power ? -1 : s_scan_min) == INPUT_OK;
}

void input_mock_clear(void)
{
    s_head = 0;
    s_count = 0;
    s_scan_last = KEY_NONE;
    s_scan_held = KEY_NONE;
    s_scan_hold = 0;
    s_scan_stable = 0;
}

unsigned input_pending(void)
{
    return s_count;
}
