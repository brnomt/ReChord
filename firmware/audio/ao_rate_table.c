/*
 * ao_rate_table.c — pure per-rate audio configuration table.
 * VERBATIM data relocation from dac_cs43131.c (commit c5d571f).
 */
#include "ao_rate_table.h"

const dac_rate_t dac_rates[13] = {
    {  8000,  4096000, { 0x00, 0x30, 0x32, 0x08 }, 0 },
    { 11025,  5644800, { 0x03, 0x12, 0x19, 0x03 }, 1 },
    { 12000,  6144000, { 0x00, 0x18, 0x19, 0x0A }, 2 },
    { 16000,  4096000, { 0x00, 0x30, 0x32, 0x08 }, 0 },
    { 22050,  5644800, { 0x03, 0x12, 0x19, 0x03 }, 1 },
    { 24000,  6144000, { 0x00, 0x18, 0x19, 0x0A }, 2 },
    { 32000,  8192000, { 0x00, 0x60, 0x32, 0x08 }, 0 },
    { 44100, 11289600, { 0x06, 0x24, 0x19, 0x03 }, 1 },
    { 48000, 12288000, { 0x00, 0x30, 0x19, 0x0A }, 2 },
    { 88200, 11289600, { 0x06, 0x24, 0x19, 0x03 }, 3 },
    { 96000, 24576000, { 0x00, 0x30, 0x19, 0x09 }, 4 },
    {176400, 22579200, { 0x04, 0x49, 0x19, 0x03 }, 5 },
    {192000, 24576000, { 0x00, 0x30, 0x19, 0x09 }, 6 },
};

const dac_rate_t *ao_rate_lookup(uint32_t rate_hz)
{
    int i;

    for (i = 0; i < 13; i++) {
        if (dac_rates[i].rate == rate_hz)
            return &dac_rates[i];
    }
    return 0;
}
