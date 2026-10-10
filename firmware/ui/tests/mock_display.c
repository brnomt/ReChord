/*
 * mock_display.c — MOCK display backend implementation (records draw calls).
 */
#include <stdio.h>
#include <string.h>

#include "mock_display.h"

static mock_rect_t  s_rects[MOCK_DISPLAY_MAX_RECT];
static int          s_rect_count;
static mock_text_t  s_texts[MOCK_DISPLAY_MAX_TEXT];
static int          s_text_count;
static mock_flush_t s_flushes[MOCK_DISPLAY_MAX_FLUSH];
static int          s_flush_count;
static int          s_clear_count;
static uint16_t     s_clear_color;

void mock_display_reset(void)
{
    s_rect_count = 0;
    s_text_count = 0;
    s_flush_count = 0;
    s_clear_count = 0;
    s_clear_color = 0;
}

int mock_display_rect_count(void)
{
    return s_rect_count;
}

const mock_rect_t *mock_display_rect(int i)
{
    return (i >= 0 && i < s_rect_count) ? &s_rects[i] : NULL;
}

int mock_display_text_count(void)
{
    return s_text_count;
}

const mock_text_t *mock_display_text(int i)
{
    return (i >= 0 && i < s_text_count) ? &s_texts[i] : NULL;
}

int mock_display_text_find(const char *substr)
{
    int i;

    for (i = 0; i < s_text_count; i++) {
        if (strstr(s_texts[i].text, substr) != NULL)
            return i;
    }
    return -1;
}

int mock_display_flush_count(void)
{
    return s_flush_count;
}

const mock_flush_t *mock_display_flush(int i)
{
    return (i >= 0 && i < s_flush_count) ? &s_flushes[i] : NULL;
}

int mock_display_clear_count(void)
{
    return s_clear_count;
}

uint16_t mock_display_clear_color(void)
{
    return s_clear_color;
}

/* ---- the contract implementation -------------------------------------- */

void display_init(void)
{
    mock_display_reset();
}

void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t rgb565)
{
    if (s_rect_count < MOCK_DISPLAY_MAX_RECT) {
        s_rects[s_rect_count].x0 = x0;
        s_rects[s_rect_count].y0 = y0;
        s_rects[s_rect_count].x1 = x1;
        s_rects[s_rect_count].y1 = y1;
        s_rects[s_rect_count].color = rgb565;
        s_rect_count++;
    }
}

void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
    if (s_text_count < MOCK_DISPLAY_MAX_TEXT) {
        mock_text_t *t = &s_texts[s_text_count];
        t->x = x;
        t->y = y;
        snprintf(t->text, sizeof(t->text), "%s", s ? s : "");
        t->fg = fg;
        t->bg = bg;
        s_text_count++;
    }
}

void display_flush_rows(int y0, int y1)
{
    if (s_flush_count < MOCK_DISPLAY_MAX_FLUSH) {
        s_flushes[s_flush_count].y0 = y0;
        s_flushes[s_flush_count].y1 = y1;
        s_flush_count++;
    }
}

void display_clear(uint16_t rgb565)
{
    s_clear_count++;
    s_clear_color = rgb565;
}
