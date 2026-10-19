# assets/ — optional bitmaps

The Companion Bot face is **vector**, like the reference: `home.skin` draws
filled rounded rectangles and a masking triangle with the engine's generic
`roundrect` / `triangle` meters. There are no bitmap frames to ship, which is
what makes the animation smooth and keeps the package tiny.

The theme system still supports optional 1-bit assets for other themes:

```
# in theme.cfg
asset_icon=assets/icon.bin,32,32
```

```
# in home.skin
[icon]
type=bitmap
asset=icon
x=8
y=8
```

Format: raw 1-bit rows, MSB-first, stride = (width + 7) / 8. Limits: max 8
assets, 64x64 px, stride <= 8 bytes, 1 KiB cache. Declaring an unreadable
asset rejects the package.

The look follows the MIT-licensed
[oled_eye_display](https://github.com/intellar/oled_eye_display): filled
rounded rectangles, no pupils, squash-widen blink, wake ramp, sleep line and a
moving lower-lid triangle for "happy". All timing/geometry lives in
`../home.skin` as expressions, not in C.
