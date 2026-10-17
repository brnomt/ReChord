/*
 * scratch.h — one shared RAM region for operations that cannot overlap.
 *
 * WHY: the AP has only a few KB of .bss headroom under the 0x0304A000 ceiling
 * (DFIU fb41cb15), so a large working buffer cannot be its own static — and
 * several of them (the USB mass-storage chunk, the media index builder) are
 * never needed at the same time. They share this region and each claims it
 * while it works.
 *
 * Ownership is a single owner id (0 = free). scratch_claim() returns NULL when
 * the region is taken, and callers must treat that as "cannot run now" instead
 * of touching the holder's data. A claim lives exactly as long as the
 * operation that made it, so every return path must release it; a claim never
 * survives a reboot, which is what makes the model self-healing.
 */
#ifndef RECHORD_SERVICES_SCRATCH_H
#define RECHORD_SERVICES_SCRATCH_H

#define SCRATCH_OWNER_NONE   0u
#define SCRATCH_OWNER_USB    1u   /* USB mass-storage session            */
#define SCRATCH_OWNER_INDEX  2u   /* media index build (BRNOMT-74)       */
#define SCRATCH_OWNER_FWUPD  3u   /* firmware update / retire            */

/* Size of the region: the largest single user's need (the USB chunk). */
#define SCRATCH_BYTES 32768u

/* Take the region for `owner`. Returns its base address, or NULL when another
 * owner holds it (or the owner id is NONE). */
void *scratch_claim(unsigned owner);

/* Release the region: only the current owner can, anything else is ignored. */
void scratch_release(unsigned owner);

/* Current owner, SCRATCH_OWNER_NONE when free. */
unsigned scratch_owner(void);

/* The region when `owner` is its current owner, else NULL. */
void *scratch_for(unsigned owner);

#endif /* RECHORD_SERVICES_SCRATCH_H */
