/*
 * main.c — host test runner for the app glue (firmware/app).
 *
 * Build + run (see firmware/app/app.mk for the exact command):
 *   cc -Wall -Werror -DRECHORD_UI_TARGET \
 *      -DRECHORD_IDLE_FRAMES=2 -DRECHORD_SLEEP_FRAMES=4 \
 *      -Ifirmware -Ifirmware/services -Ifirmware/drivers \
 *      firmware/app/main.c firmware/app/module_registry.c \
 *      firmware/app/tests/{main,mocks,test_bringup,test_registry,test_boot_params}.c \
 *      -o build/test_app && ./build/test_app
 */
#include <stdio.h>

#include "test_util.h"

int test_checks = 0;
int test_failures = 0;

void test_boot_params(void);
void test_registry(void);
void test_bringup(void);

static void run(const char *name, void (*fn)(void))
{
    int before = test_failures;

    fn();
    printf("%-28s %s\n", name, (test_failures == before) ? "PASS" : "FAIL");
}

int main(void)
{
    run("boot_params", test_boot_params);
    run("module_registry", test_registry);
    run("bringup_order", test_bringup);

    printf("----\n%d checks, %d failures\n", test_checks, test_failures);
    return (test_failures == 0) ? 0 : 1;
}
