/* cdc_overlay.ld -- CDC overlay at the free 0x0304AE64 window. */
ENTRY(cdc_overlay_main)
SECTIONS
{
    . = 0x0304AE64;
    .text : { *(.text*) *(.rodata*) }
    .data : { *(.data*) }
    .bss  : { *(.bss*) *(COMMON) }
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes*) *(.comment) *(.note*) }
}
