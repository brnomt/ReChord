/*
 * power_save.h — idle power-down policy (pure, host-testable).
 *
 * EchoPod's device notes: three seconds after playback stops, the DAC path
 * powers down (and its second core stops). ReChord keeps the DAC registers,
 * the headphone amps and Core B alive forever unless something turns them
 * off, so a stopped player still burns battery.
 *
 * Rules:
 *   - Only while the player is idle (stopped/error); playing or paused resets
 *     the timer, so resume stays instant.
 *   - After POWER_SAVE_STOP_MS: power the DAC path down if it is up.
 *   - Stop Core B after the same delay, EXCEPT while a boot preload holds a
 *     stream: that warm start is worth keeping for a while, so it gets
 *     POWER_SAVE_PRELOAD_MS before the preload is discarded and B stops.
 *
 * The caller performs the actions; this module only decides.
 */
#ifndef RECHORD_SERVICES_POWER_SAVE_H
#define RECHORD_SERVICES_POWER_SAVE_H

#define POWER_SAVE_STOP_MS     3000UL
#define POWER_SAVE_PRELOAD_MS  60000UL

enum {
    POWER_SAVE_DAC    = 1u << 0,   /* audio_output_power_off()          */
    POWER_SAVE_CORE_B = 1u << 1    /* discard preload + stop Core B     */
};

typedef struct {
    unsigned long since;
    unsigned char armed;
} power_save_t;

void     power_save_init(power_save_t *ps);
unsigned power_save_step(power_save_t *ps, int idle, int dac_ready,
                         int preloaded, unsigned long now);

#endif /* RECHORD_SERVICES_POWER_SAVE_H */
