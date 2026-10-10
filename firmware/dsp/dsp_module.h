/*
 * dsp_module.h — ReChord DSP effect-module interface (the only effect API).
 *
 * An effect is a small vtable + per-instance state, so the user can add,
 * remove and reorder effects per config (architecture.md §3). The chain
 * (dsp_chain.h) owns ordering and bypass; modules only know their own state.
 *
 * Stream conventions (the Echo Mini is stereo, I2S 24-bit out):
 *   - `samples` is INTERLEAVED stereo int16: L,R,L,R,... (host-testable
 *     everywhere; the 24-bit depth is kept inside the filter math);
 *   - `n` counts int16 VALUES (frames*2). Odd n is processed anyway but
 *     callers should keep it even so channel phase stays aligned;
 *   - processing is in-place and must be click-free at block boundaries
 *     (module state is carried across calls).
 *
 * Parameter conventions:
 *   - `param`/`value` are plain ints so the interface stays C89-simple and
 *     config files can carry them as `key=value`; gains use TENTHS OF dB
 *     ("+60" = +6.0 dB) so integers keep 0.1 dB resolution;
 *   - set_param returns 0 on success, <0 on error (unknown param / range);
 *   - get_param returns the value, or INT_MIN on error. INT_MIN is chosen
 *     because every legal parameter is bounded far away from it, so a
 *     negative parameter value (e.g. -600 = -60 dB) is never ambiguous.
 *
 * Modules are plain data: `state_size` lets the caller place state in its
 * own static buffers (no heap anywhere) — use DSP_STATE_STORAGE() below.
 */
#ifndef DSP_MODULE_H
#define DSP_MODULE_H

#include <stdint.h>
#include <limits.h>

typedef struct dsp_module
{
    const char *name;
    int (*init)(void *state, int sample_rate);
    int (*process)(void *state, int16_t *samples, int n);
    int (*set_param)(void *state, int param, int value);
    int (*get_param)(void *state, int param);
    int state_size;
} dsp_module_t;

/* Declare aligned storage for one module instance's state. */
#define DSP_STATE_STORAGE(name, size)                     \
    union {                                               \
        double   _align_d;                                \
        void    *_align_p;                                \
        int64_t  _align_i;                                \
        uint8_t  bytes[(size)];                           \
    } name

/* ---- built-in effect modules (firmware/dsp/mod_*.c) -------------------- */

/*
 * mod_eq — 3-band parametric EQ (low shelf / mid peaking / high shelf) on
 * the proven Q2.30 biquad math of rechord_dsp_core.h.
 * Params (tenths of dB, range -200..+200):
 */
enum {
    MOD_EQ_LOW = 0,       /* low shelf   (core band 0, 60 Hz)            */
    MOD_EQ_MID = 1,       /* mid peaking (core band 2, 1 kHz)           */
    MOD_EQ_HIGH = 2,      /* high shelf  (core band 4, 8 kHz)           */
    MOD_EQ_PARAM_COUNT = 3
};

/*
 * mod_bass — adjustable bass boost (low shelf) on the same core.
 * Params (tenths of dB, range 0..+150; default +100 = +10 dB):
 */
enum {
    MOD_BASS_BOOST = 0,
    MOD_BASS_PARAM_COUNT = 1
};

/*
 * mod_volume — gain with soft ramp (no clicks) and int16 saturation.
 * Params:
 */
enum {
    MOD_VOLUME_GAIN = 0,      /* target gain, tenths of dB, -600..+60    */
    MOD_VOLUME_RAMP_MS = 1,   /* ramp time in ms, 0..1000 (0 = instant)  */
    MOD_VOLUME_PARAM_COUNT = 2
};

extern const dsp_module_t dsp_mod_eq;
extern const dsp_module_t dsp_mod_bass;
extern const dsp_module_t dsp_mod_volume;

#endif /* DSP_MODULE_H */
