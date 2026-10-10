/*
 * test_util.h -- minimal check harness for ReChord host tests (plain C).
 *
 * Each test binary includes this once, calls CHECK(...) for every
 * assertion and finishes with TEST_END("name").
 */

#ifndef RECHORD_TEST_UTIL_H
#define RECHORD_TEST_UTIL_H

#include <stdio.h>

static int test_checks = 0;
static int test_failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        test_checks++;                                                     \
        if (!(cond)) {                                                     \
            test_failures++;                                               \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
        }                                                                  \
    } while (0)

#define TEST_END(name)                                                     \
    do {                                                                   \
        printf("%s: %d checks, %d failures\n",                             \
               (name), test_checks, test_failures);                        \
        return (test_failures > 0) ? 1 : 0;                                \
    } while (0)

#endif /* RECHORD_TEST_UTIL_H */
