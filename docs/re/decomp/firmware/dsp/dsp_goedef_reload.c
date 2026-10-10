#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/firmware/dsp/dsp_goedef_reload.c
 * Reconstructed from Ghidra decompilation of:
 *   DSP_GOODEF_Reload @ 0x0301022c
 *
 * Reloads the GOODEF DSP firmware blob ("WOOOOONXBIN") and optionally
 * re-runs DSP_GOODEF_Process after restoring the edit-buffer cursor ('|').
 * Called from ~15 UI/DSP toggle sites (FUN_03007896 family).
 *
 * Formerly misnamed DICTDECODER_DispatchFile.
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "dsp_api.h"

extern int  rom_event_pending(int id);
extern void rom_post_event(int id);
extern int  rom_event_clear_bit(int id);     /* func_0x02ffe664 */
extern void rom_status_led(int a, int b);    /* func_0x02ffec94 */
extern int  media_reopen(void *path);        /* FUN_03020810 */
extern void debug_print(const char *s);      /* FUN_030047cc */
extern void dsp_preview(void *ctx);          /* FUN_0300fa06 */

extern int  DSP_GOODEF_Init(const char *bin, int mode);
extern void DSP_GOODEF_Process(void);

#define CURSOR_CHAR 0x7c

/*
 * DSP_GOODEF_Reload @ 0x0301022c
 */
void DSP_GOODEF_Reload(void)
{
    uint8_t *gs = g_global_state;           /* DAT_03010254 */
    char *name = g_filename_buf;            /* *(ctx+8) */
    uint8_t *ed = g_edit_state;
    int ok_flag = 0;

    /* Place cursor marker at start of name buffer */
    name[1] = 0;
    name[0] = CURSOR_CHAR;
    ed[8] = 0;
    gs[0x1bb] = 0;
    ed[0] = 0;
    ed[2] = 0;
    ed[3] = 0;

    if (rom_event_pending(0x159) == 0) {
        /* Cold reload — mode 6 */
        char rc = (char)DSP_GOODEF_Init("WOOOOONXBIN", 6);
        ok_flag = (rc == 1) ? 1 : 0;
    } else {
        /* Warm reload while EQ event pending — mode 2 */
        DSP_GOODEF_Init("WOOOOONXBIN", 2);
    }

    rom_status_led(0x3b, 7);
    g_dsp_status[1] = (uint8_t)media_reopen(g_media_path); /* store fd @ +4 */

    if (rom_event_clear_bit(0x159) == 0) {
        rom_post_event(0x32);
    } else {
        /* Restore 2-char EQ tag into name, then process */
        uint8_t cur = *g_eq_slot;
        uint32_t k;
        ed[0] = cur;
        ed[4] = cur;
        for (k = 0; k < 2; k++)
            name[k] = (char)g_eq_tag[k];

        if (gs[0x1bc] == 0) {               /* STAGE_EDIT */
            name[ed[0] + 1] = 0;
            name[ed[0]] = 0;
        }
        if (gs[0x356])
            dsp_preview(g_dsp_preview_ctx);
        if (gs[0x1bc] == 0)
            name[ed[0]] = CURSOR_CHAR;

        DSP_GOODEF_Process();
    }

    (void)ok_flag;
    debug_print("V333!!!\n");
}
