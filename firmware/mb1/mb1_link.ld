/* mb1_link.ld — ReChord M-B1 stub link map.
 *
 * .text/.rodata live in the code cave immediately before the Reset vector
 * (verified zero padding in the RefCFW RAM image, ramimg 0x13400..0x134DE).
 * .hook is the 4-byte Reset-entry replacement at 0x0306296E.
 * RAM image base is 0x0304F490; these are absolute runtime addresses.
 */
ENTRY(rechord_boot)
SECTIONS
{
    .text 0x03062890 : {
        *(.text*)
        *(.rodata*)
    }
    .hook 0x0306296E : {
        *(.hook)
    }
    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) *(.comment) }
}
