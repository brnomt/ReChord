/*
 * rechord_dsp.h — from-source modular DSP framework (ReChord).
 *
 * Replaces the binary RkNano_EQ_24BIT_20150630.lib (a 31-band fixed-point
 * graphic EQ) AND the SDK's Effect.c glue (which assumed the EQ was a
 * flash overlay module). The result is a compile-time plugin framework:
 *
 *   - bypass    : passthrough (EQ_NOR)
 *   - param_eq  : 5-band parametric (low-shelf + 3 peaking + high-shelf),
 *                 driven by the SDK RKEffect / dbGain[5] contract.
 *   - bass      : single low-shelf bass boost (EQ_BASS).
 *
 * To add an effect, write one .c that implements the vtable below, register
 * it in the dispatch table in rechord_dsp.c, and (if it needs its own EQ
 * mode) map that mode to it. Everything is fixed-point on the per-sample
 * hot path (Q2.30 coefficients, Direct Form I, 64-bit accumulators).
 */
#ifndef RECHORD_DSP_H
#define RECHORD_DSP_H

#include "Effect.h"   /* EQ_TYPE, RKEffect, eEQMode */

typedef struct RchDspPlugin
{
    const char *name;

    /* Recompute coefficients/state from the current RKEffect. */
    long (*adjust)(const RKEffect *cfg, unsigned long sample_rate);

    /* Process one interleaved frame in place. `samples` is the total
     * sample count (both channels); `channels` is the channel count. */
    long (*process)(EQ_TYPE *buf, long samples, int channels);
} RchDspPlugin;

/*
 * Public EQ API — the symbols the rest of the firmware (AudioControl.c,
 * Service.c) calls under _RK_EQ_. Previously split across Effect.c (glue)
 * and RkNano_EQ lib (RockEQAdjust/RockEQProcess); now all from source here.
 */
long EffectInit(void);
long EffectEnd(void);
long EffectAdjust(void);
long EffectProcess(EQ_TYPE *pBuffer, long PcmLen);
long RKEQAdjust(RKEffect *pEft);

#endif /* RECHORD_DSP_H */
