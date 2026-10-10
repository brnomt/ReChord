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
