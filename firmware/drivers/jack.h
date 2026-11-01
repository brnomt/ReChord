/*
 * jack.h — headphone jack detection (GPIO sense pins).
 *
 * Pins (same player, from EchoPod's hardware map):
 *   GPIO0.13  single-ended (3.5 mm) sense
 *   GPIO2.8   balanced (4.4 mm) sense
 *
 * The pins are muxed to GPIO inputs; readings are debounced by
 * services/jack_sense.c. Callers must ignore the readings while audio is
 * routed to Bluetooth (the routing fakes a plugged headphone) — the app does
 * that by not calling jack_poll() then.
 */
#ifndef RECHORD_DRIVERS_JACK_H
#define RECHORD_DRIVERS_JACK_H

#include "../services/jack_sense.h"

/* Mux both sense pins as GPIO inputs. Idempotent. */
void jack_init(void);

/* Sample both pins. Returns 1 when the stable jack changed and writes the new
 * state to *out; 0 otherwise (or while debouncing). */
int jack_poll(jack_kind_t *out);

/* Last stable state (JACK_NONE before the first poll). */
jack_kind_t jack_current(void);

#endif /* RECHORD_DRIVERS_JACK_H */
