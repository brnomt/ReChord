/*
 * ReChord linker script — Echo Mini (RKnanoC) custom firmware.
 *
 * Layout follows the stock section_3 + segment table (docs/HARDWARE.md):
 *   - Code + rodata in the firmware window at 0x03000000
 *   - The 16-byte RKnanoFW header at 0x03000000 (magic filled by pack_img)
 *   - firmware_entry (reset vector) at 0x03000010
 *   - Data/bss in on-chip SRAM (0x03005AFC .. 0x03060000)
 *   - SDK named sections (bb_vect, BBSysData, APMainStack, ...) placed per
 *     the stock segment table so runtime addresses match.
 */

ENTRY(firmware_entry)

MEMORY
{
    /* Code window (section_3 region, SPI-loaded) */
    FW_CODE (rx)  : ORIGIN = 0x03000000, LENGTH = 0x00939000
    /* On-chip SRAM for data/stack */
    FW_RAM  (rwx) : ORIGIN = 0x03005AFC, LENGTH = 0x0005A504
}

/* Main stack top — 16 KiB block @ 0x0301E794 (segment table) */
__stack_top = 0x0301E794 + 0x4000;
__main_stack = 0x0301E794;

/* Keil scatter-file symbols the SDK references */
Image$$BB_MAIN_STACK$$ZI$$Limit = __main_stack + 0x4000;
Image$$BB_SYS_DATA$$ZI$$Base = ORIGIN(FW_RAM);
Image$$BB_SYS_DATA$$ZI$$Length = LENGTH(FW_RAM);

SECTIONS
{
    /* ---- RKnanoFW 16-byte header (magic/load-addr written by pack_img) ---- */
    . = ORIGIN(FW_CODE);
    .fw_header 0x03000000 :
    {
        KEEP(*(.fw_header))
    } > FW_CODE

    /* ---- firmware_entry + ROM-dispatch trampolines @ 0x03000010 ---- */
    .text 0x03000010 :
    {
        KEEP(*(.text.boot))
        *(.text*)
        *(.rodata*)
        *(.ARM.extab*)
        *(.ARM.exidx*)
        . = ALIGN(4);
        *(.driver_code)
        . = ALIGN(4);
        *(.SysInitCode)
        . = ALIGN(4);
        *(.SysCode)
        . = ALIGN(4);
        *(.SysReservedCode)
        . = ALIGN(4);
        *(.FindFileCode)
        . = ALIGN(4);
        *(.AudioInitCode)
        . = ALIGN(4);
        *(.AudioCode)
        . = ALIGN(4);
        *(.FlacDecCode)
    } > FW_CODE

    .vectors (NOLOAD) :
    {
        KEEP(*(.vectors))
    } > FW_CODE

    /* ---- Vector table (exceptions_table2). The SDK names this section
           "bb_vect" (no leading dot). It MUST be 256-byte aligned so VTOR's
           low 7 bits (forced to 0) still point at the real table. ---- */
    .bb_vect ALIGN(256) :
    {
        KEEP(*(bb_vect))
    } > FW_RAM

    /* ---- SDK named data sections ---- */
    .data ALIGN(4) :
    {
        _data_start = .;
        *(.data*)
        *(.BBSysData)
        *(.SysData)
        *(.SysReservedData)
        *(.AudioData)
        _data_end = .;
    } > FW_RAM

    /* ---- SDK named bss sections ---- */
    .bss ALIGN(4) :
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        *(.SysBss)
        *(.AudioBss)
        _bss_end = .;
    } > FW_RAM

    . = ORIGIN(FW_RAM);

    /* ---- Buffer regions from the segment table (NOLOAD, placed at their
           stock addresses so absolute pointers in code match) ---- */
    .audio_buffer 0x03005AFC (NOLOAD) : { . = 0x18C50; } > FW_RAM   /* 101,456 */
    .dsp_buffers  0x0301E74C (NOLOAD) : { . = 0x28; }  > FW_RAM
    .id3_parse    0x0301E770 (NOLOAD) : { . = 0x36; }  > FW_RAM
    .fat_cache    0x0301E778 (NOLOAD) : { . = 0x10000; } > FW_RAM  /* 64KB */
    .flac_work    0x03041550 (NOLOAD) : { . = 0x4; }  > FW_RAM
    .ogg_decode   0x0301E7D4 (NOLOAD) : { . = 0x242; } > FW_RAM
    .mp3_frame    0x0301EB30 (NOLOAD) : { . = 0x12810; } > FW_RAM  /* 75,792 */
    .wma_work     0x03031454 (NOLOAD) : { . = 0x100D4; } > FW_RAM  /* 65,748 */
    .ape_work     0x0301E76C (NOLOAD) : { . = 0x35; }  > FW_RAM
    .dsd_buffer   0x0301E7D0 (NOLOAD) : { . = 0x14; }  > FW_RAM
    .aac_decoder  0x0301EE5C (NOLOAD) : { . = 0x6CDF; } > FW_RAM  /* 27,871 */
    .wav_decode   0x03025F90 (NOLOAD) : { . = 0x93B4; } > FW_RAM  /* 37,812 */
    .dict_decoder 0x0301E770 (NOLOAD) : { . = 0x199; } > FW_RAM
    .media_index  0x030234B8 (NOLOAD) : { . = 0x12ED; } > FW_RAM
    .ui_fb        0x03024868 (NOLOAD) : { . = 0xFB20; } > FW_RAM  /* 64,288 */
    .bt_stream    0x030247BC (NOLOAD) : { . = 0x8444; } > FW_RAM  /* 33,860 */
    .usb_dma      0x0301E78C (NOLOAD) : { . = 0x8; }  > FW_RAM
    .record_buf   0x0301EABC (NOLOAD) : { . = 0x6; }  > FW_RAM
    .jpeg_decode  0x0301E784 (NOLOAD) : { . = 0x11D6; } > FW_RAM  /* 4,566 */

    /* ---- Heap / remaining SRAM ---- */
    .heap (NOLOAD) :
    {
        __heap_start = .;
        . = ORIGIN(FW_RAM) + LENGTH(FW_RAM) - .;
        __heap_end = .;
    } > FW_RAM
}
