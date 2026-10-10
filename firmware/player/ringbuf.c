/*
 * ringbuf.c — lock-free SPSC byte ring buffer (see ringbuf.h).
 *
 * No libc, no allocation: usable from the freestanding target build and
 * from host tests alike.
 */
#include "ringbuf.h"

/*
 * Memory barrier around index publication.
 *  - Cortex-M3 (ARMv7-M): a DMB keeps the PCM byte stores ahead of the
 *    index store as observed by the other core / DMA consumer.
 *  - Host (x86 TSO): a compiler barrier is sufficient for the host tests,
 *    which exercise data movement and index arithmetic, not SMP races.
 */
#if defined(__arm__)
#define RB_MEMBAR() __asm__ __volatile__("dmb sy" ::: "memory")
#else
#define RB_MEMBAR() __asm__ __volatile__("" ::: "memory")
#endif

/* Free-running counters wrap safely with unsigned subtraction as long as
 * used <= capacity, which every mutator below maintains. */
static int rb_is_pow2(size_t v)
{
    return (v != 0) && ((v & (v - 1)) == 0);
}

int rb_init(ringbuf_t *rb, void *storage, size_t capacity)
{
    if (rb == NULL || storage == NULL || !rb_is_pow2(capacity))
        return -1;
    rb->data = (uint8_t *)storage;
    rb->capacity = capacity;
    rb->rd = 0;
    rb->wr = 0;
    return 0;
}

void rb_reset(ringbuf_t *rb)
{
    if (rb == NULL)
        return;
    rb->rd = 0;
    rb->wr = 0;
    RB_MEMBAR();
}

size_t rb_capacity(const ringbuf_t *rb)
{
    return (rb != NULL) ? rb->capacity : 0;
}

size_t rb_used(const ringbuf_t *rb)
{
    if (rb == NULL)
        return 0;
    return rb->wr - rb->rd;   /* unsigned: survives counter wrap-around */
}

size_t rb_free(const ringbuf_t *rb)
{
    if (rb == NULL)
        return 0;
    return rb->capacity - rb_used(rb);
}

size_t rb_write(ringbuf_t *rb, const void *src, size_t len)
{
    size_t free_bytes, n, head;
    size_t w;
    const uint8_t *s = (const uint8_t *)src;

    if (rb == NULL || (src == NULL && len != 0))
        return 0;

    free_bytes = rb_free(rb);
    n = (len < free_bytes) ? len : free_bytes;   /* clamp on overflow */
    if (n == 0)
        return 0;

    w = rb->wr;
    head = w & (rb->capacity - 1);
    if (head + n <= rb->capacity) {
        /* single contiguous segment */
        size_t i;
        for (i = 0; i < n; i++)
            rb->data[head + i] = s[i];
    } else {
        /* split across the wrap point */
        size_t first = rb->capacity - head;
        size_t i;
        for (i = 0; i < first; i++)
            rb->data[head + i] = s[i];
        for (i = first; i < n; i++)
            rb->data[i - first] = s[i];
    }

    RB_MEMBAR();          /* data must land before the index is published */
    rb->wr = w + n;
    return n;
}

size_t rb_read(ringbuf_t *rb, void *dst, size_t len)
{
    size_t used, n, head, r;
    uint8_t *d = (uint8_t *)dst;

    if (rb == NULL || (dst == NULL && len != 0))
        return 0;

    used = rb_used(rb);
    n = (len < used) ? len : used;   /* clamp on underflow */
    if (n == 0)
        return 0;

    r = rb->rd;
    head = r & (rb->capacity - 1);
    if (head + n <= rb->capacity) {
        size_t i;
        for (i = 0; i < n; i++)
            d[i] = rb->data[head + i];
    } else {
        size_t first = rb->capacity - head;
        size_t i;
        for (i = 0; i < first; i++)
            d[i] = rb->data[head + i];
        for (i = first; i < n; i++)
            d[i] = rb->data[i - first];
    }

    RB_MEMBAR();          /* consume the data before publishing the index */
    rb->rd = r + n;
    return n;
}

size_t rb_skip(ringbuf_t *rb, size_t len)
{
    size_t used, n;

    if (rb == NULL)
        return 0;
    used = rb_used(rb);
    n = (len < used) ? len : used;
    RB_MEMBAR();
    rb->rd = rb->rd + n;
    return n;
}
