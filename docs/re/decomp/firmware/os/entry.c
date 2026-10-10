#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/firmware/os/entry.c
 * Reconstructed from Ghidra decompilation of firmware v3.7.0
 *
 * firmware_entry @ 0x03000010
 * boot_param_layout @ 0x030000da  (NOT C runtime / BSS init)
 *
 * Actual Ghidra decomp of firmware_entry:
 *   boot_param_layout(param);
 *   ctx = rom_alloc(0x1dc);
 *   if (!ctx) rom_hw_init2(0x16f);
 *   else { rom_hw_init(0x1dc); rom_hw_init(0x16f)×2; rom_hw_init2(0x171); rom_hw_init(0x170); }
 *   rom_early_init();
 *   if (*param != 0xb) { rom_hw_init2(399); return; }
 *   rom_hw_init2(0x191); return;
 *
 * NOTE: MusicInit / FormatList_Init are NOT called directly from entry —
 * they are reached later via FUN_0304d022 / event paths.
 */

#include "firmware.h"
#include <stddef.h>
#include "os_api.h"
#include "../../rom_api.h"
#include "../../drivers.h"

/* Layout state written by boot_param_layout (DAT_03000164 / DAT_03000168) */
typedef struct {
    uint8_t  base;       /* +0  forced to 8 */
    uint8_t  cols;       /* +1  ((mode/6)*3)*2, possibly -6 */
    uint8_t  rem;        /* +2  mode - cols */
    uint8_t  zero;       /* +3  always 0 */
    uint8_t  pad;        /* +4  min(6, 8-cols) */
    uint8_t  mode_clamped; /* +5  input mode, max 0xd */
} BootLayout;

extern BootLayout *g_boot_layout;   /* DAT_03000164 */
extern uint16_t   *g_boot_mode;     /* DAT_03000168 */
extern void  boot_param_layout(uint16_t *param);

/*
 * firmware_entry @ 0x03000010
 *
 * Bootloader jumps here with a short* boot parameter block.
 * Mode 0x0B selects the "full" init path (rom_hw_init2(0x191));
 * any other mode takes the early-exit path (rom_hw_init2(399)).
 */
void firmware_entry(uint16_t *param)
{
    void *ctx;

    boot_param_layout(param);

    ctx = rom_alloc(0x1dc);
    if (ctx == NULL) {
        rom_hw_init2(0x16f);
    } else {
        rom_hw_init(0x1dc);
        rom_hw_init(0x16f);
        rom_hw_init(0x16f);
        rom_hw_init2(0x171);
        rom_hw_init(0x170);
    }

    rom_early_init();

    if (*param != 0x0b) {
        rom_hw_init2(399);
        return;
    }
    rom_hw_init2(0x191);
}

/*
 * boot_param_layout @ 0x030000da
 *
 * Derives a small display/layout descriptor from *param.
 * Accepted mode codes: 0,1,2,3,4,5,8,10 (else treated as 0).
 * Clamps mode to ≤ 0x0D, then computes column/padding fields.
 */
void boot_param_layout(uint16_t *param)
{
    uint16_t mode = 0;
    uint32_t m, cols;
    int pad;

    g_boot_layout->base = 8;

    switch (*param) {
    case 0: case 1: case 2: case 3: case 4: case 5: case 8: case 10:
        mode = *param;
        break;
    default:
        break;
    }

    *g_boot_mode = mode;
    g_boot_layout->mode_clamped = (uint8_t)mode;
    if ((mode & 0xff) > 0x0c)
        g_boot_layout->mode_clamped = 0x0d;
    if (*g_boot_mode > 0x0c)
        *g_boot_mode = 0x0d;

    m = g_boot_layout->mode_clamped;
    cols = ((m / 6) * 3 & 0x7f) * 2;
    g_boot_layout->cols = (uint8_t)cols;
    if (m <= cols && cols != m) {
        cols -= 6;
        g_boot_layout->cols = (uint8_t)cols;
    }
    g_boot_layout->rem = (uint8_t)(m - cols);
    pad = 8 - (int)(cols & 0xff);
    if (pad > 6) pad = 6;
    g_boot_layout->pad = (uint8_t)pad;
    g_boot_layout->zero = 0;
}
