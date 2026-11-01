/*
 * jack_sense.c — see jack_sense.h.
 *
 * Behaviour: the first sample is adopted immediately; afterwards a reading
 * must repeat JACK_SENSE_SAMPLES times before it becomes the stable state.
 * A reading that does not match the candidate restarts the count, so a
 * bouncing contact cannot produce a new state.
 */
#include "jack_sense.h"

jack_kind_t jack_kind_of(int single_ended_high, int balanced_high)
{
    if (!balanced_high)
        return JACK_BALANCED;
    return single_ended_high ? JACK_SINGLE_ENDED : JACK_NONE;
}

int jack_sense_sample(jack_sense_t *sense, jack_kind_t reading)
{
    if (sense == 0)
        return 0;
    if (!sense->started) {
        sense->started = 1;
        sense->stable = reading;
        sense->candidate = reading;
        sense->matches = 0;
        return 1;
    }
    if (reading == sense->stable) {
        sense->matches = 0;
        return 0;
    }
    if (reading != sense->candidate) {
        sense->candidate = reading;
        sense->matches = 0;
    }
    sense->matches++;
    if (sense->matches < JACK_SENSE_SAMPLES)
        return 0;
    sense->stable = reading;
    sense->matches = 0;
    return 1;
}
