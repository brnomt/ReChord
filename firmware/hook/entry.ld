/* ENTRY test (make entry-img): our standalone fw1 (ap_startup.c with
 * AP_CHAIN_REF) with its code in the reference's zero run and the SAME
 * RAM roles as firmware_ap.ld (stack below 0x03004000, data/bss above). */
ENTRY(ChainEntry)
MEMORY
{
    CODE (rx)  : ORIGIN = 0x03064300, LENGTH = 0x03074B70 - 0x03064300
    RAM  (rwx) : ORIGIN = 0x03004000, LENGTH = 0x03039128 - 0x03004000
}
SECTIONS
{
    .text : {
        KEEP(*ap_startup.o(.vectors))
        *(.text*) *(.rodata*)
        . = ALIGN(4);
    } > CODE
    .data : { _data_start = .; *(.data*) . = ALIGN(4); _data_end = .; } > RAM AT> CODE
    _data_load_start = LOADADDR(.data);
    .bss (NOLOAD) : { _bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); _bss_end = .; } > RAM
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) *(.note*) *(.vectors) }
}
ASSERT(ap_vectors == 0x03064300, "ap_vectors must head the blob (256-aligned)")
