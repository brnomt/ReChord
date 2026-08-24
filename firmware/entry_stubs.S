/* entry_stubs.S — ReChord boot entry + ROM-dispatch trampolines.
 *
 * The RKnano boot ROM / loader calls FIXED offsets in section_3 for
 * specific purposes (see docs/bootloader-analysis.md):
 *   0x03000010  firmware_entry       (boot, HW init, RETURNS)
 *   0x03000162  FUN_03000162         (USB/MSC misc — stock is `bx lr`)
 *   0x0300016c  WriteData_To_Flash   (USB MSC write)
 *   0x0300024e  MscTestUnitReady     (USB MSC)
 *   0x03000296  FUN_03000296
 *   0x030004f4  FUN_030004f4
 *   0x03000546  RKDev_Close
 *   0x03000a72  FUN_03000a72
 *   0x03000a74  DeleteDeviceListEmmcDataBase
 *   0x03000aba  main2_entry          (USB mass-storage mode)
 *   0x0300710a  application_start    (UI event callback — V0.15 hooks the
 *                                     draw test here)
 *
 * V0.15 FIX (root cause of V0.1-V0.14 "nothing custom ever shows"):
 * the stock firmware_entry is an HW-init callback that RETURNS to the ROM.
 * Ghidra disasm of stock @ 0x03000010:
 *     push {r4,lr}; mov r4,r0; bl boot_param_layout; rom_alloc(0x1dc);
 *     [rom_hw_init(0x1dc/0x16f/0x16f) rom_hw_init2(0x171) rom_hw_init(0x170)]
 *     rom_early_init(); ldrh r0,[r4]; cmp r0,#0xb;
 *     pop {r4,lr}; movw r0,#0x18f; b.w rom_hw_init2   <- early path
 *     pop {r4,lr}; movw r0,#0x191; b.w rom_hw_init2   <- app path
 * Our V0.1-V0.14 entries did `b rechord_app` -> infinite loop -> the ROM
 * boot sequence never continued -> cassette stays, no text, no app, and a
 * ~23s poweroff (ROM-side timeout).
 *
 * V0.15 mirrors stock exactly: rechord_firmware_entry() (rechord_app.c)
 * runs the stock init sequence and returns the rom_hw_init2 code
 * (0x18f/0x191) in r0; we restore the caller's LR and TAIL-CALL
 * rom_hw_init2, exactly like stock, so control returns to the ROM inside
 * rom_hw_init2.
 */
    .syntax unified
    .thumb

    .section .text.boot, "ax"
    .balign 2

    .equ ROM_HW_INIT2, 0x02feee7c    /* rom_hw_init2 (tail-call target) */

    /* firmware_entry @ 0x03000010 — bootloader jumps here, r0 = boot params
     *
     * Option B boot path (V0.20):
     *   1. rechord_hw_init(r0) — runs the stock ROM init sequence
     *      (boot_param_layout + rom_alloc + rom_hw_init + rom_early_init)
     *      so PLLs, clocks, and the LCD controller are properly configured.
     *   2. rechord_main() — our from-source app: ScatterLoader2, BSP_Init2,
     *      mailbox handshake (SYSTEM_START_OK), then our own UI loop with
     *      the ReChord menu drawn into the LCD framebuffer.
     *
     * This replaces V0.17's `b Main2` (which skipped rom_hw_init, leaving
     * the VOP/display uninitialized — heartbeat VOP writes went nowhere).
     * The ROM HW init must run first so the LCD DMA scans from our
     * framebuffer at 0x03024868. */
    .global firmware_entry
    .thumb_func
firmware_entry:
    push {r4}
    mov  r4, r0
    bl   rechord_hw_init         /* ROM HW init (PLLs, clocks, LCD) — r0 = boot params */
    b    rechord_main            /* from-source main: mailbox + UI (never returns) */

    /* --- ROM/loader dispatch trampolines (bx lr = return) ---
     * NOTE: stub_03000162 stays a 2-byte `bx lr`: the bytes at 0x03000164+
     * (immediately after it) are free padding used at runtime by
     * boot_param_layout() writes (see rechord_app.c). */
    .org 0x152                    /* -> 0x03000162 */
    .thumb_func
stub_03000162:
    bx   lr

    .org 0x15c                    /* -> 0x0300016c */
    .thumb_func
stub_0300016c:
    bx   lr

    .org 0x1f0                    /* -> RAM 0x03000200 (256-aligned, VTOR-ready) */
    /* ReChord vector table (fault routing): entry 0 = MSP, 1 = reset,
     * 2 = NMI, 3 = HardFault, 4 = MemManage, 5 = BusFault, 6 = UsageFault,
     * 7 = SVCall. All faults -> IntDefaultHandler2 (SDK) ->
     * __CPU_IntDefaultHandler2 (fault.c). NOTE: vector entries for Thumb
     * handlers must be ODD (|1); only the MSP entry is even. */
    .global rechord_vectors
rechord_vectors:
    .long __stack_top
    .long firmware_entry + 1
    .long IntDefaultHandler2 + 1
    .long IntDefaultHandler2 + 1
    .long IntDefaultHandler2 + 1
    .long IntDefaultHandler2 + 1
    .long IntDefaultHandler2 + 1
    .long IntDefaultHandler2 + 1

    .org 0x23e                    /* -> 0x0300024e */
    .thumb_func
stub_0300024e:
    bx   lr

    .org 0x286                    /* -> 0x03000296 */
    .thumb_func
stub_03000296:
    bx   lr

    .org 0x4e4                    /* -> 0x030004f4 */
    .thumb_func
stub_030004f4:
    bx   lr

    .org 0x536                    /* -> 0x03000546 */
    .thumb_func
stub_03000546:
    bx   lr

    .org 0xa62                    /* -> 0x03000a72 */
    .thumb_func
stub_03000a72:
    bx   lr

    .org 0xa64                    /* -> 0x03000a74 (2 bytes after prev) */
    .thumb_func
stub_03000a74:
    bx   lr

    .org 0xaaa                    /* -> 0x03000aba */
    .thumb_func
stub_03000aba:
    bx   lr

    .org 0x70fa                   /* -> 0x0300710a  application_start */
    .thumb_func
stub_0300710a:
    push {lr}
    bl   rechord_ui_event          /* draw test via the loader display API */
    pop  {pc}

    .org 0x101fc                   /* -> 0x0301020c  MainUI_KeyHandler */
    .thumb_func
stub_0301020c:
    push {lr}
    bl   rechord_key_handler       /* our key handler (navigates the menu) */
    pop  {pc}

    .org 0x10210                   /* pad past the key handler stub */
    .section .text, "ax"          /* continue normal code layout after */
