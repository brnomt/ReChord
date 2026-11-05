/*
 * crash_record.c — see crash_record.h.
 */
#include <stdio.h>
#include <string.h>

#include "crash_record.h"

static const char k_head[] = "ReChord crash\nstate=new\n";

unsigned crash_format(char *out, unsigned cap, const crash_regs_t *r,
                      const char *build, unsigned long uptime_ms,
                      const char *tail)
{
    int n;
    unsigned len, t;

    if (out == 0 || cap == 0u || r == 0)
        return 0;
    n = snprintf(out, cap,
                 "%sbuild %s\nuptime %lu ms\n"
                 "fault pc 0x%08x lr 0x%08x psr 0x%08x sp 0x%08x\n"
                 "cfsr 0x%08x hfsr 0x%08x mmfar 0x%08x bfar 0x%08x\n"
                 "--- log tail ---\n",
                 k_head, build != 0 ? build : "?", uptime_ms,
                 (unsigned)r->pc, (unsigned)r->lr, (unsigned)r->psr,
                 (unsigned)r->sp, (unsigned)r->cfsr, (unsigned)r->hfsr,
                 (unsigned)r->mmfar, (unsigned)r->bfar);
    len = (n < 0) ? 0u : ((unsigned)n >= cap ? cap - 1u : (unsigned)n);
    if (tail != 0) {
        t = (unsigned)strlen(tail);
        if (t > cap - 1u - len)
            t = cap - 1u - len;
        memcpy(out + len, tail, t);
        len += t;
    }
    while (len < cap)
        out[len++] = '\n';
    return cap;
}

int crash_find_new(const char *text, int len, char *summary, unsigned cap)
{
    int i;

    if (text == 0 || len < (int)sizeof k_head - 1 ||
        memcmp(text, k_head, sizeof k_head - 1u) != 0)
        return -1;
    if (summary != 0 && cap > 0u) {
        summary[0] = '\0';
        for (i = 0; i + 6 < len; i++) {
            if ((i == 0 || text[i - 1] == '\n') &&
                memcmp(text + i, "fault ", 6) == 0) {
                unsigned k = 0;

                while (i + (int)k < len && text[i + (int)k] != '\n' &&
                       k < cap - 1u) {
                    summary[k] = text[i + (int)k];
                    k++;
                }
                summary[k] = '\0';
                break;
            }
        }
    }
    return (int)(sizeof "ReChord crash\nstate=" - 1u);
}
