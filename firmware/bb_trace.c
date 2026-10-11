/*
 * bb_trace.c - crash-hunt breadcrumbs for the baseband.
 *
 * Each call opens BBTRACE.TXT, appends one line and closes it again: the
 * close is what updates the directory entry, so the line survives the
 * watchdog power-off that follows a mailbox deadlock. The file accumulates
 * across boots (append), so one test run leaves the whole history.
 *
 * Uses the real file layer (filesys/) directly - if the FS itself is what
 * breaks, the trace simply stays empty and that is also an answer.
 */
#include "typedef.h"
#include "filesys/file.h"
#include "bb_trace.h"

static unsigned int s_off;
static unsigned int s_init;

static void bb_trace_raw(const char *msg, unsigned int n)
{
    HANDLE h;

    if (n == 0)
        return;

    h = FileOpenA((uint8 *)"\\", (uint8 *)"BBTRACE.TXT", (uint8 *)"WR");
    if (h == NOT_OPEN_FILE) {
        h = FileCreate((uint8 *)"\\", (uint8 *)"BBTRACE.TXT");
        s_init = 0;
    }
    if (h == NOT_OPEN_FILE)
        return;

    if (!s_init) {
        s_off = OpenFileInfo[h].FileSize;   /* append across boots */
        s_init = 1;
    }
    FileWrite((uint8 *)msg, s_off, n, h);
    s_off += n;
    FileClose(h);
}

void bb_trace(const char *msg)
{
    unsigned int n = 0;
    while (msg[n])
        n++;
    bb_trace_raw(msg, n);
}

void bb_trace_num(const char *msg, unsigned int v)
{
    char buf[48];
    unsigned int n = 0, sh, d;

    while (msg[n] && n < 32) {
        buf[n] = msg[n];
        n++;
    }
    buf[n++] = '0';
    buf[n++] = 'x';
    for (sh = 28;; sh -= 4) {
        d = (v >> sh) & 15u;
        buf[n++] = (char)(d < 10 ? '0' + d : 'A' + d - 10);
        if (sh == 0)
            break;
    }
    buf[n++] = '\n';
    bb_trace_raw(buf, n);
}
