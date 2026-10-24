# ReChord theming guide

A ReChord theme is **pure data**. The firmware contains a small generic
interpreter and a set of drawing primitives; it never contains theme-specific
code, and a theme never needs to be compiled. A theme lives entirely in
`/.rechord/themes/<theme_id>/` on internal storage (or SD, if the theme root is
pointed there):

```
/.rechord/themes/<theme_id>/
  theme.cfg          palette, layout, home selector, skin paths, assets
  home.skin          optional declarative main/home screen
  now_playing.skin   optional Now Playing screen
  library.skin       optional Library root
  library_cat.skin   optional category (names/songs)
  browser.skin       optional file browser
  settings.skin      optional Settings
  assets/*.bin       optional 1-bit bitmaps referenced by any skin
```

The reference theme is `themes/companion-bot/`.

---

## 1. Lifecycle

1. **Boot**: `ap_ui_loop()` reads `theme=<id>` from `/.rechord/rechord.cfg`.
   If the key is absent, it reads the legacy `/.rechord/active_theme`.
2. `theme_pkg_apply()` reads `theme.cfg` **once**, parses palette + layout into
   scratch structs, validates both, and only then commits them together.
3. If the package declares `home=skin`, the `skin=` file and every declared
   asset are loaded from the same directory. A missing/invalid file or asset
   rejects the whole package.
4. On failure the boot falls back to `mono`, then to the built-in Mono
   palette/default layout. The UI is never left half-themed.
5. **Settings → Theme** enumerates the installed packages, applies the next one
   atomically, persists `theme=<id>`, and inserts/removes the skin home live.
6. The skin text is cached for the active theme; there is no per-frame file
   I/O. The engine evaluates expressions and draws meters each refresh.

Firmware log markers:

```
boot: m9e theme 'companion-bot' home 1 nav 1 skin v4 'home.skin'
theme: scan 5 active 'companion-bot'
theme: applied 'night' home 0
theme: apply 'broken' failed
```

`skin vN` is the interpreter revision. If a package is rejected, the previous
theme stays active.

---

## 2. `theme.cfg`

Same `key=value` dialect as `rechord.cfg`. `#`/`;` comments and `[sections]`
are allowed (the flat parser ignores section headers). Keys:

| Key | Meaning |
|---|---|
| `format` | must be `1` |
| `name` | display name in Settings |
| `home` | `menu` (classic list) or `skin` (interpreter) |
| `skin` / `skin_main` | main/home skin file, relative to the theme directory (required for `home=skin`) |
| `skin_now_playing` | Now Playing screen skin (optional) |
| `skin_library` | Library category list skin (optional) |
| `skin_library_cat` | one library category (names/songs) skin (optional) |
| `skin_browser` | file browser skin (optional) |
| `skin_settings` | Settings screen skin (optional) |
| `home_nav` | default list axis: `vertical` / `horizontal` |
| palette | `background surface text text_dim accent danger success spacing radius` |
| list geometry | `pad title_h item_h` |
| Now Playing origins | `np_title_y np_artist_y np_state_y np_info_y np_bar_y np_bar_h np_time_y np_mode_y np_vol_y np_vol_h` |
| `asset_<name>` | `<relative path>,<w>,<h>` — optional 1-bit asset |

Limits: 8 assets, 64x64 px, stride <= 8 bytes, asset cache <= 1 KiB. Asset
paths are relative, may not contain `..`, and a declared-but-unreadable asset
rejects the package. `companion` is accepted as a legacy alias for `home=skin`.

Per-screen skins are optional and independent. A screen whose path is empty,
unreadable, or declares a layout the screen cannot fit (for example the
browser needs `rows <= 12`) falls back to its compiled UI. A broken per-screen
skin never blocks the rest of the package; the main skin is still validated
atomically at apply time.

Everything that is not a theme key is ignored, so packages remain forward
compatible.

---

## 3. Per-screen skins

A theme may ship one skin per screen. They all use the same interpreter and
state variables; what changes is the layout, the animation and the list data
the compiled screen hands over.

| Manifest key | Screen | Data supplied by the app |
|---|---|---|
| `skin` / `skin_main` | Main / home | static `item0..item3` (the skin declares them) |
| `skin_now_playing` | Now Playing | no list; full runtime state |
| `skin_library` | Library root | `source` list: 8 categories |
| `skin_library_cat` | one category | `source` list: names or songs, live cursor/total |
| `skin_browser` | file browser | `source` list: current directory window |
| `skin_settings` | Settings | `source` list: visible rows + right-column values |

The compiled screen still owns behavior: cursor/scroll state, opening a
folder, cycling a setting, volume keys, saving. The theme owns the visual
structure and may make each screen completely different — an animated eye
home, a plain list for Settings, no animation at all on Now Playing. The app
loads the screen's skin on entry and re-registers its list source; returning
to the home reloads the main skin. Changing the theme from Settings reloads
the Settings skin for the new package.

### Dynamic lists

A `list` meter becomes dynamic by naming a source:

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

- `source=<name>` tells the engine to use the callback table the compiled
  screen registered (the name is documentation; there is one source per
  screen). Without `source`, `item0..` static labels are used as on the home.
- `rows` is the visible window. The screen uses the same value for paging, so
  the cursor and the drawn window always agree. If omitted, a vertical list
  fills the area down to `H`; the browser rejects more than 12 rows.
- The screen passes `top` and `sel`; the engine draws `top .. top+rows-1` and
  inverts `sel`. The skin can draw a footer with `(sel + 1) + " / " + n`;
  `n` is the source's total, not just the window.
- The source may provide a right-aligned value column (Settings does). Values
  are drawn by the engine so themes keep the same selected-row inversion.

The skin cannot sort, filter or invent rows: it controls how the screen's
data is presented, not what the screen means. That keeps the app's behavior
and the theme's visuals independent.

---

## 4. `home.skin` / screen skin syntax

```
[background]
type=fill
x=0
y=0
w=W
h=H
color=background

[eye_left]
type=roundrect
x=lx
y=ly
w=w_l
h=h_l
r=r_l
color=text

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

### Meters

| type | properties |
|---|---|
| `fill` | `x y w h color` |
| `roundrect` | `x y w h r color` (filled, true corner arcs) |
| `triangle` | `x1 y1 x2 y2 x3 y3 color` (filled; degenerate = no draw) |
| `gradient` | `x y w h from to dir` (two RGB565 colours; `dir=vertical` or `diagonal`; 4x4 ordered dithering hides RGB565 banding; draws only inside the rect) |
| `bitmap` | `asset x y color bg` (asset dimensions become `aw`/`ah`) |
| `art` | `x y size` (`small`\|`large`) `color` (placeholder tint, default `surface`) |
| `text` | `x y text color bg [maxc] [align]` (`maxc` cuts to N chars ending in `~`; `align=right` makes `x` the right edge) |
| `bar` | `x y w h value max color bg` |
| `list` | `axis x y height selected rows source prev next item0..item3` |

Any meter may set `visible=<expr>`; a false value skips it.

Colors may be a palette name (`background surface text text_dim accent danger
success`) or an RGB565 integer.

### `[vars]`

Defines named expressions evaluated once per frame **in definition order**.
Meters and later vars read the cached values.

```
[vars]
eye_w=70
eye_h=70
blink_on=((mod(t, 3600) < 600) && (idle > 300))
blink_h=(blink_on ? 24 : eye_h)
h_l=blink_h

[vars2]
w_l=(blink_on ? eye_w + 5 : eye_w)
r_l=(h_l / 4)
```

Use `[vars]`, `[vars2]` (and `[vars3]`) to stay inside the 12-property section
cap. Vars are ints or strings; a var that references a later var sees the
previous frame's value, so keep dependencies ordered.

### Expressions

Integers, strings, parentheses, `+ - * / %`, comparisons
(`== != < <= > >=`), logical `&& || !`, ternary `? :`.

Functions: `min max abs clamp mod noise sin sqrt seq if` (`sqrt` is an
integer floor, for area-true radii).

- `noise(x, amp)` — deterministic value in `[-amp, amp]`.
- `sin(x, period, amp)` — integer sine, `amp * sin(2*pi*x/period)`.
- `seq(t, period, dwell, "a,b,c")` — frame selection by time; outside `dwell`
  it returns the first frame.
- `if(c, a, b)` — same as ternary.

A bare word is a string (`asset=eye_open`, `text=LIBRARY`). Unknown identifiers
evaluate to that string; numeric contexts read it as 0.

### Keyboard

Only **vol up / vol down** are theme-mappable, through the list meter:

```
# vertical lists (Library, Settings, Browser, categories):
prev=up
next=down

# horizontal home bar (Companion Bot):
prev=down
next=up
```

`prev`/`next` are **logical** keys; the Echo Mini reports vol+ as `down` and
vol- as `up`. For a vertical list the intuitive mapping is `prev=up`,
`next=down` (vol+ moves down). On a horizontal bar `prev=down` makes vol+ move
left. The compiled fallback uses the same vertical convention.

PLAY is always SELECT, BACK is always BACK, and power is handled globally
(short = backlight off/on, Power+MENU = Hold, hold ~2 s = power off). A theme
cannot steal those keys.

---

## 5. Runtime state available to the theme

The UI adapter fills a read-only `theme_skin_state_t` before drawing:

| Variable | Type | Source / meaning |
|---|---|---|
| `t` | ms | monotonic app clock (`ap_ms`) |
| `idle` | ms | time since the last key |
| `sel` | int | selected list item |
| `top` | int | first visible list row (screen scroll position) |
| `n` | int | number of list items |
| `W`, `H` | int | display size (320, 170) |
| `aw`, `ah` | int | current bitmap asset dimensions |
| `playing` / `paused` / `stopped` | 0/1 | `player_get_state()` |
| `volume` | dB | `player_get_volume()` |
| `battery` | % | `battery_percent()`, `-1` when unknown |
| `art_ok` | 0/1 | the current track has a cover in `ARTPACK.BIN` |
| `art_c1` / `art_c2` | RGB565 | cover dominant / secondary colour |
| `art_mean` / `art_accent` | RGB565 | cover mean / the mockup's accent |
| `art_bg_a` / `art_bg_b` | RGB565 | deep gradient ends (dominant V=0.42 / secondary V=0.24) |
| `art_glow` / `art_panel` | RGB565 | mid glow (V=0.55) / card panel (mean V=0.16) |
| `art_label` | RGB565 | cream label (cream mixed 10 % with the mean) |
| `art_ink` / `art_ink_dim` | RGB565 | fixed label inks (#221E1C / #706860) |
| `title` | string | `player_get_track().title` |
| `artist` | string | `player_get_track().artist` |
| `album` | string | `player_get_track().album` |
| `path` | string | `player_get_track().path` |
| `heading` | string | screen title/path (Library, Settings, browser path, category) |
| `state` | string | `"Playing"` / `"Paused"` / `"Stopped"` |
| `pos` / `dur` | s | `player_get_position_ms()`, track duration |
| `pos_ms` / `dur_ms` | ms | same values in milliseconds |
| `pos_fmt` / `dur_fmt` | string | `M:SS` formatting of the two values |
| `bits` | int | source depth (16/24/32, 0 unknown) |
| `rate` | Hz | sample rate (44100/48000/…, 0 unknown) |
| `pl_i` / `pl_n` | int | 1-based playlist position / count (0 = no list) |
| `shuffle` | 0/1 | `player_get_shuffle()` |
| `repeat` | 0/1/2 | off / all / one (`player_get_repeat()`) |
| palette names | RGB565 | active theme colors |

Where the values live:

- `firmware/player/player.c` owns the runtime `g` state and `g.track`
  (`track_info_t`: title, artist, album, genre, path, duration, bits, sample
  rate, playlist position/count, gapless, ReplayGain).
- Public read-only accessors: `player_get_state()`, `player_get_volume()`,
  `player_get_position_ms()`, `player_get_track()`, `player_get_shuffle()`,
  `player_get_repeat()`.
- Persistent settings live in `/.rechord/rechord.cfg`, loaded by
  `settings_screen_reload()`.
- Resume state (last track/folder/row) lives in `/.rechord/state`.
- The skin adapter (`firmware/ui/screens/skin_screen.c`) copies these values
  into `theme_skin_state_t` for the interpreter; the engine never touches the
  player directly.

### Example: a now-playing info skin

```
[title]
type=text
x=4
y=20
text=title
color=text

[artist]
type=text
x=4
y=32
text=artist
color=text_dim

[pos]
type=text
x=4
y=44
text=pos_fmt + " / " + dur_fmt
color=text_dim

[bar]
type=bar
x=4
y=60
w=W - 8
h=2
value=pos_ms
max=dur_ms
color=accent
bg=surface

[fmt]
type=text
x=4
y=72
text=bits + "/" + (rate / 1000) + "kHz"
color=text_dim

[pl]
type=text
x=4
y=84
text=pl_i + "/" + pl_n
color=text_dim
```

String concatenation with `+` keeps the expression self-contained.
Missing values are empty strings / 0, never a crash.

### Example: album art + cover colours

The device does no image decoding: the PC tool (`tools/artpack`, see
`docs/ART_PACK.md`) precomputes a large (120x120 default) and a small (48x48)
RGB565 cover per track plus a palette. `art` streams the current track's cover
from `/Internal/.rechord/ARTPACK.BIN`; when the track has no cover (or the pack
is missing) it draws a filled rounded rect in `color` (default `surface`), so
the layout never jumps.

```
[backdrop]
type=fill
x=0
y=0
w=W
h=H
color=art_c1
visible=art_ok

[cover]
type=art
x=8
y=8
size=large

[title]
type=text
x=136
y=24
text=title
color=text

[artist]
type=text
x=136
y=38
text=artist
color=text_dim

[accent_rule]
type=fill
x=136
y=54
w=W - 144
h=2
color=art_accent
visible=art_ok
```

`art_c1`/`art_c2` (and `art_mean`/`art_accent`) are plain RGB565 integers, so
they work anywhere a colour expression does (`color=`, `bg=`,
`visible=art_ok`). A static skin (no `refresh_ms`) draws the cover once; an
animated skin whose refresh band covers the `art` meter re-streams it on every
refresh, which costs sectors — keep covers outside the animated band.

---

## 6. Animation model

There is no compiled animation state. Motion is a function of `t`, `idle`,
`playing` and the package's expressions. The Companion Bot is the reference:

```
blink_t=mod(t, 3600)
blink_on=((blink_t < 600) && (idle > 300))
blink_step=(blink_t * 7) / 600
blink_raw=(blink_step <= 3 ? eye_h - (blink_step * eye_h) / 3 : eye_h - ((7 - blink_step) * eye_h) / 3)
blink_h=(blink_raw < 4 ? 4 : blink_raw)
idle_h=(idle > 6000 ? 4 : (idle < 300 ? 4 + (idle * (eye_h - 4)) / 300 : eye_h))
```

`[skin] refresh_ms` opts a screen into animation and controls its refresh
rate (Companion home: 50 ms); `refresh_h` is the fallback band the UI
re-flushes when the skin declares no dynamic rects. A skin with no
`refresh_ms` is static: the app repaints it only on input or on new player
state. That is why the Now Playing skin in this package costs nothing while
paused, and why Settings/Library/Browser do not wake the CPU.

**Static/dynamic split (`dyn`).** Re-rendering the whole skin every tick is
what makes an animated theme feel heavy: a full-screen gradient plus a
120x120 cover stream are drawn again just to move a 10-px hand. Mark the
primitives that actually change with `dyn=1` and declare up to three rects
that contain them:

```
[skin]
refresh_ms=66
dyn=58,12,262,100      # label + reels
dyn2=56,120,264,134    # play symbol + counter
dyn3=0,146,319,169     # bottom bar + items
```

On a tick or a key that only changes the selection, the app calls the
engine with `theme_skin_set_dynamic_only(1)`: it draws only the `dyn=1`
sections (no clear, no background, no `art`) and flushes exactly the three
rects with `display_flush_rect()`. Everything else keeps the pixels of the
last full draw, so the cover is never re-read and the panel only receives
those rects. The dynamic sections must therefore cover every pixel they
change; if a shape can shrink (a tape pack), redraw its background inside
the dynamic set (the cassette marks the label for this). A skin without
`dyn` rects falls back to the `refresh_h` band, which is correct but
expensive.

`[skin]` accepts `refresh_ms`, `refresh_h`, `dyn`, `dyn2` and `dyn3`.

The per-screen refresh is driven by the app loop only while that screen is on
top; the home keeps its own tick. A theme can animate any screen by declaring
`refresh_ms`, or leave it static by omitting it.

---

## 7. Assets (optional)

Raw 1-bit rows, MSB-first, stride = `(w + 7) / 8`. Declare in `theme.cfg`:

```
asset_icon=assets/icon.bin,32,32
```

and use:

```
[icon]
type=bitmap
asset=icon
x=8
y=8
color=text
bg=background
```

`bitmap` meters expose the selected asset dimensions as `aw`/`ah`. The cache is
1 KiB; assets are optional. Companion Bot is pure vector (`roundrect` +
`triangle`), exactly like its reference.

---

## 8. Limits and safety

- Skin text 4 KiB, 52 sections, 12 properties/section, 144-byte values.
  (Sections are parsed, not stored, so the section cap costs no RAM; the
  Cassette home uses 50.)
- Expressions: 2048 steps, recursion depth 12.
- 8 assets max, 64x64, stride <= 8 B, 1 KiB cache.
- A malformed `theme.cfg`, skin, or declared asset rejects the package
  atomically; the active theme stays.
- No theme can execute code, allocate memory, or take the player keys.
- Boot fallback is always the built-in Mono palette/default layout.

Measured on the current AP build: `.bss_end 0x03049CA8` (gate `0x0304A000`),
with the engine, per-screen bridge and the additional manifest paths. The
interpreter keeps fixed buffers only; no per-theme allocation and no heap.

---

## 9. Current scope

The declarative interpreter drives the **home/main, Now Playing, Library,
library category, browser and Settings** screens when the package declares
the matching skin:

```
skin=home.skin
skin_now_playing=now_playing.skin
skin_library=library.skin
skin_library_cat=library_cat.skin
skin_browser=browser.skin
skin_settings=settings.skin
```

The compiled screens remain the fallback and keep all behavior/actions. The
classic root menu can still be the home (`home=menu`); the system and boot
screens are not skinned. The `firmware/ui/screens/theme_ui.c` bridge maps each
screen's rows and player state into the engine; `theme_ui_load()` is called on
entry and `theme_ui_nav()` translates the skin's vol mapping into the screen's
existing movement/action code.
