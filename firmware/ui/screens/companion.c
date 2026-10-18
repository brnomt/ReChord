/*
 * companion.c — "companion-bot" home: procedural eyes + 3-item bar.
 *
 * Eyes ported from intellar/oled_eye_display (MIT, control_display.ino):
 *   - solid filled ROUNDED RECTANGLES, no pupil (drawRBox semantics)
 *   - BLINK squashes height and widens slightly, radius follows the height
 *   - SLEEP is a 2 px line; WAKEUP ramps the height back
 *   - SACCADE shifts both eyes (±8 x / ±6 y) with a brief squint
 *   - HAPPY masks the lower lid (a black rounded box over the bottom half),
 *     which is what the reference does with a black triangle
 *
 * 1-bit renderer only: display_fill_rect / display_draw_text. Fixed statics,
 * no heap, no bitmaps (0 B of asset cache).
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "companion.h"
#include "../theme.h"
#include "../internal/hal.h"

#define FPS           8      /* the app ticks the home ~8x/s          */
#define BAR_H         14
#define BAR_Y         (DISPLAY_H - 18)
#define BASE_W        40     /* reference eye size                    */
#define BASE_H        40
#define EYE_GAP       14
#define EYE_R          9

static void (*s_open_lib)(void);
static void (*s_open_np)(void);
static void (*s_open_set)(void);

void companion_home_set_actions(void (*library)(void),
                                void (*now_playing)(void),
                                void (*settings)(void))
{
    s_open_lib = library;
    s_open_np = now_playing;
    s_open_set = settings;
}

/* ---- animation state ---------------------------------------------------- */

static uint32_t s_frame;
static int s_blink;        /* frames left in the current blink      */
static int s_next_blink;
static int s_wake;         /* frames left of the wake ramp          */
static int s_idle;
static int s_dx, s_dy;     /* saccade offset                        */
static int s_gaze = 1;     /* last bar selection seen               */
static int s_sel = 1;      /* bar selection (0..2)                  */

static uint32_t rnd(void)
{
    s_frame = s_frame * 1103515245u + 12345u;
    return s_frame >> 16;
}

/* 1-bit rounded box: a filled rect with stair-cut corners (drawRBox at 1 bit). */
static void rbox(int x, int y, int w, int h, int r, uint16_t color)
{
    int i;

    if (w <= 0 || h <= 0)
        return;
    display_fill_rect(x, y, x + w - 1, y + h - 1, color);
    if (r <= 0)
        return;
    for (i = 0; i < r && i * 2 < h; i++) {
        int cut = r - i;

        if (cut <= 0 || cut * 2 > w)
            break;
        display_fill_rect(x, y + i, x + cut - 1, y + i, THEME_COLOR_BG);
        display_fill_rect(x + w - cut, y + i, x + w - 1, y + i,
                          THEME_COLOR_BG);
        display_fill_rect(x, y + h - 1 - i, x + cut - 1, y + h - 1 - i,
                          THEME_COLOR_BG);
        display_fill_rect(x + w - cut, y + h - 1 - i, x + w - 1,
                          y + h - 1 - i, THEME_COLOR_BG);
    }
}

void companion_draw(int x, int y, int w, int h, int playing, int cursor)
{
    int base_h, base_w, ew, eh, er, ex0, ex1, ey, i;
    int sleeping, happy, ramping;

    if (w < 60 || h < 20)
        return;

    s_frame++;
    s_idle++;

    /* Gaze follows the bar selection; a new item wakes the face. */
    if (cursor != s_gaze) {
        s_gaze = cursor;
        s_wake = 5;
        s_idle = 0;
        s_dx = (cursor - 1) * 7;
    }
    if (playing)
        s_idle = 0;

    if (s_next_blink == 0)
        s_next_blink = 18 + (int)(rnd() % 22u);
    if ((int)s_frame >= s_next_blink) {
        s_blink = 6;
        s_next_blink = (int)s_frame + 16 + (int)(rnd() % 26u);
        if ((rnd() & 7u) == 0u)
            s_blink = 12;                       /* occasional long blink   */
    }

    /* Random saccade while awake. */
    if ((s_frame & 7u) == 0u) {
        s_dx = (int)(rnd() % 17u) - 8;
        s_dy = (int)(rnd() % 13u) - 6;
    }

    sleeping = (!playing && s_wake == 0 && s_idle > FPS * 20);
    happy = playing != 0;
    ramping = s_wake > 0;
    if (ramping)
        s_wake--;

    /* Eye geometry, scaled to the band the theme reserved. */
    base_h = (h > 2 * EYE_GAP + BAR_H) ? (h - EYE_GAP - BAR_H) : h;
    if (base_h > BASE_H)
        base_h = BASE_H;
    if (base_h < 12)
        base_h = 12;
    base_w = base_h + 6;
    if (base_w > BASE_W + 16)
        base_w = BASE_W + 16;

    eh = base_h;
    ew = base_w;
    if (sleeping)
        eh = 2;
    else if (ramping)
        eh = (base_h * (6 - s_wake)) / 6;
    else if (s_blink > 0) {
        int ph = 6 - s_blink;                   /* 0..5                    */

        if (s_blink > 6)
            ph = 0;                             /* long blink: closed hold */
        eh = (ph < 3) ? (base_h * (ph + 1)) / 3
                      : (base_h * (6 - ph)) / 3;
        if (eh < 2)
            eh = 2;
        ew = base_w + (base_h - eh) / 3 + 2;    /* squash widens           */
        s_blink--;
    }
    if (eh > h)
        eh = h;

    er = (sleeping || eh < 6) ? 0 : EYE_R;
    if (er * 2 >= eh)
        er = eh / 2;
    if (er * 2 >= ew)
        er = ew / 2;

    ey = y + (h - BAR_H - eh) / 2;
    if (ey < y)
        ey = y;
    ex0 = x + w / 2 - EYE_GAP / 2 - ew;
    ex1 = x + w / 2 + EYE_GAP / 2;
    ex0 += s_dx;
    ex1 += s_dx;
    ey += s_dy;
    if (ey < y)
        ey = y;
    if (ey + eh > y + h - BAR_H)
        ey = y + h - BAR_H - eh;

    for (i = 0; i < 2; i++) {
        int ex = i ? ex1 : ex0;

        rbox(ex, ey, ew, eh, er, THEME_COLOR_FG);
        if (happy && !sleeping && eh > 8) {
            /* Lower-lid mask: a black rounded box over the bottom half, which
             * leaves the top arc (the reference's black triangle). */
            rbox(ex, ey + eh / 2, ew, eh - eh / 2 + 2, er / 2, THEME_COLOR_BG);
        }
    }
}

/* ---- home screen: face + 3-item horizontal bar -------------------------- */

static const char *const k_labels[3] = { "LIBRARY", "NOW PLAYING", "SETTINGS" };

static void bar_draw(int sel)
{
    int widths[3];
    int total = 0;
    int x, i;

    for (i = 0; i < 3; i++) {
        widths[i] = THEME_TEXT_W((int)strlen(k_labels[i]));
        total += widths[i];
    }
    total += 16 * 2;                       /* gaps between the three items   */
    x = (DISPLAY_W - total) / 2;
    if (x < THEME_PAD)
        x = THEME_PAD;

    for (i = 0; i < 3; i++) {
        if (i == sel) {
            /* 1-bit reverse video: solid bar, glyphs punched out. */
            display_fill_rect(x - 4, BAR_Y - 1, x + widths[i] + 3,
                              BAR_Y + BAR_H - 2, THEME_COLOR_FG);
            display_draw_text(x, BAR_Y + THEME_TEXT_YOFF, k_labels[i],
                              THEME_COLOR_BG, THEME_COLOR_FG);
        } else {
            display_draw_text(x, BAR_Y + THEME_TEXT_YOFF, k_labels[i],
                              THEME_COLOR_FG, THEME_COLOR_BG);
        }
        x += widths[i] + 16;
    }
}

static void home_on_enter(void)
{
    s_sel = 1;
    s_gaze = -1;            /* force a wake/saccade on the next draw */
}

static void home_draw(void)
{
    int band = theme_layout()->face_h;

    if (band < 90)
        band = 90;          /* room for the reference-size eyes           */
    display_clear(THEME_COLOR_BG);
    companion_draw(0, 6, DISPLAY_W, band, player_get_state() == PLAYER_PLAYING,
                   s_sel);
    display_fill_rect(0, BAR_Y - 6, DISPLAY_W - 1, BAR_Y - 6, THEME_COLOR_DIM);
    bar_draw(s_sel);
}

static void home_event(const input_event_t *ev)
{
    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;

    switch (ev->type) {
    case KEY_UP:
        s_sel = (s_sel + 2) % 3;
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        s_sel = (s_sel + 1) % 3;
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_sel == 0 && s_open_lib != 0)
            s_open_lib();
        else if (s_sel == 1 && s_open_np != 0)
            s_open_np();
        else if (s_sel == 2 && s_open_set != 0)
            s_open_set();
        break;
    case KEY_BACK:
        ui_pop_screen();    /* the classic menu stays underneath */
        break;
    default:
        break;
    }
}

const ui_screen_t companion_home_screen = {
    "home",
    home_on_enter,
    NULL,
    home_draw,
    home_event,
};
