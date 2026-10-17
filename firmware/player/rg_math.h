/*
 * rg_math.h — ReplayGain fixed-point gain helpers.
 *
 * Gain values are Q1.15 fixed-point: 32768 == 1.0x (neutral). The `gain_q16`
 * parameter names are historical; the value passed is Q1.15. peak_q15 is also
 * Q1.15 (32768 == 1.0 peak = full scale; 0 means "no peak information").
 */
#ifndef RECHORD_RG_MATH_H
#define RECHORD_RG_MATH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* dB*10 -> linear Q1.15 (32768 = 1.0x). */
int32_t rg_gain_q16(int gain_tenth_db);

/* Effective Q1.15 gain for a track: dB gain, hard-limited by `peak_q15` so the
 * known peak never clips. peak<=0 => no peak info => dB gain as-is. */
int32_t rg_effective_gain_q16(int gain_tenth_db, int peak_q15);

/* Scale `n` interleaved int32 samples (left-justified pipeline format) in
 * place by `gain_q15` (32768 = 1.0x), saturating to the int32 range. */
void rg_scale_pcm(int32_t *pcm, long n, int32_t gain_q15);

#ifdef __cplusplus
}
#endif

#endif /* RECHORD_RG_MATH_H */
