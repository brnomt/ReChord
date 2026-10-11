/* Hook blob inside the reference fw1's zero run (RAM 0x03064280..0x03074B70).
 * Everything, .bss included, is emitted as bytes so the patch is one flat
 * blob and nothing lands outside the zero run. */
ENTRY(ref_hook)
MEMORY { HOOK (rwx) : ORIGIN = 0x03064300, LENGTH = 0x03074B70 - 0x03064300 }
SECTIONS
{
    .hook : {
        KEEP(*(.text.entry))
        *(.text*) *(.rodata*) *(.data*)
        *(.bss*) *(COMMON)
        . = ALIGN(4);
    } > HOOK
    /DISCARD/ : { *(.ARM.exidx*) *(.comment) }
}
