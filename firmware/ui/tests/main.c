/*
 * main.c — host test runner for the ReChord frontend (firmware/ui).
 */
#include <stdio.h>

#include "test_util.h"
#include "mock_display.h"
#include "mock_services.h"

int test_checks = 0;
int test_failures = 0;

void test_screen_stack(void);
void test_ui_redraw(void);
void test_menu_nav(void);
void test_settings_keys(void);
void test_browser_pagination(void);

static void run(const char *name, void (*fn)(void))
{
    int before = test_failures;

    fn();
    printf("%-28s %s\n", name, (test_failures == before) ? "PASS" : "FAIL");
}

int main(void)
{
    mock_display_reset();
    mock_services_reset();

    run("screen_stack", test_screen_stack);
    run("ui_redraw", test_ui_redraw);
    run("menu_nav", test_menu_nav);
    run("settings_keys", test_settings_keys);
    run("browser_pagination", test_browser_pagination);

    printf("----\n%d checks, %d failures\n", test_checks, test_failures);
    return (test_failures == 0) ? 0 : 1;
}
