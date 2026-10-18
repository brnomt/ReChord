# ReChord theme packages

One directory per theme, each containing a `theme.cfg` (palette + layout) in the
format `firmware/theme/theme_file.c` and `firmware/theme/theme_pkg.c` parse.
These are the **source copies of the built-in themes**: the firmware compiles
the palettes in (`firmware/theme/themes_builtin.c`), so nothing is created on
the device at boot — a package only exists on the device once you copy it.

## Install / activate (current firmware)

The firmware applies **one** package, read from the fixed path
`/Internal/.rechord/theme.cfg` at boot (palette + layout):

```
# copy the package you want as the active manifest
cp themes/mono/theme.cfg            /Internal/.rechord/theme.cfg
# reboot
```

You can also copy the whole directory for the future multi-package browser:

```
/Internal/.rechord/themes/<id>/theme.cfg
```

and edit `theme.cfg` in place on the device (plain text). A malformed or
out-of-range **layout** value rejects the whole layout and the built-in defaults
stay; a bad palette value is skipped line by line.

## Format

```
name=Nothing Mono
background=0x0000      # RGB565 palette
surface=0x0000
text=0xFFFF
text_dim=0x8410
accent=0xFFFF
danger=0xC618
success=0xFFFF
spacing=8
radius=0

pad=4                  # layout (theme_pkg.c), bounded
title_h=16
item_h=12
np_title_y=24
np_artist_y=36
np_state_y=48
np_info_y=60
np_bar_y=74
np_bar_h=2
np_time_y=84
np_mode_y=148
np_vol_y=162
np_vol_h=2
```

Comments start with `#` or `;`; unknown keys are ignored (forward compatible).
Limits: `pad` 0..16, `title_h` 0..32, `item_h` 8..24, `np_*_y` 0..169,
`np_bar_h`/`np_vol_h` 1..8. Full design: `docs/rewrite/themes.md`.
