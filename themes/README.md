# ReChord theme packages

One directory per theme, each containing a `theme.cfg` (palette + layout) in the
format `firmware/theme/theme_file.c` and `firmware/theme/theme_pkg.c` parse.
These are the **authoring copies of the built-in themes**. The palettes are also
compiled in (`firmware/theme/themes_builtin.c`), and the first boot writes a
copy of each package to `/.rechord/themes/` (see below).

## Install / activate

**The device creates these packages itself on first boot** (the FS create +
mkdir path, no pre-existing files): `/.rechord/themes/{mono,classic,paper,night,companion-bot}/theme.cfg`.

Activate any package from **Settings → Theme** (first row): SELECT cycles the
installed packages. The full package is applied atomically (palette + layout +
home renderer) and the choice is written as `theme=<id>` in
`/.rechord/rechord.cfg`, so it survives a reboot. No recompilation and no
reboot are needed. (`/.rechord/active_theme` is the legacy fallback for configs
that predate the `theme=` key.)

The active package is always the one declared by `home=`, so a theme that does
not ask for the companion never runs the companion renderer. To add a package,
copy a directory under `/.rechord/themes/<id>/` with its `theme.cfg`; it shows
up in the picker on the next opening (existing files are never overwritten).

## Format

```
format=1
name=Nothing Mono
home=menu              # home renderer: menu | companion
home_nav=vertical      # item axis: vertical | horizontal
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
face_h=0               # companion face band (required with home=companion)
home_bar_y=152         # companion item band (companion only)
home_bar_h=14
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
Limits: `format` must be 1; `home` is `menu` or `companion` (anything else
rejects the package); `home=companion` requires `face_h` >= 20; `home_nav` is
`vertical` or `horizontal`; `pad` 0..16, `title_h` 0..32, `item_h` 8..24,
`face_h` 0..80, `home_bar_y` 0..169, `home_bar_h` 8..32, `np_*_y` 0..169,
`np_bar_h`/`np_vol_h` 1..8. Full design: `docs/rewrite/themes.md`.

`home_nav` chooses the item axis for the selected home renderer: the classic
menu is a vertical list, while the companion renderer can lay its three items
out as a row (`horizontal`) or a column (`vertical`). The physical UP/DOWN pair
always means previous/next, so the theme decides whether it feels like up/down
or left/right.

Older manifests that predate `home=` keep working: a non-zero `face_h` selects
the companion renderer (explicit `home=` always wins).
