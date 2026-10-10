#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/apps/ui/bromem_sel_key_menu.c
 * Reconstructed from Ghidra decompilation of:
 *   BroMemSelKeyMenu_Handler @ 0x03013280
 *   Debug string: "BroMemSelKeyMenu"
 *
 * Browse-memory / select-key menu. Syncs the current storage selection
 * (flash vs card, etc.) with the UI list, optionally remaps keys via
 * KEYCHANGE_* bitmaps, then refreshes the on-screen menu.
 *
 * Echo Mini input is a physical LADC button matrix — not touch.
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "ui_theme.h"

extern void ui_log(const char *tag);                 /* func_0x03007118 */
extern int  rom_storage_cmp(void *a, void *b);       /* func_0x02ff0020 */
extern void rom_storage_copy(void *a, void *b);      /* func_0x02feff78 */
extern void rom_key_remap_apply(void);               /* func_0x02fff4f4 */
extern char rom_set_key_mode(uint8_t mode);          /* func_0x02ff0852 */
extern void rom_draw_dialog(void *a, void *b, void *msg); /* func_0x02fefe90 */
extern void rom_key_remap_done(void);                /* func_0x02fff4f6 */
extern void ui_list_refresh(int flag);               /* FUN_03012630 */
extern void ui_draw_item(uint32_t a, void *b, uint8_t c); /* FUN_0301245e */
extern void ui_draw_row(uint32_t a, uint32_t b, void *c, uint16_t d); /* FUN_030122a2 */
extern void event_pulse(int id);                     /* FUN_03000f94 */

/*
 * BroMemSelKeyMenu_Handler @ 0x03013280
 */
void BroMemSelKeyMenu_Handler(void)
{
    uint8_t *sel = g_mem_sel_ctx;       /* DAT_03013684 — +0xe = current mode */
    uint16_t *menu = (uint16_t *)g_menu_ctx; /* DAT_03013688 — +0x1ee = index */
    uint8_t *modes = g_mode_table;      /* DAT_0301368c */

    ui_log("BroMemSelKeyMenu");

    /* If displayed mode != table[index], resync storage + key map */
    if ((uint16_t)sel[0xe] !=
        modes[menu[0xf7 /* 0x1ee/2 */]]) {

        if (rom_storage_cmp(g_stor_a, g_stor_b) == 1)
            rom_storage_copy(g_stor_a, g_stor_b);

        sel[0x6c] = 0;
        sel[0xe] = modes[menu[0xf7]];

        if (sel[0xe] == 4) {
            /* Key-remapping mode (KEYCHANGE_*.BMP overlay) */
            rom_key_remap_apply();
            ui_enable_keychange(1);     /* FUN_03001776(1) */
        }

        if (rom_set_key_mode(sel[0xe]) != 0) {
            /* Prompt: dialog yes/no (glyph ids 0x38/0x37) */
            uint32_t msg[3];
            msg[0] = 0x370038;
            ((uint16_t *)msg)[2] = 3;
            ((uint16_t *)msg)[3] = 0;
            ((uint16_t *)msg)[4] = 1;
            rom_draw_dialog(g_dlg_a, g_dlg_b, msg);
            rom_key_remap_done();
            sel[0xe] = 2;
            rom_set_key_mode(2);
        }
    }

    /* Publish selection into list widget and redraw */
    {
        uint8_t *list = g_list_widget;  /* DAT_030136a0 */
        *(uint16_t *)(list + 10) = sel[0xe];
        *(uint16_t *)(list + 12) = 0;
        ui_list_refresh(1);
        ui_draw_item(*(uint32_t *)(menu[0x7d] + 4),
                     menu + menu[0x73] * 0xe + 3,
                     list[8]);
        ui_draw_row(*(uint32_t *)(menu[0x7d] + 4),
                    *(uint32_t *)menu,
                    menu + menu[0x73] * 0xe + 3,
                    menu[menu[0x73] + 0xe7 /* 0x1ce/2 */]);
    }

    event_pulse(0x14d);
    event_pulse(0x15c);
}
