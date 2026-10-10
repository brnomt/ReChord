#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/apps/ui/main_ui_handler.c
 * Reconstructed from Ghidra decompilation of:
 *   MainUI_KeyHandler @ 0x0301020c
 *   Body spans ~0x0300fbde – 0x03010217
 *
 * Main-screen PHYSICAL KEYPAD handler (LADC button matrix — Echo Mini has no
 * capacitive touch). The LCD draws soft-keyboard letter cells and list rows;
 * rom_gui_check_area(x1,y1,x2,y2) tests whether the current key/focus maps into
 * those drawn rectangles (SDK-style GUI hit test, not a finger touch).
 *
 * Handles:
 *   - On-screen soft keyboard (a–z), 6 columns × 5 rows of LCD cells
 *   - Filename edit cursor ('|' = 0x7c)
 *   - List-mode row selection (boot_stage == 2)
 *   - EQ/DSP apply via DSP_GOODEF_Init("WOOOOONXBIN") + GOODEFGHMP3
 *   - Mode transitions (edit / browse / list)
 *
 * Global state offsets (DAT_03010254 / DAT_0300fe10):
 *   +0x1ba  media_type / file_count related
 *   +0x1bb  file_count flag
 *   +0x1bc  boot_stage: 0=edit, 1=browse, 2=list
 *   +0x21d  lcd_width
 *   +0x356  dsp_active
 *
 * Input-event bit codes (from rom_get_input_event):
 *   0x20000040  EQ / DSP apply
 *   0x20000002  soft-key "u" path
 *   0x20001000  cancel / close
 *   0x40000100  confirm / open
 *   0x40000002  secondary confirm
 *   0x20000400 / 0x20000800  cursor nav
 *   0x40000080  primary key/focus event (triggers GUI area walk)
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../../firmware/dsp/dsp_api.h"
#include "ui_theme.h"

/* Edit-buffer state (DAT_0300fe0c) */
typedef struct {
    uint8_t cursor;       /* +0 current edit position */
    uint8_t pending_eq;   /* +1 EQ apply pending */
    uint8_t key_col;      /* +2 soft-keyboard column (0..5) */
    uint8_t key_row;      /* +3 soft-keyboard row (0..4) */
    uint8_t saved_cursor; /* +4 */
    uint8_t _pad[3];
    uint8_t flag8;        /* +8 */
    uint8_t flag9;        /* +9 */
} EditState;

/* ROM UI helpers (stubbed in Ghidra ROM_API) */
extern int  rom_get_input_event(void);
extern int  rom_gui_check_area(int x1, int y1, int x2, int y2, int unused);
extern int  rom_event_pending(int id);
extern void rom_event_clear_id(int id);
extern void rom_post_event(int id);
extern void rom_ui_cmd(int cmd, void *arg);

extern int  DSP_GOODEF_Init(const char *bin, int mode);
extern void DSP_GOODEF_Process(void);
extern int  dsp_load_config(int handle, void *name_buf, const char *cfg); /* FUN_0300f8ba */
extern void dsp_close(int handle);                                       /* FUN_0300f8a2 */

/* Event / UI command IDs observed in the handler */
#define EVT_EQ_APPLY     0x20000040
#define EVT_SOFT_U       0x20000002
#define EVT_CANCEL       0x20001000
#define EVT_CONFIRM      0x40000100
#define EVT_NAV_PREV     0x20000400
#define EVT_NAV_NEXT     0x20000800
#define EVT_FOCUS_PRIMARY 0x40000080   /* key/focus — walk GUI rects */

#define STAGE_EDIT   0
#define STAGE_BROWSE 1
#define STAGE_LIST   2

#define CURSOR_CHAR  0x7c   /* '|' */

/*
 * MainUI_KeyHandler @ 0x0301020c
 *
 * Returns true when a consuming action (confirm/cancel) completed.
 */
bool MainUI_KeyHandler(void)
{
    EditState *ed = (EditState *)g_edit_state;   /* DAT_0300fe0c */
    uint8_t *gs = g_global_state;                /* DAT_03010254 / DAT_0300fe10 */
    int evt = rom_get_input_event();
    int action = evt;
    bool consumed = false;
    uint32_t i;

    /* --- Map key/focus into on-screen soft-key / list rectangles --- */
    if (evt == EVT_FOCUS_PRIMARY || evt == g_focus_sentinel ||
        (evt - 0x50000000) == 0x80) {

        if (gs[0x1bc] == STAGE_LIST) {
            /* Header strip (0,0)-(0x3c,0x32); then 6 list rows */
            if (rom_gui_check_area(0, 0, 0x3c, 0x32, 0) == 0) {
                for (i = 0; i < 6; i++) {
                    if (rom_gui_check_area(0, i * 0x18 + 0x1b, 0x140, i * 0x18 + 0x33, 0)) {
                        uint8_t count = gs[0x1ba];
                        if (count != 0 && ed->cursor != (uint8_t)i) {
                            uint8_t idx = (uint8_t)(i + (ed->cursor / 6) * 6);
                            if (idx < count) {
                                ed->cursor = idx;
                                ed->saved_cursor = idx;
                                rom_post_event(0x33);
                                rom_post_event(0x1c3);
                            }
                        }
                        break;
                    }
                }
            }
        } else {
            /* Edit / browse: header, mid button, right button, then 6×5 key grid */
            if (rom_gui_check_area(0, 0, 0x3c, 0x32, 0) == 0) {
                if (rom_gui_check_area(0x46, 0, 0xa0, 0x46, 0) != 0) {
                    if (evt != g_focus_sentinel)
                        action = EVT_EQ_APPLY;          /* mid → EQ */
                } else if (rom_gui_check_area(0xbc, 0, 0xf0, 0x3c, 0) != 0) {
                    if (gs[0x1bc] < 2)
                        action = 0x40000002;            /* right confirm */
                } else if (evt != g_focus_sentinel && gs[0x1bc] == STAGE_EDIT) {
                    for (i = 0; i < 6; i++) {
                        uint32_t j;
                        for (j = 0; j < 5; j++) {
                            if (rom_gui_check_area(i * 0x26 + 8, j * 0x26 + 0x7e,
                                                   i * 0x26 + 0x2e, j * 0x26 + 0xa4, 0)) {
                                ed->key_col = (uint8_t)i;
                                ed->key_row = (uint8_t)j;
                                rom_post_event(0x33);
                                rom_post_event(0x1c3);
                                i = 6;                  /* break outer */
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    /* Pending event 0x158 gate (LCD-width UI cmd on some paths) */
    if (rom_event_pending(0x158) != 0) {
        rom_event_clear_id(0x1bb);
        if (action == 0x40000040 || action == 0x40000200 ||
            action == 0x50000040 || action == 0x50000200) {
            uint16_t w = *(uint16_t *)(gs + 0x21d);
            rom_ui_cmd(8, &w);
            consumed = true;
            if (action != EVT_EQ_APPLY)
                return consumed;
        }
    }

    /* --- Action dispatch --- */
    switch (action) {
    case EVT_CONFIRM:           /* 0x40000100 */
    case 0x50000100:
    confirm_open:
        rom_event_clear_id(0x159);
        if (g_media_ctx->file_fd != -1)
            media_close_file();
        {
            uint16_t cmd = 6;
            rom_ui_cmd(10, &cmd);
        }
        return true;

    case EVT_CANCEL:            /* 0x20001000 */
        if (g_media_ctx->file_fd != -1)
            media_close_file();
        gs[0x1bb] = 0;
        rom_ui_cmd(0, NULL);
        return true;

    case EVT_SOFT_U:            /* 0x20000002 — insert soft glyph */
        if (gs[0x1bc] != STAGE_EDIT || ed->cursor == 0)
            return consumed;
        {
            uint8_t msg[5] = {1, 0x38, 0, 0x75, 0};
            rom_draw_glyph(g_font_a, g_font_b, msg);
        }
        return consumed;

    case EVT_EQ_APPLY: {        /* 0x20000040 — load DSP EQ */
        char *name = g_filename_buf;            /* *(DAT_0300fe08+8) */
        if (ed->pending_eq == 0)
            return consumed;
        if (ed->cursor == 0) {
            ed->pending_eq = 0;
            rom_post_event(0x32);
            return consumed;
        }
        {
            int h = DSP_GOODEF_Init("WOOOOONXBIN", 1);
            int ok;
            if (h == 0)
                return consumed;
            if (gs[0x1bc] == STAGE_EDIT) {
                name[ed->cursor + 1] = 0;
                name[ed->cursor] = 0;
            }
            ok = dsp_load_config(h, name, "GOODEFGHMP3");
            if (gs[0x1bc] == STAGE_EDIT)
                name[ed->cursor] = CURSOR_CHAR;
            dsp_close(h);
            rom_lcd_refresh(g_lcd_ctx);
            if (ok == 0) {
                rom_usb_status(0x3b, 7);
                g_media_ctx->file_fd = media_reopen(g_media_ctx->path);
                return consumed;
            }
            if (gs[0x1bc] == STAGE_EDIT) {
                *g_eq_slot = ed->cursor;
                name[ed->cursor] = 0;
            }
            /* copy first 2 name bytes into EQ tag buffer */
            {
                uint32_t k;
                for (k = 0; k < 2; k++)
                    g_eq_tag[k] = (uint16_t)(uint8_t)name[k];
            }
            rom_post_event(0x159);
            {
                uint16_t cmd = 1;
                rom_ui_cmd(3, &cmd);
            }
            return true;
        }
    }

    case EVT_NAV_PREV:          /* 0x20000400 */
    case EVT_NAV_NEXT:          /* 0x20000800 */
    case 0x20001400:
    nav_row:
        if (gs[0x1bc] == STAGE_EDIT) {
            /* soft-keyboard row wrap */
            if (action == EVT_NAV_PREV || action == 0x40000040 /* approx */) {
                ed->key_row = (ed->key_row == 0) ? 3 : (uint8_t)(ed->key_row - 1);
            } else {
                ed->key_row = (ed->key_row < 3) ? (uint8_t)(ed->key_row + 1) : 0;
            }
        } else if (gs[0x1bc] == STAGE_BROWSE) {
            ed->flag8 = 1;
            ed->flag9 = 0;
            return consumed;
        } else {
            /* STAGE_LIST — step cursor among files */
            uint8_t count = gs[0x1ba];
            if (count < 2)
                return consumed;
            ed->saved_cursor = ed->cursor;
            if (action == EVT_NAV_NEXT || action == 0x20000800) {
                ed->cursor = (ed->cursor + 1 < count) ? (uint8_t)(ed->cursor + 1) : 0;
            } else {
                ed->cursor = (ed->cursor == 0) ? (uint8_t)(count - 1)
                                              : (uint8_t)(ed->cursor - 1);
            }
            rom_post_event(0x33);
            return consumed;
        }
        rom_post_event(0x1c3);
        return consumed;

    default:
        break;
    }

    /* Soft-keyboard letter commit (area-check set key_col/key_row) */
    if (gs[0x1bc] == STAGE_EDIT && ed->key_col < 4) {
        if (ed->key_row == 4 && (ed->key_col == 2 || ed->key_col == 3)) {
            /* special keys — soft "u" path */
            if (ed->cursor != 0) {
                uint8_t msg[5] = {1, 0x38, 0, 0x75, 0};
                rom_draw_glyph(g_font_a, g_font_b, msg);
            }
            return consumed;
        }
        {
            uint32_t letter = ed->key_col + (uint32_t)ed->key_row * 6;
            char *name = g_filename_buf;
            if (letter > 0x1a)
                return consumed;
            name[ed->cursor] = (char)(letter + 'a');
            ed->cursor = (ed->cursor < 0xf) ? (uint8_t)(ed->cursor + 1) : 0;
            name[ed->cursor + 1] = 0;
            name[ed->cursor] = 0;
            if (gs[0x356])
                dsp_preview(g_dsp_preview_ctx);
            name[ed->cursor] = CURSOR_CHAR;
            DSP_GOODEF_Process();
            rom_post_event(0x33);
            return consumed;
        }
    }

    /* Stage transitions (browse ↔ list ↔ edit) */
    if (gs[0x1bc] == STAGE_BROWSE) {
        ed->saved_cursor = ed->cursor;
        gs[0x1bc] = STAGE_LIST;
    } else if (gs[0x1bc] == STAGE_LIST) {
        ed->cursor = 0;
        ed->key_col = 0;
        ed->key_row = 0;
        g_filename_buf[1] = 0;
        g_filename_buf[0] = CURSOR_CHAR;
        gs[0x1bc] = STAGE_EDIT;
    } else {
        if (g_media_ctx->file_fd != -1)
            media_close_file();
        gs[0x1bb] = 0;
        rom_event_clear_id(0x159);
        {
            uint16_t cmd = 6;
            rom_ui_cmd(10, &cmd);
        }
        consumed = true;
    }
    rom_post_event(0x32);
    return consumed;
}
