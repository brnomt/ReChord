/*
 * overlay_demo.c -- tiny code overlay linked at 0x0304AE64.
 *
 * Stored in the AP image below the initial-row gate, copied to the free
 * loader-data window at runtime and called through a Thumb function
 * pointer. Returning the magic proves code execution from the window.
 */
#include <stdint.h>

__attribute__((section(".text.overlay"), used, noinline))
uint32_t overlay_demo(void)
{
    return 0xC0DE1234u;
}
