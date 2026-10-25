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
    KEY_POWER = 5,
    KEY_MENU = 6
} input_key_t;

/* One key event.
 *   type      — one of the KEY_* constants above
 *   adc_value — raw ADC reading that produced the event, or -1 for the
 *               GPIO power key / scripted events with no ADC value
 *   kind      — INPUT_*: distinguishes a SHORT press from a LONG one
 *   long_press— meaningful on INPUT_RELEASE: 1 when the press was held
 *               past the long-press threshold (LONG/REPEAT were emitted),
 *               0 for a short press
 *
 * Press model (input_scan): a SHORT press produces exactly one PRESS and
 * a RELEASE{long_press=0}. A held press produces PRESS, then one LONG at
 * INPUT_LONG_SCANS, then a REPEAT every INPUT_REPEAT_SCANS — and, after
 * INPUT_FAST_HOLD_SCANS more of holding, every INPUT_FAST_REPEAT_SCANS
 * (the repeat accelerates; keys in INPUT_REPEATS only) — then
 * RELEASE{long_press=1}. Consumers act on PRESS/REPEAT and may use
 * LONG/RELEASE to tell short from long; the acceleration needs no consumer
 * change (same REPEAT event, shorter interval). */
typedef enum input_kind {
    INPUT_PRESS = 0,
    INPUT_LONG = 1,
    INPUT_REPEAT = 2,
    INPUT_RELEASE = 3
} input_kind_t;

typedef struct {
    int type;
    int adc_value;
    int kind;
    int long_press;
    unsigned now_ms;   /* input clock when the event was produced (ms) */
} input_event_t;

/* Pop the oldest queued event into *ev.
 * Returns 1 if an event was delivered, 0 if the queue is empty,
 * INPUT_ERR_ARG if ev is NULL. */
int input_poll(input_event_t *ev);

/* Push an event (ISR / driver / host-mock entry point).
 * Returns INPUT_OK, or INPUT_ERR_ARG for an unknown type, or INPUT_ERR_FULL
 * when the queue overflows (event dropped). */
int input_push(int type, int adc_value);

/* Map a raw ADC reading to a key type (KEY_NONE when idle/unmapped).
 * Bands measured on hardware (see input.c). */
int input_map_adc(int adc_value);

/* Feed one key sample (ADC ladder reading + power GPIO level, 1 = pressed).
 * Pushes one event per debounced press: the key must map identically for
 * INPUT_DEBOUNCE_SCANS consecutive samples (INPUT_DEBOUNCE_SCANS_VOL for
 * the vol+- pair), matching the stock key scan (ref 0x03057eb0). Release
 * and hold produce nothing. Returns 1 if an event was pushed. */
int input_scan(int adc_value, int power);

/* Host-mock helper: drop all queued events (also useful between test cases). */
void input_mock_clear(void);

/* Debounce for the volume keys (KEY_UP/KEY_DOWN), in scan samples. The
 * default is INPUT_DEBOUNCE_SCANS_VOL; a screen where those keys are the
 * primary navigation (e.g. the skin home) can lower it for responsiveness.
 * Clamped to 1..16. */
void input_set_vol_debounce(int scans);

/* Number of queued events. */
unsigned input_pending(void);

/* Queue capacity (events). */
#ifndef INPUT_QUEUE_LEN
#define INPUT_QUEUE_LEN 16
#endif

/* Debounce: consecutive identical samples required before a press
 * registers (stock key scan ref 0x03057eb0 uses 3, and 8 for vol+-). */
#define INPUT_DEBOUNCE_SCANS     3
#define INPUT_DEBOUNCE_SCANS_VOL 8

/* Hold timing, in MILLISECONDS of held time (BRNOMT-80). The ladder used to
 * be counted in input_scan() samples, which made the feel depend on the UI
 * pass rate — and that rate is repaint-bound, not the ~10 ms it was tuned
 * for: a cursor move in a full-screen list repaints and flushes the whole
 * panel (~50-100 ms per pass), while the player screen's band redraws run
 * much faster. Time is now the unit; input_set_clock() supplies it.
 *
 * At the ~50-100 ms/pass of a listing this means: LONG after ~0.5 s of hold,
 * one REPEAT every ~0.25 s, and — after INPUT_FAST_HOLD_MS more of holding —
 * one REPEAT every ~0.05 s, i.e. one per pass, the fastest the UI can move.
 * The ladder table in input.c takes one row per tier; these are the knobs. */
#define INPUT_LONG_MS            500
#define INPUT_REPEAT_MS          250
/* Extra hold past LONG before the fast tier engages. */
#define INPUT_FAST_HOLD_MS       800
/* Fast-tier repeat interval (~one step per UI pass). */
#define INPUT_FAST_REPEAT_MS      50

/* Millisecond clock for the hold ladder (input.c). The target passes ap_ui's
 * DWT-based counter; passing NULL restores the standalone behaviour. If the
 * clock never advances it is dropped automatically and the service falls
 * back to a synthetic per-scan clock (INPUT_FALLBACK_MS_PER_SCAN, sized for
 * the slowest real pass), so repeats keep working either way. */
void input_set_clock(unsigned (*ms)(void));
#define INPUT_FALLBACK_MS_PER_SCAN 80
/* Equal readings before the clock is declared dead. A SAMPLED millisecond
 * clock legitimately repeats when two scans land in the same millisecond
 * (the playing loop runs several passes per ms), so this must be well above
 * that: 3 was declaring the freshly-working SysTick clock dead and dropping
 * the ladder back to scan timing. A truly dead clock stays 0 for this long. */
#define INPUT_CLOCK_STUCK_SCANS     64

/* Which keys auto-repeat while held (the user's hold-to-repeat: the vol
 * keys double as UI up/down). No other key repeats. */
#define INPUT_REPEATS(k) ((k) == KEY_UP || (k) == KEY_DOWN)

#define INPUT_OK        0
#define INPUT_ERR_ARG (-1)
#define INPUT_ERR_FULL (-2)

#endif /* RECHORD_SERVICES_INPUT_H */
