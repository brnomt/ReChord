#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/apps/audio/music_init.c
 * Reconstructed from Ghidra decompilation of MusicInit @ 0x0302b9d8
 *
 * This is the main audio subsystem initialization function.
 * It configures the DAC, sets sample rate, routes I2S channels,
 * and starts the playback engine.
 */

#include "../../firmware.h"
#include "../../rom_api.h"
#include "../../firmware/os/os_api.h"
#include "../../codecs/codec_api.h"
#include "audio_service.h"
#include "../../drivers.h"

/* 
 * MusicInit @ 0x0302b9d8
 *
 * Called during boot and on playback mode switch (e.g., USB DAC → playback).
 *
 * Sequence:
 *   1. Stop current audio path (mute DAC, stop DMA)
 *   2. Configure sample rate to 48000 Hz (default)
 *   3. Unmute DAC
 *   4. Read track metadata
 *   5. Configure codec based on format
 *   6. Route I2S channels (left ↔ right swap configurable)
 *   7. Start DMA transfer
 *   8. Signal ready (event 0x23)
 */
void MusicInit(void) {
    /* --- Stop all audio paths --- */
    rom_audio_path_disable(2, 0);
    rom_audio_path_disable(2, 0xe);
    rom_dac_mute(1, 4);
    rom_audio_clock_off();     /* func_0x02ff5c30 */

    /* --- Set default sample rate: 48000 Hz --- */
    rom_sample_rate_set(1, 2, 1, 48000);  /* func_0x02ffa410: ch=1, mode=2, en=1, rate=48000 */
    rom_dac_mute(1, 4);                    /* func_0x02ff44ce: mute channel 1, sub 4 */
    
    hifi_busy_delay(100);  /* after DAC mute; BL target 0x030098e4 in binary */
    
    rom_dac_unmute(1, 4);   /* func_0x02ff4580 */
    rom_dma_config(4, 1);    /* func_0x02ff6814: DMA ch=4, mode=1 */
    
    debug_printf("MusicInit!!");

    /* --- Read current track state --- */
    event_clear(0x2a);       /* Clear music init flag */
    
    MusicServiceCtx *ctx = (MusicServiceCtx *)g_music_ctx_ptr;
    ctx->codec_cfg.sample_rate = 0;       /* Will be filled by format parser */
    ctx->state = STATE_IDLE;               /* = 4 before parse */
    
    event_clear(0x114);      /* Clear system ready */
    
    /* Read track index from playlist */
    uint16_t track_idx = g_playlist_current;
    if (track_idx > 0 && track_idx < g_playlist_count) {
        ctx->track_index = track_idx;
    }
    
    /* --- Select codec based on file format --- */
    uint8_t usb_dac_mode = g_state.usb_dac_active;
    
    if (usb_dac_mode == 99) {
        /*
         * USB DAC mode — data comes from USB host, not file
         * Format: PCM stereo 16-bit
         */
        if (usb_dac_open_stream(3) != 0) {
            goto init_done;
        }
    } 
    else if (usb_dac_mode == 10) {
        /*
         * Bluetooth mode — data from UART/SPI BT module
         */
        if (rom_bt_stream_open(g_bt_dev_addr, 1) != 0) {
            ctx->track_index = 0;
            event_set(0xf5);  /* BT ready */
            goto init_done;
        }
    }
    
    /* Standard file playback — read file header to detect format */
    int fd = g_current_file_handle;
    HifiFile_Seek(fd, 0);
    
    /* Read first bytes for format detection */
    uint16_t header;
    HifiFile_Read(fd, &header, 2);
    
    ctx->codec_cfg.sample_rate = header;
    ctx->current_time_ms = 0;
    
    /* Read track duration from metadata */
    uint32_t duration_seconds = g_metadata_duration;
    ctx->codec_cfg.duration_ms = duration_seconds * 1000;
    
    /* Read audio parameters from file header */
    uint8_t volume_level = g_saved_volume;
    uint8_t gain_mode = g_saved_gain;
    uint8_t eq_preset = g_saved_eq;
    
    ctx->volume_level = volume_level;
    ctx->gain_mode = gain_mode;
    ctx->eq_preset = g_codec_params[eq_preset];  /* offset 0x28 */
    g_dac_filter_delay = g_codec_params[eq_preset]; /* offset 0x0c */
    
    /* Copy 5-band EQ settings */
    for (int i = 0; i < 5; i++) {
        ctx->eq_bands[i] = g_eq_settings[i];
    }
    
    /* --- Copy DSP config from context --- */
    rom_memcpy(g_dac_config_ptr + 0x0c, ctx, 0x0d);  /* func_0x02ff952e */
    
    /* --- Configure channel count --- */
    uint32_t ch_count = ctx->codec_cfg.num_channels;
    if (ch_count == 2 || ch_count == 3) {
        ctx->codec_cfg.bits_per_sample = 2;
    } else {
        ctx->codec_cfg.bits_per_sample = 24;  /* might be DSD */
    }
    
    /* --- USB/BT codec buffer setup --- */
    if (usb_dac_mode == 99 || usb_dac_mode == 10) {
        /* External source — configure codec buffer */
        DAC_BufferSetup(g_codec_buf, ctx->track_index, 
                        ctx->codec_cfg.bits_per_sample,
                        ctx->codec_cfg.duration_ms);
        
        if (usb_dac_mode != 99) {
            /* BT mode — additional setup */
            ctx->codec_cfg.sample_rate = ctx->volume_level;
            g_bt_ctx = g_bt_stream_ptr;
            g_bt_state = 0;
            g_bt_ready = 0;
            g_bt_active = 0;
            g_bt_counter = ctx->track_index;
        }
    }
    
    /* --- Configure sample buffer --- */
    if (ctx->codec_cfg.sample_rate > 0x2000) {
        ctx->codec_cfg.sample_rate = 0x2000;
    }
    if (ctx->sample_count > 0x2000) {
        ctx->sample_count = 0x1FFF;
    }
    
    /* --- Channel routing (mono → stereo, swap channels) --- */
    if (ctx->codec_cfg.num_channels == 1) {
        /*
         * Mono source — duplicate to both channels
         * Offset 0x50 controls channel duplication
         */
        if (ctx->buffer_pos == 1) {
            ctx->sample_count = 0;  /* Already duplicated */
        } else if (ctx->buffer_pos == 2) {
            if (ctx->sample_count != 1) {
                ctx->sample_count = 0xFF;  /* Mark for duplication */
            }
        } else {
            /* Calculate channel offset */
            ctx->sample_count = g_buffer_size % ctx->codec_cfg.sample_rate;
            
            /* Wait for DMA buffer alignment */
            int retry = 0;
            do {
                if (ctx->eq_preset != ctx->sample_count &&
                    g_playlist_current - 1 != ctx->sample_count) {
                    break;
                }
                ctx->sample_count = g_buffer_size % ctx->codec_cfg.sample_rate;
                hifi_busy_delay(15);
                retry++;
            } while (retry < 200);
        }
    }
    
    /* --- Apply EQ/DSP config --- */
    /*** These ROM calls set up the audio processing chain ***/
    
    rom_dsp_param_set(ctx->codec_cfg.sample_rate, ctx->sample_count);
    ctx->eq_preset = ctx->sample_count;
    ctx->buffer_pos = 0;
    
    /* --- Finalize --- */
    event_set(0x23);  /* Audio init complete */
    
                hifi_busy_delay(15);
    
    /* Start DSP processing */
    rom_dsp_start();  /* func_0x02ff55c0 */
    rom_dsp_bypass_disable();  /* func_0x02ff55ba */
    
    /* Configure audio buffer */
    /*** These ROM calls configure the I2S/DMA buffer chain ***/
    
    /* I2S left channel config */
    rom_i2s_channel_setup(0x1a, g_audio_master_clock);  /* func_0x02ffb3e6(0x1a) */
    rom_i2s_format_set(0, 2);  /* FUN_03012358: 16-bit stereo */
    rom_i2s_dma_start(0, 2);  /* FUN_0301231c */
    
    /* I2S right channel config */
    rom_i2s_channel_setup(0x1b, g_audio_slave_clock);  /* func_0x02ffb3e6(0x1b) */
    rom_i2s_format_set(0, 4);  /* 24-bit mode? */
    rom_i2s_dma_start(0, 4);
    
init_done:
    /* --- Switch audio routing: internal DAC or line-out --- */
    if (g_audio_routing_config == 0) {
        /* Route to internal headphone amp */
        rom_audio_path_set(2, 0);      /* func_0x02ff68f0: HP left */
        rom_audio_path_enable(2, 0, 1); /* func_0x02ffa6f0 */
        rom_audio_path_disable(2, 0);   /* func_0x02ffa72a */
        rom_audio_path_set(2, 0xe, 0); /* HP right */
        rom_audio_path_enable(2, 0xe, 1);
        rom_audio_path_disable(2, 0xe);
    } else {
        /* Route to line-out */
        rom_audio_path_set(2, 0);
        rom_audio_path_enable(2, 0, 1);
        rom_audio_path_disable(2, 0);
        rom_audio_path_set(2, 0xe, 0);
        rom_audio_path_enable(2, 0xe, 1);
        rom_audio_path_disable(2, 0xe);
    }
    
    /* --- Signal ready and start playback loop --- */
    event_set(0x23);
    g_audio_dma_read_pos = 0x50;  /* DMA buffer pointer */
    g_audio_buffer_base = g_buffer_size;
    g_audio_volume_step = 0xFF;   /* Volume interpolation step */
}
