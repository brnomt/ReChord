/*
 * test_boot_params.c — boot param parsing (firmware/app/boot_params.h).
 *
 * What "parsing" means here: the 4 raw ROM handoff words are captured
 * bit-exact into the documented struct and read back through the
 * bounds-checked accessors. Field identity is an open item (TODO), so the
 * contract under test is exact preservation, not interpretation.
 */
#include <stdint.h>

#include "app/boot_params.h"

#include "mocks.h"
#include "test_util.h"

void test_boot_params(void)
{
    const rechord_boot_params_t *bp;

    /* Fresh, uncaptured state. */
    rechord_boot_params_captured = 0;
    CHECK(!boot_params_captured(), "captured flag clear before capture");
    CHECK(boot_params_get() == 0, "get() NULL before capture");

    /* Capture 4 words with high bits set (must not be sign-extended or
     * truncated — the handoff is bit-exact). */
    boot_params_capture(0x11111111u, 0x80000002u, 0xF0F0F0F3u, 0x44444444u);

    CHECK(boot_params_captured(), "captured flag set after capture");
    bp = boot_params_get();
    CHECK(bp != 0, "get() returns the block after capture");
    if (bp != 0) {
        CHECK(bp->raw[0] == 0x11111111u, "word 0 preserved (order)");
        CHECK(bp->raw[1] == 0x80000002u, "word 1 preserved (high bit)");
        CHECK(bp->raw[2] == 0xF0F0F0F3u, "word 2 preserved (all bits)");
        CHECK(bp->raw[3] == 0x44444444u, "word 3 preserved");
    }

    CHECK(boot_params_word(bp, 0) == 0x11111111u, "word(0) accessor");
    CHECK(boot_params_word(bp, 3) == 0x44444444u, "word(3) accessor");
    CHECK(boot_params_word(bp, 4) == 0u, "word(4) out of range -> 0");
    CHECK(boot_params_word(bp, 99) == 0u, "word(99) out of range -> 0");
    CHECK(boot_params_word(0, 0) == 0u, "word(NULL block) -> 0");

    /* Re-capture overwrites every word (startup does this by design:
     * second store after the .bss zero). */
    boot_params_capture(0xAAAAAAAAu, 0xBBBBBBBBu, 0xCCCCCCCCu, 0xDDDDDDDDu);
    bp = boot_params_get();
    CHECK(bp != 0 && bp->raw[0] == 0xAAAAAAAAu &&
          bp->raw[1] == 0xBBBBBBBBu && bp->raw[2] == 0xCCCCCCCCu &&
          bp->raw[3] == 0xDDDDDDDDu,
          "re-capture replaces all four words");
}
