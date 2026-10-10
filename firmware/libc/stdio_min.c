/* stdio_min.c — minimal formatted output (see libc.h for supported forms).
 *
 * WHY this exists: the SDK's logging/printf paths need vsnprintf/printf/puts
 * and the toolchain has no newlib. The device's own log formats never use
 * %f (they print "%u.%03u" style), so float conversion is intentionally out
 * of scope; anything else the linker asks for joins the tested surface.
 */
#include <stdint.h>
#include "libc.h"

/* Weak console sink: the board glue provides the real one (UART/USB log);
 * host tests override it to capture output. */
__attribute__((weak)) void rechord_console_write(const char *s, int n)
{
    (void)s;
    (void)n;
}

static int out_char(char **p, char *end, char c)
{
    if (*p < end)
        *(*p)++ = c;
    return 1;                          /* return the would-be length */
}

static int out_str(char **p, char *end, const char *s, int width, int left, int pad)
{
    int len = 0;
    const char *q = s;
    while (*q++)
        len++;
    int padn = width > len ? width - len : 0;
    int written = 0;
    if (!left)
        while (padn-- > 0)
            written += out_char(p, end, (char)pad);
    while (*s)
        written += out_char(p, end, *s++);
    if (left)
        while (padn-- > 0)
            written += out_char(p, end, (char)pad);
    return written;
}

static int out_num(char **p, char *end, unsigned long v, int base, int upper,
                   int neg, int width, int left, int pad, int prec)
{
    char tmp[24];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int n = 0;
    if (v == 0)
        tmp[n++] = '0';
    while (v) {
        tmp[n++] = digits[v % (unsigned)base];
        v /= (unsigned)base;
    }
    while (n < prec && n < (int)sizeof tmp)
        tmp[n++] = '0';                /* zero precision extension */
    int body = n + (neg ? 1 : 0);
    int padn = width > body ? width - body : 0;
    int written = 0;
    if (!left && pad == ' ')           /* sign before zero padding */
        while (padn-- > 0)
            written += out_char(p, end, ' ');
    if (neg)
        written += out_char(p, end, '-');
    if (!left && pad == '0')
        while (padn-- > 0)
            written += out_char(p, end, '0');
    while (n > 0)
        written += out_char(p, end, tmp[--n]);
    if (left)
        while (padn-- > 0)
            written += out_char(p, end, ' ');
    return written;
}

int vsnprintf(char *buf, size_t cap, const char *fmt, va_list ap)
{
    char *p = buf;
    char *end = (cap > 0) ? buf + cap - 1 : buf;   /* always NUL-terminate */

    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            out_char(&p, end, *fmt);
            continue;
        }
        fmt++;
        int left = 0, pad = ' ', width = 0, prec = -1;
        for (; *fmt == '-' || *fmt == '0' || *fmt == '+' || *fmt == ' ';
             fmt++) {
            if (*fmt == '-')
                left = 1;
            else if (*fmt == '0')
                pad = '0';
            /* '+', ' ' flags are parsed and ignored: SDK formats don't rely
             * on them and ignoring keeps the parser simple and total. */
        }
        while (*fmt >= '0' && *fmt <= '9')
            width = width * 10 + (*fmt++ - '0');
        if (*fmt == '.') {
            fmt++;
            prec = 0;
            while (*fmt >= '0' && *fmt <= '9')
                prec = prec * 10 + (*fmt++ - '0');
        }
        if (*fmt == 'l') {             /* long modifier: consume, same path */
            fmt++;
            if (*fmt == 'l')
                fmt++;
        }
        switch (*fmt) {
        case 'c': {
            char c = (char)va_arg(ap, int);
            out_char(&p, end, c);
            break;
        }
        case 's': {
            const char *s = va_arg(ap, const char *);
            out_str(&p, end, s ? s : "(null)", width, left, pad);
            break;
        }
        case 'd':
        case 'i': {
            long v = va_arg(ap, int);
            unsigned long uv = (v < 0) ? (unsigned long)(-v) : (unsigned long)v;
            out_num(&p, end, uv, 10, 0, v < 0, width, left, pad, prec > 0 ? prec : 0);
            break;
        }
        case 'u': {
            unsigned long v = va_arg(ap, unsigned int);
            out_num(&p, end, v, 10, 0, 0, width, left, pad, prec > 0 ? prec : 0);
            break;
        }
        case 'x':
        case 'X': {
            unsigned long v = va_arg(ap, unsigned int);
            out_num(&p, end, v, 16, *fmt == 'X', 0, width, left, pad, prec > 0 ? prec : 0);
            break;
        }
        case 'p': {
            unsigned long v = (unsigned long)(uintptr_t)va_arg(ap, void *);
            out_char(&p, end, '0');
            out_char(&p, end, 'x');
            out_num(&p, end, v, 16, 0, 0, 0, 0, ' ', 0);
            break;
        }
        case '%':
            out_char(&p, end, '%');
            break;
        case '\0':
            goto done;
        default:                       /* unknown spec: print it verbatim */
            out_char(&p, end, '%');
            out_char(&p, end, *fmt);
            break;
        }
    }
done:
    if (cap > 0)
        *p = '\0';
    return (int)(p - buf);
}

int snprintf(char *buf, size_t cap, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(buf, cap, fmt, ap);
    va_end(ap);
    return r;
}

int printf(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    rechord_console_write(buf, r < (int)sizeof buf ? r : (int)sizeof buf - 1);
    return r;
}

int puts(const char *s)
{
    int n = (int)strlen(s);
    rechord_console_write(s, n);
    rechord_console_write("\n", 1);
    return n + 1;
}
