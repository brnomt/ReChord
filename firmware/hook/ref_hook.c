/*
 * ref_hook.c -- HOOK TEST: our panel driver running inside the VERBATIM
 * reference fw1 (docs/HANDOVER.md, "HOOK TEST").
 *
 * tools/make_hook_img.py patches the reference main's `bl 0x03051DD8`
 * (backlight level 100, RAM 0x03050B3C) into `bl ref_hook`. ref_hook lives
 * in the reference's zero run RAM 0x03064280..0x03074B70 and must not touch
 * the reference's .data/.bss (0x03004000..0x03039128); IRQs are live.
 *
 * Expected on screen: "ReChord hook OK" (reference printer, proves the hook
 * ran) for ~2 s, then 8 vertical colour bars (OUR panel_io + panel_init)
 * for ~3 s, then the reference UI continues.
 */
#include <stdint.h>

#include "display.h"
#include "panel_io.h"

void panel_init_sequence(void);

/* Reference functions (Thumb bit set). */
#define REF_BACKLIGHT ((void (*)(int))0x03051DD9u)
#define REF_PRINT     ((void (*)(int, int, const char *, ...))0x03050551u)
#define REF_DELAY_MS  ((void (*)(int))0x03051B05u)

static void send_u16(uint16_t v)
{
    panel_io_send_data((uint8_t)(v >> 8));
    panel_io_send_data((uint8_t)v);
}

static void colour_bars(void)
{
    static const uint16_t bar[8] = {0xFFFF, 0xFFE0, 0x07FF, 0x07E0,
                                    0xF81F, 0xF800, 0x001F, 0x0000};
    panel_io_send_cmd(0x2A);
    send_u16(0);
    send_u16(DISPLAY_W - 1);
    panel_io_send_cmd(0x2B);
    send_u16(PANEL_ROW_OFFSET);
    send_u16(PANEL_ROW_OFFSET + DISPLAY_H - 1);
    panel_io_send_cmd(0x2C);
    for (int y = 0; y < DISPLAY_H; y++)
        for (int x = 0; x < DISPLAY_W; x++)
            panel_io_send_pixel(bar[x / (DISPLAY_W / 8)]);
}

#ifdef HOOK_EARLY
/* Replaces ref main's 2nd call (`bl 0x030514C0`, backlight GPIO high) at
 * RAM 0x03050B12: ROM handoff + ref startup only (IRQs off, no CRU/clock/
 * SysTick/display setup yet). Uses NO reference function. Expect black ->
 * colour bars (~3 s) -> normal reference boot. */
__attribute__((section(".text.entry"), used))
void ref_hook(void)
{
    backlight_on();
    panel_io_init();
    panel_init_sequence();
    colour_bars();
    panel_io_delay_ms(500);   /* ~3 s at the 24 MHz boot clock */
}
#else
__attribute__((section(".text.entry"), used))
void ref_hook(int level)
{
    REF_BACKLIGHT(level);
    REF_PRINT(3, 0xFFFF, "ReChord hook OK");
    REF_DELAY_MS(2000);
    panel_io_init();
    panel_init_sequence();
    colour_bars();
    REF_DELAY_MS(3000);
}
#endif
