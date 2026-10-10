/*
 * ReChord — free firmware for the Fiio Echo Mini (RKnanoC)
 * Built from the Rockchip RKnanoD SDK source + our app layer.
 *
 * Architecture: RKnano ARM Cortex-M (Thumb-2), little-endian
 * Firmware base: ram:0x03000000
 * Hardware abstraction calls RKnano boot ROM at 0x02FExxxx-0x02FFxxxx
 *
 * Module Index:
 *   firmware/os/         - Event system, delays, OS primitives
 *   firmware/filesystem/  - HifiFile I/O layer, FAT wrapper
 *   firmware/dsp/         - GOODE DSP, EQ processing
 *   firmware/media/       - Media library, ID3/APEv2 tags
 *   firmware/usb/         - USB DAC mode, mass storage
 *   firmware/power/       - Battery, sleep, charging
 *   codecs/               - Audio codec implementations
 *     mp3/    - libMad MP3 decoder
 *     wma/    - WMA/ASF parser + decoder
 *     aac/    - AAC/M4A decoder + MOV file parser
 *     flac/   - HiFi FLAC decoder
 *     ape/    - Monkey's Audio decoder
 *     wav/    - WAV/RIFF parser
 *     ogg/    - Vorbis decoder + OGG container
 *     dsd/    - DSD/DSF/DFF/IFF reader
 *   apps/audio/          - MusicService, MusicInit, playback control
 *   apps/ui/             - Display, themes, menus, album art
 *   apps/recorder/       - RecordWinSvc
 *   apps/bluetooth/       - BT Music handler
 *   drivers/             - Hardware: LCD, keypad, codec IC
 *   resource/            - Bitmaps, fonts, ROCK26IMAGERES table
 */

#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <stdint.h>
#include <stdbool.h>

/* Address space */
#define RAM_BASE         0x03000000
#define ROM_BASE         0x02FE0000
#define RESOURCE_BASE    0x04F00000

/* Firmware segments (from section_1 segmentation table) */
#define SEG_MAIN_HEAP         0x03050000
#define SEG_AUDIOBUF          0x03005AFC
#define SEG_MUSIC_CTX         0x0301E7D0
#define SEG_DSP_IN            0x0301E750
#define SEG_DSP_OUT           0x0301E74C
#define SEG_CODEC_WORKBUF     0x0301E76C
#define SEG_ID3_BUF           0x0301E770
#define SEG_FAT_CACHE         0x0301E778
#define SEG_STACK_MAIN        0x0301E794
#define SEG_FLAC_BUF          0x03041550
#define SEG_OGG_DECBUF        0x0301E7D4
#define SEG_MP3_FRAME         0x0301EB30
#define SEG_WMA_WORKBUF       0x03031454
#define SEG_APE_BUF           0x0301E76C
#define SEG_DSD_BUF           0x0301E7D0
#define SEG_AAC_DECBUF        0x0301EE5C
#define SEG_WAV_DECBUF        0x03025F90
#define SEG_DICTDECODER_BUF  0x0301E770
#define SEG_MEDIALIB_INDEX    0x030234B8
#define SEG_UI_FRAMEBUF       0x03024868
#define SEG_BT_STREAM_BUF     0x030247BC
#define SEG_USB_DMA_BUF       0x0301E78C
#define SEG_RECORD_BUF        0x0301EABC
#define SEG_JPEG_DECODEBUF    0x0301E784

#endif /* FIRMWARE_H */
