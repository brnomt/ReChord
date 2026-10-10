#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/apps/audio/volume_control.c
 * Reconstructed from Ghidra decompilation of:
 *   FUN_030059bc @ 0x030059bc — volume sync/restore
 *   FUN_03004608 @ 0x03004608 — lyrics/tag parsing helper
 *
 * Volume system: 120 levels (0-119), 0.375dB per step
 *   DAC chip supports digital volume -95.5dB range
 *   + analog volume -12dB to +3dB in 1dB steps
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../../codecs/codec_api.h"

static uint8_t  g_volume_level = 50;     /* default volume */
static int16_t  g_volume_file_fd = -1;   /* volume save file handle */
static uint8_t  g_volume_saved = 0;      /* 1 = volume loaded from flash */

/*
 * volume_sync @ 0x030059bc
 *
 * Called during MusicInit/Service init to restore saved volume.
 * Reads volume from persistent storage file.
 *
 * param:
 *   force_reload = 1: close open handle, re-read from flash
 *   force_reload = 0: use cached volume if loaded
 */
void volume_sync(int force_reload) {
    if (force_reload) {
        /* Close existing volume file handle */
        if (g_volume_file_fd != -1) {
            rom_file_close(g_volume_file_fd);
            g_volume_file_fd = -1;
        }
        g_volume_saved = 0;
    }
    
    /* Open volume file if not cached */
    if (g_volume_file_fd == -1) {
        g_volume_file_fd = rom_file_close(g_volume_file_fd);
        g_volume_file_fd = -1;
    }
    
    /* Load OGG picture metadata? (PICTURE OGG string — likely album art in OGG) */
    rom_ogg_picture_init("PICTURE OGG");
    
    /* Open the settings file */
    int settings_fd = rom_file_open(
        g_settings_flash_addr + 0x14,      /* flash address */
        g_settings_flash_size,             /* flash file size */
        g_settings_file_index - 1          /* file index */
    );
    
    g_volume_saved = (settings_fd != -1);
    
    if (settings_fd != -1) {
        /* Try to load from flash settings file */
        int result = rom_file_load_block(settings_fd, g_volume_buf);
        
        if (result == 0) {
            goto volume_load_fail;
        }
        
        /* Parse volume/display settings from settings file */
        result = rom_file_read_sync(settings_fd, g_volume_offset);
        
        if (result == 0) {
            goto volume_load_fail;
        }
        
        g_volume_saved = 1;
        
        /* Check if volume/display values are valid (> 1000) */
        if (g_volume_value < 1000 && g_hw_display_value < 1000) {
            /* Write volume to hardware */
            rom_spi_write_reg(0x15c, 0x11, 0x16);  /* DAC volume register? */
            rom_dac_volume_set(0x1a);                /* Apply volume */
            rom_dac_output_config(g_volume_offset);  /* Output config */
            rom_dac_commit(0x1a);                    /* Commit */
            rom_file_close(settings_fd);
            g_volume_file_fd = -1;
            g_volume_saved = 1;
            return;
        }
    }
    
volume_load_fail:
    g_volume_saved = 0;
}

/*
 * DAC gain configuration — called from MusicInit
 * @ 0x030054ba
 *
 * Sets the DAC filter mode (fast/slow roll-off) based on
 * sample rate and chip type.
 */
void dac_gain_config(uint16_t sample_rate, uint32_t dac_mode, uint32_t filter_type) {
    uint16_t out_buf[16];
    
    /* Format sample rate to display string (e.g., "44.1k", "48k") */
    out_buf[0] = sample_rate / 10 + 0x30;
    out_buf[1] = sample_rate % 10 + 0x30;
    out_buf[2] = 0x6B;  /* 'k' */
    out_buf[3] = (filter_type & 0xFF);
    out_buf[4] = 0x70;  /* 'p' */
    out_buf[5] = 0x73;  /* 's' */
    
    /* Select DAC filter register based on mode */
    uint32_t dac_register;
    if (dac_mode == 3 || dac_mode == 5 || dac_mode == 6 || dac_mode == 7) {
        if (filter_type < 0x18) {
            if (g_dac_sample_rate_cfg <= sample_rate) {
                goto dac_fast;
            }
            if (sample_rate < 0xAC43) {
                goto dac_slow;
            }
dac_fast:
            dac_register = 0x13B;  /* Fast roll-off */
        } else {
            if (sample_rate < 0xAC43) {
dac_slow:
                dac_register = 0x13D;  /* Slow roll-off */
            } else {
                dac_register = 0x13B;
            }
        }
    } else {
        dac_register = 0x13D;  /* Default: slow roll-off */
    }
    
    /* Write register via SPI (0xFC, 0x76 = SPI address) */
    rom_spi_write_reg(dac_register, 0xFC, 0x76);
    
    /* Null-terminate output */
    out_buf[7] = 0;
    out_buf[8] = 0;
    out_buf[9] = 0;
    out_buf[10] = 0;
    
    /* Push to display queue */
    uint8_t display_cmd[4] = {0x1E, 0x76, 0x122, 0x85};
    rom_spi_send(1);
    
    /* Set display color based on theme */
    if (g_current_theme == 4) {
        g_theme_color = 0xE162;  /* Theme E color */
    } else {
        g_theme_color = 0x44DE;  /* Default FLAC string color */
    }
    
    /* Update display */
    rom_display_update(display_cmd);
    rom_spi_send(2);
}
