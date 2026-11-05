/* mem.c — memory primitives for the freestanding libc (see libc.h). */
#include <stdint.h>

#include "libc.h"

/* Word-at-a-time fills and copies (BRNOMT-97). The byte loops these replace
 * were ~60 % of an animated skin frame on the AP (struct copies and the
 * per-section memset compile to these calls) and run on Core B too. Word
 * accesses only where both pointers share 4-byte alignment; the rest stays
 * bytewise. GCC must not turn these loops back into memset/memcpy calls. */
typedef uint32_t __attribute__((may_alias)) mem_word_t;
#define MEM_NO_LIBCALL __attribute__((optimize("no-tree-loop-distribute-patterns")))

MEM_NO_LIBCALL void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    unsigned char b = (unsigned char)c;

    while (n > 0 && ((uintptr_t)d & 3u) != 0) {
        *d++ = b;
        n--;
    }
    if (n >= 4) {
        mem_word_t *w = (mem_word_t *)(void *)d;
        uint32_t v = (uint32_t)b * 0x01010101u;

        for (; n >= 16; n -= 16, w += 4) {
            w[0] = v;
            w[1] = v;
            w[2] = v;
            w[3] = v;
        }
        for (; n >= 4; n -= 4)
            *w++ = v;
        d = (unsigned char *)w;
    }
    while (n--)
        *d++ = b;
    return dst;
}

/* Forward copy; word loop when dst and src are congruent mod 4. Like the old
 * byte loop it is also safe for overlapping dst < src. */
MEM_NO_LIBCALL static void copy_fwd(unsigned char *d, const unsigned char *s,
                                    size_t n)
{
    if ((((uintptr_t)d ^ (uintptr_t)s) & 3u) == 0) {
        while (n > 0 && ((uintptr_t)d & 3u) != 0) {
            *d++ = *s++;
            n--;
        }
        {
            mem_word_t *wd = (mem_word_t *)(void *)d;
            const mem_word_t *ws = (const mem_word_t *)(const void *)s;

            for (; n >= 16; n -= 16, wd += 4, ws += 4) {
                uint32_t a = ws[0], b = ws[1], c = ws[2], e = ws[3];

                wd[0] = a;
                wd[1] = b;
                wd[2] = c;
                wd[3] = e;
            }
            for (; n >= 4; n -= 4)
                *wd++ = *ws++;
            d = (unsigned char *)wd;
            s = (const unsigned char *)ws;
        }
    }
    while (n--)
        *d++ = *s++;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    copy_fwd((unsigned char *)dst, (const unsigned char *)src, n);
    return dst;
}

MEM_NO_LIBCALL void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    if (d == s || n == 0)
        return dst;
    if (d < s) {                       /* forward copy is safe when dst < src */
        copy_fwd(d, s, n);
    } else {
        d += n;                        /* copy backwards to survive overlap */
        s += n;
        while (n--)
            *--d = *--s;
    }
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = (const unsigned char *)a;
    const unsigned char *y = (const unsigned char *)b;
    for (; n--; x++, y++)
        if (*x != *y)
            return (int)*x - (int)*y;
    return 0;
}


/* ARM C Library ABI memory routines (called by the prebuilt codec .lib
 * objects). The toolchain ships no newlib, so we provide them over the
 * primitives above - required for linking the vendor codec archives. */
void __aeabi_memcpy(void *dst, const void *src, unsigned int n)  { memcpy(dst, src, n); }
void __aeabi_memcpy4(void *dst, const void *src, unsigned int n) { memcpy(dst, src, n); }
void __aeabi_memcpy8(void *dst, const void *src, unsigned int n) { memcpy(dst, src, n); }
void __aeabi_memmove(void *dst, const void *src, unsigned int n) { memmove(dst, src, n); }
void __aeabi_memmove4(void *dst, const void *src, unsigned int n){ memmove(dst, src, n); }
void __aeabi_memmove8(void *dst, const void *src, unsigned int n){ memmove(dst, src, n); }
void __aeabi_memset(void *dst, unsigned int n, int c)            { memset(dst, c, n); }
void __aeabi_memset4(void *dst, unsigned int n, int c)           { memset(dst, c, n); }
void __aeabi_memset8(void *dst, unsigned int n, int c)           { memset(dst, c, n); }
void __aeabi_memclr(void *dst, unsigned int n)                   { memset(dst, 0, n); }
void __aeabi_memclr4(void *dst, unsigned int n)                  { memset(dst, 0, n); }
void __aeabi_memclr8(void *dst, unsigned int n)                  { memset(dst, 0, n); }
