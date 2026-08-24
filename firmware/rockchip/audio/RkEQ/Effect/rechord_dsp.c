/*
 * rechord_dsp.c — SDK→core EQ/DSP adapter (ReChord).
 *
 * Thin boundary between the stock SDK and the freestanding, host-tested DSP
 * core (rechord_dsp_core.c). The per-sample biquad math lives in the core;
 * this file only translates the SDK's RKEffect contract into a
 * rch_dsp_config_t and forwards it.
 *
 *   bypass   : passthrough                        (EQ_NOR)
 *   param_eq : 5-band parametric EQ               (EQ_HEAVY/POP/JAZZ/CLASS/
 *                                                   UNIQUE/ROCK/USER)
 *   bass     : low-shelf bass boost               (EQ_BASS)
 *
 * The SDK's RKEffect is the parameter contract: Mode selects the plugin, and
 * for EQ_USER the 5 gains come from RKCoef.dbGain[5] in "dB+12" form
 * (12 == flat), i.e. gain_dB = dbGain[i] - 12. Presets use g_preset_db.
 */
#include "SysInclude.h"
#include "audio_globals.h"
#include "AudioControl.h"
#include "rechord_dsp.h"

#include "rechord_dsp_core.h"

/* Preset curves (dB), indexed by eEQMode. EQ_BASS(5)/EQ_USER(7)/EQ_NOR(8)
 * are not stored here — they route to bass / dbGain / bypass respectively. */
static const int8_t g_preset_db[EQ_ROCK + 1][5] = {
    [EQ_HEAVY]  = {  6,  4, -2,  4,  6 },
    [EQ_POP]    = {  2,  3,  1,  3,  2 },
    [EQ_JAZZ]   = {  3,  2, -1,  2,  4 },
    [EQ_UNIQUE] = {  5,  3, -1,  1,  3 },
    [EQ_CLASS]  = {  4,  2, -1,  2,  4 },
    [EQ_ROCK]   = {  5,  3, -1,  3,  5 },
};

/* Map an RKEffect onto the core config and (re)compute coefficients. */
static void apply_effect(const RKEffect *cfg, unsigned long sr, int channels)
{
    rch_dsp_config_t c;
    int i;

    c.sample_rate = sr;
    c.channels    = channels;

    switch (cfg->Mode)
    {
    case EQ_BASS: c.mode = RCH_DSP_BASS;     break;
    case EQ_NOR:  c.mode = RCH_DSP_BYPASS;   break;
    default:      c.mode = RCH_DSP_PARAM_EQ; break;
    }

    if (cfg->Mode == EQ_USER)
    {
        for (i = 0; i < 5; i++)
            c.db_gain[i] = (double)(cfg->RKCoef.dbGain[i] - 12);  /* dB+12 -> dB */
    }
    else if (cfg->Mode < EQ_USER)           /* EQ_HEAVY..EQ_ROCK */
    {
        for (i = 0; i < 5; i++)
            c.db_gain[i] = (double)g_preset_db[cfg->Mode][i];
    }
    else                                    /* EQ_NOR (unused by bypass) */
    {
        for (i = 0; i < 5; i++)
            c.db_gain[i] = 0.0;
    }

    rch_dsp_configure(&c);
}

long EffectInit(void)
{
    rch_dsp_reset();
    return 0;
}

long EffectEnd(void)
{
    rch_dsp_reset();
    return 0;
}

long RKEQAdjust(RKEffect *pEft)
{
    unsigned long sr = 44100;

    if (pEft == 0)
        return 1;

    CodecGetSampleRate(&sr);
    apply_effect(pEft, sr, 2);
    return 0;
}

long EffectAdjust(void)
{
    RKEffect *cfg = &AudioIOBuf.EffectCtl;
    unsigned long sr = 44100;
    unsigned long ch = 2;

    /* Stock behavior: a fully-flat user EQ collapses to EQ_NOR. */
    if (cfg->Mode == EQ_USER)
    {
        int i, zero = 1;
        for (i = 0; i < 5; i++)
            if (cfg->RKCoef.dbGain[i] != 12)
                zero = 0;
        if (zero)
        {
            cfg->Mode = EQ_NOR;
            IsEQNOR = TRUE;
        }
    }

    CodecGetSampleRate(&sr);
    CodecGetChannels(&ch);
    if (ch < 1 || ch > 2)
        ch = 2;

    apply_effect(cfg, sr, (int)ch);
    return 0;
}

long EffectProcess(EQ_TYPE *pBuffer, long PcmLen)
{
    if (pBuffer == 0)
        return 1;
    if (PcmLen <= 0)
        return 0;
    if (pAudioRegKey->IsEQUpdate)
        return 0;
    if (AudioIOBuf.EffectCtl.Mode == EQ_NOR)
        return 0;

    return rch_dsp_process((int32_t *)pBuffer, (int)PcmLen);
}
