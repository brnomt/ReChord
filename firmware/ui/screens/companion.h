/*
 * companion.h — procedural 1-bit companion face (theme "companion-bot").
 *
 * The theme system (BRNOMT-69) is declarative for palette/layout; an animated
 * face needs code. This is the small, bounded hook the renderer allows: it
 * draws with the existing primitives only (display_fill_rect / draw_text),
 * keeps fixed static state and never allocates.
 *
 * States (adapted from intellar/oled_eye_display, MIT):
 *   WAKEUP  eyes open progressively after boot / interaction
 *   BLINK   short and occasional long blinks
 *   SACCADE pupils shift randomly, and toward the selected item
 *   SLEEP   long idle: closed eyes + "zZ"
 *   HAPPY   playback active: arc eyes + smile
 */
#ifndef RECHORD_UI_COMPANION_H
#define RECHORD_UI_COMPANION_H

/* Draw the face inside (x,y,w,h). `playing` selects HAPPY, `cursor` nudges the
 * gaze toward the selected item. Cheap enough to call once per frame. */
void companion_draw(int x, int y, int w, int h, int playing, int cursor);

#endif /* RECHORD_UI_COMPANION_H */
