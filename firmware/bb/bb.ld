/*
 * bb.ld — Core B (CAL_M3) image layout, BRNOMT-68.
 *
 * Memory map per the vendor B-core scatter file
 * (community/sdks/.../Scatter/B_Core.sct): Core 1 runs code from HIGHRAM0
 * (the AP copies the image to 0x01000000 and releases CAL_CORE_SRST), and its
 * DATA / STACK live in the region from 0x01020000 upward (HRAM_DATA_BASE =
 * 0x01020000, size 0x40000 = 256 KiB, up to HIFIACC at 0x01060000). Core B
 * already proves it can reach that region: it reads the control block at
 * 0x01020000 every poll in phase 2. (Pointing B's SP at SYSRAM1 0x03090000
 * made B fault; see docs/rewrite/core-b-phase3.md.)
 *
 *   0x01000000..0x01010000   BCODE   (64 KiB) vectors + code + rodata + .data LMA
 *   0x01010000..0x01020000   PCM ring (owned by B)
 *   0x01020000..0x01021000   control block (shared with the AP)
 *   0x01021000..0x01029000   A->B file ring (shared with the AP)
 *   0x01031000..0x01060000   BB_RAM  (188 KiB) .data + .bss + stack
 */
ENTRY(bb_reset)

MEMORY
{
    BCODE  (rx) : ORIGIN = 0x01000000, LENGTH = 0x00010000   /* < PCM ring */
    BB_RAM (rw) : ORIGIN = 0x01031000, LENGTH = 0x0002F000   /* < 0x01060000 */
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

/* Stack grows down from the top of the B data region (HIFIACC is at
 * 0x01060000). */
__bb_stack_top = ORIGIN(BB_RAM) + LENGTH(BB_RAM);
