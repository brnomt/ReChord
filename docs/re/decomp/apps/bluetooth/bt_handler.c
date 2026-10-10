#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/apps/bluetooth/bt_handler.c
 * Reconstructed from Ghidra decompilation of BT_Music_Handler @ 0x0300ddea
 *
 * The Echo Mini has a BT audio module (likely connected via UART).
 * This handler manages BT track metadata display and audio routing.
 *
 * BT metadata fields:
 *   offset 0x328: track name (Unicode, 16 bytes = 8 chars)
 *   offset 0x329: track name continued
 *   offset 0x23e: track info (6 bytes)
 *   offset 0x34a: audio config
 */
 
#include "../../firmware.h"
#include "../../firmware/os/os_api.h"

static uint16_t g_bt_track_name[128];
static uint8_t  g_bt_metadata[6];        /* offset 0x23e */
static uint8_t  g_bt_audio_cfg;          /* offset 0x34a */
static uint8_t  g_bt_field_count;        /* field at 0x328 */

void BT_Music_Handler(void) {
    uint16_t *display_buf = g_display_buffer;
    uint16_t disp_size = *display_buf;
    uint16_t disp_flags = display_buf[1];
    
    /* Save audio context */
    uint32_t saved_audio_ctx = rom_audio_save(1);
    uint32_t saved_dsp_ctx = rom_dsp_save(2);
    
    /* Display region for BT track name */
    uint16_t x = 0x1C;   /* column */
    uint16_t w = 0x81;   /* width */
    
    *display_buf = g_bt_state;
    display_buf[1] = 0xFFFF;
    
    /* Check BT event */
    int bt_event = rom_bt_event_check(0x1C6);
    
    if (bt_event == 0) {
        goto bt_done;
    }
    
    /* Signal BT processing start */
    uint32_t counter = 0;
    rom_bt_processing_start(0x7A);
    rom_bt_display_lock(1);
    
    uint16_t *bt_buf = g_bt_display_ptr;
    uint8_t *bt_state = g_bt_state_ptr;
    
    *display_buf = *bt_buf;
    
    /* Check audio config for BT routing */
    if (g_bt_audio_cfg == 0) {
        uint8_t bt_field = g_bt_field_count;
        
        if (bt_field == 0) {
            /* No BT metadata */
            rom_bt_display_default(0, 3, 0x140, 0xC, 2, 0x12D);
        } else if (bt_field == 1) {
            rom_bt_display_default(0, 3, 0x140, 0xC, 2, 0x7B);
        } else if (bt_field == 2) {
            rom_bt_display_default(0, 3, 0x140, 0xC, 2, 0x7C);
        } else {
            rom_bt_display_default(0, 3, 0x140, 0xC, 2, 0x12D);
        }
    } else {
        rom_bt_display_default(0, 3, 0x140, 0xC, 2, 0x12E);
    }
    
    /* Check BT track metadata available */
    int bt_ready = rom_bt_metadata_ready(0x1CF);
    
    uint8_t *field_ptr = g_bt_field_ptr;
    
    if (bt_ready == 0 && *field_ptr < 7) {
        /* Has track metadata — build display */
        do {
            x = 0x1C;
            uint16_t y_track = counter * 0x18;
            uint16_t y_disp = y_track + 0x1C;
            w = 0x81;
            uint16_t y_offset = y_track + 0x2B;
            
            /* Copy track name from BT buffer */
            rom_bt_copy_track_name(
                g_bt_track_buf + counter * 0x26 + 6,
                g_bt_track_name,
                NULL
            );
            
            if (field_ptr[1] == counter + 1) {
                /* Current track — highlight display */
                *display_buf = g_bt_track_ptr;
                
                uint32_t cmd1, cmd2;
                if (counter == 0) {
                    cmd1 = 0x86;
                    cmd2 = 0x17;
                } else if (counter == 5) {
                    cmd1 = 0x88;
                    cmd2 = 0x8F;
                } else {
                    cmd1 = 0x87;
                    cmd2 = counter * 0x18 + 0x17;
                }
                rom_bt_display_set(cmd1, 4, cmd2);
                
                /* Copy track name to display buffer */
                uint32_t name_len = rom_bt_string_len(g_bt_track_name);
                uint16_t *disp = g_bt_disp_ptr;
                
                if (name_len < (w - x)) {
                    rom_bt_display_error(0xAE);
                } else {
                    uint32_t i = 0;
                    do {
                        if (g_bt_track_name[i] == 0) break;
                        disp[i] = g_bt_track_name[i];
                        i++;
                    } while (i < 0x80);
                    
                    disp[i] = 0;
                    rom_bt_display_complete(&x, 0x8D, disp, 0x1E);
                    rom_bt_display_clear(0xAE);
                }
            } else {
                *display_buf = *bt_buf;
            }
            
            rom_bt_display_next(&x, &x, g_bt_track_name, 0);
            counter++;
        } while (counter < *field_ptr && counter < 6);
        
    } else if (field_ptr[1] < 7) {
        /* Second pass — partial metadata */
        do {
            rom_bt_copy_track_name(
                g_bt_track_buf + counter * 0x26 + 6,
                g_bt_track_name,
                NULL
            );
            
            x = 0x1C;
            uint16_t y_track = counter * 0x18;
            uint16_t y_disp = y_track + 0x1C;
            w = 0x81;
            uint16_t y_offset = y_track + 0x2B;
            
            if (field_ptr[1] == counter + 1) {
                *display_buf = g_bt_track_ptr;
                
                uint32_t cmd1, cmd2;
                if (counter == 0) {
                    cmd1 = 0x86;
                    cmd2 = 0x17;
                } else if (counter == 5) {
                    cmd1 = 0x88;
                    cmd2 = 0x8F;
                } else {
                    cmd1 = 0x87;
                    cmd2 = counter * 0x18 + 0x17;
                }
                rom_bt_display_set(cmd1, 4, cmd2);
                
                uint32_t name_len = rom_bt_string_len(g_bt_track_name);
                uint16_t *disp = g_bt_disp_ptr;
                
                if (name_len < (w - x)) {
                    rom_bt_display_error(0xAE);
                } else {
                    uint32_t i = 0;
                    do {
                        if (g_bt_track_name[i] == 0) break;
                        disp[i] = g_bt_track_name[i];
                        i++;
                    } while (i < 0x80);
                    
                    disp[i] = 0;
                    rom_bt_display_complete(&x, 0x8D, disp, 0x1E);
                    rom_bt_display_clear(0xAE);
                }
            } else {
                *display_buf = *bt_buf;
            }
            
            rom_bt_display_next(&x, &x, g_bt_track_name, 0);
            counter++;
        } while (counter < 6);
        
    } else {
        /* Full metadata — display all tracks */
        int32_t page = field_ptr[1];
        
        while (counter < *field_ptr + (page / 6) - (page / 6) * 6) {
            int offset = (page - 1) * 0xAB;
            rom_bt_copy_track_name(
                g_bt_track_ptr + (counter + ((offset >> 10) - (offset / 6)) * 6) * 0x26 + 6,
                g_bt_track_name,
                NULL
            );
            
            x = 0x1C;
            w = 0x81;
            
            if ((*field_ptr + (page / 6) - (page / 6) * 6) - counter == 1) {
                *display_buf = g_bt_track_ptr_all;
                
                uint32_t cmd1, cmd2;
                if (counter == 0) {
                    cmd1 = 0x86;
                    cmd2 = 0x17;
                } else if (counter == 5) {
                    cmd1 = 0x88;
                    cmd2 = 0x8F;
                } else {
                    cmd1 = 0x87;
                    cmd2 = counter * 0x18 + 0x17;
                }
                rom_bt_display_set(cmd1, 4, cmd2);
                
                uint32_t name_len = rom_bt_string_len(g_bt_track_name);
                uint16_t *disp = g_bt_disp_ptr;
                
                if (name_len < (w - x)) {
                    rom_bt_display_error(0xAE);
                } else {
                    uint32_t i = 0;
                    do {
                        if (g_bt_track_name[i] == 0) break;
                        disp[i] = g_bt_track_name[i];
                        i++;
                    } while (i < 0x80);
                    
                    disp[i] = 0;
                    rom_bt_display_complete(&x, 0x8D, disp, 0x1E);
                    rom_bt_display_clear(0xAE);
                }
            } else {
                *display_buf = g_bt_buf_ptr;
            }
            
            rom_bt_display_next(&x, &x, g_bt_track_name, 0);
            counter++;
        }
    }
    
    rom_bt_display_clear(0x21);
    
bt_done:
    /* Restore display buffer */
    int result = rom_bt_metadata_ready(0xAE);
    if (result == 1) {
        *display_buf = *g_bt_buf_ptr;
        rom_bt_display_commit(g_bt_disp_ptr);
        *display_buf = 0xFFFF;
    }
    
    rom_bt_display_unlock(0);
    rom_audio_save(saved_audio_ctx);
    
    display_buf[1] = disp_size;
    *display_buf = disp_flags;
    rom_dsp_save(saved_dsp_ctx);
}
