#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/firmware/media/format_list.c
 * Reconstructed from Ghidra decompilation of FormatList_Init @ 0x03013c10
 *
 * Builds the supported format string table used by the UI to show
 * supported codecs (shown on the "About" screen).
 *
 * Format string: "MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP43GPDFFDSF"
 * Memory layout: 14 entries × 14 bytes each (struct size 0x0E)
 *
 * Each entry:
 *   offset 0x00: format code (0-13)
 *   offset 0x02: flags
 *   offset 0x04: track count / playlist position
 *   offset 0x06: file index / handle
 *   offset 0x08: playback state
 *   offset 0x0A: duration
 *   offset 0x0C: unknown
 */

#include "../../firmware.h"
#include "../os/os_api.h"

/* Format list struct (14 bytes per entry) */
typedef struct {
    uint16_t format_code;    /* 0x00: FORMAT_MP1..FORMAT_DSF */
    uint16_t flags;           /* 0x02 */
    int16_t  track_count;     /* 0x04: tracks in list */
    uint16_t file_index;      /* 0x06 */
    uint8_t  state;           /* 0x08: playback state */
    uint8_t  media_source;    /* 0x09: 0=flash, 1=card, 3=USB, 4=BT */
    uint32_t duration;        /* 0x0A: total playlist duration */
} FormatEntry;

/* Format name table */
static const char FORMAT_NAMES[] = 
    "MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP43GPDFFDSF";
static const uint8_t FORMAT_LENGTHS[] = {
    3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 6, 3, 3
};

/* Global state */
static FormatEntry g_playlist[8];        /* max 8 format lists */
static uint16_t    g_playlist_count;     /* active playlist count */
static uint16_t    g_current_playlist;   /* currently selected */
static uint8_t     g_media_type;         /* 0=flash, 1=card */

/*
 * FormatList_Init @ 0x03013c10
 *
 * Initializes the supported format list with detected codecs.
 * Called during boot after hardware init.
 *
 * Returns 1 if format table was built, 0 if no formats detected.
 */
int FormatList_Init(void) {
    uint16_t entry_count = g_playlist_count;
    FormatEntry *current;
    uint32_t i;

    /* Find current playlist entry */
    current = &g_playlist[0];
    for (i = 0; i < entry_count && i < 8; i++) {
        current = (FormatEntry *)((uint32_t)current + 4);  
    }

    /* Check if this is first format entry */
    if (current->state == 0) {
        /* Read current track from metadata */
        uint16_t track_idx = g_playlist_count;
        
        if (g_playlist[track_idx].track_count != 0) {
            /* Track exists in current playlist */
            int16_t file_status = g_file_info.flags;
            
            /* Check if playlist is empty and file is valid */
            if (g_playlist[track_idx].format_code == 0 && 
                entry_count == 0 && 
                file_status == 1) {
                
                /* First scan — build format table from detected codecs */
                rom_media_scan_start(1);
                
                /* Format detection by file header */
                uint16_t handle = rom_format_detect(
                    1,                              /* media=flash */
                    (uint32_t)current + 8,          /* file path */
                    FORMAT_NAMES,                   /* format string table */
                    g_file_info.media               /* media flags */
                );
                
                /* Notify system of format table rebuild */
                rom_event_notify(3, g_event_handle, 0x80);
                
                /* Check result */
                int32_t result = rom_format_open(
                    g_format_db_handle,
                    g_format_db_size
                );
                
                if (result == 1) {
                    rom_format_close(
                        g_format_db_handle,
                        g_format_db_size
                    );
                }
                
                /* Set USB/BT mode states */
                if (g_media_type == 3) {
                    /* USB DAC mode */
                    g_usb_state = 99;
                } else {
                    /* File playback mode */
                    g_usb_state = 10;
                }
                
                /* Copy track info to playlist entry */
                g_playlist[track_idx].format_code = track_idx;
                g_playlist[track_idx].flags = 0;
                g_playlist[track_idx].track_count = current->track_count;
                g_playlist[track_idx].file_index = current->file_index;
                g_playlist[track_idx].state = current->state;
                
                /* Mark scan complete events */
                event_set(0x18C);
                event_set(0x18B);
                event_clear(0x15D);
                
                /* Start media library build */
                rom_media_lib_start(3, &handle);
                return 1;
            }
            
            /* Append new entry to playlist (max 7 entries) */
            if (track_idx < 7) {
                g_playlist[entry_count].format_code = entry_count;
                g_playlist_count = track_idx + 1;
                g_playlist[entry_count].format_code = 0;
                g_playlist_count = 0;
                
                /* Copy metadata */
                g_playlist[track_idx].track_count = current->file_index;
                g_playlist[track_idx].duration = (current->duration << 16) | current->duration;
                
                if (g_media_type == 3) {
                    /* USB mode: use duration from metadata */
                    g_playlist[track_idx].media_source = current->duration & 0xFF;
                    g_playlist[track_idx].duration = current->duration & 0xFFFF;
                } else {
                    g_playlist[track_idx].media_source = 1;
                }
                
                /* Save to settings */
                rom_settings_save_entry(
                    g_playlist[track_idx].format_code,
                    &g_playlist[track_idx].track_count,
                    track_idx
                );
                
                rom_settings_save_track(
                    g_playlist[track_idx].format_code,
                    g_playlist[track_idx].format_code,
                    &g_playlist[track_idx].track_count,
                    g_playlist[entry_count].format_code,
                    g_file_info.media
                );
                
                if (g_playlist[track_idx].track_count == 0) {
                    event_clear(0x15C);
                } else {
                    event_clear(0x14F);
                    event_set(0x154);
                }
            }
        }
    } 
    else {
        /* Re-scan existing playlist */
        uint8_t old_media = current->state;
        int media_result = rom_media_scan_start(old_media);
        
        if (g_file_info.flags == 3) {
            /* USB mode */
            if (old_media != 1 && old_media != 4) {
                uint16_t handle = rom_media_find(current, 0);
                handle = rom_format_detect(
                    handle,
                    (uint32_t)current + 8,
                    FORMAT_NAMES,
                    g_file_info.media
                );
                g_current_playlist = handle;
            }
            current->track_count = rom_format_scan(current, 1);
        } else {
            /* Flash/SD card mode */
            current->track_count = (
                g_playlist[g_playlist_count].format_code +
                g_playlist_count -
                g_playlist[g_playlist_count].track_count + 1
            );
        }
        
        if (current->track_count != 0) {
            g_playlist[g_playlist_count].format_code = 
                g_playlist[entry_count].format_code;
            
            if (old_media == 1) {
                rom_format_close(g_format_db_handle);
                
                if (g_bt_active != 1) {
                    /* File mode — set format */
                    uint16_t handle = rom_media_find(current, 0);
                    handle = rom_format_detect(
                        handle,
                        (uint32_t)current + 8,
                        FORMAT_NAMES,
                        g_file_info.media
                    );
                    g_current_playlist = handle;
                    current->track_count = rom_format_scan(current, 1);
                }
                
                /* Start media */
                uint16_t entry_handle = rom_format_detect(
                    current->track_count,
                    (uint32_t)current + 8,
                    FORMAT_NAMES,
                    g_file_info.media
                );
                
                rom_event_notify(3, g_event_handle, 0x80);
                
                int32_t result = rom_format_open(
                    g_format_db_handle,
                    g_format_db_size
                );
                
                if (result == 1) {
                    int16_t usb_dac = g_usb_state;
                    
                    if (usb_dac == 99) {
                        if (g_file_info.flags == 3) {
                            uint16_t val = g_playlist_count;
                            
                            if (g_playlist[g_playlist_count].track_count == 2) {
                                val = rom_format_detect(
                                    val,
                                    (uint32_t)current + 0x34,
                                    FORMAT_NAMES,
                                    3
                                );
                                if (val == 0) {
                                    val = g_playlist_count;
                                }
                            }
                            
                            if (entry_handle == val) {
                                goto format_done;
                            }
                            g_playlist_count = entry_handle;
                        }
                    } else {
                        int16_t bt_mode = usb_dac;
                        if (bt_mode == 10) {
                            bt_mode = g_file_info.flags;
                        }
                        
                        if (bt_mode == 10 && bt_mode == 1) {
                            uint16_t val = g_playlist_count;
                            
                            if (g_playlist[g_playlist_count].track_count == 2) {
                                val = rom_format_detect(
                                    val,
                                    (uint32_t)current + 0x34,
                                    FORMAT_NAMES,
                                    1
                                );
                                if (val == 0) {
                                    val = g_playlist_count;
                                }
                            }
                            
                            if (entry_handle == val) {
                                event_set(0x18C);
format_done:
                                event_clear(0x15D);
                                rom_media_lib_start(3, NULL);
                                return 1;
                            }
                        }
                    }
                }
                
                /* Reset scan state */
                g_bt_active = 0;
                g_bt_counter = 0;
                rom_format_close(g_format_db_handle);
                g_bt_active = 0;
                
                if (g_file_info.flags == 3) {
                    g_usb_state = 99;
                    g_playlist_count = 0;
                    g_current_playlist = 0;
                } else {
                    g_playlist_count = 0;
                    g_current_playlist = 0;
                    g_usb_state = 10;
                }
                
                event_set(0x18C);
                event_clear(0x15D);
                event_set(0x18B);
                rom_media_lib_start(3, &entry_handle);
                return 1;
            }
        }
    }
    
    return 0;
}
