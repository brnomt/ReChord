/*
 * test_panel_init.c -- host tests for the extracted panel init sequence
 * (panel_init_rknanoc.c).
 *
 * Asserts, byte for byte, that both extracted DCS init variants leave the
 * wire exactly as the reference firmware sends them (expectations typed
 * from docs/rewrite/drivers.md "Panel init sequence", which documents the
 * extraction), including the 0x11 sleep-out delay.
 *
 * Build: see run_tests.sh (cc -std=c99 -Wall -Werror).
 */

#include <stdint.h>

#include "display.h"
#include "panel_io_sim.h"
#include "test_util.h"

#define MAX_ARGS 16

typedef struct {
    uint8_t cmd;
    uint8_t nargs;
    uint8_t args[MAX_ARGS];
} exp_step_t;

/* Expected wire stream: variant A (ID byte 0xE8 panel). */
static const exp_step_t exp_a[] = {
    { 0x11, 0, { 0 } },
    { 0xDF, 4, { 0x5A, 0x69, 0x02, 0x01 } },
    { 0x36, 1, { 0x60 } },
    { 0x3A, 1, { 0x05 } },
    { 0xB2, 5, { 0x0B, 0x0B, 0x00, 0x33, 0x33 } },
    { 0xB7, 1, { 0x75 } },
    { 0xBB, 1, { 0x28 } },
    { 0xC0, 1, { 0x2C } },
    { 0xC2, 1, { 0x01 } },
    { 0xC3, 1, { 0x1F } },
    { 0xC5, 1, { 0x2A } },
    { 0xC6, 1, { 0x13 } },
    { 0xD0, 1, { 0xA7 } },
    { 0xD0, 2, { 0xA4, 0xA1 } },
    { 0xD6, 1, { 0xA1 } },
    { 0xE0, 14, { 0xF0, 0x05, 0x0A, 0x06, 0x06, 0x03, 0x2B, 0x32,
                  0x43, 0x36, 0x11, 0x10, 0x2B, 0x32 } },
    { 0xE1, 14, { 0xF0, 0x08, 0x0C, 0x0B, 0x09, 0x24, 0x2B, 0x22,
                  0x43, 0x38, 0x15, 0x16, 0x2F, 0x37 } },
    { 0x21, 0, { 0 } },
    { 0x29, 0, { 0 } },
    { 0x2C, 0, { 0 } }
};

/* Expected wire stream: variant B (every other panel ID). */
static const exp_step_t exp_b[] = {
    { 0x11, 0, { 0 } },
    { 0x36, 1, { 0x60 } },
    { 0x3A, 1, { 0x05 } },
    { 0xA4, 2, { 0x44, 0x44 } },
    { 0xB2, 5, { 0x0C, 0x0C, 0x00, 0x33, 0x33 } },
    { 0xB7, 1, { 0x35 } },
    { 0xBB, 1, { 0x1A } },
    { 0xC0, 1, { 0x2C } },
    { 0xC2, 1, { 0x01 } },
    { 0xC3, 1, { 0x0B } },
    { 0xC4, 1, { 0x20 } },
    { 0xC6, 1, { 0x0F } },
    { 0xD0, 2, { 0xA4, 0xA1 } },
    { 0xD6, 1, { 0xA1 } },
    { 0xE0, 14, { 0xF0, 0x06, 0x0B, 0x07, 0x07, 0x24, 0x2E, 0x32,
                  0x46, 0x37, 0x13, 0x13, 0x2D, 0x33 } },
    { 0xE1, 14, { 0xF0, 0x02, 0x06, 0x09, 0x08, 0x05, 0x29, 0x44,
                  0x42, 0x38, 0x14, 0x14, 0x2A, 0x30 } },
    { 0x2A, 4, { 0x00, 0x00, 0x00, 0xEF } },
    { 0x2B, 4, { 0x00, 0x23, 0x00, 0xCC } },
    { 0x21, 0, { 0 } },
    { 0x29, 0, { 0 } },
    { 0x2C, 0, { 0 } }
};

/* The single delay fact both variants share: 120 ms after sleep out. */
#define EXP_DELAY_TOTAL_MS 120

static void check_stream(const exp_step_t *exp, int nexp, const char *what)
{
    int i, k;

    CHECK(panel_io_sim_cmd_count() == nexp);
    for (i = 0; i < nexp && i < panel_io_sim_cmd_count(); i++) {
        if (panel_io_sim_cmd_at(i) != exp[i].cmd) {
            CHECK(0);
            printf("  %s: cmd %d is 0x%02X, expected 0x%02X\n",
                   what, i, panel_io_sim_cmd_at(i), exp[i].cmd);
            continue;
        }
        CHECK(1);
        CHECK(panel_io_sim_cmd_nargs(i) == (int)exp[i].nargs);
        for (k = 0; k < (int)exp[i].nargs; k++) {
            uint8_t got = panel_io_sim_cmd_arg(i, k);
            if (got != exp[i].args[k]) {
                CHECK(0);
                printf("  %s: cmd %d (0x%02X) arg %d is 0x%02X, "
                       "expected 0x%02X\n",
                       what, i, exp[i].cmd, k, got, exp[i].args[k]);
            } else {
                CHECK(1);
            }
        }
    }
    CHECK(panel_io_sim_delay_total_ms() == EXP_DELAY_TOTAL_MS);
}

static void test_variant_a(void)
{
    panel_io_sim_reset();
    CHECK(panel_init_run_variant(0) == 0);
    check_stream(exp_a, (int)(sizeof exp_a / sizeof exp_a[0]), "variant A");
    /* Sleep out is first and carries the 120 ms delay. */
    CHECK(panel_io_sim_cmd_at(0) == 0x11);
    CHECK(panel_io_sim_cmd_nargs(0) == 0);
}

static void test_variant_b(void)
{
    panel_io_sim_reset();
    CHECK(panel_init_run_variant(1) == 0);
    check_stream(exp_b, (int)(sizeof exp_b / sizeof exp_b[0]), "variant B");
    CHECK(panel_io_sim_cmd_at(0) == 0x11);
    CHECK(panel_io_sim_cmd_nargs(0) == 0);
    /* Variant B programs the extracted visible-band window verbatim. */
    CHECK(panel_io_sim_cmd_at(16) == 0x2A);
    CHECK(panel_io_sim_cmd_at(17) == 0x2B);
}

static void test_bad_variant(void)
{
    panel_io_sim_reset();
    CHECK(panel_init_run_variant(-1) == -1);
    CHECK(panel_init_run_variant(2) == -1);
    CHECK(panel_io_sim_cmd_count() == 0);   /* nothing emitted */
}

static void test_default_matches_a_variant(void)
{
    int n, i, matches_a = 1, matches_b = 1;

    panel_io_sim_reset();
    panel_init_sequence();
    n = panel_io_sim_cmd_count();

    if (n != (int)(sizeof exp_a / sizeof exp_a[0])) {
        matches_a = 0;
    }
    if (n != (int)(sizeof exp_b / sizeof exp_b[0])) {
        matches_b = 0;
    }
    for (i = 0; i < n; i++) {
        if (i >= (int)(sizeof exp_a / sizeof exp_a[0]) ||
            panel_io_sim_cmd_at(i) != exp_a[i].cmd) {
            matches_a = 0;
        }
        if (i >= (int)(sizeof exp_b / sizeof exp_b[0]) ||
            panel_io_sim_cmd_at(i) != exp_b[i].cmd) {
            matches_b = 0;
        }
    }

    /* The compiled-in default must be one of the two extracted streams
     * (PANEL_INIT_VARIANT picks which; both are documented). */
    CHECK(matches_a || matches_b);
    CHECK(panel_io_sim_delay_total_ms() == EXP_DELAY_TOTAL_MS);
}

int main(void)
{
    test_variant_a();
    test_variant_b();
    test_bad_variant();
    test_default_matches_a_variant();
    TEST_END("test_panel_init");
}
