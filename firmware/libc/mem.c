/* mem.c — memory primitives for the freestanding libc (see libc.h). */
#include "libc.h"

void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    while (n--)
        *d++ = (unsigned char)c;
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    if (d == s || n == 0)
        return dst;
    if (d < s) {                       /* forward copy is safe when dst < src */
        while (n--)
            *d++ = *s++;
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
