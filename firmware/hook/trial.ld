/* Trial vehicle (trial_entry.c): code in the reference fw1's zero run,
 * .data/.bss inside the reference's (still unused) .bss. */
ENTRY(ref_hook)
MEMORY
{
    CODE (rx)  : ORIGIN = 0x03064300, LENGTH = 0x03074B70 - 0x03064300
    RAM  (rwx) : ORIGIN = 0x03005B00, LENGTH = 0x03039128 - 0x03005B00
}
SECTIONS
{
    .text : {
        KEEP(*(.text.entry))
        *(.text*) *(.rodata*)
        . = ALIGN(4);
    } > CODE
    .data : { _data_start = .; *(.data*) . = ALIGN(4); _data_end = .; } > RAM AT> CODE
    _data_load_start = LOADADDR(.data);
    .bss (NOLOAD) : { _bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); _bss_end = .; } > RAM
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) *(.note*) *(.vectors) }
}
ASSERT(_bss_end <= 0x03039128, "trial .bss overflows the reference .bss")
