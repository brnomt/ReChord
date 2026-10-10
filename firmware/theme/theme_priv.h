/*
 * theme_priv.h — private to firmware/theme/ (built-in table access).
 *
 * Other modules must NOT include this file: they consume theme.h only.
 * It exists so theme.c can enumerate the .c data table without exposing
 * the table layout to the rest of the firmware.
 */
#ifndef THEME_PRIV_H
#define THEME_PRIV_H

#include "theme.h"

int           theme_builtin_count(void);
const theme_t *theme_builtin_get(int i);   /* NULL if i out of range */

#endif /* THEME_PRIV_H */
