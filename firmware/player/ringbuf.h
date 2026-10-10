/*
 * ringbuf.h — lock-free SPSC byte ring buffer (ReChord player module).
 *
 * Used to stream decoded PCM from the AP-side player (producer) to the
 * B-core audio path (consumer). Reference protocol fact: the reference
 * firmware logs the pair as "ring rd %u wr %u".
 *
 * Design (why):
 *  - Single-producer / single-consumer needs no locks as long as each index
 *    is written by exactly one side and only read by the other. On the
 *    dual Cortex-M3 we still need memory barriers: the store buffer must
 *    not let the index update overtake the PCM byte copies.
 *  - Free-running counters (no modulo) make used/free arithmetic
 *    wrap-around safe with a single unsigned subtract and match the
 *    "ring rd/wr" wire format.
 *  - Capacity must be a power of two so the wrap is a mask, not a branch
 *    or divide on the M3.
 *  - Overflow (producer outruns consumer) and underflow (consumer underrun)
 *    are handled by clamping: rb_write/rb_read return the byte count
 *    actually transferred and never corrupt indices.
 */
#ifndef RECHORD_RINGBUF_H
#define RECHORD_RINGBUF_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t       *data;      /* backing store, `capacity` bytes        */
    size_t         capacity;  /* power of two                           */
    volatile size_t rd;       /* free-running read index (consumer)     */
    volatile size_t wr;       /* free-running write index (producer)    */
} ringbuf_t;

/* Initialize an empty ring. `capacity` MUST be a power of two (checked).
 * Returns 0 on success, -1 on bad arguments. */
int rb_init(ringbuf_t *rb, void *storage, size_t capacity);

/* Force back to empty. ONLY legal while the other side is stopped (e.g.
 * flush-on-seek while the B core is idle) — both indices move together. */
void rb_reset(ringbuf_t *rb);

size_t rb_capacity(const ringbuf_t *rb);
size_t rb_used(const ringbuf_t *rb);   /* bytes buffered   (wr - rd)   */
size_t rb_free(const ringbuf_t *rb);   /* bytes available  (cap-used)  */

/* Copy up to `len` bytes in/out; returns the byte count actually
 * transferred (clamped to free/used). `src`/`dst` may be NULL only when
 * `len` is 0. */
size_t rb_write(ringbuf_t *rb, const void *src, size_t len);
size_t rb_read(ringbuf_t *rb, void *dst, size_t len);

/* Drop up to `len` buffered bytes from the read side (consumer). Returns
 * the byte count actually dropped. */
size_t rb_skip(ringbuf_t *rb, size_t len);

#endif /* RECHORD_RINGBUF_H */
