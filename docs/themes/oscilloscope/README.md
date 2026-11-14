# Oscilloscope — laboratory scope (20 or 60 fps)

`themes/oscilloscope/` is a data-only package that turns the 320x170 panel
into the front of a 1980s lab oscilloscope: a green CRT with an 8x6
graticule, a phosphor trace with a baked halo, and a brushed-aluminium
control panel with VOLTS/TIME knobs, a TRIG'D lamp, a battery gauge and a BNC
input. It is the implementation of the approved `fosforo` mockup
(`~/rechord-themes-mockups-v3/fosforo/`) with the theme named **Oscilloscope**.

Every pixel is drawn by the generic skin interpreter: the package contains no
code. The three trace shapes are 1-bit assets baked with their halo; the
theme switches the asset *name* per frame, so no extra sections are spent.

## One package, two sweep rates

The scope trace runs on a redrawn-by-the-app animation; how fast it moves and
how much of the panel is re-flushed per frame is a trade-off between smoothness
and battery. That used to be two separate packages
(`oscilloscope` = 20 fps, `oscilloscope-60` = 60 fps). It is now one package
with an option:

```
opt_fps=Refresh,20 fps|60 fps
```

`opt_fps` is read by `[skin] refresh_ms`, by all three `dyn` rects and their
cadence, by the trace band geometry, by the scope `gain`/`halo`, and by the
`OSC-20 RC` / `OSC-60 RC` plate label. The **20 fps** choice is the default and
renders byte-identically to the old `oscilloscope` package; **60 fps** matches
the old `oscilloscope-60`.

Switch it with a long press on **Settings → Interface → Theme** (see
`docs/THEMING.md` §3). The choice is stored per theme id
(`theme_opts=oscilloscope:1;`), so it survives switching to another theme and
back.

## Palette

| slot | RGB888 | RGB565 | use |
|---|---|---|---|
| background | `#03100A` | `0x0081` | CRT face |
| surface | `#10261A` | `0x1123` | fallback plates |
| text | `#7CFFB2` | `0x7FF6` | phosphor / labels |
| text_dim | `#2E8A57` | `0x2C4A` | readouts, footers |
| accent | `#FFB347` | `0xFD88` | trigger marker, RUN |
| danger | `#FF5A4E` | `0xFAC9` | low battery trace / LOW BAT |
| success | `#7CFFB2` | `0x7FF6` | same as text |

Device hardware colours (aluminium, bezel, knurled knobs, BNC metal, gold
TIME cap) are fixed RGB565 literals inside the skins. Skins use **decimal**
literals only: the expression parser has no `0x` numbers.

## Screens

* **Home** — panel gradient > bezel > tube > graticule frame/rule > three
  64x32 trace tiles (`w0`/`w1`/`w2`, picked per frame) > flat line when
  stopped > graticule on top > trigger marker > CH1/title readouts > knobs,
  lamp, battery and BNC > engine list. 49 sections.
* **Now Playing** — the cover is "CH2" on the left; the right tube is a
  goniometer: a 48x40 dense X-Y scribble jittered +-2 px with two bright beam
  slivers, L/R axes and a centred dot when paused. It plots mid `(L+R)/2` on
  the vertical axis and side `(L-R)/2` on the horizontal one, so a signal only
  in one channel leans onto that channel's diagonal (`L` upper-left, `R`
  upper-right) instead of collapsing onto the centre line. A dark readout
  window carries title/artist/time/playlist, a progress strip runs underneath,
  and the status bar shows RUN/STOP + format + SHUF/REP.
* **Library / category** — instrument menu (`MENU > LIBRARY`, or
  `MENU > <category>`), position counter, mini sweep line, header knob.
* **Browser** — same instrument frame with `MENU > <path>`.
* **Settings** — `UTILITY > SETTINGS` with the battery in the header.

## Home moods

`[vars] md` maps the runtime to six states, exactly like the mockup:

| md | condition | result |
|---|---|---|
| 0 | playing, volume <= -6 dB | three tiles alternate `w1`/`w2` |
| 5 | playing, volume > -6 dB | `w3`, the clipped square wave |
| 4 | not playing | trace hidden, flat line + READY |
| 2 | `idle < 700` ms | loud `w2`/`w3` tiles (key pressed) |
| 3 | paused and `idle > 60 s` | flat line (asleep) |
| 1 | battery 0..14 % | trace in `danger`, LOW BAT, red battery bar |

Play/pause also flips the TRIG'D lamp to READY and the right readout to
`STOP m:ss`.

## Animation and redraw

`refresh_ms=125` (8 fps) with three dynamic rectangles:

```
dyn=13,13,213,136    graticule, trace, trigger, readouts
dyn2=222,20,319,105  TIME knob, TRIG lamp, battery bar
dyn3=0,146,319,169   nav bar + list selection
```

Every section that can change carries `dyn=1` and the static artefacts
(panel, bezel, brand plate, VOL knob) keep their pixels between ticks. The
TIME knob follows `pos_ms`, the trace tiles follow `t`/`volume`, and the
battery bar blinks in the low state.

## Engine bug found (fixed)

The mockup expressions use `a && b ? x : y`. The interpreter evaluated
`&&`/`||` with C short-circuit, so when the left operand already decided the
result, the right operand was **never consumed from the input** and the
following `?:` was left unparsed (the whole expression collapsed to an
integer). This affected every mood expression. `firmware/theme/theme_skin.c`
now evaluates both operands before combining them; a regression case lives in
`firmware/ui/tests/test_skin_engine.c` (`OKA`/`OKB`/`NOC`/`NOD`).

## Budget (measured)

| file | bytes / 4096 | sections / 52 |
|---|---|---|
| home.skin | 3871 | 44 |
| now_playing.skin | 3617 | 44 |
| library.skin | 1010 | 13 |
| library_cat.skin | 1022 | 13 |
| browser.skin | 1018 | 13 |
| settings.skin | 1036 | 13 |

Assets: `w1 w2 w3` 64x32 (256 B each) + `xy` 48x40 (240 B) = **1008 B** of
the 1024 B cache.

## Differences from the mockup

These are deliberate budget cuts, not engine limits:

* the third (dimmest) goniometer beam sliver is omitted;
* the tube's CH1 ground arrow, the `1M 300V` spec line and the BNC
  `CH1` label (redundant with the `CH1 -18dB` readout) are omitted;
* the trace shape is `noise()`-driven, so a still frame never matches the
  mockup's generated tile pixel for pixel (the tile *set* is identical);
* text always paints its background, so panel labels show a faint flat box
  over the gradient (the mockup has the same constraint).

Screenshots: `home_*.png`, `np_*.png`, `library*.png`, `settings.png`,
`compare_*.png` (mockup | engine) and `moods.png` were produced by
`tools/theme_preview/run.sh` through the real interpreter + display driver;
covers are placeholders drawn by the previewer.

`options_2x.png` is the **Theme options** sub-screen for this package (the
`skin_settings` skin driving the option rows), and `pair_fps_home_2x.png` /
`pair_fps_np_2x.png` compare the two `opt_fps` values side by side
(20 fps | 60 fps), produced with:

```
tools/theme_preview/run.sh ../../themes oscilloscope /tmp/o20
tools/theme_preview/run.sh ../../themes oscilloscope /tmp/o60 opt_fps=1
```

```
tools/theme_preview/check_theme.py themes oscilloscope
tools/theme_preview/run.sh ../../themes oscilloscope \
    docs/themes/oscilloscope
tools/theme_preview/compare.py ~/rechord-themes-mockups-v3/fosforo \
    docs/themes/oscilloscope
```
