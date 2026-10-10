#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/firmware/dsp/dsp_goedef.c
 * Reconstructed from Ghidra decompilation of:
 *   DSP_GOODEF_Init    @ 0x0300f7dc
 *   DSP_GOODEF_Process @ 0x0300fb0e
 *
 * The Echo Mini uses a GOODE family DSP chip for audio processing.
 * The DSP loads two firmware blobs:
 *   "WOOOOONXBIN" — DSP binary firmware (loaded via SPI)
 *   "GOODEFGHMP3" — DSP configuration / coefficients
 *
 * DSP_GOODEF_Init parameters:
 *   param2=0: Init "GOODEFGHMP3" config, then load "WOOOOONXBIN" firmware
 *   param2=1: Init "GOODEFGHMP3" config only (skip firmware load)
 *   param2=2: Abort/cleanup (returns NULL)
 *   param2=6: Init "WOOOOONXBIN" only, return success
 *
 * DSP_GOODEF_Process:
 *   Called AFTER DSP_GOODEF_Init with the audio frame to process.
 *   Reads the current DSP context, applies EQ/DSP effects,
 *   writes processed samples to output buffer.
 */

#include "../firmware.h"
#include "../os/os_api.h"
#include "dsp_api.h"

/* DSP communication struct (SPI/I2C register layout) */
typedef struct {
    int32_t  status;       /* -1 = uninitialized */
    int32_t  dsp_version;  /* offset 0x04 */
    int32_t  dsp_type;     /* offset 0x08 */
    int32_t  config_size;  /* offset 0x0c */
} DSPContext;

static DSPContext g_dsp_ctx;

/* DSP SPI file handles */
static const char *DSP_FIRMWARE_BIN  = "WOOOOONXBIN";  /* DSP binary */
static const char *DSP_CONFIG_MP3    = "GOODEFGHMP3";  /* DSP config */
static const char *DSP_ERROR_STR     = "error: !!!!!!\n";

/*
 * DSP_GOODEF_Init() — initialize GOODE DSP
 * @ 0x0300f7dc
 *
 * Returns NULL on failure, DSPContext* on success.
 *
 * The DSP is accessed via SPI (func_0x02fef2aa = SPI open file,
 * func_0x02fef470 = SPI read byte, func_0x02feeef6c = spi_flush,
 * func_0x02ff2e4a = SPI close) and the config is read via 
 * func_0x02fef58a which reads 0x24 bytes of DSP configuration.
 */
void *DSP_GOODEF_Init(const char *config_name, uint32_t mode) {
    int fd;
    
    if (mode < 3) {
        /*
         * Mode 0/1: Load DSP config file from flash
         * "GOODEFGHMP3" contains DSP coefficients/configuration
         */
        fd = rom_spi_open_file(DSP_CONFIG_MP3, DSP_ERROR_STR, &g_dsp_ctx);
        if (fd >= 0) {
            /* Read config byte by byte via SPI */
            for (uint32_t i = 0; (int)i <= fd; i++) {
                rom_spi_read_byte((uint8_t)i);
            }
            rom_spi_flush();
            rom_spi_close_file(DSP_CONFIG_MP3, DSP_ERROR_STR);
        }
        
        if (mode == 2) {
            return NULL;  /* Abort init */
        }
    }
    
    /*
     * Mode >= 3: Load DSP firmware binary
     * "WOOOOONXBIN" is the DSP's executable firmware
     */
    rom_spi_init(g_dsp_ctx_ptr, 0x24);  /* func_0x02feb510: init SPI transaction */
    g_dsp_ctx.status = -1;               /* Mark as loading */
    
    fd = rom_spi_open_file(config_name, config_name, &g_dsp_ctx);
    g_dsp_ctx.status = fd;
    
    if (fd == -1) {
        debug_printf(DSP_ERROR_STR);
        return (void *)-1;
    }
    
    if (mode == 6) {
        /* Quick init mode — just load firmware, no config */
        rom_spi_read_byte((uint8_t)fd);
        return (void *)1;
    }
    
    /*
     * Read DSP firmware descriptor (0x24 bytes)
     * Fields: version, type, config_size
     */
    uint8_t desc_buf[0x24] = {0};
    int bytes_read = rom_spi_read_block(desc_buf, 0x24, (uint8_t)fd);
    
    if (bytes_read == 0x24) {
        /* Parse descriptor into DSP context */
        g_dsp_ctx.dsp_version = *(int32_t *)(desc_buf + 0x00);
        g_dsp_ctx.dsp_type    = *(int32_t *)(desc_buf + 0x04);
        g_dsp_ctx.config_size = *(int32_t *)(desc_buf + 0x08);
        return &g_dsp_ctx;
    }
    
    debug_printf(DSP_ERROR_STR);
    return (void *)-1;
}

/*
 * DSP_GOODEF_Process() — process audio through DSP
 * @ 0x0300fb0e
 *
 * Called per audio frame (~10ms of audio).
 * Input:  g_current_buffer (pointer to PCM data)
 * Output: g_output_buffer (processed audio)
 *
 * The DSP path:
 *   1. If EQ=flat (state==1) and DSP loaded → apply DSP processing
 *   2. Mark output with separator byte 0x7C ('|')
 *   3. Return processed status
 */
uint32_t DSP_GOODEF_Process(void) {
    uint8_t *ctx_ptr = g_global_ctx;
    uint32_t result = 0;
    int dsp_fd;
    
    ctx_ptr[1] = 0;     /* Reset DSP processing flag */
    
    if (*ctx_ptr == 0) {
        return 1;         /* No data to process */
    }
    
    /* Check current EQ mode */
    if (g_eq_mode == 1) {
        /* EQ active — initialize DSP */
        dsp_fd = (int)DSP_GOODEF_Init(DSP_FIRMWARE_BIN, 0);
        
        uint8_t *state_ptr = g_state_data;
        uint8_t *audio_ptr = g_audio_buffer;
        
        if (dsp_fd != 0) {
            /* Clear current output buffer */
            if (g_boot_stage == 0) {
                *(audio_ptr + *ctx_ptr + 1) = 0;
                *(audio_ptr + *ctx_ptr) = 0;
            }
            
            /* Apply DSP processing */
            int apply_result = rom_dsp_apply(dsp_fd, audio_ptr, NULL);
            
            if (apply_result != 0) {
                ctx_ptr[1] = 1;  /* Mark as processed */
            }
        }
        
        /* Insert separator byte for downstream processing */
        if (g_boot_stage == 0) {
            *(audio_ptr + *ctx_ptr) = 0x7C;  /* '|' separator */
        }
        
        result = rom_dsp_get_status(dsp_fd);
        return result;
    }
    
    return g_eq_mode;  /* Return current EQ mode as status */
}
