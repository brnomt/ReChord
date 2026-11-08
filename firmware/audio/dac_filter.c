/*
 * dac_filter.c — the five CS43131 hardware reconstruction filters.
 *
 * Glyphs are 24x8 1-bit stem plots of the impulse response (one sample
 * every other column, the horizontal axis at row 4): the shape communicates
 * pre/post ringing and decay without any runtime DSP. They live in flash
 * (const), cost 24 bytes each and are blitted with display_bitmap1().
 *
 * Register map: PCM filter = CS43131 MAP 0x090000 (see audio_output.c).
 */
#include <string.h>

#include "dac_filter.h"

/* Fast roll-off, linear phase: symmetric ringing before and after the peak. */
static const uint8_t g_fast_lin[DAC_FILTER_GLYPH_H * DAC_FILTER_GLYPH_STRIDE] = {
    0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x88, 0x80,
    0x02, 0xAA, 0xA0, 0x02, 0x22, 0x20, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00,
};

/* Slow roll-off, linear phase: low-amplitude symmetric ringing, wider spread. */
static const uint8_t g_slow_lin[DAC_FILTER_GLYPH_H * DAC_FILTER_GLYPH_STRIDE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x88, 0x88,
    0x2A, 0xAA, 0xAA, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Fast roll-off, minimum phase: zero pre-ringing, short sharp post-ringing. */
static const uint8_t g_fast_min[DAC_FILTER_GLYPH_H * DAC_FILTER_GLYPH_STRIDE] = {
    0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x80,
    0x00, 0x0A, 0xA0, 0x00, 0x02, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
};

/* Slow roll-off, minimum phase: zero pre-ringing, long gentle post-decay. */
static const uint8_t g_slow_min[DAC_FILTER_GLYPH_H * DAC_FILTER_GLYPH_STRIDE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x88,
    0x00, 0x0A, 0xAA, 0x00, 0x02, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Non-oversampling: one clean Dirac pulse, no ringing. */
static const uint8_t g_nos[DAC_FILTER_GLYPH_H * DAC_FILTER_GLYPH_STRIDE] = {
    0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
    0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Index 0 is the bring-up default (0x02), so an unset/unknown cfg value
 * lands on the filter ReChord has always used. */
const dac_filter_t dac_filters[DAC_FILTER_COUNT] = {
    { "Fast-Lin", "fast_lin", "Flat, Neutral",     0x02, g_fast_lin },
    { "Slow-Lin", "slow_lin", "Soft Highs",        0x42, g_slow_lin },
    { "Fast-Min", "fast_min", "Punchy Transients", 0x82, g_fast_min },
    { "Slow-Min", "slow_min", "Acoustic, Smooth",  0xC2, g_slow_min },
    { "NOS",      "nos",      "Vintage Analog",    0x22, g_nos },
};

int dac_filter_clamp(int index)
{
    if (index < 0)
        return 0;
    if (index >= DAC_FILTER_COUNT)
        return DAC_FILTER_COUNT - 1;
    return index;
}

const dac_filter_t *dac_filter_get(int index)
{
    return &dac_filters[dac_filter_clamp(index)];
}

int dac_filter_index_from_text(const char *text)
{
    int i;

    if (text == 0 || text[0] == '\0')
        return 0;
    for (i = 0; i < DAC_FILTER_COUNT; i++) {
        if (strcmp(text, dac_filters[i].config) == 0 ||
            strcmp(text, dac_filters[i].name) == 0)
            return i;
    }
    /* Legacy RECHORD.CFG from the numeric-filter build: dac_filter=1..5. */
    if (text[0] >= '1' && text[0] <= '5' && text[1] == '\0')
        return (int)(text[0] - '1');
    return 0;
}
