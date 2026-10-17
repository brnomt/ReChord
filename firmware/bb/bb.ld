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
    BCODE (rx) : ORIGIN = 0x01000000, LENGTH = 0x00008000   /* < file ring */
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

/* Stack top just BELOW the 32 KiB file-service ring (0x01008000..0x01010000);
 * it grows down into the (tiny) image region, never into a ring. */
__bb_stack_top = 0x01008000;
