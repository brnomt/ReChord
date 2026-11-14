# Custom bitmap fonts for themes — feasibility

Status: **report + working PC-side tool.** The engine change is specified but
**not enabled** (see §8 for why, and what the prototype already proves).

Scope: let a theme package ship its own bitmap font (`assets/font.bin`) and
let `text` meters name it (`font=<name>`), instead of every skin getting the
single built-in 8x8 font on a solid background.

---

## 1. What exists today

| fact | value | where |
|---|---|---|
| built-in font | 8x8 cell, ASCII 0x20..0x7E, 95 glyphs, 1-bit | `firmware/drivers/font.h`, `font8x8.c` |
| glyph table size | **760 B** (`95 x 8`) | `font8x8_data` in the map |
| drawing | opaque: every cell pixel written fg or bg, no kerning | `font_draw_text()` |
| text meter | `type=text` with `x y text color bg [maxc] [align]` | `theme_skin.c:render_section` |
| text measuring | `strlen * 8`, i.e. the cell is assumed fixed | `SKIN_FONT_W`, `align=right` |
| assets | 1-bit bitmaps: 8 max, 64x64, stride <= 8 B, **1 KiB cache** | `theme_assets.c` |
| asset cache RAM | 1024 B static (`s_cache`) | `.bss` |
| theme text budget | 4 KiB skin, 52 sections | `theme_skin.h` |
| flash headroom | `FW_CODE` 0x38000 = 229,376 B, used 214,492 B → **~14.5 KiB free** | `firmware_ap.ld` + `arm-none-eabi-size` |
| RAM headroom | `_bss_end 0x0303A6E0`, gate `0x0304A000` → **~62 KiB free** | `firmware_ap.ld` |

Consequence for authors, and the reason this came up: a theme cannot ship a
second typeface at all, and Glifo had to pack two digits per bitmap to fit
its clock into the 8-asset cap (`themes/glifo/theme.cfg`). It also cannot use
anything but a solid background behind text, because the blit is opaque.

---

## 2. Proposed format

`font_<name>=assets/font.bin` in `theme.cfg`, mirroring `asset_<name>`:

```
font_body=assets/body.bin
font_tiny=assets/tiny.bin

[title]
type=text
x=8
y=40
text=title
font=body
color=text
bg=background
```

`font=` is optional; without it the meter uses the built-in 8x8 exactly as
today, so every existing package renders identically.

The atlas is self-describing so the firmware needs no font engine:

```
offset  size  field
0       4     magic "RCHF"
4       1     version (1)
5       1     bits per pixel (1, 2 or 4)
6       1     first codepoint
7       1     last codepoint
8       1     cell height, px (1..32)
9       1     baseline offset from the cell top, px
10      1     max cell width, px (1..32)
11      1     flags (bit0 = proportional advances)
12      2     glyph count
14      2     total file size
16      8*n   per glyph: advance u8, xoff i8, yoff i8, width u8,
                        height u8, bitmap offset u16, pad u8
...           bitmaps, MSB-first, row stride = ceil(width * bits / 8)
```

- **Proportional advances.** `advance` per glyph is what buys the large
  proportional title the mockups asked for; `xoff`/`yoff` place the ink box.
- **1/2/4-bit.** 1-bit is an opaque two-colour blit (same as today). 2 and
  4-bit are alpha: `display_fb = mix(bg_or_under, color, alpha)`, so a glyph
  can be anti-aliased *or* drawn with a transparent background over a
  gradient — both impossible today.
- **Transparent bg** needs only a flag on the meter (`bg=none`) and an
  alpha-blended blit; alpha=0 pixels are skipped entirely.

`tools/mkfont.py` generates it (see §5). `tools/test_mkfont.py` re-reads the
packed file, checks every offset/size and confirms the ink survived, so the
encoder and decoder can never drift.

---

## 3. Size

Measured on this repo's machine with DejaVu Sans, ASCII 0x20..0x7E
(95 glyphs) — `python3 tools/test_mkfont.py`:

| px | bits | file | bytes/glyph | fits the 4 KiB asset cap |
|---|---|---|---|---|
| 8 | 1 | **1,284 B** | 13.5 | yes |
| 9 | 1 | **1,387 B** | 14.6 | yes |
| 9 | 2 | **1,818 B** | 19.1 | yes |
| 9 | 4 | **2,484 B** | 26.1 | yes |
| 10 | 2 | 1,869 B | 19.7 | yes |
| 11 | 4 | 3,062 B | 32.2 | yes (tight) |
| 9 | 2, Latin-1 (224 glyphs) | **4,626 B** | 20.6 | **no** |

Notes that matter for the design:

- A 9 px 4-bit ASCII atlas (2,484 B) is about **3.3x the whole current 760 B
  8x8 table**; the 1-bit 9 px one (1,387 B) is 1.8x. A theme shipping one font
  costs one read of that size per skin load, not per frame.
- **Latin-1 does not fit.** The device cap is a 4 KiB file, and 9 px 2-bit
  Latin-1 is 4,626 B — over it. So either the cap rises for fonts or v1 stays
  ASCII-only. ASCII is what every shipped theme actually draws (all UI text is
  ASCII by the no-Spanish/no-accent rule), so ASCII-only is the lazy correct
  v1.
- The `font.bin` is a *file*, so it does not have to live in the 1 KiB
  "small bitmap" cache; §6 proposes where it should live instead.

---

## 4. Where the glyphs live, and what the cache has to do

Three options; the recommendation is (B).

**(A) Load into the existing 1 KiB asset cache.** Impossible without
shrinking fonts: one 9 px 2-bit atlas is 1.8 KiB. Raising
`THEME_ASSET_CACHE_BYTES` to 4 KiB costs 3 KiB of the ~62 KiB `.bss` headroom
and would then also let all eight 64x64 1-bit bitmaps (8 x 512 B = 4 KiB)
co-exist with a font. Simple, but it wastes RAM on themes that ship no font.

**(B) Stream the atlas from storage, sector-aligned, once per skin load, into
a font-sized static buffer (recommended).** The atlas is read at
`theme_skin_load()`/`theme_pkg_apply()` time into one buffer sized to the
declared file (cap it at 4 KiB). Cost: one buffer, 0..4 KiB of `.bss`
depending on what the active theme declares — but the firmware cannot size a
static buffer per package, so it is a fixed 4 KiB reservation for all themes.
That is still only 6.5 % of the RAM headroom, costs 0 B of flash, and needs
**no per-frame I/O**. It also means the first `text` with `font=` after a
track change is free.

> If 4 KiB of always-reserved `.bss` is unacceptable, cap the atlas at 2 KiB
> and keep 4-bit 9 px (the largest useful case measured) just under it: the
> 9 px 1-bit and 2-bit atlases are 1.4/1.8 KiB, and 4-bit at 9 px is 2.5 KiB,
> so 2 KiB would force 2-bit. Reserve 3 KiB to keep every option.

**(C) Stream the glyph bitmap per draw from storage.** Rejected: at 20/60 fps
a title plus metadata is ~10..14 KiB of glyphs per frame, and reading 14 KiB
per frame from the eMMC path the album art already competes for would blow
the frame budget and the card's endurance long before the pixels did.

### Per-frame render cost

Measured on this machine (x86, `-O2`) as the ratio between an opaque blit
and an alpha blend over the same 200x12 box:

```
opaque   0.060 ns/px
alpha    0.300 ns/px   (5.0x)
```

The absolute numbers are irrelevant; the **5x** is the portable part, and it
is per *drawn pixel*. What a frame actually draws, for the busiest shipped
skins (counted characters x cell):

| skin | chars | 8x8 opaque | 9 px 1-bit (prop.) | 9 px 2-bit AA | 11 px 4-bit AA |
|---|---|---|---|---|---|
| oscilloscope np | 210 | 13,440 px | 9,355 px | 9,355 px | 13,975 px |
| cassette np | 192 | 12,288 px | 8,553 px | 8,553 px | 12,777 px |
| 808 np | 183 | 11,712 px | 8,152 px | 8,152 px | 12,178 px |
| aura np | 126 | 8,064 px | 5,613 px | 5,613 px | 8,385 px |
| glifo np | 80 | 5,120 px | 3,564 px | 3,564 px | 5,324 px |

- **1-bit proportional is a net win** (~30 % fewer pixels than the fixed 8x8
  cell, because a proportional box has less empty space). Same per-pixel cost
  as today.
- **2-bit alpha costs 5x per pixel** on the pixels it touches, so ~9.4k px
  costs about what 47k opaque pixels would. Against a 40 ms frame budget
  (25 fps) at 200 MHz with no cache that is a few milliseconds — acceptable
  on Now Playing (a static screen) but not on a 60 fps animated home.
- **4-bit at 11 px is the worst case**: 5x the per-pixel cost *and* more
  pixels than the current cell. It is only sensible on static screens.

At 20 fps (50 ms/frame) all three fit. At 60 fps (16 ms/frame) the skin's
own `dyn` split decides it: only the sections inside the declared dynamic
rects are redrawn, so a 4-bit title outside the rects costs nothing per frame.
A theme that puts animated anti-aliased text inside a `dyn` rect will drop
frames; the fix is theme-side (don't), not engine-side.

### Flash

The decoder is the whole engine change: ~250..400 B of `.text` (header
validate + a metrics lookup + a proportional row blit with an optional alpha
mix). Against ~14.5 KiB of free `FW_CODE` that is 3 % of the headroom — not a
risk, but it is the one budget that is genuinely tight on this device.

---

## 5. `tools/mkfont.py`

```
python3 tools/mkfont.py --font Inter-Regular.ttf --px 10 --bits 2 \
    --out themes/glifo/assets/body.bin --preview /tmp/body.png
```

- Rasterises ASCII 0x20..0x7E (or `--first/--last` up to 0xFF) at `--px`.
- Emits 1/2/4-bit with proportional advances, the `advance`/`xoff`/`yoff`
  ink box per glyph, and a bounds-checked header.
- `--preview` renders a contact sheet **from the packed bytes**, so the PNG
  proves the packing rather than the source font.
- Enforces the per-cell cap and warns when a file exceeds 4 KiB.
- One real fix found while building it: 1-bit thresholding erased thin
  glyphs (`'` at 8 px is a single grey column). The tool now keeps the
  brightest pixel when a glyph would otherwise be blank, and
  `test_mkfont.py` asserts every glyph with an advance has ink.

This sits next to the existing PC-side asset tooling (`tools/artpack/`,
`tools/theme_preview/`) and follows the same rule: the device does no
decoding and no writes.

---

## 6. Minimal engine change set

If it is greenlit, the smallest complete change is:

1. **`theme_pkg.h/.c`** — `font_<name>=<path>` declarations, capped at 2 per
   package, reusing the asset path validator; a declared-but-unreadable font
   rejects the package like an asset does. `theme_skin_font_decl_t` holds
   `{ name[16], path[48] }`.
2. **`theme_assets.c`** — one `font_assets_load()` that validates the header
   (magic, version, bits, codepoint range, every glyph offset inside the
   file) and copies the atlas into the reserved buffer. A malformed header
   drops just that font and logs, mirroring the option-line policy.
3. **`theme_skin.c`** — `font=<name>` on `text`; a proportional measure
   (`theme_skin_text_width(font, s)`) replacing `strlen * SKIN_FONT_W` for
   `align=right` and `maxc`; and a blit that walks the metrics.
4. **`display.c`** — one new primitive,
   `display_blit_alpha(x, y, w, h, stride, bits, bits_ptr, color, bg, transparent)`,
   used only by the font path. Keeping it here (not in the skin engine) keeps
   the engine hardware-agnostic and host-testable.

Not needed: any new section type, any change to the 52-section parser, any
change to the option system, any new file on the device.

---

## 7. Recommendation

**Go for 1-bit proportional ASCII fonts. Hold 2/4-bit.**

- **1-bit proportional** is a straight win on every axis that matters: fewer
  pixels per frame than the fixed cell, 1.4 KiB per atlas, ~300 B of flash,
  and it delivers the one thing themes actually need from a custom font —
  a large proportional title with a real typeface instead of `maxc=22` on a
  rigid 8x8 grid. It also **retires Glifo's 2-digits-per-bitmap packing**:
  a 19 px 1-bit clock atlas is ~1.5 KiB and a single `text` section replaces
  the five paired-digit bitmaps plus two masking strips.
- **2-bit alpha** is worth it on Now Playing only. Its 5x per-pixel cost is
  fine on a static screen and is the only way to get anti-aliased text or
  transparent text over a gradient. Recommend it *behind the same format*
  (the header already carries `bits`), enabled per theme, with the
  "don't animate it" guidance in `THEMING.md` rather than a firmware guard.
- **4-bit** has no case on this panel at this budget: 11 px 4-bit costs more
  pixels *and* 5x per pixel, and the screen is 170 px tall. Skip it; the
  format keeps the door open.
- **Latin-1** does not fit the current 4 KiB file cap at 2-bit (4,626 B
  measured). Ship ASCII-only; the project's own no-accent rule makes Latin-1
  dead weight today.

**Why the engine change is not in this PR.** It is ~350 B of `.text`, a
4 KiB `.bss` reservation and a new `display` primitive on a device that has
bricked twice over a few KiB of layout, and it cannot be verified without the
hardware in hand (the device was not connected while this was written — see
§9). Landing the format, the generator, its self-check and the size/perf
analysis first is the cheap half; the engine change is a separate PR once
someone can flash and look at a proportional title on the actual panel.

**Estimated effort for the follow-up:** the engine change set in §6 is one
afternoon of code plus one flash cycle; the atlas/format/tooling half is
already done and tested here.

---

## 8. What this PR includes

- `tools/mkfont.py` — TTF/OTF → `font.bin`, 1/2/4-bit, proportional metrics,
  `--preview` that reads its own output.
- `tools/test_mkfont.py` — 7 cases: header sanity, offset bounds, no invisible
  glyphs, proportional advances, and the ASCII/Latin-1 size boundary the
  recommendation rests on. `python3 tools/test_mkfont.py` passes.
- This report.
- No firmware change: no new `.bin`, no new `.bss`, no new flash.

## 9. Not verified

- **The font engine change was never flashed** (it is not in this PR). The
  Part 1 feature *was* verified on the device: the new image installed over
  the stock updater and booted, logging `boot: opt face=0/2`, and a
  `theme_opts=glifo:1;` selection written into `rechord.cfg` came back as
  `opt face=1/2` after a power cycle.
- The per-frame numbers are measured on the host and scaled by the *ratio*
  between the two blit paths, not profiled on the M3. The instruction-count
  method used for the earlier skin work (`docs/HANDOVER.md` §CR, ~0.80M
  instructions per dynamic frame) is the right tool for a real number, and
  should be run before committing to 4-bit anywhere.
