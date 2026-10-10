/*
 * test_util.h — shared helpers for firmware/modules/tests/ host tests.
 *
 * Style follows firmware/test_dsp_core.c and firmware/dsp/tests/test_util.h:
 * PASS/FAIL lines and a final tally (exit code = failure count).
 */
#ifndef RECHORD_MODULES_TEST_UTIL_H
#define RECHORD_MODULES_TEST_UTIL_H

#include <stdint.h>
#include <stdio.h>

static int failures = 0;

#define CHECK(cond, name) do { \
    if (cond) { printf("  PASS: %s\n", name); } \
    else { printf("  FAIL: %s\n", name); failures++; } \
} while (0)

#define CHECK_EQ_U32(got, want, name) do { \
    uint32_t g_ = (uint32_t)(got), w_ = (uint32_t)(want); \
    if (g_ == w_) { printf("  PASS: %s\n", name); } \
    else { \
        printf("  FAIL: %s (got 0x%08x, want 0x%08x)\n", name, g_, w_); \
        failures++; \
    } \
} while (0)

#endif /* RECHORD_MODULES_TEST_UTIL_H */
