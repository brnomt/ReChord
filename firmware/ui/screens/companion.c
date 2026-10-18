/*
 * companion.c — procedural companion face (theme "companion-bot").
 *
 * Ported/adapted from intellar/oled_eye_display (MIT, control_display.ino):
 * the same expressive states, re-implemented for ReChord's 320x170 monochrome
 * renderer with drawing primitives only. No bitmaps, no allocation, fixed
 * static state: it costs a few dozen bytes of .bss and nothing of the asset
 * cache. The caller drives it once per frame (see ap_ui: the face band is
 * invalidated ~8x/s while the root menu is up).
 */
#include <stdint.h>

#include "companion.h"
#include "../theme.h"
#include "../internal/hal.h"

/* Nominal frame rate: the app invalidates the band every ~125 ms. */
#define FACE_FPS 8

static uint32_t s_frame;       /* animation clock (frames)            */
static int      s_wake;        /* frames left of the wake-up ramp     */
static int      s_blink_until; /* eyes closed until this frame        */
static int      s_next_blink;  /* frame of the next blink             */
static int      s_look;        /* pupil x offset (saccade)            */
static int      s_idle;        /* frames since the last interaction   */
static int      s_prev_cursor;

/* Deterministic per-draw pseudo-random (no libc rand, no state growth). */
static uint32_t rnd(uint32_t salt)
{
    uint32_t v = s_frame * 1103515245u + 12345u + salt;
    v ^= v >> 13;
    v *= 2654435761u;
    return v >> 16;
}

void companion_draw(int x, int y, int w, int h, int playing, int cursor)
{
    int closed, sleeping, happy, i;
    int ew, eh, ey, ex0, ex1;

    if (w < 60 || h < 20)
        return;

    s_frame++;
    s_idle++;

    /* Gaze follows navigation (saccade toward the selection). */
    if (cursor != s_prev_cursor) {
        s_prev_cursor = cursor;
        s_wake = 5;
        s_idle = 0;
        s_look = (cursor & 1) ? 2 : -2;
    }
    if (playing)
        s_idle = 0;
    if (s_wake > 0)
        s_wake--;

    /* Random saccades while awake. */
    if ((s_frame & 7u) == 0u)
        s_look = (int)(rnd(1u) % 5u) - 2;

    /* Blinks: short most of the time, occasionally long. */
    if (s_next_blink == 0)
        s_next_blink = 20 + (int)(rnd(2u) % 24u);
    if ((int)s_frame >= s_next_blink) {
        s_blink_until = (int)s_frame + ((rnd(3u) & 1u) ? 2 : 5);
        s_next_blink = (int)s_frame + 20 + (int)(rnd(4u) % 28u);
    }

    closed = (int)s_frame < s_blink_until;
    happy = playing != 0;
    sleeping = (!happy && s_wake == 0 && s_idle > FACE_FPS * 20);

    ew = w / 4;
    if (ew < 16)
        ew = 16;
    if (ew > 34)
        ew = 34;
    eh = h / 3;
    if (eh < 10)
        eh = 10;
    if (eh > 20)
        eh = 20;
    ey = y + h / 3;
    ex0 = x + w / 2 - ew - ew / 2;
    ex1 = x + w / 2 + ew / 2;

    for (i = 0; i < 2; i++) {
        int ex = i ? ex1 : ex0;

        if (closed || sleeping) {
            /* Closed lid: one 2 px line. */
            display_fill_rect(ex, ey + eh / 2, ex + ew - 1, ey + eh / 2 + 1,
                              THEME_COLOR_FG);
        } else if (happy) {
            /* Smiling eye: a stepped ∩. */
            int top = ey + 2;

            display_fill_rect(ex + 3, top, ex + ew - 4, top + 1,
                              THEME_COLOR_FG);
            display_fill_rect(ex, top + 1, ex + 2, top + eh / 2,
                              THEME_COLOR_FG);
            display_fill_rect(ex + ew - 3, top + 1, ex + ew - 1, top + eh / 2,
                              THEME_COLOR_FG);
        } else {
            /* Open eye: a rounded white shape with a moving pupil. */
            int open = (eh * ((s_wake > 0) ? (6 - s_wake) : 6)) / 6;
            int px, py;

            if (open < 4)
                open = 4;
            display_fill_rect(ex + 3, ey, ex + ew - 4, ey + open / 4,
                              THEME_COLOR_FG);
            display_fill_rect(ex, ey + open / 4, ex + ew - 1,
                              ey + open - open / 4, THEME_COLOR_FG);
            display_fill_rect(ex + 3, ey + open - open / 4, ex + ew - 4,
                              ey + open, THEME_COLOR_FG);

            px = ex + ew / 2 - 3 + s_look;
            py = ey + open / 2 - 3;
            display_fill_rect(px, py, px + 5, py + 5, THEME_COLOR_BG);
        }
    }

    if (happy) {
        /* Smile: a stepped U under the eyes. */
        int my = ey + eh + 6;

        if (my + 2 < y + h) {
            display_fill_rect(x + w / 2 - 14, my, x + w / 2 + 13, my + 1,
                              THEME_COLOR_FG);
            display_fill_rect(x + w / 2 - 17, my - 4, x + w / 2 - 14, my,
                              THEME_COLOR_FG);
            display_fill_rect(x + w / 2 + 13, my - 4, x + w / 2 + 16, my,
                              THEME_COLOR_FG);
        }
    } else if (sleeping) {
        display_draw_text(x + w - 26, y + 2, "z", THEME_COLOR_DIM,
                          THEME_COLOR_BG);
        display_draw_text(x + w - 16, y + 12, "Z", THEME_COLOR_DIM,
                          THEME_COLOR_BG);
    }
}
