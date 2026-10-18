# assets/ — intentionally empty

The Companion Bot face is **procedural**: `firmware/ui/screens/companion.c`
draws it with the renderer primitives (`display_fill_rect` / `display_draw_text`)
only. No 1-bit bitmap, no asset cache, no allocation is used, which keeps the
package inside the 8 KiB asset budget (it uses none) and the `.bss` gate.

Animation logic adapted from the MIT-licensed
[oled_eye_display](https://github.com/intellar/oled_eye_display)
(`control_display.ino`): WAKEUP, SLEEP, BLINK (short/long), SACCADE and HAPPY,
re-implemented for ReChord's fixed-resolution monochrome renderer.
