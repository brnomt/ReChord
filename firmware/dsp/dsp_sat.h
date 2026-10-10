/*
 * dsp_sat.h — private int16 saturation/format helpers for firmware/dsp/.
 *
 * WHY not from the core: rechord_dsp_core.h exports only configure/process/
 * reset — its saturate/fixed-point helpers are file-static and the core files
 * must not be modified. These are the module layer's own equivalents.
 */
#ifndef DSP_SAT_H
#define DSP_SAT_H

#include <stdint.h>

/* Clamp a 32-bit value to the int16 range (never wraps — wrapping in the
 * audio path is what produces the classic "pop to opposite rail"). */
static inline int16_t dsp_sat16(int32_t v)
{
    if (v > 32767) return 32767;
    if (v < -32768) return (int16_t)-32768;
    return (int16_t)v;
}

/*
 * Convert a 24-bit-container sample back to int16 with round-to-nearest.
 * The DSP core treats its int32 signal as 24-bit (Echo Mini I2S is 24-bit);
 * modules carry int16 in/out and use the 8 spare bits as guard bits.
 */
static inline int16_t dsp_i24_to_i16(int32_t v)
{
    int32_t r = (v + (v >= 0 ? 128 : -128)) >> 8;
    return dsp_sat16(r);
}

/* int16 -> 24-bit container (lossless: 8 guard bits). */
static inline int32_t dsp_i16_to_i24(int16_t v)
{
    return ((int32_t)v) << 8;
}

#endif /* DSP_SAT_H */
