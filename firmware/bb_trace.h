/*
 * bb_trace.h - crash-hunt breadcrumbs for the baseband.
 *
 * Writes one line at a time to BBTRACE.TXT on the user volume so that after
 * a mailbox deadlock (watchdog power-off) the last line shows where the BB
 * died. Thread-mode only (file I/O).
 */
#ifndef BB_TRACE_H
#define BB_TRACE_H

void bb_trace(const char *msg);
void bb_trace_num(const char *msg, unsigned int v);

#endif /* BB_TRACE_H */
