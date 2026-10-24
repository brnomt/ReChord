/*
 * skin_screen.h — generic UI adapter for the declarative skin engine.
 *
 * This file contains NO theme-specific behavior. It exposes the skin home as
 * a normal ui_screen_t and maps the engine's generic results (actions, back,
 * redraw) to the app callbacks registered by the app layer.
 */
#ifndef RECHORD_UI_SKIN_SCREEN_H
#define RECHORD_UI_SKIN_SCREEN_H

#include "screens.h"

extern const ui_screen_t skin_home_screen;

void skin_home_set_actions(void (*library)(void),
                           void (*player)(void),
                           void (*settings)(void));

/* Feed the millisecond clock used by skin expressions. Called once per UI
 * pass; cheap. */
void skin_home_tick(unsigned now_ms);

/* Repaint just the package's dynamic primitives (0 = no dyn rects). */
int  skin_home_animate(void);

#endif /* RECHORD_UI_SKIN_SCREEN_H */
