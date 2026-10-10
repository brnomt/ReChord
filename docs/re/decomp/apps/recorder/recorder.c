#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/apps/recorder/recorder.c
 * Reconstructed from Ghidra decompilation of:
 *   RecordWinSvc_Start @ 0x0300aabc
 *
 * Recording service — captures line-in audio to U:\RECORD
 * Uses SAR-ADC or I2S input from PMU_INL/PMU_INR pins.
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"

/*
 * RecordWinSvc_Start @ 0x0300aabc
 *
 * Called when user activates recording mode.
 * Checks if previous recording exists, offers to append or
 * start new recording.
 */
void RecordWinSvc_Start(void *record_context) {
    debug_printf("RecordWinSvcStart \n");

    /* Check if record file exists */
    int result = rom_record_file_exists(
        g_record_path,
        g_record_file_size
    );
    
    if (result == 1) {
        /* Existing recording found — resume/append */
        debug_printf("Record again \n");
    } else {
        /* New recording */
        rom_record_start(
            g_record_path,
            g_record_file_size,
            record_context
        );
    }
    
    /* Recording started — enter service loop */
}
