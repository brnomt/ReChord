/*
 * skin_screen.c — generic UI adapter for the declarative skin engine.
 *
 * The engine (firmware/theme/theme_skin.c) parses the active package's skin
 * file and draws it. This adapter only owns the selected index, the idle
 * timer and the mapping from the engine's actions to app callbacks.
 */
#include <string.h>

#include "skin_screen.h"
#include "theme_ui.h"
#include "../theme.h"
#include "../internal/hal.h"
#include "../../theme/theme_skin.h"

static int      s_sel;
static unsigned s_now_ms;
static unsigned s_last_input_ms;

static void (*s_open_lib)(void);
static void (*s_open_np)(void);
static void (*s_open_set)(void);

void skin_home_set_actions(void (*library)(void),
                           void (*player)(void),
                           void (*settings)(void))
{
    s_open_lib = library;
    s_open_np = player;
    s_open_set = settings;
}

void skin_home_tick(unsigned now_ms)
{
    s_now_ms = now_ms;
}

/* Cheap repaint: only the package's dynamic primitives, flushed to their own
 * rects. The gradient/art/labels keep the pixels from the last full draw. */
int skin_home_animate(void)
{
    theme_skin_state_t st;

    theme_ui_fill_state(&st, "");
    return theme_ui_draw_dynamic(&st, s_sel, 0);
}

static void skin_on_enter(void)
{
    /* A child screen may have loaded its own skin; the home always renders
     * from the package's main skin again. Host tests load the engine directly
     * without a package layout, so an empty path leaves it untouched. */
    {
        const char *p = theme_ui_path(THEME_UI_MAIN);

        if (p != 0 && p[0] != '\0')
            (void)theme_ui_load(THEME_UI_MAIN);
    }
    s_sel = theme_skin_selected_default();
    s_last_input_ms = s_now_ms;
    ui_invalidate_all();
}

static void skin_draw(void)
{
    theme_skin_state_t st;

    theme_ui_fill_state(&st, "");
    display_clear(THEME_COLOR_BG);
    theme_skin_render(s_now_ms, s_now_ms - s_last_input_ms, s_sel, 0, &st);
}

static void skin_handle_event(const input_event_t *ev)
{
    int r;

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;
    s_last_input_ms = s_now_ms;
    r = theme_skin_key(ev->type, &s_sel);
    switch (r) {
    case THEME_SKIN_REDRAW: {
        int y, h;

        /* Selection changes only touch the bar; repaint the dynamic layer
         * (the package marks the bar sections) instead of re-rendering the
         * whole skin. Themes without dynamic rects fall back to the bar
         * band; their animated parts refresh on the next tick anyway. */
        if (skin_home_animate())
            break;
        if (theme_skin_list_bounds(&y, &h) == 0 && h > 0)
            ui_invalidate(0, y, DISPLAY_W - 1, y + h - 1);
        break;
    }
    case THEME_SKIN_BACK:
        ui_pop_screen();
        break;
    case THEME_SKIN_LIBRARY:
        if (s_open_lib)
            s_open_lib();
        break;
    case THEME_SKIN_PLAYER:
        if (s_open_np)
            s_open_np();
        break;
    case THEME_SKIN_SETTINGS:
        if (s_open_set)
            s_open_set();
        break;
    default:
        break;
    }
}

const ui_screen_t skin_home_screen = {
    "skin",
    skin_on_enter,
    NULL,
    skin_draw,
    skin_handle_event,
    1,
};
