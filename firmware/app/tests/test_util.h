/*
 * test_util.h — shared CHECK helpers for firmware/app/tests (style follows
 * firmware/dsp/tests and firmware/ui/tests: PASS/FAIL lines + a tally in
 * the runner, so `cc -Wall -Werror` output reads like the other suites).
 */
#ifndef RECHORD_APP_TEST_UTIL_H
#define RECHORD_APP_TEST_UTIL_H

#include <stdio.h>

extern int test_checks;
extern int test_failures;

#define CHECK(cond, name) do {                     \
    test_checks++;                                 \
    if (cond) {                                    \
        printf("  PASS: %s\n", name);              \
    } else {                                       \
        printf("  FAIL: %s\n", name);              \
        test_failures++;                           \
    }                                              \
} while (0)

#endif /* RECHORD_APP_TEST_UTIL_H */
