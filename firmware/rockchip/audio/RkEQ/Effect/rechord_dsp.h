/*
 * rechord_dsp.h — ReChord EQ/DSP public API (SDK boundary).
 *
 * This header defines the symbols the rest of the firmware (AudioControl.c,
 * Service.c) calls under _RK_EQ_. The per-sample DSP math now lives in the
 * freestanding core (rechord_dsp_core.h / rechord_dsp_core.c), which is
 * host-tested and has no SDK dependencies. rechord_dsp.c is a thin adapter
 * that maps the SDK RKEffect contract onto the core.
 */
#ifndef RECHORD_DSP_H
#define RECHORD_DSP_H

#include "effect.h"   /* EQ_TYPE, RKEffect, eEQMode */

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
