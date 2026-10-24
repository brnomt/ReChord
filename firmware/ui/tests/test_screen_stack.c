/*
 * test_screen_stack.c — screen stack push/pop semantics (ui.c).
 */
#include "test_util.h"
#include "ui.h"
#include "mock_display.h"

static int s_enter[3], s_exit[3];

static void enter_0(void) { s_enter[0]++; }
static void exit_0(void)  { s_exit[0]++; }
static void enter_1(void) { s_enter[1]++; }
static void exit_1(void)  { s_exit[1]++; }
static void enter_2(void) { s_enter[2]++; }
static void exit_2(void)  { s_exit[2]++; }

static const ui_screen_t screen_a = { "a", enter_0, exit_0, NULL, NULL };
static const ui_screen_t screen_b = { "b", enter_1, exit_1, NULL, NULL };
static const ui_screen_t screen_c = { "c", enter_2, exit_2, NULL, NULL };

void test_screen_stack(void)
{
    s_enter[0] = s_enter[1] = s_enter[2] = 0;
    s_exit[0] = s_exit[1] = s_exit[2] = 0;

    ui_init();
    mock_display_reset();

    /* empty stack */
    CHECK_EQ_INT(ui_stack_depth(), 0);
    CHECK(ui_top_screen() == NULL);
    CHECK(!ui_pop_screen());
    CHECK(!ui_push_screen(NULL));

    /* push A */
    CHECK(ui_push_screen(&screen_a));
    CHECK_EQ_INT(ui_stack_depth(), 1);
    CHECK(ui_top_screen() == &screen_a);
    CHECK_EQ_INT(s_enter[0], 1);
    CHECK_EQ_INT(s_exit[0], 0);

    /* push B over A: A stays on the stack untouched (covered) */
    CHECK(ui_push_screen(&screen_b));
    CHECK_EQ_INT(ui_stack_depth(), 2);
    CHECK(ui_top_screen() == &screen_b);
    CHECK_EQ_INT(s_enter[1], 1);
    CHECK_EQ_INT(s_enter[0], 1);
    CHECK_EQ_INT(s_exit[0], 0);

    /* pop B: B exits, A re-enters */
    CHECK(ui_pop_screen());
    CHECK_EQ_INT(ui_stack_depth(), 1);
    CHECK(ui_top_screen() == &screen_a);
    CHECK_EQ_INT(s_exit[1], 1);
    CHECK_EQ_INT(s_enter[0], 2);
    CHECK_EQ_INT(s_exit[0], 0);

    /* stack overflow: depth capped at UI_STACK_MAX (6) */
    CHECK(ui_push_screen(&screen_b));
    CHECK(ui_push_screen(&screen_c));
    CHECK(ui_push_screen(&screen_a));
    CHECK(ui_push_screen(&screen_b));
    CHECK(ui_push_screen(&screen_c));
    CHECK_EQ_INT(ui_stack_depth(), UI_STACK_MAX);
    CHECK(!ui_push_screen(&screen_a));       /* full: refused */
    CHECK_EQ_INT(ui_stack_depth(), UI_STACK_MAX);
    CHECK(ui_top_screen() == &screen_c);

    /* drain */
    while (ui_stack_depth() > 0)
        CHECK(ui_pop_screen());
    CHECK_EQ_INT(ui_stack_depth(), 0);
    CHECK(ui_top_screen() == NULL);
    CHECK(!ui_pop_screen());

    /* every screen left the stack exactly as often as it was pushed
     * (re-enter on pop of the covering screen adds enters on purpose):
     * A pushed 2x, B pushed 3x, C pushed 2x */
    CHECK_EQ_INT(s_exit[0], 2);
    CHECK_EQ_INT(s_exit[1], 3);
    CHECK_EQ_INT(s_exit[2], 2);
    /* enters = pushes + re-enters: A 2+3, B 3+2, C 2+1 */
    CHECK_EQ_INT(s_enter[0], 5);
    CHECK_EQ_INT(s_enter[1], 5);
    CHECK_EQ_INT(s_enter[2], 3);

    /* BRNOMT-69: replace a covered screen without disturbing the top one
     * (the theme picker swaps the home renderer under open Settings). */
    ui_init();
    CHECK(ui_push_screen(&screen_a));
    CHECK(ui_insert_screen(1, &screen_b));
    CHECK_EQ_INT(ui_stack_depth(), 2);
    CHECK(ui_stack_at(0) == &screen_a);
    CHECK(ui_stack_at(1) == &screen_b);
    CHECK(ui_push_screen(&screen_c));
    CHECK_EQ_INT(ui_stack_depth(), 3);
    CHECK(ui_remove_screen(1));           /* drop B, C stays on top */
    CHECK_EQ_INT(ui_stack_depth(), 2);
    CHECK(ui_stack_at(0) == &screen_a);
    CHECK(ui_stack_at(1) == &screen_c);
    CHECK(ui_top_screen() == &screen_c);
    CHECK(!ui_remove_screen(-1));
    CHECK(!ui_remove_screen(ui_stack_depth()));
    CHECK(!ui_insert_screen(UI_STACK_MAX + 1, &screen_b));
    CHECK(ui_stack_at(ui_stack_depth()) == NULL);
}
