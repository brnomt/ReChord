# ReChord theme packages

A theme is **pure data**. Nothing in it is compiled into the firmware: the
firmware ships a small generic interpreter (palette/layout + declarative skin
engine) and reads everything from `/.rechord/themes/<theme_id>/`.

```
/.rechord/themes/<theme_id>/
  theme.cfg        # palette, list geometry, home selector, asset declarations
  home.skin        # optional declarative home screen (meters + expressions)
  assets/*.bin     # 1-bit bitmaps referenced by home.skin
```

## Install / activate

Copy the whole directory to `/.rechord/themes/<id>/` (e.g. over USB storage),
then choose it in **Settings → Theme**. The choice is written as `theme=<id>` in
`/.rechord/rechord.cfg`; `/.rechord/active_theme` is the legacy fallback for
configs without that key. No recompilation and no reboot are needed.

The repository ships authoring copies under `themes/`; the firmware does not
create them (no theme content is embedded in the binary).

## theme.cfg

```
format=1
name=My Theme
home=menu              # menu | skin
home_nav=vertical      # default list axis: vertical | horizontal

background=0x0000      # RGB565 palette
surface=0x0000
text=0xFFFF
text_dim=0x8410
accent=0xFFFF
danger=0xC618
success=0xFFFF
spacing=8
radius=0

pad=4                  # list geometry
title_h=16
item_h=12

np_title_y=24          # Now Playing origins
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

For a declarative home screen:

```
home=skin
skin=home.skin
face_h=146             # animated band the UI re-flushes (optional)

# optional bitmaps: asset_<name>=<relative path>,<w>,<h>
# asset_icon=assets/icon.bin,32,32
```

Limits: `format` must be 1; `home=skin` requires a readable `skin=` file;
asset paths are relative and may not contain `..`; max 8 assets, 64x64 px,
stride <= 8 bytes, cache <= 1 KiB; `home`/`home_nav` values are validated and an
unknown value rejects the whole package. `companion` is accepted as a legacy
alias for `home=skin`.

## home.skin

`home.skin` is a bounded, Rockbox/Rainmeter-style declarative scene. Sections
are drawn in file order. Each section has a `type` and properties whose values
may be expressions. Companion Bot is pure vector, exactly like the reference:

```
[vars]
eye_w=100
eye_h=100
blink_on=(mod(t, 3600) < 600)
h_l=(blink_on ? eye_h / 3 : eye_h)
w_l=(blink_on ? eye_w + 14 : eye_w)
r_l=(h_l / 4)
lx=(98 - w_l / 2 + sin(t, 2400, 14))
ly=(85 - h_l / 2 + sin(t + 700, 3100, 7))

[eye_left]
type=roundrect
x=(98 - w_l / 2 + sin(t, 2400, 14))
y=(85 - h_l / 2 + sin(t + 700, 3100, 7))
w=w_l
h=h_l
r=r_l
color=text

[rule]
type=fill
x=0
y=146
w=W
h=1
color=text_dim

[items]
type=list
axis=horizontal
y=152
height=14
selected=1
prev=down
next=up
item0=LIBRARY:library
item1=NOW PLAYING:player
item2=SETTINGS:settings
```

Meter types:

| type | properties |
|---|---|
| `fill` | `x y w h color` |
| `roundrect` | `x y w h r color` (filled, true corner arcs) |
| `triangle` | `x1 y1 x2 y2 x3 y3 color` (filled; degenerate = no draw) |
| `bitmap` | `asset x y color bg` (asset dimensions become `aw`/`ah`) |
| `text` | `x y text color bg` |
| `bar` | `x y w h value max color bg` |
| `list` | `axis x y height selected prev next item0..item3` |

Any meter can set `visible=<expr>` to skip drawing. A `[vars]` section defines
named expressions reused by meters (bounded recursion), which keeps complex
layouts readable. Example from Companion Bot:

```
[vars]
eye_w=100
eye_h=100
blink_on=(mod(t, 3600) < 600)
h_l=(blink_on ? blink_h : idle_h)

[eye_left]
type=roundrect
x=(98 - w_l / 2 + gaze + sin(t, 2400, 14))
y=(85 - h_l / 2 + sin(t + 700, 3100, 7))
w=w_l
h=h_l
r=r_l
color=text
```

Expressions support integers, strings, `+ - * / %`, comparisons, `&& || !`,
ternary `? :`, and functions `min max abs clamp mod noise sin seq if`.
Variables: `t` (ms), `idle` (ms since last key), `playing paused stopped
volume battery W H sel n aw ah title artist album state`, plus palette names
(`background surface text text_dim accent danger success`). A bare word is a
string (useful for `asset=eye_open`).

`prev`/`next` bind movement to the logical `up`/`down` keys. This is enough to
fix inverted hardware: with the Echo Mini's vol+ reported as `down`,
`prev=down` makes vol+ move left.

Bounded by design: 4 KiB skin text, 32 sections, 12 properties/section,
192-byte values, expression recursion depth 12, 2048 evaluation steps.

## Assets

1-bit raw rows, MSB-first. `theme.cfg` declares the width/height and the file.
The cache holds up to 1 KiB; a declared-but-unreadable asset rejects the whole
package (no half-themed home). Assets are optional: Companion Bot is vector
(`roundrect` + `triangle`) exactly like the reference.
