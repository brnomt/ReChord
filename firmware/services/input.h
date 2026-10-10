/*
 * input.h — ReChord key input: event queue + ADC threshold mapping.
 *
 * WHY: the UI must consume key events at its own pace while the ADC scan (or,
 * on the host, a test script) produces them asynchronously, so events go
 * through a small queue. input_poll() is the single consumption API.
 *
 * Hardware contract (target): the Echo Mini keys are resistor-ladder ADC keys
 * plus a power GPIO (per the documented bring-up log: "keys: adc ch ...",
 * "keys: power pin"). The SDK AD_KEY driver will feed input_push(); until it
 * is hooked up (TODO), only host-side scripted events exist.
 */
#ifndef RECHORD_SERVICES_INPUT_H
#define RECHORD_SERVICES_INPUT_H

/* Event types. The enum is named so both services and UI code can use the
 * `input_key_t` type (added during integration: the UI workstream expects
 * the name, the values are unchanged). */
typedef enum input_key {
    KEY_NONE = 0,   /* not a key event (ADC idle) */
    KEY_UP = 1,
    KEY_DOWN = 2,
    KEY_SELECT = 3,
    KEY_BACK = 4,
    KEY_POWER = 5
} input_key_t;

/* One key event.
 *   type      — one of the KEY_* constants above
 *   adc_value — raw ADC reading that produced the event, or -1 for the
 *               GPIO power key / scripted events with no ADC value */
typedef struct { int type; int adc_value; } input_event_t;

/* Pop the oldest queued event into *ev.
 * Returns 1 if an event was delivered, 0 if the queue is empty,
 * INPUT_ERR_ARG if ev is NULL. */
int input_poll(input_event_t *ev);

/* Push an event (ISR / driver / host-mock entry point).
 * Returns INPUT_OK, or INPUT_ERR_ARG for an unknown type, or INPUT_ERR_FULL
 * when the queue overflows (event dropped). */
int input_push(int type, int adc_value);

/* Map a raw ADC reading to a key type (KEY_NONE when idle/unmapped).
 * See the provisional band table in input.c — TODO: calibrate against the
 * SDK AD_KEY driver readings. */
int input_map_adc(int adc_value);

/* Host-mock helper: drop all queued events (also useful between test cases). */
void input_mock_clear(void);

/* Number of queued events. */
unsigned input_pending(void);

/* Queue capacity (events). */
#ifndef INPUT_QUEUE_LEN
#define INPUT_QUEUE_LEN 16
#endif

#define INPUT_OK        0
#define INPUT_ERR_ARG (-1)
#define INPUT_ERR_FULL (-2)

#endif /* RECHORD_SERVICES_INPUT_H */
