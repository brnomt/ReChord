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
 * Provisional ADC band table.
 *
 * TODO(services): calibrate from the SDK AD_KEY driver. The documented
 * bring-up log shows a 10-bit ladder ADC (idle reading 1023, per-channel
 * calibration to ~400 for held keys) and a separate power GPIO. Until the
 * real ladder values are extracted, we use contiguous bands across the
 * 0..1022 range so every reading maps deterministically and tests can pin
 * the mapping logic. Replace adc_min/adc_max with measured values.
 */
typedef struct {
    int type;
    int adc_min;
    int adc_max;
} adc_band_t;

static const adc_band_t s_adc_bands[] = {
    { KEY_UP,       0,  200 },
    { KEY_DOWN,   201,  400 },
    { KEY_SELECT, 401,  600 },
    { KEY_BACK,   601,  800 },
    { KEY_POWER,  801, 1022 },
};

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

    if (type < KEY_UP || type > KEY_POWER) {
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

void input_mock_clear(void)
{
    s_head = 0;
    s_count = 0;
}

unsigned input_pending(void)
{
    return s_count;
}
