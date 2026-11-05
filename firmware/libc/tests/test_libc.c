/*
 * test_libc.c — host tests for the freestanding libc (see architecture.md §5).
 *
 * Expected values are analytic constants (not libm comparisons): our sin/
 * cos/sqrt/pow intentionally define the SAME symbol names the SDK links
 * against, so referencing libm for a baseline would collide at link time.
 */
#include <stdio.h>
#include <string.h>
#include <math.h>   /* only for M_PI-ish constants; our symbols shadow libm */
#include "libc.h"

static int fails;

static void check(int cond, const char *name)
{
    if (cond) {
        fprintf(stdout, "  PASS: %s\n", name);
    } else {
        fprintf(stdout, "  FAIL: %s\n", name);
        fails++;
    }
}

static int approx(double a, double b, double tol)
{
    double d = a - b;
    if (d < 0)
        d = -d;
    return d <= tol;
}

int main(void)
{
    char buf[128];

    /* mem */
    char m[16];
    memset(m, 0xAA, sizeof m);
    check((unsigned char)m[0] == 0xAA && (unsigned char)m[15] == 0xAA, "memset fills");
    memcpy(m, "hello", 6);
    check(memcmp(m, "hello", 6) == 0, "memcpy+memcmp");
    memmove(m + 1, m, 5);              /* overlapping forward<->back */
    check(memcmp(m, "hhello", 6) == 0, "memmove overlap");

    /* Word-at-a-time paths: every alignment of dst/src and every length up to
     * 70 against a byte-loop reference, guard bytes included; memmove also in
     * both overlap directions. */
    {
        static unsigned char ref[96], got[96], src[96];
        int bad = 0, i, n, d0, s0, sh;

        for (i = 0; i < 96; i++)
            src[i] = (unsigned char)(i * 37 + 11);
        for (n = 0; n <= 70; n++)
            for (d0 = 0; d0 < 4; d0++) {
                for (i = 0; i < 96; i++)
                    ref[i] = got[i] = 0x5A;
                for (i = 0; i < n; i++)
                    ref[8 + d0 + i] = 0xC3;
                memset(got + 8 + d0, 0xC3, (size_t)n);
                bad += memcmp(ref, got, 96) != 0;
                for (s0 = 0; s0 < 4; s0++) {
                    for (i = 0; i < 96; i++)
                        ref[i] = got[i] = 0x5A;
                    for (i = 0; i < n; i++)
                        ref[8 + d0 + i] = src[4 + s0 + i];
                    memcpy(got + 8 + d0, src + 4 + s0, (size_t)n);
                    bad += memcmp(ref, got, 96) != 0;
                }
                for (sh = -6; sh <= 6; sh++) {
                    unsigned char tmp[96];
                    int from = 12 + d0, to = 12 + d0 + sh;

                    for (i = 0; i < 96; i++)
                        ref[i] = got[i] = src[i];
                    for (i = 0; i < n; i++)
                        tmp[i] = ref[from + i];
                    for (i = 0; i < n; i++)
                        ref[to + i] = tmp[i];
                    memmove(got + to, got + from, (size_t)n);
                    bad += memcmp(ref, got, 96) != 0;
                }
            }
        check(bad == 0, "memset/memcpy/memmove all alignments, lengths, overlaps");
    }

    /* str */
    check(strlen("abcdef") == 6, "strlen");
    char s[16];
    strcpy(s, "abc");
    check(strcmp(s, "abc") == 0 && strcmp(s, "abd") < 0, "strcpy+strcmp");
    strncpy(s, "xy", 5);
    check(s[2] == '\0' && s[4] == '\0', "strncpy pads with NUL");
    check(strncmp("abc", "abd", 2) == 0, "strncmp");

    /* stdio forms */
    snprintf(buf, sizeof buf, "%08x", 0x1234u);
    check(strcmp(buf, "00001234") == 0, "%08x zero pad");
    snprintf(buf, sizeof buf, "%-5s|", "ab");
    check(strcmp(buf, "ab   |") == 0, "%-5s left align");
    snprintf(buf, sizeof buf, "%d/%u/%x", -42, 7u, 255u);
    check(strcmp(buf, "-42/7/ff") == 0, "%d %u %x");
    snprintf(buf, sizeof buf, "100%% %c%%", 'k');
    check(strcmp(buf, "100% k%") == 0, "%% and %c");
    snprintf(buf, sizeof buf, "%5d", 42);
    check(strcmp(buf, "   42") == 0, "%5d width");
    /* The Bluetooth store saved device names with %.*s; an unsupported '*' was
     * printed verbatim ("%*s") instead of the name. */
    snprintf(buf, sizeof buf, ",%.*s\n", 31, "Nothing Ear (1)");
    check(strcmp(buf, ",Nothing Ear (1)\n") == 0, "%.*s prints the name");
    snprintf(buf, sizeof buf, "%.*s", 7, "Nothing Ear (1)");
    check(strcmp(buf, "Nothing") == 0, "%.*s truncates");
    snprintf(buf, sizeof buf, "%.3s|", "abcdef");
    check(strcmp(buf, "abc|") == 0, "%.3s truncates");
    snprintf(buf, sizeof buf, "%*d|%-*d|", 4, 7, 4, 7);
    check(strcmp(buf, "   7|7   |") == 0, "%*d and %-*d width from args");
    snprintf(buf, sizeof buf, "%*s|", -4, "x");
    check(strcmp(buf, "x   |") == 0, "negative * width left-aligns");

    /* math: analytic values */
    check(approx(sin(0.0), 0.0, 1e-12), "sin(0)");
    check(approx(sin(3.14159265358979323846 / 6), 0.5, 1e-9), "sin(pi/6)=0.5");
    check(approx(sin(3.14159265358979323846 / 2), 1.0, 1e-9), "sin(pi/2)=1");
    check(approx(cos(0.0), 1.0, 1e-12), "cos(0)");
    check(approx(cos(3.14159265358979323846), -1.0, 1e-9), "cos(pi)=-1");
    check(approx(sin(-0.7), -sin(0.7), 1e-12), "sin odd symmetry");
    check(approx(sqrt(2.0), 1.41421356237309504880, 1e-12), "sqrt(2)");
    check(approx(sqrt(1e6), 1000.0, 1e-6), "sqrt(1e6)");
    check(approx(pow(2.0, 10.0), 1024.0, 1e-9), "pow(2,10) integer");
    check(approx(pow(2.0, -2.0), 0.25, 1e-12), "pow(2,-2)");
    check(approx(pow(2.0, 0.5), 1.41421356237309504880, 1e-9), "pow(2,0.5)");
    check(approx(pow(9.0, 0.5), 3.0, 1e-9), "pow(9,0.5)");

    fprintf(stdout, fails ? "test_libc: %d FAILURES\n" : "test_libc: ALL PASS (%d failures)\n", fails);
    return fails ? 1 : 0;
}
