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

static int s_scan_last = KEY_NONE;   /* previous raw sample   */
static int s_scan_held = KEY_NONE;   /* debounced key, if any */

#define ADC_IDLE 1023   /* documented idle reading: no key pressed */

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

int input_push(int type, int adc_value)
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
    s_count++;
    return INPUT_OK;
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
    int key = power ? KEY_POWER : input_map_adc(adc_value);

    if (key != s_scan_last) {          /* not stable yet */
        s_scan_last = key;
        return 0;
    }
    if (key == s_scan_held)
        return 0;
    s_scan_held = key;
    if (key == KEY_NONE)
        return 0;
    return input_push(key, power ? -1 : adc_value) == INPUT_OK;
}

void input_mock_clear(void)
{
    s_head = 0;
    s_count = 0;
    s_scan_last = KEY_NONE;
    s_scan_held = KEY_NONE;
}

unsigned input_pending(void)
{
    return s_count;
}
