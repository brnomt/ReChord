/* str.c — string primitives for the freestanding libc (see libc.h). */
#include "libc.h"

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++) != '\0')
        ;
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    char *d = dst;
    while (n && *src) {
        *d++ = *src++;
        n--;
    }
    while (n--)                       /* strncpy pads with NULs, by contract */
        *d++ = '\0';
    return dst;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    for (; n; n--, a++, b++) {
        if (*a != *b)
            return (int)(unsigned char)*a - (int)(unsigned char)*b;
        if (!*a)
            break;
    }
    return 0;
}


/* Wide-string helpers (vendor plug/bsp/BSP.h signatures). The vendor
 * implementation lives in the AP-side BSP.c; the BB links its own so the
 * build stays self-contained (build-from-source rule). */
typedef unsigned short u16_t;

unsigned int StrLenW(u16_t *pstr)
{
    unsigned int n = 0;
    while (pstr[n])
        n++;
    return n;
}

int StrCmpW(u16_t *pstr1, u16_t *pstr2, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++) {
        if (pstr1[i] != pstr2[i])
            return (int)pstr1[i] - (int)pstr2[i];
        if (!pstr1[i])
            break;
    }
    return 0;
}


/* UTF-16 -> ASCII (vendor plug/bsp/BSP.h signature; BB links its own). */
unsigned int Unicode2Ascii(unsigned char *pbAscii, unsigned short *pwUnicode, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++) {
        unsigned short c = pwUnicode[i];
        if (!c) { pbAscii[i] = 0; break; }
        pbAscii[i] = (c < 0x80) ? (unsigned char)c : '?';
    }
    return i;
}


/* Case-insensitive bounded compare (POSIX; used by the ID3 parser). */
int strncasecmp(const char *a, const char *b, unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)a[i], cb = (unsigned char)b[i];
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return (int)ca - (int)cb;
        if (!ca) break;
    }
    return 0;
}


int strcasecmp(const char *a, const char *b)
{
    while (*a && *b) {
        unsigned char ca = (unsigned char)*a++, cb = (unsigned char)*b++;
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return (int)ca - (int)cb;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}
