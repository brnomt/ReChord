/*
 * crash_record.h — text of /.rechord/CRASH (pure, host-testable).
 *
 * The fault handler (startup/crash.c) formats the record and writes it raw to
 * the file's sectors; the next boot finds it, copies the summary into the log
 * and marks it seen. The file stays readable from a PC:
 *
 *     ReChord crash
 *     state=new
 *     build m9j
 *     uptime 123456 ms
 *     fault pc 0x0305a1b2 lr 0x0305a0f1 psr 0x21000000 sp 0x03003e40
 *     cfsr 0x00008200 hfsr 0x40000000 mmfar 0xe000ed34 bfar 0x00000004
 *     --- log tail ---
 *     ...the last log lines...
 */
#ifndef RECHORD_SERVICES_CRASH_RECORD_H
#define RECHORD_SERVICES_CRASH_RECORD_H

#include <stdint.h>

typedef struct {
    uint32_t pc, lr, psr, sp;        /* stacked by the exception           */
    uint32_t cfsr, hfsr, mmfar, bfar; /* SCB fault status / address        */
} crash_regs_t;

/* Fill `out` with the record, the log tail last, and pad the rest of `cap`
 * with '\n' (the whole buffer is written to disk). Returns cap. */
unsigned crash_format(char *out, unsigned cap, const crash_regs_t *r,
                      const char *build, unsigned long uptime_ms,
                      const char *tail);

/* A record that has not been reported yet: returns the byte offset of "new"
 * in "state=new" (patch it to "old") and copies the "fault ..." line into
 * `summary`; -1 when the text is no unseen record. */
int crash_find_new(const char *text, int len, char *summary, unsigned cap);

#endif /* RECHORD_SERVICES_CRASH_RECORD_H */
