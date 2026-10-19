# assets/ — the Companion Bot eye frames

These `.bin` files ARE the theme's visual assets. The firmware's generic skin
interpreter blits them; no eye drawing code exists in the binary.

Format: raw 1-bit rows, MSB-first, stride = (width + 7) / 8.

| file | size | used for |
|---|---|---|
| `eye_open.bin` | 64x64 r16 | idle / open |
| `eye_half.bin` | 64x32 | blink / wake |
| `eye_closed.bin` | 64x6 | blink / sleep |
| `eye_happy_left.bin` | 64x64 r16 | happy (left, masked lower lid) |
| `eye_happy_right.bin` | 64x64 r16 | happy (right, mirrored) |

Regenerate with:

```
python3 tools/gen_companion_assets.py
```

The look follows the MIT-licensed
[oled_eye_display](https://github.com/intellar/oled_eye_display): filled
rounded rectangles, no pupils, squash-widen blink and a masked lower lid for
"happy". The animation *logic* (blink period, saccade, sleep, happy) lives in
`../home.skin` as expressions, not in C.
