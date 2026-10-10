#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/apps/audio/music_service.c
 * Reconstructed from Ghidra decompilation of MusicService_Init @ 0x0302a3e0
 *
 * This is the main playback engine. Called after MusicInit configures
 * the hardware. It opens the audio codec IC, sets up the playback
 * buffer chain, and enters the main playback loop.
 *
 * The service runs continuously — it decodes frames, feeds them to
 * the DSP, and outputs via I2S to the DAC.
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../../firmware/dsp/dsp_api.h"
#include "../../codecs/codec_api.h"
#include "audio_service.h"

/* Globals referenced by MusicService_Init */
static uint8_t    g_codec_mode;        /* 0x0302a7c4: 0=flash, 1=card, 2=USB, 3=BT */
static void      *g_codec_handle;      /* 0x0302a7c8: pointer to codec instance */
static uint8_t    g_audio_routing;     /* 0x0302a840: HP vs line-out */
static uint16_t   g_sample_rate;       /* 0x0302a808+0x??: current sample rate */
static uint32_t   g_buffer_size;       /* 0x0302a814: playback buffer size */
static uint32_t   g_buffer_count;      /* 0x0302a810: number of buffer slots */
static uint32_t   g_dac_sample_rate;   /* chip-specific sample rate (48000 or 44100) */

/*
 * MusicService_Init @ 0x0302a3e0
 *
 * Parameters:
 *   param1 - unused
 *   param2 - MusicServiceCtx pointer (playback state)
 *
 * Flow:
 *   1. Check codec chip type → set DAC sample rate
 *   2. Open audio codec IC (audiocodec_open)
 *   3. Configure I2S bus
 *   4. Set up playback buffer chain
 *   5. Enter main playback loop
 */
void MusicService_Init(void *param1, MusicServiceCtx *ctx) {

    /* --- Detect codec chip and configure sample rate --- */
    uint8_t codec_mode = g_codec_mode;
    
    if (ctx->current_time_ms == 0) {
        /* First init — auto-detect DAC sample rate from codec chip */
        if (codec_mode == 2 || codec_mode == 3) {
            /* USB DAC / Bluetooth — force 48kHz */
            g_dac_sample_rate = *(uint32_t *)(g_codec_handle + 0x0d);
        }
    }
    
    if (ctx->sample_count != 1) {
        /* Check if previous playback session was stopped cleanly */
        rom_audio_cleanup();  /* FUN_0302a2be */
    }
    
    /* --- Signal init start --- */
    event_set(0xf7);   /* Mark system as initializing */
    rom_peripheral_reset();  /* FUN_03029e22 */
    
    /* Print debug and check for errors */
    debug_printf("Audio File Open Error");  /* default string, may be overwritten */
    
    /* --- Open audio codec IC --- */
    ctx->state = STATE_IDLE;
    
    int result = audiocodec_open(48000);  /* 48kHz default */
    
    if (result == -1) {
        /* Codec open failed */
        ctx->state = STATE_ERROR;
        
        if (g_sample_rate < 3) {
            event_set(0x55);  /* Report timeout */
        }
        
        debug_printf("######## AudioCodec Open Error ########");
        return;
    }
    
    debug_printf("audio codec open success.");
    
    /* --- Configure I2S and DMA --- */
    rom_i2s_master_config();    /* FUN_03029c74 */
    rom_dma_buffer_config();    /* FUN_03029b44 */
    
    /* Set DMA channel for audio */
    rom_dma_channel_set(4, 1);  /* func_0x02ff6814 */
    
    /* Copy sample rate to codec config */
    uint8_t audio_output_mode = *(uint8_t *)(g_codec_handle + 0x10);
    ctx->dac_filter = *(uint8_t *)(g_codec_handle + 0x12);
    
    /* --- I2S routing: master clock and bit clock --- */
    /* Configure I2S master for DAC communication */
    rom_i2s_master_init(0, 0x17, 0, 1);   /* func_0x02ff5752 */
    
    /* Set gain based on DAC sample rate */
    rom_dac_gain_set(*(uint32_t *)(g_codec_handle - 0x1a0 + 8));  /* FUN_030054ba */
    rom_dac_gain_set_channel(0, *(uint32_t *)(g_codec_handle - 0x1a0 + 8));  /* FUN_030054b6 */
    
    /* --- Configure playback buffer --- */
    uint32_t buf_size, buf_count;
    rom_buffer_calculate(&buf_size, &buf_count);  /* FUN_0302ca40 */
    
    /* Allocate zero-filled buffer */
    rom_memzero(buf_size, buf_count << 2);  /* func_0x02ff957c */
    
    /* Set DMA source address to buffer */
    void *dma_src = *(void **)(g_codec_handle - 0x1a0 + 0x9c);
    rom_dma_set_source(buf_size, buf_count, dma_src);  /* FUN_03009fe8 */
    
    /* --- Adjust for 44.1kHz vs 48kHz --- */
    uint32_t dac_rate = *(uint32_t *)(g_codec_handle - 0x1a0 + 8);
    
    if (dac_rate != 0x2EE00) {  /* 192000 Hz */
        uint32_t sample_count = g_buffer_count;
        
        if (sample_count < 8) {
            sample_count = *(uint32_t *)(g_codec_handle - 0x1a0 + 0x9c);
            if (sample_count < 24) {
                sample_count = sample_count;
            } else {
                sample_count = sample_count - 24;
            }
        } else {
            sample_count = sample_count - 8;
        }
        
        if (sample_count > 0) {
            /* Configure for 44.1kHz family */
            rom_clock_divider_set();  /* FUN_0302ccea */
            rom_pll_reconfig();       /* FUN_0302d808 */
            rom_audio_path_set_441(*(uint8_t *)(g_codec_handle + 0x10));  /* FUN_03029b00 */
        }
    }
    
    /* --- Set up audio output path --- */
    rom_dac_output_enable(*(uint8_t *)(g_codec_handle + 0x10));  /* FUN_03005604 */
    
    /* Reset playback counters */
    ctx->buffer_pos = 0;
    ctx->sample_count = 0;
    ctx->current_time_ms = 0;
    
    /* --- Wait for audio buffer to be ready --- */
    int ready = rom_buffer_ready(5);  /* func_0x02ffa224 */
    
    while (ready == 1) {
        hifi_busy_delay(1);  /* binary BLs 0x030098e4 — see dac_gain_curve note */
        /* Decrement retry counter (external variable) */
        if (--g_retry_counter == 0) break;
        ready = rom_buffer_ready(5);
    }
    
    /* --- Route audio to headphone or line-out --- */
    if (g_audio_routing == 0) {
        /* Headphone output route */
        rom_audio_path_set(2, 0);       /* Left channel */
        rom_audio_path_enable(2, 0, 1);
        rom_audio_path_select(2, 0);
        rom_audio_path_set(2, 0xe, 0); /* Right channel */
        rom_audio_path_enable(2, 0xe, 1);
        rom_audio_path_select(2, 0xe, 1);
    } else {
        /* Line-out route */
        rom_audio_path_set(2, 0);
        rom_audio_path_enable(2, 0, 1);
        rom_audio_path_select(2, 0, 1);
        rom_audio_path_set(2, 0xe, 0);
        rom_audio_path_enable(2, 0xe, 1);
        rom_audio_path_select(2, 0xe, 0);
    }
    
    /* --- Start playback --- */
    rom_playback_start(1, 2, 1, g_dac_sample_rate);  /* func_0x02ffa410 */
    rom_audio_path_set(1, 4, 0);                       /* func_0x02ff68f0 */
    rom_audio_path_enable(1, 4);
    rom_audio_path_select(1, 4);
    
    /* --- Start audio processing chain --- */
    if (g_sample_rate == 0) {
        /* Flash/SD card playback */
        ctx->sample_count = 1;
        /* Start DMA transfer from buffer to DAC */
        rom_dma_start(5, buf_size, g_dma_dest_addr, buf_count);  /* FUN_03004608 */
        
        ctx->playback_counter = 0;
        event_set(0x59);  /* Playback started */
        
        ctx->buffer_pos = 1;
        g_play_active = 1;
        
        rom_bt_playback_init();        /* FUN_0300bb32 - BT passthrough */
        rom_volume_sync();             /* FUN_030059bc - load saved volume */
        hifi_busy_delay(0x50);             /* 80ms-ish wait */
        
        rom_dac_unmute(1, 4);          /* func_0x02ff44ce - unmute DAC */
        
    } else if (g_sample_rate == 3) {
        /* Bluetooth mode */
        *(uint32_t *)(g_codec_handle - 0x174) = 0xFFFFFFFF;  /* BT stream position */
    }
    
    /* --- Load saved volume --- */
    rom_volume_sync();
    
    /* --- Apply saved DAC filter setting --- */
    if (*(uint8_t *)(g_codec_handle + 0x34a) != 0) {
        event_set(0x1d5);  /* DAC filter change event */
        event_clear(0x1d4); /* Clear pending filter */
    }
    
    /* --- Apply gain and EQ settings --- */
    ctx->dac_filter = *(uint8_t *)(g_codec_handle + 0x34a);
    ctx->gain_mode = 0;
    
    event_clear(0x1db);  /* EQ change event */
    event_set(0x2b);     /* Ready for playback */
    
    /* --- Signal completion --- */
    ctx->state = STATE_PLAYING;
    event_set(0x23);     /* Audio init complete */
    
    ctx->playback_counter = g_buffer_count;  /* Buffer fill level */
    
    debug_printf("MusicService start ok... ");
}
