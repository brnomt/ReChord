/*
 * armcc_compat.h — Keil/armcc keyword compatibility for arm-none-eabi-gcc.
 *
 * The Rockchip RKnanoD SDK was written for the Keil ARM compiler (armcc),
 * which has non-standard keywords that GCC does not provide. This header
 * maps them to GCC equivalents. Forced in via `-include armcc_compat.h`.
 */
#ifndef ARMCC_COMPAT_H
#define ARMCC_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include "typedef.h"   /* SDK base types (UINT32, uint8, ...) for every TU */

/* __packed — packed struct (GCC: __attribute__((packed))) */
#ifndef __packed
#define __packed __attribute__((packed))
#endif

/* __irq — interrupt handler marker. armcc puts it after the declarator
   (e.g. typedef void(*f)(void) __irq), which is invalid for GCC attributes.
   Map to empty; ISR definitions in the SDK carry their own attributes. */
#ifndef __irq
#define __irq
#endif

/* __align(n) — align to n bytes (GCC: __attribute__((aligned(n)))) */
#ifndef __align
#define __align(n) __attribute__((aligned(n)))
#endif

/* __forceinline — always inline (GCC: __attribute__((always_inline)) inline) */
#ifndef __forceinline
#define __forceinline __attribute__((always_inline)) inline
#endif

/* __weak — weak symbol */
#ifndef __weak
#define __weak __attribute__((weak))
#endif

/* __inline — inline */
#ifndef __inline
#define __inline inline
#endif

/* __noreturn — no return */
#ifndef __noreturn
#define __noreturn __attribute__((noreturn))
#endif

/* __value_in_regs — struct returns in registers (GCC default for ARM) */
#ifndef __value_in_regs
#define __value_in_regs
#endif

/* __svc — supervisor call (GCC: inline asm; used rarely) */
#ifndef __svc
#define __svc(n) __attribute__((naked))
#endif

/* __attribute__((used)) — prevent GC */
#ifndef __used
#define __used __attribute__((used))
#endif

/* __interwork — ARM/Thumb interworking (default on Cortex-M) */
#ifndef __interwork
#define __interwork
#endif

/* MSR/MRS intrinsics the SDK may use */
#ifndef __MRS
#define __MRS(reg)  ({ uint32_t __v; __asm__ volatile("mrs %0, " reg : "=r"(__v)); __v; })
#endif
#ifndef __MSR
#define __MSR(reg, val) __asm__ volatile("msr " reg ", %0" :: "r"(val))
#endif
/* ---- SDK section attributes (linker scatter files) ----
   The RKnanoD SDK places functions/data in named sections via _ATTR_*_CODE_
   /_ATTR_*_DATA_/_ATTR_*_BSS_ macros. For GCC build we map them to the
   matching named section so the linker script can place them later. */
#ifndef _ATTR_OS_CODE_
#define _ATTR_OS_CODE_       /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_OS_DATA_
#define _ATTR_OS_DATA_       /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_OS_BSS_
#define _ATTR_OS_BSS_        /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_SYS_CODE_
#define _ATTR_SYS_CODE_      /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_SYS_DATA_
#define _ATTR_SYS_DATA_      /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_SYS_BSS_
#define _ATTR_SYS_BSS_       /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_SYS_INIT_CODE_
#define _ATTR_SYS_INIT_CODE_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_DRIVERLIB_CODE_
#define _ATTR_DRIVERLIB_CODE_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_DRIVER_CODE_
#define _ATTR_DRIVER_CODE_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_DRIVER_DATA_
#define _ATTR_DRIVER_DATA_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_DRIVER_BSS_
#define _ATTR_DRIVER_BSS_    /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_BB_SYS_DATA_
#define _ATTR_BB_SYS_DATA_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_AUDIO_BSS_
#define _ATTR_AUDIO_BSS_     /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_AUDIO_TEXT_
#define _ATTR_AUDIO_TEXT_    /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_FLACDEC_TEXT_
#define _ATTR_FLACDEC_TEXT_  /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_VECTTAB_BB_
#define _ATTR_VECTTAB_BB_    /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_VECTTAB_
#define _ATTR_VECTTAB_       /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_INTRRUPT_CODE_
#define _ATTR_INTRRUPT_CODE_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_INTRRUPT_DATA_
#define _ATTR_INTRRUPT_DATA_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
#ifndef _ATTR_OVERLAY_CODE_
#define _ATTR_OVERLAY_CODE_  /* section placement dropped (2026-10-10): GCC section-type conflicts */
#endif
/* codec binary blob sections (bb_core.c) — placeholder names */
#define _ATTR_AACDEC_BIN_TEXT_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_AACDEC_BIN_DATA_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_FLACDEC_BIN_TEXT_  /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_FLACDEC_BIN_DATA_  /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_APEDEC_BIN_TEXT_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_APEDEC_BIN_DATA_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_OGGDEC_BIN_TEXT_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_OGGDEC_BIN_DATA_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_WAVDEC_BIN_TEXT_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_WAVDEC_BIN_DATA_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_DSFDEC_BIN_TEXT_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_DSFDEC_BIN_DATA_   /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_DSDIFFDEC_BIN_TEXT_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_DSDIFFDEC_BIN_DATA_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_HIFI_ALACDEC_BIN_TEXT_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_HIFI_ALACDEC_BIN_DATA_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_HIFI_APEDEC_BIN_TEXT_  /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_HIFI_APEDEC_BIN_DATA_  /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_HIFI_FLACDEC_BIN_TEXT_ /* section placement dropped (2026-10-10): GCC section-type conflicts */
#define _ATTR_HIFI_FLACDEC_BIN_DATA_ /* section placement dropped (2026-10-10): GCC section-type conflicts */

/* Windows code-page constants used by the ID3/Font layer (project-level) */
#ifndef FONT_CODEPAGE_CP1252
#define FONT_CODEPAGE_CP1252  1252
#endif
#ifndef FONT_CODEPAGE_CP1251
#define FONT_CODEPAGE_CP1251  1251
#endif
#ifndef FONT_CODEPAGE_CP936
#define FONT_CODEPAGE_CP936   936
#endif
#ifndef FONT_CODEPAGE_CP949
#define FONT_CODEPAGE_CP949   949
#endif
#ifndef FONT_CODEPAGE_CP950
#define FONT_CODEPAGE_CP950   950
#endif

#endif /* ARMCC_COMPAT_H */

/* ---- ReChord: defaults de config del SDK (definidos en codec.h/SysConfig) ---- */
#ifndef MAX_VOLUME
#define MAX_VOLUME 32
#endif

/* ---- Generated catch-all (2026-10-09) ----
 * The Keil scatter files place code/data through ~400 _ATTR_*_ macros.
 * GCC has no equivalent placement syntax at these source positions, and
 * our firmware.ld links standard sections, so every remaining macro maps
 * to empty. If one of these placements ever matters (e.g. overlay module
 * sections), map THAT one to __attribute__((section(...))) explicitly. */
#ifndef _ATTR_AACDEC_BIN_BSS_
#define _ATTR_AACDEC_BIN_BSS_
#endif
#ifndef _ATTR_AACDEC_BSS_
#define _ATTR_AACDEC_BSS_
#endif
#ifndef _ATTR_AACDEC_DATA_
#define _ATTR_AACDEC_DATA_
#endif
#ifndef _ATTR_AACDEC_TEXT_
#define _ATTR_AACDEC_TEXT_
#endif
#ifndef _ATTR_APEDEC_BIN_BSS_
#define _ATTR_APEDEC_BIN_BSS_
#endif
#ifndef _ATTR_APEDEC_BSS_
#define _ATTR_APEDEC_BSS_
#endif
#ifndef _ATTR_APEDEC_DATA_
#define _ATTR_APEDEC_DATA_
#endif
#ifndef _ATTR_APEDEC_TEXT_
#define _ATTR_APEDEC_TEXT_
#endif
#ifndef _ATTR_AUDIO_DATA_
#define _ATTR_AUDIO_DATA_
#endif
#ifndef _ATTR_AUDIO_INIT_TEXT_
#define _ATTR_AUDIO_INIT_TEXT_
#endif
#ifndef _ATTR_AUDIO_SBC_ENCODE_BSS_
#define _ATTR_AUDIO_SBC_ENCODE_BSS_
#endif
#ifndef _ATTR_AUDIO_SBC_ENCODE_DATA_
#define _ATTR_AUDIO_SBC_ENCODE_DATA_
#endif
#ifndef _ATTR_AUDIO_SBC_ENCODE_TEXT_
#define _ATTR_AUDIO_SBC_ENCODE_TEXT_
#endif
#ifndef _ATTR_BB_SYS_BSS_
#define _ATTR_BB_SYS_BSS_
#endif
#ifndef _ATTR_BB_SYS_CODE_
#define _ATTR_BB_SYS_CODE_
#endif
#ifndef _ATTR_BLUETOOTHAUDIO_BSS_
#define _ATTR_BLUETOOTHAUDIO_BSS_
#endif
#ifndef _ATTR_BLUETOOTHAUDIO_CODE_
#define _ATTR_BLUETOOTHAUDIO_CODE_
#endif
#ifndef _ATTR_BLUETOOTHAUDIO_DATA_
#define _ATTR_BLUETOOTHAUDIO_DATA_
#endif
#ifndef _ATTR_BLUETOOTHCONTROL_BSS_
#define _ATTR_BLUETOOTHCONTROL_BSS_
#endif
#ifndef _ATTR_BLUETOOTHCONTROL_CODE_
#define _ATTR_BLUETOOTHCONTROL_CODE_
#endif
#ifndef _ATTR_BLUETOOTHCONTROL_DATA_
#define _ATTR_BLUETOOTHCONTROL_DATA_
#endif
#ifndef _ATTR_BLUETOOTHPHONE_BSS_
#define _ATTR_BLUETOOTHPHONE_BSS_
#endif
#ifndef _ATTR_BLUETOOTHPHONE_CODE_
#define _ATTR_BLUETOOTHPHONE_CODE_
#endif
#ifndef _ATTR_BLUETOOTHPHONE_DATA_
#define _ATTR_BLUETOOTHPHONE_DATA_
#endif
#ifndef _ATTR_BLUETOOTHVOICENOTIFY_BSS_
#define _ATTR_BLUETOOTHVOICENOTIFY_BSS_
#endif
#ifndef _ATTR_BLUETOOTHVOICENOTIFY_CODE_
#define _ATTR_BLUETOOTHVOICENOTIFY_CODE_
#endif
#ifndef _ATTR_BLUETOOTHVOICENOTIFY_DATA_
#define _ATTR_BLUETOOTHVOICENOTIFY_DATA_
#endif
#ifndef _ATTR_BLUETOOTHWIN_CODE_
#define _ATTR_BLUETOOTHWIN_CODE_
#endif
#ifndef _ATTR_BLUETOOTHWIN_DATA_
#define _ATTR_BLUETOOTHWIN_DATA_
#endif
#ifndef _ATTR_BOOKMASK_BSS_
#define _ATTR_BOOKMASK_BSS_
#endif
#ifndef _ATTR_BOOKMASK_CODE_
#define _ATTR_BOOKMASK_CODE_
#endif
#ifndef _ATTR_BOOKMASK_DATA_
#define _ATTR_BOOKMASK_DATA_
#endif
#ifndef _ATTR_BRO_CORE_BSS_
#define _ATTR_BRO_CORE_BSS_
#endif
#ifndef _ATTR_BRO_CORE_CODE_
#define _ATTR_BRO_CORE_CODE_
#endif
#ifndef _ATTR_BRO_CORE_DATA_
#define _ATTR_BRO_CORE_DATA_
#endif
#ifndef _ATTR_BRO_UI_BSS_
#define _ATTR_BRO_UI_BSS_
#endif
#ifndef _ATTR_BRO_UI_CODE_
#define _ATTR_BRO_UI_CODE_
#endif
#ifndef _ATTR_BRO_UI_DATA_
#define _ATTR_BRO_UI_DATA_
#endif
#ifndef _ATTR_CHARGE_WIN_BSS_
#define _ATTR_CHARGE_WIN_BSS_
#endif
#ifndef _ATTR_CHARGE_WIN_CODE_
#define _ATTR_CHARGE_WIN_CODE_
#endif
#ifndef _ATTR_CHARGE_WIN_DATA_
#define _ATTR_CHARGE_WIN_DATA_
#endif
#ifndef _ATTR_DIALOGBOX_BSS_
#define _ATTR_DIALOGBOX_BSS_
#endif
#ifndef _ATTR_DIALOGBOX_CODE_
#define _ATTR_DIALOGBOX_CODE_
#endif
#ifndef _ATTR_DIALOGBOX_DATA_
#define _ATTR_DIALOGBOX_DATA_
#endif
#ifndef _ATTR_DSDIFFDEC_BIN_BSS_
#define _ATTR_DSDIFFDEC_BIN_BSS_
#endif
#ifndef _ATTR_DSDIFFDEC_BSS_
#define _ATTR_DSDIFFDEC_BSS_
#endif
#ifndef _ATTR_DSDIFFDEC_DATA_
#define _ATTR_DSDIFFDEC_DATA_
#endif
#ifndef _ATTR_DSDIFFDEC_TEXT_
#define _ATTR_DSDIFFDEC_TEXT_
#endif
#ifndef _ATTR_DSFDEC_BIN_BSS_
#define _ATTR_DSFDEC_BIN_BSS_
#endif
#ifndef _ATTR_DSFDEC_BSS_
#define _ATTR_DSFDEC_BSS_
#endif
#ifndef _ATTR_DSFDEC_DATA_
#define _ATTR_DSFDEC_DATA_
#endif
#ifndef _ATTR_DSFDEC_TEXT_
#define _ATTR_DSFDEC_TEXT_
#endif
#ifndef _ATTR_FAT_BSS_
#define _ATTR_FAT_BSS_
#endif
#ifndef _ATTR_FAT_CODE_
#define _ATTR_FAT_CODE_
#endif
#ifndef _ATTR_FAT_DATA_
#define _ATTR_FAT_DATA_
#endif
#ifndef _ATTR_FAT_FIND_BSS_
#define _ATTR_FAT_FIND_BSS_
#endif
#ifndef _ATTR_FAT_FIND_CODE_
#define _ATTR_FAT_FIND_CODE_
#endif
#ifndef _ATTR_FAT_FIND_DATA_
#define _ATTR_FAT_FIND_DATA_
#endif
#ifndef _ATTR_FAT_INIT_BSS_
#define _ATTR_FAT_INIT_BSS_
#endif
#ifndef _ATTR_FAT_INIT_CODE_
#define _ATTR_FAT_INIT_CODE_
#endif
#ifndef _ATTR_FAT_INIT_DATA_
#define _ATTR_FAT_INIT_DATA_
#endif
#ifndef _ATTR_FAT_WRITE_BSS_
#define _ATTR_FAT_WRITE_BSS_
#endif
#ifndef _ATTR_FAT_WRITE_CODE_
#define _ATTR_FAT_WRITE_CODE_
#endif
#ifndef _ATTR_FAT_WRITE_DATA_
#define _ATTR_FAT_WRITE_DATA_
#endif
#ifndef _ATTR_FLACDEC_BIN_BSS_
#define _ATTR_FLACDEC_BIN_BSS_
#endif
#ifndef _ATTR_FLACDEC_BSS_
#define _ATTR_FLACDEC_BSS_
#endif
#ifndef _ATTR_FLACDEC_DATA_
#define _ATTR_FLACDEC_DATA_
#endif
#ifndef _ATTR_FLASH_BSS_
#define _ATTR_FLASH_BSS_
#endif
#ifndef _ATTR_FLASH_CODE_
#define _ATTR_FLASH_CODE_
#endif
#ifndef _ATTR_FLASH_DATA_
#define _ATTR_FLASH_DATA_
#endif
#ifndef _ATTR_FLASH_INIT_BSS_
#define _ATTR_FLASH_INIT_BSS_
#endif
#ifndef _ATTR_FLASH_INIT_CODE_
#define _ATTR_FLASH_INIT_CODE_
#endif
#ifndef _ATTR_FLASH_INIT_DATA_
#define _ATTR_FLASH_INIT_DATA_
#endif
#ifndef _ATTR_FLASH_WRITE_BSS_
#define _ATTR_FLASH_WRITE_BSS_
#endif
#ifndef _ATTR_FLASH_WRITE_CODE_
#define _ATTR_FLASH_WRITE_CODE_
#endif
#ifndef _ATTR_FLASH_WRITE_DATA_
#define _ATTR_FLASH_WRITE_DATA_
#endif
#ifndef _ATTR_FMCONTROL_BSS_
#define _ATTR_FMCONTROL_BSS_
#endif
#ifndef _ATTR_FMCONTROL_DATA_
#define _ATTR_FMCONTROL_DATA_
#endif
#ifndef _ATTR_FMCONTROL_TEXT_
#define _ATTR_FMCONTROL_TEXT_
#endif
#ifndef _ATTR_FMDRIVERL_FM5807_DATA_
#define _ATTR_FMDRIVERL_FM5807_DATA_
#endif
#ifndef _ATTR_FMDRIVERL_QN8035_DATA_
#define _ATTR_FMDRIVERL_QN8035_DATA_
#endif
#ifndef _ATTR_FMDRIVER_FM5767_BSS_
#define _ATTR_FMDRIVER_FM5767_BSS_
#endif
#ifndef _ATTR_FMDRIVER_FM5767_TEXT_
#define _ATTR_FMDRIVER_FM5767_TEXT_
#endif
#ifndef _ATTR_FMDRIVER_FM5807_BSS_
#define _ATTR_FMDRIVER_FM5807_BSS_
#endif
#ifndef _ATTR_FMDRIVER_FM5807_TEXT_
#define _ATTR_FMDRIVER_FM5807_TEXT_
#endif
#ifndef _ATTR_FMDRIVER_QN8035_BSS_
#define _ATTR_FMDRIVER_QN8035_BSS_
#endif
#ifndef _ATTR_FMDRIVER_QN8035_TEXT_
#define _ATTR_FMDRIVER_QN8035_TEXT_
#endif
#ifndef _ATTR_FS_GET_MEM_BSS_
#define _ATTR_FS_GET_MEM_BSS_
#endif
#ifndef _ATTR_FS_GET_MEM_CODE_
#define _ATTR_FS_GET_MEM_CODE_
#endif
#ifndef _ATTR_FS_GET_MEM_DATA_
#define _ATTR_FS_GET_MEM_DATA_
#endif
#ifndef _ATTR_FTL_BSS_
#define _ATTR_FTL_BSS_
#endif
#ifndef _ATTR_FTL_CODE_
#define _ATTR_FTL_CODE_
#endif
#ifndef _ATTR_FTL_DATA_
#define _ATTR_FTL_DATA_
#endif
#ifndef _ATTR_FTL_INIT_BSS_
#define _ATTR_FTL_INIT_BSS_
#endif
#ifndef _ATTR_FTL_INIT_CODE_
#define _ATTR_FTL_INIT_CODE_
#endif
#ifndef _ATTR_FTL_INIT_DATA_
#define _ATTR_FTL_INIT_DATA_
#endif
#ifndef _ATTR_FW_UPGRADE_BSS_
#define _ATTR_FW_UPGRADE_BSS_
#endif
#ifndef _ATTR_FW_UPGRADE_CODE_
#define _ATTR_FW_UPGRADE_CODE_
#endif
#ifndef _ATTR_FW_UPGRADE_DATA_
#define _ATTR_FW_UPGRADE_DATA_
#endif
#ifndef _ATTR_HIFI_ALACDEC_BIN_BSS_
#define _ATTR_HIFI_ALACDEC_BIN_BSS_
#endif
#ifndef _ATTR_HIFI_ALACDEC_BSS_
#define _ATTR_HIFI_ALACDEC_BSS_
#endif
#ifndef _ATTR_HIFI_ALACDEC_DATA_
#define _ATTR_HIFI_ALACDEC_DATA_
#endif
#ifndef _ATTR_HIFI_ALACDEC_TEXT_
#define _ATTR_HIFI_ALACDEC_TEXT_
#endif
#ifndef _ATTR_HIFI_APEDEC_BIN_BSS_
#define _ATTR_HIFI_APEDEC_BIN_BSS_
#endif
#ifndef _ATTR_HIFI_APEDEC_BSS_
#define _ATTR_HIFI_APEDEC_BSS_
#endif
#ifndef _ATTR_HIFI_APEDEC_DATA_
#define _ATTR_HIFI_APEDEC_DATA_
#endif
#ifndef _ATTR_HIFI_APEDEC_TEXT_
#define _ATTR_HIFI_APEDEC_TEXT_
#endif
#ifndef _ATTR_HIFI_FLACDEC_BIN_BSS_
#define _ATTR_HIFI_FLACDEC_BIN_BSS_
#endif
#ifndef _ATTR_HIFI_FLACDEC_BSS_
#define _ATTR_HIFI_FLACDEC_BSS_
#endif
#ifndef _ATTR_HIFI_FLACDEC_DATA_
#define _ATTR_HIFI_FLACDEC_DATA_
#endif
#ifndef _ATTR_HIFI_FLACDEC_TEXT_
#define _ATTR_HIFI_FLACDEC_TEXT_
#endif
#ifndef _ATTR_HOLD_BSS_
#define _ATTR_HOLD_BSS_
#endif
#ifndef _ATTR_HOLD_CODE_
#define _ATTR_HOLD_CODE_
#endif
#ifndef _ATTR_HOLD_DATA_
#define _ATTR_HOLD_DATA_
#endif
#ifndef _ATTR_ID3JPG_BSS_
#define _ATTR_ID3JPG_BSS_
#endif
#ifndef _ATTR_ID3JPG_DATA_
#define _ATTR_ID3JPG_DATA_
#endif
#ifndef _ATTR_ID3JPG_TEXT_
#define _ATTR_ID3JPG_TEXT_
#endif
#ifndef _ATTR_ID3_BSS_
#define _ATTR_ID3_BSS_
#endif
#ifndef _ATTR_ID3_DATA_
#define _ATTR_ID3_DATA_
#endif
#ifndef _ATTR_ID3_TEXT_
#define _ATTR_ID3_TEXT_
#endif
#ifndef _ATTR_IMAGE_BSS_
#define _ATTR_IMAGE_BSS_
#endif
#ifndef _ATTR_IMAGE_DATA_
#define _ATTR_IMAGE_DATA_
#endif
#ifndef _ATTR_IMAGE_TEXT_
#define _ATTR_IMAGE_TEXT_
#endif
#ifndef _ATTR_INTRRUPT_BSS_
#define _ATTR_INTRRUPT_BSS_
#endif
#ifndef _ATTR_LCDDRIVER_JWM12864_BSS_
#define _ATTR_LCDDRIVER_JWM12864_BSS_
#endif
#ifndef _ATTR_LCDDRIVER_JWM12864_CODE_
#define _ATTR_LCDDRIVER_JWM12864_CODE_
#endif
#ifndef _ATTR_LCDDRIVER_JWM12864_DATA_
#define _ATTR_LCDDRIVER_JWM12864_DATA_
#endif
#ifndef _ATTR_LCDDRIVER_LD7032_BSS_
#define _ATTR_LCDDRIVER_LD7032_BSS_
#endif
#ifndef _ATTR_LCDDRIVER_LD7032_CODE_
#define _ATTR_LCDDRIVER_LD7032_CODE_
#endif
#ifndef _ATTR_LCDDRIVER_LD7032_DATA_
#define _ATTR_LCDDRIVER_LD7032_DATA_
#endif
#ifndef _ATTR_LCDDRIVER_ST7735S_BSS_
#define _ATTR_LCDDRIVER_ST7735S_BSS_
#endif
#ifndef _ATTR_LCDDRIVER_ST7735S_CODE_
#define _ATTR_LCDDRIVER_ST7735S_CODE_
#endif
#ifndef _ATTR_LCDDRIVER_ST7735S_DATA_
#define _ATTR_LCDDRIVER_ST7735S_DATA_
#endif
#ifndef _ATTR_LCDDRIVER_ST7735_BSS_
#define _ATTR_LCDDRIVER_ST7735_BSS_
#endif
#ifndef _ATTR_LCDDRIVER_ST7735_CODE_
#define _ATTR_LCDDRIVER_ST7735_CODE_
#endif
#ifndef _ATTR_LCDDRIVER_ST7735_DATA_
#define _ATTR_LCDDRIVER_ST7735_DATA_
#endif
#ifndef _ATTR_LCDDRIVER_UC1604C_BSS_
#define _ATTR_LCDDRIVER_UC1604C_BSS_
#endif
#ifndef _ATTR_LCDDRIVER_UC1604C_CODE_
#define _ATTR_LCDDRIVER_UC1604C_CODE_
#endif
#ifndef _ATTR_LCDDRIVER_UC1604C_DATA_
#define _ATTR_LCDDRIVER_UC1604C_DATA_
#endif
#ifndef _ATTR_LCD_BSS_
#define _ATTR_LCD_BSS_
#endif
#ifndef _ATTR_LCD_BUF_
#define _ATTR_LCD_BUF_
#endif
#ifndef _ATTR_LCD_CODE_
#define _ATTR_LCD_CODE_
#endif
#ifndef _ATTR_LCD_DATA_
#define _ATTR_LCD_DATA_
#endif
#ifndef _ATTR_LOWERPOWER_BSS_
#define _ATTR_LOWERPOWER_BSS_
#endif
#ifndef _ATTR_LOWERPOWER_CODE_
#define _ATTR_LOWERPOWER_CODE_
#endif
#ifndef _ATTR_LOWERPOWER_DATA_
#define _ATTR_LOWERPOWER_DATA_
#endif
#ifndef _ATTR_LWBT_BSS_
#define _ATTR_LWBT_BSS_
#endif
#ifndef _ATTR_LWBT_CODE_
#define _ATTR_LWBT_CODE_
#endif
#ifndef _ATTR_LWBT_DATA_
#define _ATTR_LWBT_DATA_
#endif
#ifndef _ATTR_LWBT_INIT_BSS_
#define _ATTR_LWBT_INIT_BSS_
#endif
#ifndef _ATTR_LWBT_INIT_CODE_
#define _ATTR_LWBT_INIT_CODE_
#endif
#ifndef _ATTR_LWBT_INIT_DATA_
#define _ATTR_LWBT_INIT_DATA_
#endif
#ifndef _ATTR_LWBT_INIT_SCRIPT_BSS_
#define _ATTR_LWBT_INIT_SCRIPT_BSS_
#endif
#ifndef _ATTR_LWBT_INIT_SCRIPT_CODE_
#define _ATTR_LWBT_INIT_SCRIPT_CODE_
#endif
#ifndef _ATTR_LWBT_INIT_SCRIPT_DATA_
#define _ATTR_LWBT_INIT_SCRIPT_DATA_
#endif
#ifndef _ATTR_LWBT_UARTIF_BSS_
#define _ATTR_LWBT_UARTIF_BSS_
#endif
#ifndef _ATTR_LWBT_UARTIF_CODE_
#define _ATTR_LWBT_UARTIF_CODE_
#endif
#ifndef _ATTR_LWBT_UARTIF_DATA_
#define _ATTR_LWBT_UARTIF_DATA_
#endif
#ifndef _ATTR_MAIN_MENU_BSS_
#define _ATTR_MAIN_MENU_BSS_
#endif
#ifndef _ATTR_MAIN_MENU_CODE_
#define _ATTR_MAIN_MENU_CODE_
#endif
#ifndef _ATTR_MAIN_MENU_DATA_
#define _ATTR_MAIN_MENU_DATA_
#endif
#ifndef _ATTR_MAIN_MENU_DEINIT_BSS_
#define _ATTR_MAIN_MENU_DEINIT_BSS_
#endif
#ifndef _ATTR_MAIN_MENU_DEINIT_CODE_
#define _ATTR_MAIN_MENU_DEINIT_CODE_
#endif
#ifndef _ATTR_MAIN_MENU_DEINIT_DATA_
#define _ATTR_MAIN_MENU_DEINIT_DATA_
#endif
#ifndef _ATTR_MAIN_MENU_INIT_BSS_
#define _ATTR_MAIN_MENU_INIT_BSS_
#endif
#ifndef _ATTR_MAIN_MENU_INIT_CODE_
#define _ATTR_MAIN_MENU_INIT_CODE_
#endif
#ifndef _ATTR_MAIN_MENU_INIT_DATA_
#define _ATTR_MAIN_MENU_INIT_DATA_
#endif
#ifndef _ATTR_MAIN_MENU_SERVICE_BSS_
#define _ATTR_MAIN_MENU_SERVICE_BSS_
#endif
#ifndef _ATTR_MAIN_MENU_SERVICE_CODE_
#define _ATTR_MAIN_MENU_SERVICE_CODE_
#endif
#ifndef _ATTR_MAIN_MENU_SERVICE_DATA_
#define _ATTR_MAIN_MENU_SERVICE_DATA_
#endif
#ifndef _ATTR_MDBBUILDWIN_BSS_
#define _ATTR_MDBBUILDWIN_BSS_
#endif
#ifndef _ATTR_MDBBUILDWIN_CODE_
#define _ATTR_MDBBUILDWIN_CODE_
#endif
#ifndef _ATTR_MDBBUILDWIN_DATA_
#define _ATTR_MDBBUILDWIN_DATA_
#endif
#ifndef _ATTR_MEDIABROSUBWIN_BSS_
#define _ATTR_MEDIABROSUBWIN_BSS_
#endif
#ifndef _ATTR_MEDIABROSUBWIN_CODE_
#define _ATTR_MEDIABROSUBWIN_CODE_
#endif
#ifndef _ATTR_MEDIABROSUBWIN_DATA_
#define _ATTR_MEDIABROSUBWIN_DATA_
#endif
#ifndef _ATTR_MEDIABROWIN_BSS_
#define _ATTR_MEDIABROWIN_BSS_
#endif
#ifndef _ATTR_MEDIABROWIN_CODE_
#define _ATTR_MEDIABROWIN_CODE_
#endif
#ifndef _ATTR_MEDIABROWIN_DATA_
#define _ATTR_MEDIABROWIN_DATA_
#endif
#ifndef _ATTR_MEDIABROWIN_DEINIT_BSS_
#define _ATTR_MEDIABROWIN_DEINIT_BSS_
#endif
#ifndef _ATTR_MEDIABROWIN_DEINIT_CODE_
#define _ATTR_MEDIABROWIN_DEINIT_CODE_
#endif
#ifndef _ATTR_MEDIABROWIN_DEINIT_DATA_
#define _ATTR_MEDIABROWIN_DEINIT_DATA_
#endif
#ifndef _ATTR_MEDIABROWIN_INIT_BSS_
#define _ATTR_MEDIABROWIN_INIT_BSS_
#endif
#ifndef _ATTR_MEDIABROWIN_INIT_CODE_
#define _ATTR_MEDIABROWIN_INIT_CODE_
#endif
#ifndef _ATTR_MEDIABROWIN_INIT_DATA_
#define _ATTR_MEDIABROWIN_INIT_DATA_
#endif
#ifndef _ATTR_MEDIABROWIN_SERVICE_BSS_
#define _ATTR_MEDIABROWIN_SERVICE_BSS_
#endif
#ifndef _ATTR_MEDIABROWIN_SERVICE_CODE_
#define _ATTR_MEDIABROWIN_SERVICE_CODE_
#endif
#ifndef _ATTR_MEDIABROWIN_SERVICE_DATA_
#define _ATTR_MEDIABROWIN_SERVICE_DATA_
#endif
#ifndef _ATTR_MEDIABRO_SORTGET_BSS_
#define _ATTR_MEDIABRO_SORTGET_BSS_
#endif
#ifndef _ATTR_MEDIABRO_SORTGET_CODE_
#define _ATTR_MEDIABRO_SORTGET_CODE_
#endif
#ifndef _ATTR_MEDIABRO_SORTGET_DATA_
#define _ATTR_MEDIABRO_SORTGET_DATA_
#endif
#ifndef _ATTR_MEDIAFAVOSUBWIN_BSS_
#define _ATTR_MEDIAFAVOSUBWIN_BSS_
#endif
#ifndef _ATTR_MEDIAFAVOSUBWIN_CODE_
#define _ATTR_MEDIAFAVOSUBWIN_CODE_
#endif
#ifndef _ATTR_MEDIAFAVOSUBWIN_DATA_
#define _ATTR_MEDIAFAVOSUBWIN_DATA_
#endif
#ifndef _ATTR_MEDIALIBWIN_BSS_
#define _ATTR_MEDIALIBWIN_BSS_
#endif
#ifndef _ATTR_MEDIALIBWIN_CODE_
#define _ATTR_MEDIALIBWIN_CODE_
#endif
#ifndef _ATTR_MEDIALIBWIN_DATA_
#define _ATTR_MEDIALIBWIN_DATA_
#endif
#ifndef _ATTR_MEDIALIBWIN_DEINIT_BSS_
#define _ATTR_MEDIALIBWIN_DEINIT_BSS_
#endif
#ifndef _ATTR_MEDIALIBWIN_DEINIT_CODE_
#define _ATTR_MEDIALIBWIN_DEINIT_CODE_
#endif
#ifndef _ATTR_MEDIALIBWIN_DEINIT_DATA_
#define _ATTR_MEDIALIBWIN_DEINIT_DATA_
#endif
#ifndef _ATTR_MEDIALIBWIN_INIT_BSS_
#define _ATTR_MEDIALIBWIN_INIT_BSS_
#endif
#ifndef _ATTR_MEDIALIBWIN_INIT_CODE_
#define _ATTR_MEDIALIBWIN_INIT_CODE_
#endif
#ifndef _ATTR_MEDIALIBWIN_INIT_DATA_
#define _ATTR_MEDIALIBWIN_INIT_DATA_
#endif
#ifndef _ATTR_MEDIALIBWIN_SERVICE_BSS_
#define _ATTR_MEDIALIBWIN_SERVICE_BSS_
#endif
#ifndef _ATTR_MEDIALIBWIN_SERVICE_CODE_
#define _ATTR_MEDIALIBWIN_SERVICE_CODE_
#endif
#ifndef _ATTR_MEDIALIBWIN_SERVICE_DATA_
#define _ATTR_MEDIALIBWIN_SERVICE_DATA_
#endif
#ifndef _ATTR_MESSAGEBOX_BSS_
#define _ATTR_MESSAGEBOX_BSS_
#endif
#ifndef _ATTR_MESSAGEBOX_CODE_
#define _ATTR_MESSAGEBOX_CODE_
#endif
#ifndef _ATTR_MESSAGEBOX_DATA_
#define _ATTR_MESSAGEBOX_DATA_
#endif
#ifndef _ATTR_MP2DEC_BSS_
#define _ATTR_MP2DEC_BSS_
#endif
#ifndef _ATTR_MP2DEC_DATA_
#define _ATTR_MP2DEC_DATA_
#endif
#ifndef _ATTR_MP2DEC_TEXT_
#define _ATTR_MP2DEC_TEXT_
#endif
#ifndef _ATTR_MP3DEC_BIN_BSS_
#define _ATTR_MP3DEC_BIN_BSS_
#endif
#ifndef _ATTR_MP3DEC_BIN_DATA_
#define _ATTR_MP3DEC_BIN_DATA_
#endif
#ifndef _ATTR_MP3DEC_BIN_TEXT_
#define _ATTR_MP3DEC_BIN_TEXT_
#endif
#ifndef _ATTR_MP3DEC_BSS_
#define _ATTR_MP3DEC_BSS_
#endif
#ifndef _ATTR_MP3DEC_DATA_
#define _ATTR_MP3DEC_DATA_
#endif
#ifndef _ATTR_MP3DEC_TEXT_
#define _ATTR_MP3DEC_TEXT_
#endif
#ifndef _ATTR_MP3ENC_BIN_BSS_
#define _ATTR_MP3ENC_BIN_BSS_
#endif
#ifndef _ATTR_MP3ENC_BIN_DATA_
#define _ATTR_MP3ENC_BIN_DATA_
#endif
#ifndef _ATTR_MP3ENC_BIN_TEXT_
#define _ATTR_MP3ENC_BIN_TEXT_
#endif
#ifndef _ATTR_MP3INIT_BSS_
#define _ATTR_MP3INIT_BSS_
#endif
#ifndef _ATTR_MP3INIT_DATA_
#define _ATTR_MP3INIT_DATA_
#endif
#ifndef _ATTR_MP3INIT_TEXT_
#define _ATTR_MP3INIT_TEXT_
#endif
#ifndef _ATTR_MSADPCM_BSS_
#define _ATTR_MSADPCM_BSS_
#endif
#ifndef _ATTR_MSADPCM_DATA_
#define _ATTR_MSADPCM_DATA_
#endif
#ifndef _ATTR_MSADPCM_TEXT_
#define _ATTR_MSADPCM_TEXT_
#endif
#ifndef _ATTR_MSEQ_BSS_
#define _ATTR_MSEQ_BSS_
#endif
#ifndef _ATTR_MSEQ_DATA_
#define _ATTR_MSEQ_DATA_
#endif
#ifndef _ATTR_MSEQ_TEXT_
#define _ATTR_MSEQ_TEXT_
#endif
#ifndef _ATTR_MUSIC_BSS_
#define _ATTR_MUSIC_BSS_
#endif
#ifndef _ATTR_MUSIC_CODE_
#define _ATTR_MUSIC_CODE_
#endif
#ifndef _ATTR_MUSIC_DATA_
#define _ATTR_MUSIC_DATA_
#endif
#ifndef _ATTR_MUSIC_DEINIT_BSS_
#define _ATTR_MUSIC_DEINIT_BSS_
#endif
#ifndef _ATTR_MUSIC_DEINIT_CODE_
#define _ATTR_MUSIC_DEINIT_CODE_
#endif
#ifndef _ATTR_MUSIC_DEINIT_DATA_
#define _ATTR_MUSIC_DEINIT_DATA_
#endif
#ifndef _ATTR_MUSIC_INIT_BSS_
#define _ATTR_MUSIC_INIT_BSS_
#endif
#ifndef _ATTR_MUSIC_INIT_CODE_
#define _ATTR_MUSIC_INIT_CODE_
#endif
#ifndef _ATTR_MUSIC_INIT_DATA_
#define _ATTR_MUSIC_INIT_DATA_
#endif
#ifndef _ATTR_MUSIC_LRCCOMMON_BSS_
#define _ATTR_MUSIC_LRCCOMMON_BSS_
#endif
#ifndef _ATTR_MUSIC_LRCCOMMON_CODE_
#define _ATTR_MUSIC_LRCCOMMON_CODE_
#endif
#ifndef _ATTR_MUSIC_LRCCOMMON_DATA_
#define _ATTR_MUSIC_LRCCOMMON_DATA_
#endif
#ifndef _ATTR_MUSIC_LRCPLAY_BSS_
#define _ATTR_MUSIC_LRCPLAY_BSS_
#endif
#ifndef _ATTR_MUSIC_LRCPLAY_CODE_
#define _ATTR_MUSIC_LRCPLAY_CODE_
#endif
#ifndef _ATTR_MUSIC_LRCPLAY_DATA_
#define _ATTR_MUSIC_LRCPLAY_DATA_
#endif
#ifndef _ATTR_MUSIC_LRC_INIT_BSS_
#define _ATTR_MUSIC_LRC_INIT_BSS_
#endif
#ifndef _ATTR_MUSIC_LRC_INIT_CODE_
#define _ATTR_MUSIC_LRC_INIT_CODE_
#endif
#ifndef _ATTR_MUSIC_LRC_INIT_DATA_
#define _ATTR_MUSIC_LRC_INIT_DATA_
#endif
#ifndef _ATTR_MUSIC_SERVICE_BSS_
#define _ATTR_MUSIC_SERVICE_BSS_
#endif
#ifndef _ATTR_MUSIC_SERVICE_CODE_
#define _ATTR_MUSIC_SERVICE_CODE_
#endif
#ifndef _ATTR_MUSIC_SERVICE_DATA_
#define _ATTR_MUSIC_SERVICE_DATA_
#endif
#ifndef _ATTR_OGGDEC_BIN_BSS_
#define _ATTR_OGGDEC_BIN_BSS_
#endif
#ifndef _ATTR_OGGDEC_BSS_
#define _ATTR_OGGDEC_BSS_
#endif
#ifndef _ATTR_OGGDEC_DATA_
#define _ATTR_OGGDEC_DATA_
#endif
#ifndef _ATTR_OGGDEC_TEXT_
#define _ATTR_OGGDEC_TEXT_
#endif
#ifndef _ATTR_OVERLAY_BSS_
#define _ATTR_OVERLAY_BSS_
#endif
#ifndef _ATTR_OVERLAY_DATA_
#define _ATTR_OVERLAY_DATA_
#endif
#ifndef _ATTR_OVERLAY_INIT_BSS_
#define _ATTR_OVERLAY_INIT_BSS_
#endif
#ifndef _ATTR_OVERLAY_INIT_CODE_
#define _ATTR_OVERLAY_INIT_CODE_
#endif
#ifndef _ATTR_OVERLAY_INIT_DATA_
#define _ATTR_OVERLAY_INIT_DATA_
#endif
#ifndef _ATTR_PIC_BSS_
#define _ATTR_PIC_BSS_
#endif
#ifndef _ATTR_PIC_CODE_
#define _ATTR_PIC_CODE_
#endif
#ifndef _ATTR_PIC_DATA_
#define _ATTR_PIC_DATA_
#endif
#ifndef _ATTR_RADIOSUBFREQWIN_BSS_
#define _ATTR_RADIOSUBFREQWIN_BSS_
#endif
#ifndef _ATTR_RADIOSUBFREQWIN_CODE_
#define _ATTR_RADIOSUBFREQWIN_CODE_
#endif
#ifndef _ATTR_RADIOSUBFREQWIN_DATA_
#define _ATTR_RADIOSUBFREQWIN_DATA_
#endif
#ifndef _ATTR_RADIOSUBFUNCWIN_BSS_
#define _ATTR_RADIOSUBFUNCWIN_BSS_
#endif
#ifndef _ATTR_RADIOSUBFUNCWIN_CODE_
#define _ATTR_RADIOSUBFUNCWIN_CODE_
#endif
#ifndef _ATTR_RADIOSUBFUNCWIN_DATA_
#define _ATTR_RADIOSUBFUNCWIN_DATA_
#endif
#ifndef _ATTR_RADIOWIN_BSS_
#define _ATTR_RADIOWIN_BSS_
#endif
#ifndef _ATTR_RADIOWIN_CODE_
#define _ATTR_RADIOWIN_CODE_
#endif
#ifndef _ATTR_RADIOWIN_DATA_
#define _ATTR_RADIOWIN_DATA_
#endif
#ifndef _ATTR_RADIOWIN_DEINIT_BSS_
#define _ATTR_RADIOWIN_DEINIT_BSS_
#endif
#ifndef _ATTR_RADIOWIN_DEINIT_CODE_
#define _ATTR_RADIOWIN_DEINIT_CODE_
#endif
#ifndef _ATTR_RADIOWIN_DEINIT_DATA_
#define _ATTR_RADIOWIN_DEINIT_DATA_
#endif
#ifndef _ATTR_RADIOWIN_INIT_BSS_
#define _ATTR_RADIOWIN_INIT_BSS_
#endif
#ifndef _ATTR_RADIOWIN_INIT_CODE_
#define _ATTR_RADIOWIN_INIT_CODE_
#endif
#ifndef _ATTR_RADIOWIN_INIT_DATA_
#define _ATTR_RADIOWIN_INIT_DATA_
#endif
#ifndef _ATTR_RADIOWIN_SERVICE_BSS_
#define _ATTR_RADIOWIN_SERVICE_BSS_
#endif
#ifndef _ATTR_RADIOWIN_SERVICE_CODE_
#define _ATTR_RADIOWIN_SERVICE_CODE_
#endif
#ifndef _ATTR_RADIOWIN_SERVICE_DATA_
#define _ATTR_RADIOWIN_SERVICE_DATA_
#endif
#ifndef _ATTR_RECORDWIN_BSS_
#define _ATTR_RECORDWIN_BSS_
#endif
#ifndef _ATTR_RECORDWIN_CODE_
#define _ATTR_RECORDWIN_CODE_
#endif
#ifndef _ATTR_RECORDWIN_DATA_
#define _ATTR_RECORDWIN_DATA_
#endif
#ifndef _ATTR_RECORDWIN_DEINIT_BSS_
#define _ATTR_RECORDWIN_DEINIT_BSS_
#endif
#ifndef _ATTR_RECORDWIN_DEINIT_CODE_
#define _ATTR_RECORDWIN_DEINIT_CODE_
#endif
#ifndef _ATTR_RECORDWIN_DEINIT_DATA_
#define _ATTR_RECORDWIN_DEINIT_DATA_
#endif
#ifndef _ATTR_RECORDWIN_INIT_BSS_
#define _ATTR_RECORDWIN_INIT_BSS_
#endif
#ifndef _ATTR_RECORDWIN_INIT_CODE_
#define _ATTR_RECORDWIN_INIT_CODE_
#endif
#ifndef _ATTR_RECORDWIN_INIT_DATA_
#define _ATTR_RECORDWIN_INIT_DATA_
#endif
#ifndef _ATTR_RECORDWIN_SERVICE_BSS_
#define _ATTR_RECORDWIN_SERVICE_BSS_
#endif
#ifndef _ATTR_RECORDWIN_SERVICE_CODE_
#define _ATTR_RECORDWIN_SERVICE_CODE_
#endif
#ifndef _ATTR_RECORDWIN_SERVICE_DATA_
#define _ATTR_RECORDWIN_SERVICE_DATA_
#endif
#ifndef _ATTR_RECORD_CONTROL_BSS_
#define _ATTR_RECORD_CONTROL_BSS_
#endif
#ifndef _ATTR_RECORD_CONTROL_CODE_
#define _ATTR_RECORD_CONTROL_CODE_
#endif
#ifndef _ATTR_RECORD_CONTROL_DATA_
#define _ATTR_RECORD_CONTROL_DATA_
#endif
#ifndef _ATTR_SBCDEC_BSS_
#define _ATTR_SBCDEC_BSS_
#endif
#ifndef _ATTR_SBCDEC_DATA_
#define _ATTR_SBCDEC_DATA_
#endif
#ifndef _ATTR_SBCDEC_TEXT_
#define _ATTR_SBCDEC_TEXT_
#endif
#ifndef _ATTR_SD_BSS_
#define _ATTR_SD_BSS_
#endif
#ifndef _ATTR_SD_CODE_
#define _ATTR_SD_CODE_
#endif
#ifndef _ATTR_SD_DATA_
#define _ATTR_SD_DATA_
#endif
#ifndef _ATTR_SD_INIT_BSS_
#define _ATTR_SD_INIT_BSS_
#endif
#ifndef _ATTR_SD_INIT_CODE_
#define _ATTR_SD_INIT_CODE_
#endif
#ifndef _ATTR_SD_INIT_DATA_
#define _ATTR_SD_INIT_DATA_
#endif
#ifndef _ATTR_SD_WRITE_BSS_
#define _ATTR_SD_WRITE_BSS_
#endif
#ifndef _ATTR_SD_WRITE_CODE_
#define _ATTR_SD_WRITE_CODE_
#endif
#ifndef _ATTR_SD_WRITE_DATA_
#define _ATTR_SD_WRITE_DATA_
#endif
#ifndef _ATTR_SPECTRUM_BSS_
#define _ATTR_SPECTRUM_BSS_
#endif
#ifndef _ATTR_SPECTRUM_DATA_
#define _ATTR_SPECTRUM_DATA_
#endif
#ifndef _ATTR_SPECTRUM_TEXT_
#define _ATTR_SPECTRUM_TEXT_
#endif
#ifndef _ATTR_SYSRESERVED_OP_BSS_
#define _ATTR_SYSRESERVED_OP_BSS_
#endif
#ifndef _ATTR_SYSRESERVED_OP_CODE_
#define _ATTR_SYSRESERVED_OP_CODE_
#endif
#ifndef _ATTR_SYSRESERVED_OP_DATA_
#define _ATTR_SYSRESERVED_OP_DATA_
#endif
#ifndef _ATTR_SYS_FINDFILE_BSS_
#define _ATTR_SYS_FINDFILE_BSS_
#endif
#ifndef _ATTR_SYS_FINDFILE_DATA_
#define _ATTR_SYS_FINDFILE_DATA_
#endif
#ifndef _ATTR_SYS_FINDFILE_TEXT_
#define _ATTR_SYS_FINDFILE_TEXT_
#endif
#ifndef _ATTR_SYS_INIT_BSS_
#define _ATTR_SYS_INIT_BSS_
#endif
#ifndef _ATTR_SYS_INIT_DATA_
#define _ATTR_SYS_INIT_DATA_
#endif
#ifndef _ATTR_SYS_REBOOT_BSS_
#define _ATTR_SYS_REBOOT_BSS_
#endif
#ifndef _ATTR_SYS_SET_BRIGHT_BSS_
#define _ATTR_SYS_SET_BRIGHT_BSS_
#endif
#ifndef _ATTR_SYS_SET_BRIGHT_CODE_
#define _ATTR_SYS_SET_BRIGHT_CODE_
#endif
#ifndef _ATTR_SYS_SET_BRIGHT_DATA_
#define _ATTR_SYS_SET_BRIGHT_DATA_
#endif
#ifndef _ATTR_SYS_SET_BSS_
#define _ATTR_SYS_SET_BSS_
#endif
#ifndef _ATTR_SYS_SET_BT_BSS_
#define _ATTR_SYS_SET_BT_BSS_
#endif
#ifndef _ATTR_SYS_SET_BT_CODE_
#define _ATTR_SYS_SET_BT_CODE_
#endif
#ifndef _ATTR_SYS_SET_BT_DATA_
#define _ATTR_SYS_SET_BT_DATA_
#endif
#ifndef _ATTR_SYS_SET_CODE_
#define _ATTR_SYS_SET_CODE_
#endif
#ifndef _ATTR_SYS_SET_COMMON_BSS_
#define _ATTR_SYS_SET_COMMON_BSS_
#endif
#ifndef _ATTR_SYS_SET_COMMON_CODE_
#define _ATTR_SYS_SET_COMMON_CODE_
#endif
#ifndef _ATTR_SYS_SET_COMMON_DATA_
#define _ATTR_SYS_SET_COMMON_DATA_
#endif
#ifndef _ATTR_SYS_SET_DATA_
#define _ATTR_SYS_SET_DATA_
#endif
#ifndef _ATTR_SYS_SET_DEINIT_BSS_
#define _ATTR_SYS_SET_DEINIT_BSS_
#endif
#ifndef _ATTR_SYS_SET_DEINIT_CODE_
#define _ATTR_SYS_SET_DEINIT_CODE_
#endif
#ifndef _ATTR_SYS_SET_DEINIT_DATA_
#define _ATTR_SYS_SET_DEINIT_DATA_
#endif
#ifndef _ATTR_SYS_SET_INIT_BSS_
#define _ATTR_SYS_SET_INIT_BSS_
#endif
#ifndef _ATTR_SYS_SET_INIT_CODE_
#define _ATTR_SYS_SET_INIT_CODE_
#endif
#ifndef _ATTR_SYS_SET_INIT_DATA_
#define _ATTR_SYS_SET_INIT_DATA_
#endif
#ifndef _ATTR_SYS_SET_MUSIC_BSS_
#define _ATTR_SYS_SET_MUSIC_BSS_
#endif
#ifndef _ATTR_SYS_SET_MUSIC_CODE_
#define _ATTR_SYS_SET_MUSIC_CODE_
#endif
#ifndef _ATTR_SYS_SET_MUSIC_DATA_
#define _ATTR_SYS_SET_MUSIC_DATA_
#endif
#ifndef _ATTR_SYS_SET_RADIO_BSS_
#define _ATTR_SYS_SET_RADIO_BSS_
#endif
#ifndef _ATTR_SYS_SET_RADIO_CODE_
#define _ATTR_SYS_SET_RADIO_CODE_
#endif
#ifndef _ATTR_SYS_SET_RADIO_DATA_
#define _ATTR_SYS_SET_RADIO_DATA_
#endif
#ifndef _ATTR_SYS_SET_RECORD_BSS_
#define _ATTR_SYS_SET_RECORD_BSS_
#endif
#ifndef _ATTR_SYS_SET_RECORD_CODE_
#define _ATTR_SYS_SET_RECORD_CODE_
#endif
#ifndef _ATTR_SYS_SET_RECORD_DATA_
#define _ATTR_SYS_SET_RECORD_DATA_
#endif
#ifndef _ATTR_SYS_SET_SERVICE_BSS_
#define _ATTR_SYS_SET_SERVICE_BSS_
#endif
#ifndef _ATTR_SYS_SET_SERVICE_CODE_
#define _ATTR_SYS_SET_SERVICE_CODE_
#endif
#ifndef _ATTR_SYS_SET_SERVICE_DATA_
#define _ATTR_SYS_SET_SERVICE_DATA_
#endif
#ifndef _ATTR_SYS_SET_SYSTEM_BSS_
#define _ATTR_SYS_SET_SYSTEM_BSS_
#endif
#ifndef _ATTR_SYS_SET_SYSTEM_CODE_
#define _ATTR_SYS_SET_SYSTEM_CODE_
#endif
#ifndef _ATTR_SYS_SET_SYSTEM_DATA_
#define _ATTR_SYS_SET_SYSTEM_DATA_
#endif
#ifndef _ATTR_SYS_SET_TEXT_BSS_
#define _ATTR_SYS_SET_TEXT_BSS_
#endif
#ifndef _ATTR_SYS_SET_TEXT_CODE_
#define _ATTR_SYS_SET_TEXT_CODE_
#endif
#ifndef _ATTR_SYS_SET_TEXT_DATA_
#define _ATTR_SYS_SET_TEXT_DATA_
#endif
#ifndef _ATTR_TEXT_BSS_
#define _ATTR_TEXT_BSS_
#endif
#ifndef _ATTR_TEXT_CODE_
#define _ATTR_TEXT_CODE_
#endif
#ifndef _ATTR_TEXT_DATA_
#define _ATTR_TEXT_DATA_
#endif
#ifndef _ATTR_TEXT_DEINIT_BSS_
#define _ATTR_TEXT_DEINIT_BSS_
#endif
#ifndef _ATTR_TEXT_DEINIT_CODE_
#define _ATTR_TEXT_DEINIT_CODE_
#endif
#ifndef _ATTR_TEXT_DEINIT_DATA_
#define _ATTR_TEXT_DEINIT_DATA_
#endif
#ifndef _ATTR_TEXT_INIT_CODE_
#define _ATTR_TEXT_INIT_CODE_
#endif
#ifndef _ATTR_TEXT_INIT_DATA_
#define _ATTR_TEXT_INIT_DATA_
#endif
#ifndef _ATTR_TEXT_INIT_TBSS_
#define _ATTR_TEXT_INIT_TBSS_
#endif
#ifndef _ATTR_TEXT_SERVICE_BSS_
#define _ATTR_TEXT_SERVICE_BSS_
#endif
#ifndef _ATTR_TEXT_SERVICE_CODE_
#define _ATTR_TEXT_SERVICE_CODE_
#endif
#ifndef _ATTR_TEXT_SERVICE_DATA_
#define _ATTR_TEXT_SERVICE_DATA_
#endif
#ifndef _ATTR_USBCONTROL_BSS_
#define _ATTR_USBCONTROL_BSS_
#endif
#ifndef _ATTR_USBCONTROL_CODE_
#define _ATTR_USBCONTROL_CODE_
#endif
#ifndef _ATTR_USBCONTROL_DATA_
#define _ATTR_USBCONTROL_DATA_
#endif
#ifndef _ATTR_USB_AUDIO_BSS_
#define _ATTR_USB_AUDIO_BSS_
#endif
#ifndef _ATTR_USB_AUDIO_CODE_
#define _ATTR_USB_AUDIO_CODE_
#endif
#ifndef _ATTR_USB_AUDIO_DATA_
#define _ATTR_USB_AUDIO_DATA_
#endif
#ifndef _ATTR_USB_DRIVER_BSS_
#define _ATTR_USB_DRIVER_BSS_
#endif
#ifndef _ATTR_USB_DRIVER_CODE_
#define _ATTR_USB_DRIVER_CODE_
#endif
#ifndef _ATTR_USB_DRIVER_DATA_
#define _ATTR_USB_DRIVER_DATA_
#endif
#ifndef _ATTR_USB_MSC_BSS_
#define _ATTR_USB_MSC_BSS_
#endif
#ifndef _ATTR_USB_MSC_CODE_
#define _ATTR_USB_MSC_CODE_
#endif
#ifndef _ATTR_USB_MSC_DATA_
#define _ATTR_USB_MSC_DATA_
#endif
#ifndef _ATTR_USB_SRL_BSS_
#define _ATTR_USB_SRL_BSS_
#endif
#ifndef _ATTR_USB_SRL_CODE_
#define _ATTR_USB_SRL_CODE_
#endif
#ifndef _ATTR_USB_SRL_DATA_
#define _ATTR_USB_SRL_DATA_
#endif
#ifndef _ATTR_USB_UI_BSS_
#define _ATTR_USB_UI_BSS_
#endif
#ifndef _ATTR_USB_UI_CODE_
#define _ATTR_USB_UI_CODE_
#endif
#ifndef _ATTR_USB_UI_DATA_
#define _ATTR_USB_UI_DATA_
#endif
#ifndef _ATTR_VIDEOWIN_BSS_
#define _ATTR_VIDEOWIN_BSS_
#endif
#ifndef _ATTR_VIDEOWIN_CODE_
#define _ATTR_VIDEOWIN_CODE_
#endif
#ifndef _ATTR_VIDEOWIN_DATA_
#define _ATTR_VIDEOWIN_DATA_
#endif
#ifndef _ATTR_WAVDEC_BIN_BSS_
#define _ATTR_WAVDEC_BIN_BSS_
#endif
#ifndef _ATTR_WAVDEC_BSS_
#define _ATTR_WAVDEC_BSS_
#endif
#ifndef _ATTR_WAVDEC_DATA_
#define _ATTR_WAVDEC_DATA_
#endif
#ifndef _ATTR_WAVDEC_INIT_BSS_
#define _ATTR_WAVDEC_INIT_BSS_
#endif
#ifndef _ATTR_WAVDEC_INIT_DATA_
#define _ATTR_WAVDEC_INIT_DATA_
#endif
#ifndef _ATTR_WAVDEC_INIT_TEXT_
#define _ATTR_WAVDEC_INIT_TEXT_
#endif
#ifndef _ATTR_WAVDEC_TEXT_
#define _ATTR_WAVDEC_TEXT_
#endif

/* ---- Generated placement macros before EXT (2026-10-09) ----
 * Same rationale as the _ATTR_* catch-all above: Keil scatter placement
 * names that GCC cannot express at these positions -> empty. */
#ifndef DRAM_FAT
#define DRAM_FAT
#endif
