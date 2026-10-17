/*
 * bb.ld — Core B (CAL_M3) image layout, BRNOMT-68.
 *
 * The reference copies its B image to HIGHRAM0 (0x01000000) and releases
 * CAL_CORE_SRST; Core B vectors from HIGHRAM0 (REA: fw1 0x0305B93C), so the
 * code image MUST stay in HIGHRAM. Core 1 also has its own 256 KiB SYSRAM1 at
 * 0x03090000 (docs/TRM-notes.md: "SYSRAM1 (256 KB) @ 0x03090000 (Core 1)") —
 * that is where B's data (and, in phase 3, the FLAC pool) live, because
 * HIGHRAM alone cannot hold image + pool + the PCM ring.
 *
 *   BCODE  : 0x01000000 (HIGHRAM0) — vectors + code + rodata + .data LMA
 *   BB_RAM : 0x03090000 (SYSRAM1)  — .data VMA + .bss + stack
 *
 * The AP copies the flat blob to HIGHRAM0; bb_reset() then copies .data down
 * to SYSRAM1 and zeroes .bss before entering the renderer.
 */
ENTRY(bb_reset)

MEMORY
{
    BCODE  (rx) : ORIGIN = 0x01000000, LENGTH = 0x00008000   /* < file ring */
    BB_RAM (rw) : ORIGIN = 0x03090000, LENGTH = 0x00040000   /* SYSRAM1 */
}

SECTIONS
{
    .text ORIGIN(BCODE) :
    {
        KEEP(*(.vectors_bb))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > BCODE

    .data : ALIGN(4)
    {
        _bb_data_start = .;
        *(.data*)
        . = ALIGN(4);
        _bb_data_end = .;
    } > BB_RAM AT> BCODE
    _bb_data_load = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        _bb_bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bb_bss_end = .;
    } > BB_RAM
}

/* Stack grows down from the top of SYSRAM1 (4 KiB margin below the top). */
__bb_stack_top = ORIGIN(BB_RAM) + LENGTH(BB_RAM) - 0x1000;
