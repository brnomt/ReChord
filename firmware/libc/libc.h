/*
 * libc.h — minimal freestanding C library for ReChord (build-from-source rule:
 * the toolchain on this host has no arm-none-eabi newlib, so the firmware
 * provides the handful of libc routines the SDK needs — implemented in
 * mem.c / str.c / stdio_min.c / math_min.c and host-tested).
 *
 * Scope is deliberately narrow: exactly the symbols the SDK link demands
 * (memset/memcpy/strlen/vsnprintf/printf/puts + sin/cos/sqrt/pow). Anything
 * new the linker asks for gets added HERE with a test, not by linking a
 * prebuilt libc.
 */
#ifndef RECHORD_LIBC_H
#define RECHORD_LIBC_H

#include <stddef.h>
#include <stdarg.h>

/* ---- mem.c / str.c ---- */
void  *memset(void *dst, int c, size_t n);
void  *memcpy(void *dst, const void *src, size_t n);
void  *memmove(void *dst, const void *src, size_t n);
int    memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *s);
char  *strcpy(char *dst, const char *src);
char  *strncpy(char *dst, const char *src, size_t n);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);

/* ---- stdio_min.c ----
 * vsnprintf supports: %c %s %d %i %u %x %X %p %% and the width/zero-pad/
 * left-align forms the SDK format strings use (no float conversion: the
 * device's own log formats use %u.%03u instead of %f). */
int vsnprintf(char *buf, size_t cap, const char *fmt, va_list ap);
int snprintf(char *buf, size_t cap, const char *fmt, ...);
/* printf/puts route through the weak board hook (host tests redirect it). */
int printf(const char *fmt, ...);
int puts(const char *s);
void rechord_console_write(const char *s, int n);   /* weak, board provides */

/* ---- math_min.c (double precision, ~1 ulp not guaranteed but well under
 * the tolerance of audio EQ coefficient math) ---- */
double sin(double x);
double cos(double x);
double sqrt(double x);
double pow(double x, double y);

#endif /* RECHORD_LIBC_H */
