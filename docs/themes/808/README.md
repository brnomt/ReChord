# 808 — RC-808 Rhythm Composer

`themes/808/` is the data-only implementation of the approved `ritmo` mockup
(`~/rechord-themes-mockups-v3/ritmo/`) on the 320x170 panel. The theme is
named **808** (directory id `808`).

The home screen is the front of an 808-style drum machine: the red/orange/
yellow/cream stripe, three knobs (VOLUME, ACCENT, BATT), a masking-tape label
with the song, a red 7-segment time display and **16 step keys** in four
colour groups (BASS, SNARE, TOMS, HATS). A LED runs over the steps at 120 BPM
(one sixteenth = 125 ms) and the key under it lights up.

## Palette

| slot | RGB888 | RGB565 | use |
|---|---|---|---|
| background | `#1E1F21` | `0x18E4` | body |
| surface | `#2C2E31` | `0x2966` | panel |
| text | `#EDE6D3` | `0xEF3A` | cream labels |
| text_dim | `#8C887E` | `0x8C4F` | footers |
| accent | `#F0861F` | `0xF423` | orange stripe / ACCENT |
| danger | `#E4412A` | `0xE205` | red group, BATT low |
| success | `#F2C53A` | `0xF627` | yellow group |

Key/segment/tape hardware colours are decimal RGB565 literals inside the
skins. A dim key keeps its hue (`#8E2B1D`, `#A0561A`, `#B08A2C`, `#A29D90`).

## Sequencer (home)

```
st = mod(t/125, 16)          # 120 BPM sixteenths
bk = mod(t,1000) < 500       # 2 Hz blink
pattern(k) = noise(pl_i*31+k, 4) > -3
```

`pl_i` seeds the pattern, so every track has a different step pattern with
no extra state. The 16 keys are drawn from that; the cursor is a single
`[cur]` overlay at `x = 17+18*st` (white-hot) that is redrawn after the keys
every tick, so the previous key is cleared without 16 `st==k` branches in the
colour expressions. `[low]` is a second overlay on step 1 for the low-battery
blink.

## Home moods

| condition | result |
|---|---|
| playing | LED marches, active key lit, programmed steps on |
| `idle < 700` ms | ACCENT knob moves, LED bright (pressed) |
| paused | LED blinks (the "cursor"), active-key overlay blinks |
| loud (`volume > -6`) | ACCENT knob at 85 %, downbeat keys hot |
| battery <15 % | BATT knob red, step 1 blinks red, LED hidden |
| asleep (paused >60 s) | keys dim, cursor/LED hidden |

## Screens

* **Now Playing** — cover, tape label (title/artist/album), 7-segment
  `pos / dur`, the 16 step keys reused as a progress bar, VOL/ACC knobs with
  their dotted ring scales, START/STOP, format and SHUF/REP.
* **Library / Category / Browser** — the 808 header: title, eight miniature
  step keys (the selected one lights), the four-colour stripe and the list.
* **Settings** — same header with `SYSTEM SETUP`, the battery in the footer.

## Animation and redraw

Home: `refresh_ms=50`, `dyn=0,88,319,133` (LED + keys + overlays),
`dyn2=250,4,314,18` (time display), `dyn3=136,33,169,76` (BATT knob).
Now Playing: `dyn=0,130,319,146` (progress keys), `dyn2=0,90,319,169`
(knobs + bottom bar), `dyn3=136,56,250,72` (time display).

## Budget (measured)

| file | bytes / 4096 | sections / 52 |
|---|---|---|
| home.skin | 4095 | 47 |
| now_playing.skin | 3988 | 49 |
| library.skin | 1353 | 18 |
| library_cat.skin | 1357 | 18 |
| browser.skin | 1355 | 18 |
| settings.skin | 1384 | 18 |

Assets: `ring` 32x32 = **128 B** (used by the Now Playing knobs).

## Differences from the mockup

Budget cuts, all decorative, in order of the 4096 B limit:

* the knobs on **Home** have no dotted scale ring (each ring section costs
  ~59 B and the 16-key row is the priority); Now Playing and Settings do use
  the `ring` asset;
* the step keys are sharp rectangles (the 2 px corner radius cost 4 B per
  key);
* the active key is white-hot instead of a lightened group colour;
* the tape's third line is `PTN <n>` instead of `PTN 03 24/96`;
* in the low-battery state only step 1 blinks and the BATT knob turns red;
  the other keys are not dimmed;
* the pressed state is shown by the LED and the ACCENT knob rather than a
  whole-row flash (the 16 key colours would need another 240 B).

Screenshots and `compare_*.png` (mockup | engine) were produced by
`tools/theme_preview/run.sh` through the real interpreter and display driver.

```
tools/theme_preview/check_theme.py themes 808
tools/theme_preview/run.sh ../../themes 808 ../../docs/themes/808
tools/theme_preview/compare.py ~/rechord-themes-mockups-v3/ritmo \
    docs/themes/808
```
