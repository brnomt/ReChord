/*
 * ReChord startup — Echo Mini (RKnanoC) boot entry.
 *
 * The RKnano bootloader loads section_3 to RAM, reads the 16-byte
 * RKnanoFW header, configures the stack, and jumps to the code at
 * 0x03000010 (firmware_entry), passing boot parameters in r0.
 *
 * Header format (byte-identical to stock v3.7.0):
 *   [0:8]  "RKnanoFW"  magic
 *   [8:12] 0x0301e794  initial SP (main stack base, segment table)
 *   [12:16]0x00000052  count / flags (matches stock)
 *
 * This file:
 *   1. Provides the RKnanoFW header (pack_img only splices, does not
 *      rewrite the magic — it must be correct here).
 *   2. firmware_entry @ 0x03000010 is in entry_stubs.S: rechord_hw_init -> rechord_main.
 *   3. Provides a minimal vector table + Default_Handler for Cortex-M3.
 */
#include <stdint.h>
#include <stddef.h>

typedef unsigned int uint32;

extern void rechord_app(void *boot_params);
extern void firmware_entry(void *boot_params);

/* ---- 16-byte RKnanoFW header (byte-exact with stock v3.7.0) ---- */
const uint8_t fw_image_header[16]
    __attribute__((section(".fw_header"), used, aligned(16))) = {
    'R', 'K', 'n', 'a', 'n', 'o', 'F', 'W',  /* magic */
    0x94, 0xE7, 0x01, 0x03,                  /* 0x0301E794 LE: initial SP */
    0x52, 0x00, 0x00, 0x00,                  /* 0x52: count/flags */
};

/* ---- firmware_entry @ 0x03000010 is now defined in entry_stubs.S
 *      (boot entry + ROM-dispatch trampolines). ---- */

/* ---- Minimal Cortex-M3 vector table (first two entries) ---- */
extern uint32 __stack_top;
void Default_Handler(void) __attribute__((naked));

void Default_Handler(void)
{
    for (;;)
        ;
}

const uint32_t vectors[8]
    __attribute__((section(".vectors"), used, aligned(256))) = {
    (uint32_t)&__stack_top,   /* initial MSP */
    (uint32_t)&firmware_entry, /* Reset_Handler -> firmware_entry */
    0, 0, 0, 0, 0, 0,
};
