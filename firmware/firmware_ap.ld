/*
 * ReChord AP (fw1) linker script -- clean-room A_CORE image (milestone 1).
 *
 * VERIFIED BOOT MODEL (byte audit 2026-10-11, docs/HANDOVER.md "PHASE A
 * AUDIT RESULT"; supersedes the earlier XIP assumption):
 *
 *   The loader (Mask ROM/sec2) loads the fw1 IMG body CONTIGUOUSLY into RAM:
 *       RAM = 0x0304F298 + IMG offset
 *   (IMG 0x1F8 -> RAM 0x0304F490; for any image whose vector table sits at
 *   IMG 0xD68 the base is exactly 0x03050000 - 0xD68).  Scatter-row "src"
 *   fields are addresses in that loaded RAM image, NOT flash XIP addresses.
 *
 *   Entry contract: the vector table must land at IMG 0xD68 == RAM 0x03050000
 *   (the RKnanoFW header base).  The payload therefore starts with a 0x5B0
 *   byte prefix before the vector table (docs/fw1-packing.md).  The ROM reads
 *   word0 = initial MSP and word1 = reset from it.
 *
 * Memory roles (matched to the reference image's roles):
 *
 *   0x03000000..0x03004000  AP MAIN STACK only (SP = 0x03004000, grows down).
 *                           NEVER place data here (2026-08-25 brick cause B).
 *   0x03004000..            SYS_DATA: .data (VMA) then .bss (NOLOAD).
 *   0x03050000..            FW_CODE: .vectors + .text + .rodata + .data LMA
 *                           image (the whole blob is loaded in place and
 *                           executes in place - the reference does exactly
 *                           this; its startup even holds a pointer to itself).
 *
 * .data is loaded from an LMA inside the code image (AT> FW_CODE) and copied
 * to 0x03004000.. by PowerOn_Reset (bounded copy, the reference recipe).
 * The LMA image is contiguous with the end of .text so the payload stays one
 * flat blob.  _data_load_start = LOADADDR(.data).
 *
 * .bss is NOLOAD: the ARMCC zero_init attribute is ignored by GCC, so the
 * *_BSS_ named sections land in .bss (NOBITS) and must NOT be part of the
 * payload (docs/fw1-packing.md section 6).  PowerOn_Reset zeroes the bounded
 * .bss span itself (the reference does the same).
 *
 * Per R-a2d2b19d3577: the vector table is isolated in its own section and
 * --gc-sections is never used on this link (see firmware/ap.mk).
 */

ENTRY(PowerOn_Reset)

MEMORY
{
    /* FW_CODE: code + rodata + vector table + the .data load image.
       Executes in place from the loaded image at 0x03050000-based addresses.
       Length mirrors the reference's resident span (its row 0 ends at
       0x030771D4); the hard payload limit is 0x56AB8 (0x57068 minus the
       0x5B0 prefix) which is far above. */
    FW_CODE (rx)  : ORIGIN = 0x03050000, LENGTH = 0x0003C000

    /* FW_RAM: initialized data + bss.  Starts at the top of the main stack
       (SP = 0x03004000); the 16 KiB BELOW 0x03004000 is the stack zone
       0x03000000..0x03004000 and must never hold data.

       Upper bound 0x0304A000 — NOT 0x0304C000 (2026-10-17, DFIU 15e1c9f5 /
       fa3968cd): every AP build whose _bss_end crossed 0x0304A000 bricked
       on hardware (config/persistence 0x0304A8AC, tag-metadata 0x0304A694),
       while every build below it boots (verified-good a1c3a9f 0x03049F7C,
       size-neutral 021513b 0x03049F84). Whatever sits at 0x0304A000+ is not
       ours; the linker now refuses to lay .bss into it. */
    FW_RAM  (rwx) : ORIGIN = 0x03004000, LENGTH = 0x00046000   /* to 0x0304A000 */
}

/* ---- stacks ----------------------------------------------------------- */
/* Initial MSP = the vector table's word 0 (byte-verified 0x03004000 in the
 * reference image); the stack is the 16 KiB below it and nothing else may
 * live there.  0x03000000 is the floor of AP RAM. */
__stack_top   = 0x03004000;
__stack_limit = __stack_top - 0x4000;   /* 0x03000000: the stack zone */

/* Keil scatter-file symbols the SDK-style startup references */
Image$$AP_MAIN_STACK$$ZI$$Limit = __stack_top;

SECTIONS
{
    /* ---- vector table FIRST at the entry base -------------------------
       ap_vectors (ap_startup.c) must be the first bytes of the image:
       0x03050000 is where the loader lands IMG 0xD68 and where the ROM reads
       the vector table (VTOR requires 256-alignment).  The *ap_startup.o()
       spec pins it to the section head regardless of link order; vectors.c's
       legacy Route-B table ("vectors") is kept after it and is not wired. */
    .vectors ORIGIN(FW_CODE) : ALIGN(256)
    {
        KEEP(*ap_startup.o(.vectors))
        KEEP(*(.vectors))
    } > FW_CODE

    .text ALIGN(4) :
    {
        *(.text*)
        *(.rodata*)
        /* unwind metadata emitted by the ARM EABI compiler */
        *(.ARM.extab*)
        *(.ARM.exidx*)
        . = ALIGN(4);
    } > FW_CODE

    /* ---- SYS_DATA: initialized data ---------------------------------- */
    /* VMA at the top of the stack zone (0x03004000, the reference's row-1
     * window start); LMA continues the FW_CODE blob right after .text so the
     * loaded image stays contiguous and PowerOn_Reset can copy it down. */
    .data ORIGIN(FW_RAM) : ALIGN(4)
    {
        . = ALIGN(4);
        _data_start = .;
        KEEP(*(.data.ram_vectors))
        *(.data*)
        . = ALIGN(4);
        _data_end = .;
    } > FW_RAM AT> FW_CODE
    ASSERT(ap_ram_vectors == 0x03004000, "ROM enters via the VT at the start of .data")

    /* ---- SYS_DATA: zero-initialized data (bounded, NOLOAD) ----------- */
    /* PowerOn_Reset zeroes exactly [_bss_start, _bss_end); keep this span
       tight and 4-byte aligned at both ends (the clear loop is word-wise). */
    .bss ALIGN(4) (NOLOAD) :
    {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > FW_RAM

    /* Remaining SRAM between .bss and the top of FW_RAM is the heap
       (symbolic only: no allocator is wired in milestone 1).  The stack zone
       below 0x03004000 is NOT heap. */
    __heap_start = _bss_end;
    __heap_end   = ORIGIN(FW_RAM) + LENGTH(FW_RAM);

    /* Flash/load-resident address of the .data init image (inside the blob). */
    _data_load_start = LOADADDR(.data);

    /* Keil ZI symbols for SDK-style startup code: MUST be the real .bss
       span (pointing these at the whole region wipes .data at boot). */
    Image$$AP_SYS_DATA$$ZI$$Base   = _bss_start;
    Image$$AP_SYS_DATA$$ZI$$Length = _bss_end - _bss_start;

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
    }
}

/* ---- layout contracts (fail the link loudly on violation) ------------- */
ASSERT(ap_vectors == 0x03050000,
       "ap_vectors must sit at the fw1 entry base 0x03050000 (IMG 0xD68)")
ASSERT((ap_vectors & 0xFF) == 0, "ap_vectors must be 256-byte aligned (VTOR)")
ASSERT(PowerOn_Reset != 0, "PowerOn_Reset must be linked")
ASSERT(_data_start == 0x03004000,
       ".data must start at the stack top 0x03004000 (reference row-1 window)")
ASSERT(_data_start >= __stack_top,
       "no data may sit in the stack zone 0x03000000..0x03004000")
ASSERT(_bss_end <= ORIGIN(FW_RAM) + LENGTH(FW_RAM),
       "SYS_DATA overflow: .data+.bss must stay below 0x0304A000 (AP RAM ceiling, DFIU 15e1c9f5)")
ASSERT(LOADADDR(.data) == ADDR(.text) + SIZEOF(.text),
       ".data LMA must be contiguous with the end of .text (flat blob)")
ASSERT(LOADADDR(.data) + SIZEOF(.data) <= ORIGIN(FW_CODE) + LENGTH(FW_CODE),
       "the .data load image must fit inside FW_CODE")
ASSERT(SIZEOF(.vectors) + SIZEOF(.text) + SIZEOF(.data) <= 0x56AB8,
       "payload blob exceeds the fw1 window (0x57068 minus the 0x5B0 prefix)")
