/*
 * firmware/stubs/fiio_globals.c
 * Placeholder BSS for Ghidra-recovered globals (v3.7.0).
 * Replace with real addresses from fiio_map.h as RE progresses.
 */

#include <stdint.h>
#include "decomp_support.h"
#include "decomp_globals.h"

/* entry.c */
typedef struct {
    uint8_t base, cols, rem, zero, pad, mode_clamped;
} BootLayout;

BootLayout g_boot_layout_storage;
BootLayout *g_boot_layout = &g_boot_layout_storage;
uint16_t g_boot_mode_storage;
uint16_t *g_boot_mode = &g_boot_mode_storage;

/* usb_dac.c */
void *g_usb_dev;

/* music_service.c — minimal placeholders */
int g_retry_counter;
uint32_t g_dma_dest_addr;
int g_play_active;
uint32_t g_dac_sample_rate = 48000;
void *g_codec_handle;
