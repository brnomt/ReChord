/*
 * power_save.c — idle power-down decision (see power_save.h).
 */
#include "power_save.h"

void power_save_init(power_save_t *ps)
{
    if (ps == 0)
        return;
    ps->since = 0;
    ps->armed = 0;
}

unsigned power_save_step(power_save_t *ps, int idle, int dac_ready,
                         int preloaded, unsigned long now)
{
    unsigned flags = 0;

    if (ps == 0 || !idle) {
        if (ps != 0)
            ps->armed = 0;
        return 0;
    }
    if (!ps->armed) {
        ps->armed = 1;
        ps->since = now;
        return 0;
    }
    if (now - ps->since < POWER_SAVE_STOP_MS)
        return 0;

    if (dac_ready)
        flags |= POWER_SAVE_DAC;
    if (!preloaded || now - ps->since >= POWER_SAVE_PRELOAD_MS)
        flags |= POWER_SAVE_CORE_B;
    return flags;
}
