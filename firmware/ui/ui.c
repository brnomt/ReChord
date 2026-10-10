/*
 * ui.c — ReChord UI core implementation (see ui.h for the design).
 *
 * Dirty-region model: rows are tracked as a small set of inclusive bands
 * [y0, y1].  Bands are kept sorted, merged when they overlap or touch, and
 * coalesced into a bounding band if the set would overflow — the screen is
 * small (170 rows) so degradation is harmless and the code stays simple.
 */
#include <stddef.h>
#include "ui.h"
#include "theme.h"

#define UI_DIRTY_MAX_BANDS 8

typedef struct dirty_band {
    int y0;   /* inclusive */
    int y1;   /* inclusive */
} dirty_band_t;

static const ui_screen_t *s_stack[UI_STACK_MAX];
static int                s_depth;
static int                s_running;
static dirty_band_t       s_dirty[UI_DIRTY_MAX_BANDS];
static int                s_dirty_count;

/* ---- dirty band helpers ---------------------------------------------- */

static void dirty_sort(void)
{
    int i, j;

    for (i = 1; i < s_dirty_count; i++) {
        dirty_band_t b = s_dirty[i];
        for (j = i - 1; j >= 0 && s_dirty[j].y0 > b.y0; j--)
            s_dirty[j + 1] = s_dirty[j];
        s_dirty[j + 1] = b;
    }
}

static void dirty_normalize(void)
{
    int i, j;

    dirty_sort();
    for (i = 0; i + 1 < s_dirty_count; ) {
        /* merge overlapping OR adjacent bands (touching rows flush together) */
        if (s_dirty[i].y1 + 1 >= s_dirty[i + 1].y0) {
            if (s_dirty[i + 1].y1 > s_dirty[i].y1)
                s_dirty[i].y1 = s_dirty[i + 1].y1;
            for (j = i + 1; j + 1 < s_dirty_count; j++)
                s_dirty[j] = s_dirty[j + 1];
            s_dirty_count--;
        } else {
            i++;
        }
    }
}

void ui_invalidate(int x0, int y0, int x1, int y1)
{
    (void)x0;
    (void)x1;   /* the panel flush is row-granular; x range is implied */

    if (y0 < 0)
        y0 = 0;
    if (y1 > DISPLAY_H - 1)
        y1 = DISPLAY_H - 1;
    if (y0 > y1)
        return;

    if (s_dirty_count == UI_DIRTY_MAX_BANDS) {
        /* overflow: fold into a bounding band of band 0 and renormalize */
        if (y0 < s_dirty[0].y0)
            s_dirty[0].y0 = y0;
        if (y1 > s_dirty[0].y1)
            s_dirty[0].y1 = y1;
    } else {
        s_dirty[s_dirty_count].y0 = y0;
        s_dirty[s_dirty_count].y1 = y1;
        s_dirty_count++;
    }
    dirty_normalize();
}

void ui_invalidate_all(void)
{
    ui_invalidate(0, 0, DISPLAY_W - 1, DISPLAY_H - 1);
}

bool ui_is_dirty(void)
{
    return s_dirty_count > 0;
}

int ui_dirty_band_count(void)
{
    return s_dirty_count;
}

bool ui_dirty_band(int idx, int *y0, int *y1)
{
    if (idx < 0 || idx >= s_dirty_count)
        return false;
    if (y0)
        *y0 = s_dirty[idx].y0;
    if (y1)
        *y1 = s_dirty[idx].y1;
    return true;
}

/* ---- screen stack ----------------------------------------------------- */

bool ui_push_screen(const ui_screen_t *screen)
{
    if (screen == NULL || s_depth >= UI_STACK_MAX)
        return false;

    s_stack[s_depth++] = screen;
    if (screen->on_enter)
        screen->on_enter();
    /* the new screen owns the whole panel */
    ui_invalidate_all();
    return true;
}

bool ui_pop_screen(void)
{
    const ui_screen_t *leaving;
    const ui_screen_t *below;

    if (s_depth == 0)
        return false;

    leaving = s_stack[--s_depth];
    if (leaving->on_exit)
        leaving->on_exit();

    below = ui_top_screen();
    if (below && below->on_enter)
        below->on_enter();   /* re-enter the screen that was covered */

    ui_invalidate_all();
    return true;
}

const ui_screen_t *ui_top_screen(void)
{
    return (s_depth > 0) ? s_stack[s_depth - 1] : NULL;
}

int ui_stack_depth(void)
{
    return s_depth;
}

/* ---- event dispatch --------------------------------------------------- */

void ui_handle_event(const input_event_t *ev)
{
    const ui_screen_t *top = ui_top_screen();

    if (ev == NULL || top == NULL || top->handle_event == NULL)
        return;
    top->handle_event(ev);
}

/* ---- render ----------------------------------------------------------- */

void ui_render(void)
{
    const ui_screen_t *top = ui_top_screen();
    int i;

    if (top == NULL || s_dirty_count == 0)
        return;
    if (top->draw)
        top->draw();

    for (i = 0; i < s_dirty_count; i++)
        display_flush_rows(s_dirty[i].y0, s_dirty[i].y1);
    s_dirty_count = 0;
}

/* ---- event loop skeleton ---------------------------------------------- */

void ui_quit(void)
{
    s_running = 0;
}

void ui_run(void)
{
    input_event_t ev;

    s_running = 1;
    while (s_running) {
        if (input_poll(&ev))
            ui_handle_event(&ev);
        ui_render();
    }
}

void ui_init(void)
{
    s_depth = 0;
    s_running = 0;
    s_dirty_count = 0;
}
