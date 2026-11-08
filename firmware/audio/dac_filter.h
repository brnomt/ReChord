/*
 * dac_filter.h — the five CS43131 hardware reconstruction filters.
 *
 * The CS43131 selects its PCM reconstruction filter with MAP 0x090000.
 * These are the five values EchoPod writes on the same player; FiiO itself
 * only surfaces fast/slow roll-off, so the community labels below are the
 * conventional audiophile names for the four oversampling shapes plus the
 * non-oversampling mode. The old UI numbered them 1..5 (1 = index 0, ...,
 * 5 = index 4); that legacy number remains a valid cfg value.
 *
 * PURE DATA: no MMIO, no allocation, no maths. The audio HAL consumes
 * `reg`; the Settings screen consumes `name`/`hint`/`glyph`.
 */
#ifndef RECHORD_AUDIO_DAC_FILTER_H
#define RECHORD_AUDIO_DAC_FILTER_H

#include <stdint.h>

#define DAC_FILTER_COUNT        5
#define DAC_FILTER_GLYPH_W      24
#define DAC_FILTER_GLYPH_H      8
#define DAC_FILTER_GLYPH_STRIDE 3   /* bytes per bitmap row (24 px MSB-first) */
#define DAC_FILTER_CFG_MAX      16  /* RECHORD.CFG field size incl. NUL     */

typedef struct {
    const char    *name;    /* short display label: "Fast-Lin" ...          */
    const char    *config;  /* RECHORD.CFG identifier: "fast_lin" ...       */
    const char    *hint;    /* one-line character hint                      */
    uint8_t        reg;     /* CS43131 MAP 0x090000 value                   */
    const uint8_t *glyph;   /* DAC_FILTER_GLYPH_W x H 1-bit bitmap          */
} dac_filter_t;

/* Index 0..4, in the order the Settings row cycles. */
extern const dac_filter_t dac_filters[DAC_FILTER_COUNT];

/* Clamp an arbitrary index to 0..DAC_FILTER_COUNT-1. */
int dac_filter_clamp(int index);

/* Clamped table entry; never NULL. */
const dac_filter_t *dac_filter_get(int index);

/* Decode a RECHORD.CFG value: a named identifier ("fast_lin", ...) or the
 * legacy 1..5 number written by the previous firmware. Anything else falls
 * back to index 0 (Fast-Lin, the bring-up default). */
int dac_filter_index_from_text(const char *text);

#endif /* RECHORD_AUDIO_DAC_FILTER_H */
