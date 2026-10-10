/*
 * mocks.h — host-test mock layer for firmware/app tests.
 *
 * Every mock records a short tag into ONE ordered call log, so a test can
 * assert the exact bring-up/loop sequence (that ordering IS the contract
 * being tested). log_printf() is recorded as its formatted message, which
 * doubles as verification of the boot telemetry.
 */
#ifndef RECHORD_APP_TESTS_MOCKS_H
#define RECHORD_APP_TESTS_MOCKS_H

#include "app/boot_params.h"

/* ---- ordered call recorder ---- */
void        mock_rec_reset(void);
int         mock_rec_count(void);
const char *mock_rec_at(int idx);      /* NULL when out of range           */
void        mock_rec(const char *tag); /* used by the mocks below          */

/* ---- scripted input ---- */
/* Deliver (type, adc) on the next input_poll() call. */
void mock_input_queue(int type, int adc_value);
/* Deliver (type, adc) after `delay_polls` further input_poll() calls have
 * returned empty (delay_polls = 0 behaves like mock_input_queue). */
void mock_input_delay(int delay_polls, int type, int adc_value);

/* ---- app test knob (defined in firmware/app/main.c) ---- */
/* Main-loop frame budget: 0 = run forever. Tests set it per scenario. */
extern unsigned rechord_app_max_frames;

/*
 * Boot-param storage on host (see boot_params.h STORAGE CONTRACT): mocks.c
 * plays the role of firmware/startup/startup.c here, so test_boot_params
 * exercises exactly the accessor code the real startup runs.
 */
extern rechord_boot_params_t rechord_boot_params;
extern uint32_t              rechord_boot_params_captured;

#endif /* RECHORD_APP_TESTS_MOCKS_H */
