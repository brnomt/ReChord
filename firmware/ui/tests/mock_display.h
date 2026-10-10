/*
 * mock_display.h — MOCK display backend for host tests.
 *
 * Records every draw call (fill_rect / draw_text / flush_rows / clear) into
 * fixed-size arrays so tests can assert on what the UI painted and which
 * dirty rows were flushed.  Implements the same contract as the real
 * firmware/drivers/display.h (inclusive coordinates).
 */
#ifndef RECHORD_UI_MOCK_DISPLAY_H
#define RECHORD_UI_MOCK_DISPLAY_H

#include <stdint.h>

#include "internal/shims/display.h"

#define MOCK_DISPLAY_MAX_RECT  512
#define MOCK_DISPLAY_MAX_TEXT  512
#define MOCK_DISPLAY_MAX_FLUSH 64

typedef struct mock_rect {
    int x0, y0, x1, y1;
    uint16_t color;
} mock_rect_t;

typedef struct mock_text {
    int x, y;
    char text[64];
    uint16_t fg, bg;
} mock_text_t;

typedef struct mock_flush {
    int y0, y1;
} mock_flush_t;

void mock_display_reset(void);

int mock_display_rect_count(void);
const mock_rect_t *mock_display_rect(int i);

int mock_display_text_count(void);
const mock_text_t *mock_display_text(int i);
/* index of the first text call containing substr, or -1 */
int mock_display_text_find(const char *substr);

int mock_display_flush_count(void);
const mock_flush_t *mock_display_flush(int i);

int mock_display_clear_count(void);
uint16_t mock_display_clear_color(void);

#endif /* RECHORD_UI_MOCK_DISPLAY_H */
