/*
 * SysConfig.h — System build configuration for the Echo Mini (v3.7.0).
 *
 * Derives from Ghidra analysis of section_3 (which modules are compiled in)
 * and the stock feature set (changelogs + symbol-index.md):
 *   - MUSIC:      yes (core)
 *   - RADIO:      yes (FM radio — FMControlTask, FMUITask, FmDev_Shell)
 *   - RECORD:     yes (RecordWinSvc, RecordServiceGetTotalTime)
 *   - PICTURE:    yes (ID3_Picture_Parser, GUI_BmpFronDisplay)
 *   - BLUETOOTH:  yes (BT_Music_Handler, RKBT_FRead)
 *   - USB_HOST:   yes (MscSendCSW, USBMSCHost_Read)
 *   - VIDEO/EBOOK/CDROM: NO (not present in the binary)
 *
 * Confirmed via Ghidra function inventory (Aug 2026).
 */
#ifndef SYSCONFIG_H
#define SYSCONFIG_H

#include "typedef.h"

/* ---- Module selection (matches the stock firmware build) ---- */
#define _MUSIC_         1
#define _RADIO_         1
#define _RECORD_        1
#define _PICTURE_       1
#define _BLUETOOTH_     1
#ifdef RECHORD_AP_BUILD
#define _A2DP_SOUCRE_   1
#define SSRC            1
#define _FRAME_BUFFER_  1
#define BT_SBC_PROCESS_INT_ID   INT_ID_UART2
#endif
#define _USB_HOST_      1
/* #define _VIDEO_         1   // not in Echo Mini */

/* ---- Bluetooth chipset / board config (from SDK_160_128/SysConfig.h) — AP only ---- */
#ifdef RECHORD_AP_BUILD
#define BT_CHIP_CC2564              0
#define BT_CHIP_CC2564B             1
#define BT_CHIP_RTL8761             2
#define BT_CHIP_CONFIG              BT_CHIP_RTL8761
#define BT_UART_INTERFACE_H4        1
#define BT_UART_INTERFACE_H5        2
#define BT_UART_INTERFACE_CONFIG    BT_UART_INTERFACE_H5
#define _AVRCP_                     1
#define _SBC_ENCODE_                1
#define BT_VCC_ON_GPIO_CH           GPIO_CH2
#define BT_VCC_ON_GPIO_PIN          GPIOPortA_Pin2
#define BT_POWER_GPIO_CH            GPIO_CH0
#define BT_POWER_GPIO_PIN           GPIOPortB_Pin5
#define BT_HOST_RX_CH               GPIO_CH2
#define BT_HOST_RX_PIN              GPIOPortC_Pin1
#define BT_HOST_TX_CH               GPIO_CH2
#define BT_HOST_TX_PIN              GPIOPortC_Pin0
#define BT_HOST_CTS_CH              GPIO_CH2
#define BT_HOST_CTS_PIN             GPIOPortB_Pin7
#define BT_HOST_RTS_CH              GPIO_CH2
#define BT_HOST_RTS_PIN             GPIOPortB_Pin6
#define BT_UART_CH                  UART_CH1_PA
#define BT_UART_INT_ID              INT_ID_UART1
#define BT_GPIO_INT_ID              INT_ID_GPIO0
#define BT_HCI_SERVER_INT_ID        INT_ID_UART5
#define BT_H5_TX_INT_ID             INT_ID_UART3

/* Image decoder selection (Keil A_CORE project defines) */
#define JPG_DEC_INCLUDE     1
#define BMP_DEC_INCLUDE     1
#define THUMB_DEC_INCLUDE   1
#endif
/* #define _EBOOK_         1   // not in Echo Mini */
/* #define _CDROM_         1   // not in Echo Mini */

/* ---- System language count ---- */
#define LANGUAGE_MAX_COUNT  1

/* ---- Chip / platform ---- */
#define _RKNANO_        1
#define _RKNANOC_       1
#define __CPU_NANOC__   1

/* ---- Memory config (from docs/memory-map.md + segment table) ---- */
#define SYS_DRAM_SIZE   0x04000000   /* 64 MB (8G variant uses same DRAM size) */
#define SYS_SRAM_BASE   0x03000000
#define SYS_SRAM_SIZE   0x00939000

/* ---- Storage / device config (from SDK_160_128/SysConfig.h) — AP only ---- */
#ifdef RECHORD_AP_BUILD
#define FW_IN_DEV       3   /* firmware stored in: 1=nand 2=sipnor 3=emmc 4=sd */
#define _EMMC_          1
#define DEBUG_UART_PORT 0
#define ENC_WAV_H_FS    FS_96KHz
#define ENC_WAV_N_FS    FS_44100Hz
#endif

/* ---- Display ---- */
#define LCD_WIDTH       128
#define LCD_HEIGHT      160
#define LCD_BPP         16
#define LCD_PIXEL_1     1
#define LCD_PIXEL_16    16
#define LCD_PIXEL       LCD_PIXEL_16
#define BUFFER_MAX_NUM      1
#define FRAME_SUB_BUFFER_NUM    1
#define LCD_HEIGHTA         LCD_HEIGHT

/* ---- Backlight PWM (from SDK_160_128/SysConfig.h) ---- */
#define BL_LEVEL_MAX                5
#define BL_PWM_RATE_MIN             30
#define BL_PWM_RATE_MAX             80
#define BL_PWM_RATE_STEP            ((BL_PWM_RATE_MAX - BL_PWM_RATE_MIN) / (BL_LEVEL_MAX))

/* ---- ADC key (AD-key) board config (from SDK_160_128/SysConfig.h) ---- */
#define BATTERY_LEVEL               5
#define BT_HCI_UART_ID              UART_CH1

#define KEY_NUM_4                   4
#define KEY_NUM_5                   5
#define KEY_NUM_6                   6
#define KEY_NUM_7                   7
#define KEY_NUM_8                   8
#define KEY_NUM                     KEY_NUM_7

#define KEY_VAL_NONE                ((UINT32)0x0000)
#define KEY_VAL_PLAY                ((UINT32)0x0001 << 0)
#define KEY_VAL_MENU                ((UINT32)0x0001 << 1)
#define KEY_VAL_FFD                 ((UINT32)0x0001 << 2)
#define KEY_VAL_FFW                 ((UINT32)0x0001 << 3)
#define KEY_VAL_UP                  ((UINT32)0x0001 << 4)
#define KEY_VAL_DOWN                ((UINT32)0x0001 << 5)
#define KEY_VAL_ESC                 ((UINT32)0x0001 << 6)
#define KEY_VAL_UNHOLD              ((UINT32)0x0001 << 8)

#define KEY_VAL_UPGRADE             KEY_VAL_MENU
#define KEY_VAL_POWER               KEY_VAL_PLAY
#define KEY_VAL_HOLD                (KEY_VAL_MENU | KEY_VAL_PLAY)
#define KEY_VAL_VOL                 KEY_VAL_ESC

#define KEY_VAL_MASK                ((UINT32)0x0fffffff)
#define KEY_VAL_UNMASK              ((UINT32)0xf0000000)

#define KEY_VAL_ADKEY2              KEY_VAL_MENU
#define KEY_VAL_ADKEY3              KEY_VAL_UP
#define KEY_VAL_ADKEY4              KEY_VAL_FFW
#define KEY_VAL_ADKEY5              KEY_VAL_FFD
#define KEY_VAL_ADKEY6              KEY_VAL_DOWN
#define KEY_VAL_ADKEY7              KEY_VAL_ESC

#define ADKEY2_MIN                  ((0   +   0) / 2)
#define ADKEY2_MAX                  ((0   + 147) / 2)
#define ADKEY3_MIN                  ((147 +   0) / 2)
#define ADKEY3_MAX                  ((147 + 330) / 2)
#define ADKEY4_MIN                  ((330 + 147) / 2)
#define ADKEY4_MAX                  ((330 + 522) / 2)
#define ADKEY5_MIN                  ((522 + 330) / 2)
#define ADKEY5_MAX                  ((522 + 780) / 2)
#define ADKEY6_MIN                  ((780 + 522) / 2)
#define ADKEY6_MAX                  ((780 + 956) / 2)
#define ADKEY7_MIN                  ((956 + 780) / 2)
#define ADKEY7_MAX                  ((956 + 1024) / 2)

/* ---- Language IDs (from SDK_160_128/SysConfig.h) ---- */
#define LANGUAGE_CHINESE_S          0
#define LANGUAGE_CHINESE_T          1
#define LANGUAGE_ENGLISH            2
#define LANGUAGE_KOREAN             3
#define LANGUAGE_JAPANESE           4
#define LANGUAGE_SPAISH             9
#define LANGUAGE_FRENCH             5
#define LANGUAGE_GERMAN             6
#define LANGUAGE_ITALIAN            10
#define LANGUAGE_PORTUGUESE         7
#define LANGUAGE_RUSSIAN            8
#define LANGUAGE_SWEDISH            11
#define LANGUAGE_THAI               12
#define LANGUAGE_POLAND             13
#define LANGUAGE_DENISH             14
#define LANGUAGE_DUTCH              15
#define LANGUAGE_HELLENIC           16
#define LANGUAGE_CZECHIC            17
#define LANGUAGE_TURKIC             18
#define LANGUAGE_RABBINIC           19
#define LANGUAGE_ARABIC             20
#define DEFAULT_LANGUE              LANGUAGE_CHINESE_S

/* ---- Firmware identity (matches stock IMG strings) ---- */
#define FIRMWARE_NAME   "ECHO MINI"
#define FIRMWARE_MAJOR  3
#define FIRMWARE_MINOR  7
#define FIRMWARE_PATCH  0

#endif /* SYSCONFIG_H */

/* ---- Codec type constants (Main2.c) ---- */
#ifndef SYSCONFIG_CODEC
#define SYSCONFIG_CODEC
#define CODEC_MP3_DEC   1
#define CODEC_WMA_DEC   2
#define CODEC_AAC_DEC   3
#define CODEC_FLAC_DEC  4
#define CODEC_APE_DEC   5
#define CODEC_OGG_DEC   6
#define CODEC_WAV_DEC   7
#define CODEC_DSD_DEC   8
#endif

/* ---- Storage media IDs (filesys / ui) ---- */
#define FLASH0          0
#define FLASH1          1
#define CARD            1
#define TOTAL_LANAUAGE_NUM  1

/* AP: SysDiskID is #define'd by MemDev.h (FW_IN_DEV); BB keeps the extern. */
#ifndef RECHORD_AP_BUILD
extern uint32 SysDiskID;
#endif
extern uint32 SysProgRawDiskCapacity;

/* The MODULE_ID enum, CODE_INFO_T / FIRMWARE_INFO_T / SYSTEM_DEFAULT_PARA_T,
 * FM/LCD_DRIVER_*_T now come from the real SDK's ModuleInfoTab.h. */

#define EVK_LANGUAGE_MAX_COUNT  1

/* ---- Memory device info: lo define el SDK (MemDev.h / ModuleInfoTab.h) ---- */
#ifndef SYSCONFIG_MEMDEV
#define SYSCONFIG_MEMDEV
#include "MemDev.h"
#endif
