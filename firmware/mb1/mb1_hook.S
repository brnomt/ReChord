/* mb1_hook.S — ReChord M-B1 Reset hook.
 * Placed at 0x0306296E (Reset entry), replaces the original first
 * instruction (bl disableIRQinterrupts) with a 4-byte Thumb-2 branch.
 * The original instruction is re-executed indirectly: Reset+4 continues
 * the untouched original startup after the stub returns control.
 */
    .syntax unified
    .thumb
    .section .hook, "ax", %progbits
    .global mb1_hook
mb1_hook:
    b.w     rechord_boot
