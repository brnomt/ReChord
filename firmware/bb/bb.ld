/*
 * bb.ld — Core B (CAL_M3) heartbeat image, BRNOMT-68 phase 1.
 *
 * The reference copies its B image to HIGHRAM0 (0x01000000) and releases
 * CAL_CORE_SRST; Core B vectors from HIGHRAM0 (REA: fw1 0x0305B93C). This
 * image is flat and tiny: vector table first, then code. Stack grows down
 * from just under the shared block at 0x01020000.
 */
ENTRY(bb_reset)

MEMORY
{
    BCODE (rx) : ORIGIN = 0x01000000, LENGTH = 0x00010000   /* < PCM ring */
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

    .data :
    {
        *(.data*)
        . = ALIGN(4);
    } > BCODE

    .bss (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
    } > BCODE
}

/* Top of HIGHRAM0 (the shared block starts at 0x01020000). */
__bb_stack_top = 0x01020000;
