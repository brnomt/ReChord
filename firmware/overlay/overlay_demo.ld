/* overlay_demo.ld -- link the demo overlay at the free window base. */
ENTRY(overlay_demo)
SECTIONS
{
    . = 0x0304AE64;
    .text : { *(.text*) *(.rodata*) }
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes*) *(.comment) *(.note*) }
}
