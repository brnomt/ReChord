/* math_min.c — minimal double-precision math (see libc.h).
 *
 * WHY our own: no newlib on this host and the build-from-source rule says the
 * firmware ships its own libc. Accuracy target: far tighter than audio EQ
 * coefficient tolerance (~1e-9 for sin/cos in range, ~1e-12 rel. for sqrt).
 * The double arithmetic itself uses the compiler runtime (__aeabi_d* from
 * libgcc) — we implement the LIBRARY functions, not soft-float.
 */
#include "libc.h"

#define PI      3.14159265358979323846
#define HALF_PI 1.57079632679489661923

typedef union { double d; unsigned long long u; } du_t;

double fabs_min(double x) { return x < 0 ? -x : x; }

/* sqrt: bit-level estimate + Newton-Raphson (quadratically convergent). */
double sqrt(double x)
{
    if (x < 0)
        return 0.0 / 0.0;              /* NaN: firmware never passes x < 0 */
    if (x == 0)
        return 0;
    du_t v;
    v.d = x;
    v.u = (v.u >> 1) + 0x1FF8000000000000ULL;   /* ~sqrt via exponent halving */
    double y = v.d;
    for (int i = 0; i < 6; i++)        /* Newton: y = (y + x/y) / 2 */
        y = 0.5 * (y + x / y);
    return y;
}

/* sin on [-PI/2, PI/2] via Taylor (odd terms to x^13). */
static double sin_small(double x)
{
    double x2 = x * x;
    double r = x;
    double t = x;
    t = t * x2 / (2.0 * 3.0);  r -= t;
    t = t * x2 / (4.0 * 5.0);  r += t;
    t = t * x2 / (6.0 * 7.0);  r -= t;
    t = t * x2 / (8.0 * 9.0);  r += t;
    t = t * x2 / (10.0 * 11.0); r -= t;
    t = t * x2 / (12.0 * 13.0); r += t;
    return r;
}

double sin(double x)
{
    /* Range-reduce to [-PI, PI] with a 2*PI multiple; precision of the
     * reduction is sufficient for the signal frequencies we drive. */
    double twopi = 2.0 * PI;
    long long k = (long long)(x / twopi);
    x -= (double)k * twopi;
    if (x > PI)
        x -= twopi;
    if (x < -PI)
        x += twopi;
    if (x > HALF_PI)
        return sin_small(PI - x);
    if (x < -HALF_PI)
        return sin_small(-PI - x);
    return sin_small(x);
}

double cos(double x)
{
    return sin(x + HALF_PI);
}

/* log2 via exponent + mantissa series on ln(1+m), m in [-1/3, 1/2]. */
static double log2_min(double x)
{
    du_t v;
    v.d = x;
    int exp = (int)((v.u >> 52) & 0x7FF) - 1023;
    v.u = (v.u & 0x000FFFFFFFFFFFFFULL) | 0x3FF0000000000000ULL;
    double m = v.d - 1.0;              /* mantissa in [0, 1) */
    /* ln(1+m) = 2*(z + z^3/3 + ...) with z = m/(2+m): fast convergence */
    double z = m / (2.0 + m);
    double z2 = z * z;
    double s = z;
    s += z * z2 / 3.0;
    s += z * z2 * z2 / 5.0;
    s += z * z2 * z2 * z2 / 7.0;
    double ln = 2.0 * s;
    return (double)exp + ln * 1.44269504088896340736;   /* /ln(2) */
}

/* 2^f for f in [0,1): Taylor on exp(ln2*f). */
static double exp2_frac(double f)
{
    double ln2 = 0.69314718055994530942;
    double x = ln2 * f;
    double r = 1.0, t = 1.0;
    for (int i = 1; i <= 12; i++) {
        t *= x / (double)i;
        r += t;
    }
    return r;
}

double pow(double x, double y)
{
    if (y == 0.0)
        return 1.0;
    if (x == 0.0)
        return 0.0;
    /* integer exponents: exact repeated multiply (fast path for EQ tables) */
    long long n = (long long)y;
    if ((double)n == y && n > -1024 && n < 1024) {
        double r = 1.0, base = x;
        long long e = n < 0 ? -n : n;
        while (e) {
            if (e & 1)
                r *= base;
            base *= base;
            e >>= 1;
        }
        return n < 0 ? 1.0 / r : r;
    }
    if (x < 0)
        return 0.0 / 0.0;              /* NaN for negative base, fractional y */
    double l2 = y * log2_min(x);       /* x^y = 2^(y*log2 x) */
    long long k = (long long)l2;
    double frac = l2 - (double)k;
    if (frac < 0) {
        frac += 1.0;
        k -= 1;
    }
    du_t v;
    v.d = exp2_frac(frac);
    long long e = (long long)k + 1023;
    if (e <= 0)
        return 0.0;
    if (e >= 0x7FF)
        return 1.0 / 0.0;              /* +inf */
    v.u = (v.u & 0x000FFFFFFFFFFFFFULL) | ((unsigned long long)e << 52);
    return v.d;
}
