/*
 * test_crash_record.c — /.rechord/CRASH text (crash_record.c).
 */
#include <stdio.h>
#include <string.h>

#include "crash_record.h"

static int s_failures;
#define CHECK(c, m) do { if (c) printf("  PASS: %s\n", m); \
    else { printf("  FAIL: %s (line %d)\n", m, __LINE__); s_failures++; } } while (0)

int main(void)
{
    static char buf[2048], big[4096];
    crash_regs_t r = { 0x0305a1b2u, 0x0305a0f1u, 0x21000000u, 0x03003e40u,
                       0x00008200u, 0x40000000u, 0xe000ed34u, 0x00000004u };
    char summary[96];
    unsigned i;
    int off, pad = 1;

    printf("=== crash record ===\n");
    CHECK(crash_format(buf, sizeof buf, &r, "m9j", 123456ul,
                       "1.000 boot: m9j\n2.000 ipc: start\n") == sizeof buf,
          "fills the whole buffer");
    {
        static const char head[] =
            "ReChord crash\nstate=new\nbuild m9j\nuptime 123456 ms\n";

        CHECK(strncmp(buf, head, sizeof head - 1) == 0, "header, build, uptime");
    }
    CHECK(strstr(buf, "fault pc 0x0305a1b2 lr 0x0305a0f1 psr 0x21000000 "
                      "sp 0x03003e40\n") != 0, "fault registers");
    CHECK(strstr(buf, "cfsr 0x00008200 hfsr 0x40000000 mmfar 0xe000ed34 "
                      "bfar 0x00000004\n") != 0, "fault status registers");
    CHECK(strstr(buf, "--- log tail ---\n1.000 boot: m9j\n2.000 ipc: start\n")
              != 0, "log tail after the registers");
    for (i = (unsigned)(strstr(buf, "ipc: start\n") - buf) + 11u; i < sizeof buf; i++)
        if (buf[i] != '\n')
            pad = 0;
    CHECK(pad, "rest padded with newlines (no NUL, readable on a PC)");

    off = crash_find_new(buf, (int)sizeof buf, summary, sizeof summary);
    CHECK(off == 20 && memcmp(buf + off, "new", 3) == 0, "unseen record found");
    CHECK(strcmp(summary, "fault pc 0x0305a1b2 lr 0x0305a0f1 psr 0x21000000 "
                          "sp 0x03003e40") == 0, "summary is the fault line");
    memcpy(buf + off, "old", 3);
    CHECK(crash_find_new(buf, (int)sizeof buf, summary, sizeof summary) < 0,
          "marked seen: not reported again");
    CHECK(crash_find_new("# ReChord crash record: none yet\n", 33, summary,
                         sizeof summary) < 0, "the empty template is no record");

    memset(big, 'x', sizeof big - 1);
    big[sizeof big - 1] = '\0';
    crash_format(buf, sizeof buf, &r, 0, 0, big);
    CHECK(crash_find_new(buf, (int)sizeof buf, summary, sizeof summary) == 20,
          "an oversized tail is cut, the record stays valid");

    printf("%s (%d failure%s)\n", s_failures ? "FAILURES" : "ALL CRASH RECORD TESTS PASSED",
           s_failures, s_failures == 1 ? "" : "s");
    return s_failures != 0;
}
