/*
 * rechord_dsp_core.h — freestanding ReChord DSP core (no SDK deps).
 *
 * The pure per-sample DSP math, decoupled from the SDK globals
 * (AudioIOBuf / pAudioRegKey / CodecGetSampleRate) so it can be:
 *   1. unit-tested on the host (x86) — the math has NEVER been verified;
 *   2. compiled as a position-independent blob for the target patch;
 *   3. driven by an explicit config instead of reading SDK state.
 *
 * Fixed-point on the hot path: Q2.30 biquad coefficients, Direct Form I,
 * 64-bit MAC. Signal is treated as 24-bit (in a 32-bit container), matching
 * the Echo Mini's I2S 24-bit output (verified: rom_i2s_master_config 0x17).
 */
#ifndef RECHORD_DSP_CORE_H
#define RECHORD_DSP_CORE_H

#include <stdint.h>

/* Number of parametric-EQ bands (low-shelf + 3 peaking + high-shelf). */
#define RCH_DSP_BANDS 5

typedef enum
{
    RCH_DSP_BYPASS = 0,
    RCH_DSP_PARAM_EQ,
    RCH_DSP_BASS,
    RCH_DSP_COUNT
} rch_dsp_plugin_t;

/*
 * Explicit DSP configuration (replaces the SDK RKEffect globals).
 *   mode      : selects the plugin (bypass / param_eq / bass)
 *   db_gain[5]: per-band gain in dB for param_eq (USER mode); presets use
 *               built-in curves.
 */
typedef struct
{
    rch_dsp_plugin_t mode;
    double           db_gain[RCH_DSP_BANDS];
    unsigned long    sample_rate;
    int              channels;
} rch_dsp_config_t;

/* Recompute biquad coefficients from a config. Returns 0 on success. */
int rch_dsp_configure(const rch_dsp_config_t *cfg);

/* Process `samples` interleaved samples in place. Returns 0 on success. */
int rch_dsp_process(int32_t *buf, int samples);

/* Reset all filter state (call on track change / stop). */
void rch_dsp_reset(void);

#endif /* RECHORD_DSP_CORE_H */
