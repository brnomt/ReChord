/* mb1_stub.c — ReChord M-B1 boot stub (Route B hybrid image).
 *
 * Hooked from the Reset veneer at 0x0306296E (its single b.w replaced with
 * b.w here), this stub paints a boot line and writes a log entry using
 * RefCFW's own primitives, then branches to the REAL startup entry at
 * 0x030500E4 (SP init, boot param capture, .data/.bss, main with
 * USB/logging/UI) so the original boot runs untouched.
 *
 * Safety: if the panel is not up yet the draw calls merely miss; boot
 * continues normally. USB and ECHOLOG stay RefCFW's code — no lockout path.
 */
#include <stdint.h>

/* RefCFW service entry points (verified in ReChordV2, RAM image @ 0x0304F490):
 *   status_line(id, color, fmt, ...)  0x03050550  (calls draw_init one-shot)
 *   log(fmt, ...)                     0x03055B0C
 * real startup entry (veneer target)  0x030500E4
 */
typedef void (*status_fn)(int, uint32_t, const char *, ...);
typedef void (*log_fn)(const char *, ...);

static const char banner[] = "ReChord 0.1 (M-B1)";
static const char logfmt[] = "ReChord %s: boot handoff to RefCFW main";

#define STATUS_LINE ((status_fn)0x03050551u)
#define LOG         ((log_fn)0x03055B0Du)

void rechord_boot(void)
{
    STATUS_LINE(11, 0xFA47u, banner);     /* row 11 (y=160): below main's
                                             boot rows (ids 0-10) so it is
                                             not overwritten — visible proof
                                             the hook executed */
    LOG(logfmt, "0.1");
    /* branch to the real startup entry (never returns here) */
    ((void (*)(void))0x030500E4u)();
}
