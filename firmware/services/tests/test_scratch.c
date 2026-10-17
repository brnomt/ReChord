/*
 * test_scratch.c — host tests for the shared scratch region.
 *
 * Covers: claim/release, the "taken" answer, re-entrant claims by the same
 * owner, that only the owner gets the pointer, alignment, and that a release
 * by a non-owner changes nothing.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "scratch.h"

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

int main(void)
{
    void *p;

    printf("== scratch: shared region ==\n");
    CHECK(scratch_owner() == SCRATCH_OWNER_NONE);
    CHECK(scratch_for(SCRATCH_OWNER_USB) == NULL);
    CHECK(scratch_claim(SCRATCH_OWNER_NONE) == NULL);

    p = scratch_claim(SCRATCH_OWNER_USB);
    CHECK(p != NULL);
    CHECK(((uintptr_t)p % 8u) == 0u);              /* 8-byte aligned  */
    CHECK(scratch_owner() == SCRATCH_OWNER_USB);
    CHECK(scratch_for(SCRATCH_OWNER_USB) == p);
    CHECK(scratch_for(SCRATCH_OWNER_INDEX) == NULL);

    /* Taken: another owner gets nothing, the holder keeps working. */
    CHECK(scratch_claim(SCRATCH_OWNER_INDEX) == NULL);
    CHECK(scratch_owner() == SCRATCH_OWNER_USB);
    /* The same owner may claim again (idempotent) and gets the same base. */
    CHECK(scratch_claim(SCRATCH_OWNER_USB) == p);

    /* A non-owner cannot release it. */
    scratch_release(SCRATCH_OWNER_INDEX);
    CHECK(scratch_owner() == SCRATCH_OWNER_USB);

    scratch_release(SCRATCH_OWNER_USB);
    CHECK(scratch_owner() == SCRATCH_OWNER_NONE);
    CHECK(scratch_for(SCRATCH_OWNER_USB) == NULL);

    /* Handover: the next owner gets the same region. */
    p = scratch_claim(SCRATCH_OWNER_INDEX);
    CHECK(p != NULL && scratch_for(SCRATCH_OWNER_INDEX) == p);
    CHECK(scratch_claim(SCRATCH_OWNER_FWUPD) == NULL);
    scratch_release(SCRATCH_OWNER_INDEX);
    CHECK(scratch_claim(SCRATCH_OWNER_FWUPD) == p);
    scratch_release(SCRATCH_OWNER_FWUPD);
    CHECK(scratch_owner() == SCRATCH_OWNER_NONE);

    if (s_failures == 0)
        printf("test_scratch: %d checks, OK\n", s_checks);
    else
        printf("test_scratch: %d checks, %d FAILURES\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
