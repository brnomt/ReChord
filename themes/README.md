# ReChord theme packages

A theme is **pure data**. Nothing in it is compiled into the firmware: the
firmware ships a small generic interpreter (palette/layout + declarative skin
engine) and reads everything from `/.rechord/themes/<theme_id>/`.

```
/.rechord/themes/<theme_id>/
  theme.cfg        # palette, layout, home selector, skin paths, assets
  home.skin        # optional declarative main/home screen
  now_playing.skin # optional Now Playing screen
  library.skin     # optional Library root
  library_cat.skin # optional category (names/songs)
  browser.skin     # optional file browser
  settings.skin    # optional Settings
  assets/*.bin     # 1-bit bitmaps referenced by any skin
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

For a declarative home screen and optional per-screen skins:

```
home=skin
skin=home.skin
skin_now_playing=now_playing.skin
skin_library=library.skin
skin_library_cat=library_cat.skin
skin_browser=browser.skin
skin_settings=settings.skin
face_h=146             # animated band the UI re-flushes (optional)

# optional bitmaps: asset_<name>=<relative path>,<w>,<h>
# asset_icon=assets/icon.bin,32,32
```

Each screen skin is independent: omit a key to keep that screen's compiled
UI. A declared file that cannot be read (or a browser skin with more than 12
rows) falls back to the compiled screen without rejecting the package. The
home skin is still validated atomically at apply time.

Limits: `format` must be 1; `home=skin` requires a readable `skin=` file;
asset paths are relative and may not contain `..`; max 8 assets, 64x64 px,
stride <= 8 bytes, cache <= 1 KiB; `home`/`home_nav` values are validated and an
unknown value rejects the whole package. `companion` is accepted as a legacy
alias for `home=skin`.

## Screen skins

Every screen can have its own declarative layout, while the compiled screen
keeps its data and behavior (movement, selection, folder navigation, settings
persistence, volume). This is how the same theme can show an animated eye home
and a completely plain Settings list.

| key | screen | data the app hands over |
|---|---|---|
| `skin` / `skin_main` | Main/home | static `item0..` declared by the skin |
| `skin_now_playing` | Now Playing | no list; player state only |
| `skin_library` | Library root | 8 category labels |
| `skin_library_cat` | one category | names or songs, cursor and total |
| `skin_browser` | file browser | a window of directory entries |
| `skin_settings` | Settings | visible rows + right-column values |

Interactive screens mark a `list` with `source=<name>`; the app registers the
rows, and the engine draws the window `top .. top+rows-1`. The name is for
readability (one source per screen):

```
[menu]
type=list
axis=vertical
x=6
y=20
height=14
rows=9
source=settings
prev=up
next=down
```

`rows` is also the paging size used by the screen, so the cursor and the
visible window never disagree. Without `rows`, a vertical list fills down to
`H`. The footer can use `sel`, `n` and string concatenation, e.g.
`text=(sel + 1) + " / " + n`. Dynamic lists can also have a right-aligned
value column (Settings); the engine owns the selected-row inversion.

The theme cannot sort/filter or invent rows: the screen's meaning stays in
the app. A screen with no `[skin] refresh_ms` is static and repaints only on
input or new player state.

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
| `art` | `x y size color` (`size=small\|large`; RGB565 cover from ARTPACK.BIN, `color` tints the placeholder) |
| `text` | `x y text color bg` |
| `bar` | `x y w h value max color bg` |
| `list` | `axis x y height selected rows source prev next item0..item3` |

Any meter can set `visible=<expr>` to skip drawing. A `[vars]` section defines
named expressions reused by meters (bounded recursion), which keeps complex
layouts readable. Example from Companion Bot:

```
[vars]
eye_w=70
eye_h=70
blink_on=((mod(t, 3600) < 600) && (idle > 300))
h_l=(blink_on ? blink_h : idle_h)

[eye_left]
type=roundrect
x=clamp(116 - w_l / 2 + gaze + sin(t, 2400, 11), 4, 154 - w_l)
y=clamp(85 - h_l / 2 + sin(t + 700, 3100, 6), 2, 121 - h_l)
w=w_l
h=h_l
r=r_l
color=text
```

Expressions support integers, strings, `+ - * / %`, comparisons, `&& || !`,
ternary `? :`, and functions `min max abs clamp mod noise sin seq if`.
Strings concatenate with `+` and ints are converted (`pl_i + "/" + pl_n`).

### Runtime variables (read-only)

| Variable | Meaning |
|---|---|
| `t`, `idle` | app clock ms / ms since last key |
| `sel`, `n` | selected item / item count |
| `top` | first visible list row |
| `W`, `H`, `aw`, `ah` | display size / current bitmap asset size |
| `playing`, `paused`, `stopped` | player state bits |
| `volume` | volume dB |
| `battery` | battery %, -1 unknown |
| `art_ok`, `art_c1`, `art_c2`, `art_mean`, `art_accent` | cover availability + palette (RGB565), from ARTPACK.BIN |
| `title`, `artist`, `album`, `path` | current track metadata |
| `heading` | screen title/path (Library, Settings, browser path, category) |
| `state` | `"Playing"` / `"Paused"` / `"Stopped"` |
| `pos`, `dur` | position / duration in seconds |
| `pos_ms`, `dur_ms` | same in milliseconds |
| `pos_fmt`, `dur_fmt` | `M:SS` strings for the two values |
| `bits`, `rate` | source depth and sample rate |
| `pl_i`, `pl_n` | 1-based playlist position / count |
| `shuffle`, `repeat` | 0/1 and off/all/one |
| palette names | `background surface text text_dim accent danger success` |

They come from `player_get_state()/get_track()/get_position_ms()/get_volume()/
get_shuffle()/get_repeat()` (runtime state in `firmware/player/player.c`),
`battery_percent()`, the UI selection, and `ap_ms()`. Full guide, meter
reference and examples: [`docs/THEMING.md`](../docs/THEMING.md).

`prev`/`next` bind movement to the logical `up`/`down` keys. For vertical
lists use `prev=up` / `next=down` (vol+ moves down), matching the compiled
fallback. For the horizontal home bar, `prev=down` makes vol+ move left. Only
vol up/down are theme-mappable; PLAY is always SELECT, BACK is always BACK,
power is global.

Bounded by design: 4 KiB skin text, 32 sections, 12 properties/section,
144-byte values, expression recursion depth 12, 2048 evaluation steps.

Masks compose: a slanted cut (a lower lid, a roof) bounds a trapezoid, so it
needs a `fill` for the area below the low end plus a `triangle` for the wedge
above it. A single triangle covering a slanted edge leaves the opposite corner
of the shape visible. Companion Bot's `happy_left`/`happy_left_body` pair is the
working example.

## Assets

1-bit raw rows, MSB-first. `theme.cfg` declares the width/height and the file.
The cache holds up to 1 KiB; a declared-but-unreadable asset rejects the whole
package (no half-themed home). Assets are optional: Companion Bot is vector
(`roundrect` + `triangle` masks) exactly like the reference.
