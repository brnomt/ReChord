/*
 * test_util.h — shared helpers for firmware/player/tests host tests.
 *
 * Style follows firmware/dsp/tests/test_util.h: PASS/FAIL lines and a
 * final tally; main() returns the failure count so `set -e` shell runners
 * stop on the first broken test binary.
 */
#ifndef PLAYER_TEST_UTIL_H
#define PLAYER_TEST_UTIL_H

#include <stdio.h>

static int failures = 0;

#define CHECK(cond, name) do { \
    if (cond) { printf("  PASS: %s\n", name); } \
    else { printf("  FAIL: %s\n", name); failures++; } \
} while (0)

/* Integer comparison with values printed (debuggable failures). */
#define CHECK_EQI(got, want, name) do { \
    long g_ = (long)(got), w_ = (long)(want); \
    if (g_ == w_) { printf("  PASS: %s\n", name); } \
    else { printf("  FAIL: %s (got %ld, want %ld)\n", name, g_, w_); \
           failures++; } \
} while (0)

#define TEST(name) printf("== %s ==\n", name)

#endif /* PLAYER_TEST_UTIL_H */
