/*
 * test_util.h — tiny host test harness (no dependencies).
 *
 * CHECK* macros count checks and failures; the runner (tests/main.c) prints
 * a summary and exits non-zero on any failure.
 */
#ifndef RECHORD_UI_TEST_UTIL_H
#define RECHORD_UI_TEST_UTIL_H

#include <stdio.h>
#include <string.h>

extern int test_checks;
extern int test_failures;

static inline void test_record(int ok, const char *file, int line,
                               const char *what)
{
    test_checks++;
    if (!ok) {
        test_failures++;
        printf("FAIL %s:%d: %s\n", file, line, what);
    }
}

#define CHECK(expr) \
    test_record((expr) ? 1 : 0, __FILE__, __LINE__, #expr)

#define CHECK_EQ_INT(actual, expected)                                  \
    do {                                                                \
        long a_ = (long)(actual);                                       \
        long e_ = (long)(expected);                                     \
        test_checks++;                                                  \
        if (a_ != e_) {                                                 \
            test_failures++;                                            \
            printf("FAIL %s:%d: %s == %s (got %ld, want %ld)\n",        \
                   __FILE__, __LINE__, #actual, #expected, a_, e_);     \
        }                                                               \
    } while (0)

#define CHECK_STR_EQ(actual, expected)                                  \
    do {                                                                \
        const char *a_ = (actual);                                      \
        const char *e_ = (expected);                                    \
        test_checks++;                                                  \
        if (a_ == NULL || strcmp(a_, e_) != 0) {                        \
            test_failures++;                                            \
            printf("FAIL %s:%d: %s == \"%s\" (got \"%s\")\n",           \
                   __FILE__, __LINE__, #actual, e_,                     \
                   a_ ? a_ : "(null)");                                 \
        }                                                               \
    } while (0)

#endif /* RECHORD_UI_TEST_UTIL_H */
