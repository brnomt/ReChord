# VU Meter — two 1970s receiver meters

`themes/vu-meter/` recreates the approved `receiver3` mockup
(`~/rechord-themes-mockups-v3/receiver3/`) on the 320x170 panel. It is the
theme named **VU Meter** (directory id `vu-meter`).

The home screen is two backlit VU meters: chrome bezel, black gasket, cream
face with a warm lamp gradient, the real VU scale (20 10 7 5 3 2 1 0 +1 +2
+3), a red +0..+3 zone, the pivot dome and a PEAK lamp. The scale is split
into three single-colour 1-bit assets so the red band stays red.

## Palette

| slot | RGB888 | RGB565 | use |
|---|---|---|---|
| background | `#0B0B0C` | `0x0841` | glass |
| surface | `#1F2124` | `0x1904` | fallback |
| text | `#6FF0DA` | `0x6F9B` | VFD |
| text_dim | `#2C8C7E` | `0x2C6F` | VFD dim |
| accent | `#FFB547` | `0xFDA8` | pointers, numbers |
| danger | `#E8432F` | `0xEA05` | red zone, low battery |
| success | `#7CE36B` | `0x7F0D` | status LED |

The meter hardware colours (cream `#F6E7B8`, warm lamp `#F0A13A`, chrome,
gasket, red scale, ink) are fixed decimal RGB565 literals inside the skins.

## Scaling and ballistics

The scale asset is 124x40 and is drawn at `cx-60, py-86`; the pivot is
`(cx, py)`. A needle is a thin triangle from the pivot to radius 78:

```
th = ps * 9/10 - 450      # angle in tenths of a degree, -45..+45 deg
tip = (cx + sin(th,3600,78), py - sin(th+900,3600,78))
```

The deflection `ps` is a second-order IIR in integer fixed point:

```
tg = (!playing ? 0 : clamp(300+(volume+18)*20 + noise(t/90,230)
                           + sin(t,700,120) + (loud ? 250 : 0), 0, 1060))
vl = vl*55/100 + (tg-ps)*32/100
ps = clamp(ps+vl, -30, 1060)
```

`vl` reads `ps` before it is redefined, so it sees the previous frame: that
persistence is what makes the filter work (vars are evaluated once per
frame, in order, and survive). Pausing drives `tg` to 0 so the needles fall
to rest; resume overshoots slightly, like a real VU. The filter is tuned for
the ~50 ms refresh; the exact rise depends on the real frame rate.

There is no `log()`/`exp()` in the engine, so the deflection is a **linear
approximation** of the VU law, not `10^(dB/20)`. The scale artwork is the
real one and the needle still lands near 0 VU at normal levels.

## Home moods

| condition | result |
|---|---|
| playing | needles track the simulated programme level |
| `idle < 700` ms | needles kick +250 of deflection, then fall back |
| paused | needles fall to rest, VFD says PAUSA |
| asleep (paused >60 s) | faces dim, needles at rest, VFD says PAUSA |
| battery <15 % | faces dim, VFD says BATERIA BAJA |
| loud (`volume > -6`) | extra +250 of target deflection |

The VFD (`6,120 230x28`) shows the title (or PAUSA/STANDBY/BATERIA BAJA) and
the artist, with the elapsed time and format on the right; a small volume
knob and the engine list share the bottom bar.

## Screens

* **Now Playing** — framed cover, one stereo meter with two needles (L ink,
  R red) and both labels, VFD with title/artist, progress strip, PLAY/PAUSA
  button, format and SHUF/REP.
* **Library** — an FM dial: blue gradient, frequency numbers, four 64x10 tick
  assets, red pointer at the selected row, a miniature signal-strength meter,
  a light grey heading bar and the list.
* **Category** — same dial with `heading` as the title.
* **Browser** — same dial with the path as the title.
* **Settings** — brushed-aluminium bar, two knobs (the right one follows the
  battery), two VFD windows (SERVICE MODE / BAT n %) and a status LED.

## Animation and redraw

`refresh_ms=50`; three dyn rects:

```
dyn=6,4,158,116      left meter (everything, faces dim on low/asleep)
dyn2=162,4,314,116   right meter
dyn3=0,118,319,169   VFD, volume knob, bottom bar and list
```

Every section inside a meter carries `dyn=1` because its background colour
changes with the dim state; the dynamic layer repaints the whole face, then
the needles and lamps on top. Now Playing uses the same split for the stereo
meter, VFD and bottom bar.

## Budget (measured)

| file | bytes / 4096 | sections / 52 |
|---|---|---|
| home.skin | 4089 | 45 |
| now_playing.skin | 3585 | 41 |
| library.skin | 1144 | 16 |
| library_cat.skin | 1148 | 16 |
| browser.skin | 1146 | 16 |
| settings.skin | 1095 | 15 |

Assets: `vu_a` 64x40 (320 B), `vu_b` 19x40 (120 B), `vu_red` 41x40 (240 B),
`dial_ticks` 64x10 (80 B) = **760 B**.

## Differences from the mockup

* the glass reflection triangle and the pivot-dome highlight were cut for
  the 4096 B budget;
* the needle angle uses a linear approximation of the VU law (see above);
* the low-battery mockup also attenuates the needle by 0.7; the skin keeps
  the true deflection and only dims the face;
* sleep and pause share the `PAUSA` VFD text (the dim face and the needle at
  rest still separate them);
* the FM-dial pointer and the list share the selected row, so the pointer
  moves with the list rather than being independent.

Screenshots in this directory were produced by `tools/theme_preview/run.sh`
(real interpreter + display driver); `compare_*.png` pairs the mockup with
the engine output.

```
tools/theme_preview/check_theme.py themes vu-meter
tools/theme_preview/run.sh ../../themes vu-meter ../../docs/themes/vu-meter
tools/theme_preview/compare.py ~/rechord-themes-mockups-v3/receiver3 \
    docs/themes/vu-meter
```
