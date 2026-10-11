/*
 * trial_entry.c -- TRIAL VEHICLE: run OUR fw1 Main() inside the verbatim
 * reference image, then hand back to it (brick-free hardware iteration).
 *
 * Called in place of ref main's 2nd call (`bl 0x030514C0`, RAM 0x03050B12;
 * tools/make_hook_img.py --early). State there: ref startup done (IRQs off,
 * ref .data copied, ref .bss 0x03005AFC..0x03039128 zeroed and still unused).
 * Our .data/.bss live inside that ref .bss (trial.ld) and are wiped again
 * before returning, so the reference boots normally afterwards (USB/updater
 * stay reachable).
 */
#include <stdint.h>

#include "drivers/panel_io.h"

extern uint32_t Main(void);
extern uint32_t _bss_start, _bss_end;
extern uint32_t _data_start, _data_end, _data_load_start;

#define REF_BSS_START ((uint32_t *)0x03005AFCu)
#define REF_BSS_END   ((uint32_t *)0x03039128u)

__attribute__((section(".text.entry"), used))
void ref_hook(void)
{
    uint32_t *d = &_data_start;
    const uint32_t *s = &_data_load_start;
    while (d < &_data_end)
        *d++ = *s++;
    for (uint32_t *p = &_bss_start; p < &_bss_end; p++)
        *p = 0;

    Main();

    backlight_on();   /* the ref's own 0x030514C0 call we replaced */
    for (uint32_t *p = REF_BSS_START; p < REF_BSS_END; p++)
        *p = 0;
}
