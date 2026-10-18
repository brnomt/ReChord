/*
 * companion.h — "companion-bot" theme engine (BRNOMT-69).
 *
 * The eyes are PROCEDURAL (no bitmaps, no allocation): solid rounded
 * rectangles, as in intellar/oled_eye_display (MIT) — no pupils. States:
 * WAKEUP, BLINK (short/long), SACCADE, SLEEP and HAPPY (lower-eyelid mask).
 *
 * The theme's home screen is a face over a 3-item horizontal bar
 * (Library / Now Playing / Settings); the app registers the actions.
 */
#ifndef RECHORD_UI_COMPANION_H
#define RECHORD_UI_COMPANION_H

#include "screens.h"

/* Actions for the three bar items (registered once at boot). */
void companion_home_set_actions(void (*library)(void),
                                void (*now_playing)(void),
                                void (*settings)(void));

/* The home screen: face + inverted 1-bit bar. BACK pops to the classic menu. */
extern const ui_screen_t companion_home_screen;

/* Draw the eyes inside (x,y,w,h). `playing` = HAPPY, `cursor` (0..2) biases the
 * gaze toward the selected bar item. Cheap: called once per visible frame. */
void companion_draw(int x, int y, int w, int h, int playing, int cursor);

#endif /* RECHORD_UI_COMPANION_H */
