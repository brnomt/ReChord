/*
 * jack_sense.h — headphone-jack sense debouncer (pure, host-testable).
 *
 * Two sense pins name the plugged jack (hardware mapping from EchoPod's
 * bring-up on the same player):
 *
 *   balanced_high  single_ended_high   -> jack
 *   -------------------------------------------------
 *   false          (don't care)        -> balanced (4.4 mm)
 *   true           true                -> single-ended (3.5 mm)
 *   true           false               -> nothing plugged
 *
 * A contact bounces while it seats, so a change is only accepted after
 * JACK_SENSE_SAMPLES consecutive matching readings. The first reading is
 * taken at once (boot baseline). Used by drivers/jack.c and BRNOMT-70
 * (pause playback when the headphones are pulled).
 */
#ifndef RECHORD_SERVICES_JACK_SENSE_H
#define RECHORD_SERVICES_JACK_SENSE_H

#include <stdint.h>

typedef enum {
    JACK_NONE = 0,
    JACK_SINGLE_ENDED,
    JACK_BALANCED
} jack_kind_t;

#define JACK_SENSE_SAMPLES 7

typedef struct {
    uint8_t  started;
    jack_kind_t stable;
    jack_kind_t candidate;
    uint8_t  matches;
} jack_sense_t;

jack_kind_t jack_kind_of(int single_ended_high, int balanced_high);

/* Feed one reading. Returns 1 when `stable` changed (including the first
 * sample), 0 while nothing changed or the candidate is still debouncing. */
int jack_sense_sample(jack_sense_t *sense, jack_kind_t reading);

#endif /* RECHORD_SERVICES_JACK_SENSE_H */
