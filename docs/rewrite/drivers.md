# ReChord rewrite - DRIVERS workstream

> Clean-room drivers for the FiiO Echo Mini (Rockchip RKNanoC, dual
> Cortex-M3). Scope of this phase: display + font. All code here is ours,
> written from hardware CONTRACTS extracted from the reference firmware
> image (register/command values and wire behavior = facts) plus the
> vendor SDK's documented register maps. No third-party CFW code is
> copied or paraphrased.
>
> Status: **display + font + panel init landed, host-tested,
> cross-compiled.** Milestone 1b done: the DCS init sequence is extracted
> and implemented (`panel_init_rknanoc.c`), and the target pixel path is
> corrected against the extracted facts (row offset + transport; see
> sections 4-6). What remains is hardware bring-up verification (section
> 8).

## 1. What exists

```
firmware/drivers/
|-- display.h            public display API (contract below)
|-- display.c            RGB565 framebuffer driver, DCS row-flush path
|-- panel_init_rknanoc.c EXTRACTED panel init sequences (2 variants),
|                        panel_init_sequence() / panel_init_run_variant()
|-- font.h               font renderer API (8x8 cell, ASCII 0x20..0x7E)
|-- font.c               renderer: glyph lookup + clipped opaque blitting
|-- font8x8.c            glyph data (generated, our own designs)
|-- panel_io.h           transport abstraction: cmd/data/pixel/delay
|-- panel_io_rknanoc.c   TARGET backend: GPIO0 bit-banged 8080-style bus
|                        (extracted hardware facts, see section 4)
|-- panel_io_sim.c/.h    HOST backend: simulated DCS panel + PPM writer
|-- tools/gen_font8x8.py regenerates font8x8.c from the glyph designs
|-- drivers.mk           build wiring (common/target/host source lists)
`-- tests/
    |-- test_display.c    geometry, fill_rect bounds/clip, row math,
    |                     init wire trace, row-offset mapping
    |-- test_panel_init.c byte-exact checks of BOTH extracted init
    |                     variants against the documented tables
    |-- test_font_ppm.c   glyph contract, font clipping, PPM rendering
    |-- test_util.h       tiny CHECK harness
    `-- run_tests.sh      cc -std=c99 -Wall -Werror build + run
```

`panel_init_rknanoc.c` sits in the COMMON source list (not the
target-only one): it emits through `panel_io.h` only, so the exact same
init tables are exercised by the host tests and shipped in the target
image.

## 2. API contract

### 2.1 display.h (stable - other workstreams code against this)

```c
#define DISPLAY_W 320            /* hardware contract: 320 px wide       */
#define DISPLAY_H 170            /* SINGLE KNOB: observed boot-area rows */
#define PANEL_ROW_OFFSET 35      /* extracted: logical row y = panel y+35*/
#define DISPLAY_ROW_BYTES 640    /* hardware contract: 640 B/row pitch   */

void display_init(void);
void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t rgb565);
void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg);
void display_flush_rows(int y0, int y1);
void display_clear(uint16_t rgb565);
void panel_init_sequence(void);          /* extracted DCS init stream    */
int  panel_init_run_variant(int variant);/* 0 = A, 1 = B; test/bring-up  */
```

Semantics (deliberate choices, all asserted by tests):

- **Model**: draw calls write an in-RAM RGB565 framebuffer only;
  `display_flush_rows()` pushes rows to the panel. Nothing is visible on
  hardware until a flush.
- **Coordinates are inclusive** on all edges (matches the observed
  reference usage `x1 = 0x13F` = `DISPLAY_W - 1`). Full screen =
  `display_fill_rect(0, 0, DISPLAY_W - 1, DISPLAY_H - 1, c)`.
- **Everything clips** to `0..DISPLAY_W-1 / 0..DISPLAY_H-1`. Empty or
  reversed rectangles and flush ranges are no-ops; negative indexes clamp.
- `display_flush_rows(y0, y1)` sends full-width rows via DCS
  `0x2A CASET (0 .. W-1)`, `0x2B RASET`, `0x2C RAMWR` + pixels (RGB565,
  high byte first). **Row mapping (extracted)**: RASET parameters are
  `y0 + PANEL_ROW_OFFSET .. y1 + PANEL_ROW_OFFSET`, exactly the
  reference's flush routine (`RASET = row + 0x23`). Logical row 0 is
  physical panel row 35.
- `display_init()` = transport init + `panel_init_sequence()` + clear to
  black + full flush (mirrors the reference bring-up contract "panel
  init + clear screen + flush").
- `DISPLAY_H` (170) is the observed boot-area height (rows 0..0xA9) and
  is corroborated by the extracted init window (0x2B rows 35..204 = 170
  rows). One define; framebuffer, clipping and flush math all derive
  from it.

### 2.2 font.h

```c
#define FONT_CELL_W 8
#define FONT_CELL_H 8
#define FONT_FIRST_CODE 0x20
#define FONT_LAST_CODE  0x7E

typedef struct { uint16_t *fb; int w, h, pitch_px; } font_surface_t;

const uint8_t *font_glyph(uint8_t ch);
void font_draw_text(const font_surface_t *dst, int x, int y,
                    const char *s, uint16_t fg, uint16_t bg);
```

- Fixed 8x8 cell, ASCII 0x20..0x7E (95 glyphs), no kerning; out-of-range
  codes render as `?`. Opaque blit (every cell pixel written: fg/bg),
  clipped per pixel against the surface. NULL args are safe no-ops.
- Glyph bitmaps are **our own designs** (5x7 shapes in the 8x8 cell),
  drawn as ASCII art in `tools/gen_font8x8.py` and encoded as data:
  8 bytes/glyph, top row first, **bit 7 = leftmost pixel**. Provenance:
  original work, public-domain-style. Nothing from any CFW or font file.
- `font.c` never touches hardware: the display driver passes its
  framebuffer as a `font_surface_t`; tests pass plain arrays.
- Note: the reference ships a 6x11 font (stride 0xB, ASCII from 0x20,
  base literal at RAM 0x03052310 -> 0x030600B1). Ours is deliberately
  different (our own data); a 6x11 renderer can be added later as a
  `font_surface_t`-compatible alternative.

### 2.3 panel_io.h (transport abstraction)

```c
void panel_io_init(void);
void panel_io_send_cmd(uint8_t cmd);
void panel_io_send_data(uint8_t data);
void panel_io_send_pixel(uint16_t rgb565);   /* 2 wire bytes, HIGH first */
void panel_io_delay_ms(uint16_t ms);
```

Two interchangeable backends:

- **`panel_io_rknanoc.c`** (target): bit-banged 8080-style bus on GPIO0
  (section 4 - extracted hardware facts, NOT the SDK's VOP block).
- **`panel_io_sim.c`** (host tests): models a DCS panel - CASET/RASET
  window registers, RAMWR cursor with right-edge wrap, byte-pair pixel
  assembly (high byte first), strict bounds checking with a fault
  counter (`panel_io_sim_overflow()`), a command+argument log for
  wire-level assertions, and `panel_io_sim_write_ppm()`. The simulated
  framebuffer models the visible band only: RASET carries PHYSICAL rows
  (logical + PANEL_ROW_OFFSET) on the wire, while
  `panel_io_sim_pixel()` / the PPM dump use logical rows 0..H-1.

## 3. Panel init sequence (EXTRACTED, byte-exact)

### 3.1 Extraction method

From the reference RAM image (`file offset = RAM addr - 0x0304F490`),
disassembled with `arm-none-eabi-objdump -D -b binary -marm -Mforce-thumb
--adjust-vma=0x0304F490`. Findings, corrected against earlier
second-hand notes:

- `draw_init` @ 0x030503EC does **not** carry the init bytes. It is
  one-shot (once-flag at 0x0300FC40): clip-rect reset (0x03051EA8),
  `fill_rect(0,0,319,169,0)`, tail-call flush-all (0x03051DD0).
- The **DCS init lives in the bring-up function at 0x03051F14** (called
  from 0x03050B36, i.e. board init before any drawing). It resets the
  panel, probes a 4-byte panel ID, then emits the command stream through
  the command-write routine 0x03051c8c (which wraps the byte-write
  routine 0x03051c6c; section 4).
- The command/parameter payloads are const bytes in RAM; the routine
  passes `cmd, ptr, len` per step (literal `movs` for cmd/len, `ldr` for
  ptr). We transcribed the pointed-to bytes - values below are facts.

### 3.2 Variant selection (as extracted)

The bring-up routine reads a 4-byte panel ID from the GPIO0 input port
(0x40160050, bits 16..23, 4 sequential reads) and dispatches:

    (id & 0x00FF0000) == 0x00E80000 -> variant A
    anything else                   -> variant B

Additionally, if a GPIO(2.9) read returns 0 the ID is forced to
0x66666666, which also selects variant B. We cannot probe yet (no bus
read in panel_io), so `panel_init_rknanoc.c` selects at compile time:
`PANEL_INIT_VARIANT` 0 = A, 1 = B (default **1 = B**, see the caveat in
section 8). `panel_init_run_variant(int)` can send either stream at
runtime.

Both variants: one delay only - **120 ms after 0x11 sleep out** - and
both end with `0x21 INVON`, `0x29 DISPON`, `0x2C RAMWR` (no pixel
data), after which the reference clears the framebuffer and flushes
(our `display_init()` does the same).

### 3.3 Variant A byte table (ID byte 0xE8)

| # | cmd  | n  | data bytes                       | meaning (inferred)                |
|---|------|----|----------------------------------|-----------------------------------|
| 1 | 0x11 | 0  | -                                | SLPOUT, then 120 ms delay         |
| 2 | 0xDF | 4  | 5A 69 02 01                      | vendor CMD2 unlock (ST7789V2 "Enable Command 2" password) |
| 3 | 0x36 | 1  | 60                               | MADCTL: MX=1 MV=1 (landscape)     |
| 4 | 0x3A | 1  | 05                               | COLMOD: 16 bpp (DBI)              |
| 5 | 0xB2 | 5  | 0B 0B 00 33 33                   | porch control                     |
| 6 | 0xB7 | 1  | 75                               | gate control                      |
| 7 | 0xBB | 1  | 28                               | VCOMS                             |
| 8 | 0xC0 | 1  | 2C                               | LCMCTRL                           |
| 9 | 0xC2 | 1  | 01                               | VDVVRHEN                          |
|10 | 0xC3 | 1  | 1F                               | VRHS                              |
|11 | 0xC5 | 1  | 2A                               | VDVS                              |
|12 | 0xC6 | 1  | 13                               | FRCTRL2                           |
|13 | 0xD0 | 1  | A7                               | PWCTRL1 (1-arg form)              |
|14 | 0xD0 | 2  | A4 A1                            | PWCTRL1 (2-arg form, re-sent)     |
|15 | 0xD6 | 1  | A1                               | gate EQ                           |
|16 | 0xE0 | 14 | F0 05 0A 06 06 03 2B 32 43 36 11 10 2B 32 | gamma (positive)        |
|17 | 0xE1 | 14 | F0 08 0C 0B 09 24 2B 22 43 38 15 16 2F 37 | gamma (negative)        |
|18 | 0x21 | 0  | -                                | INVON (display inversion on)      |
|19 | 0x29 | 0  | -                                | DISPON (display on)               |
|20 | 0x2C | 0  | -                                | RAMWR (empty - caller clears+flushes) |

### 3.4 Variant B byte table (fallback)

| # | cmd  | n  | data bytes                       | meaning (inferred)                |
|---|------|----|----------------------------------|-----------------------------------|
| 1 | 0x11 | 0  | -                                | SLPOUT, then 120 ms delay         |
| 2 | 0x36 | 1  | 60                               | MADCTL: MX=1 MV=1 (landscape)     |
| 3 | 0x3A | 1  | 05                               | COLMOD: 16 bpp (DBI)              |
| 4 | 0xA4 | 2  | 44 44                            | vendor (UNKNOWN semantics)        |
| 5 | 0xB2 | 5  | 0C 0C 00 33 33                   | porch control                     |
| 6 | 0xB7 | 1  | 35                               | gate control                      |
| 7 | 0xBB | 1  | 1A                               | VCOMS                             |
| 8 | 0xC0 | 1  | 2C                               | LCMCTRL                           |
| 9 | 0xC2 | 1  | 01                               | VDVVRHEN                          |
|10 | 0xC3 | 1  | 0B                               | VRHS                              |
|11 | 0xC4 | 1  | 20                               | VDVS                              |
|12 | 0xC6 | 1  | 0F                               | FRCTRL2                           |
|13 | 0xD0 | 2  | A4 A1                            | PWCTRL1                           |
|14 | 0xD6 | 1  | A1                               | gate EQ                           |
|15 | 0xE0 | 14 | F0 06 0B 07 07 24 2E 32 46 37 13 13 2D 33 | gamma (positive)        |
|16 | 0xE1 | 14 | F0 02 06 09 08 05 29 44 42 38 14 14 2A 30 | gamma (negative)        |
|17 | 0x2A | 4  | 00 00 00 EF                      | CASET cols 0..239 (see note)      |
|18 | 0x2B | 4  | 00 23 00 CC                      | RASET rows 35..204 (the band!)    |
|19 | 0x21 | 0  | -                                | INVON                             |
|20 | 0x29 | 0  | -                                | DISPON                            |
|21 | 0x2C | 0  | -                                | RAMWR (empty)                     |

### 3.5 Notes and uncertainties on the sequences

- **Bytes are exact** (transcribed from the reference's const data and
  re-verified byte-for-byte against the tables in
  `tests/test_panel_init.c`). **Meanings are inferences** from the
  ST7789-family command set (the 0xDF `5A 69 02 01` unlock is a known
  ST7789V2 idiom); the actual controller part number is unverified.
- `0x3A = 0x05` (not the commonly seen 0x55): both encode 16 bpp on the
  DBI (MCU) interface; this bus is 8080-style, so 0x05 is consistent
  with the RGB565 pixel path. Kept verbatim.
- Variant B's `0x2A = 00 00 00 EF` sets columns 0..239, which disagrees
  with the runtime flush path (columns 0..319, section 5). Every flush
  re-programs both windows, so this init-time value is dead at runtime;
  kept verbatim as an extracted fact (likely a portrait-config leftover).
  Its `0x2B = 00 23 00 CC` (rows 35..204) exactly matches the flush
  path's +35 row mapping and the 170-row band - strong corroboration of
  the geometry.
- `0x2C RAMWR` with no pixel data at the end of both streams is really
  sent by the reference (verified in the disassembly: len = 0). Harmless.
- Which variant the Echo Mini's panel needs is **UNVERIFIED** (section
  8).

## 4. Transport (EXTRACTED - target backend)

The reference drives the panel over a **GPIO bit-banged 8080-style
8-bit parallel bus**, not the SDK's VOP MCU interface. (An earlier
revision of `panel_io_rknanoc.c` assumed VOP `0x60070000`
VopMcuCmd/VopMcuData from the SDK docs; the reference's byte-write
routine 0x03051c6c writes `GPIO0` port registers instead - `GPIO0_BASE
= 0x40160000` per the SDK's `driver/hw_memap.h` - so the target backend
now follows the reference's proven path. The VOP route is unused and
unverified.)

Extracted bus facts:

| Signal        | GPIO            | Mechanism                                    |
|---------------|-----------------|----------------------------------------------|
| D0..D7        | GPIO0.16..23    | byte value in DR bits 16..23                 |
| WR            | GPIO0.24        | DR bit 24 strobe (latch on rising edge)      |
| D/C           | GPIO0.25        | DR bit 25: 0 = command byte, 1 = data byte   |
| CS/RD ctl     | GPIO0.12        | held high outside ID probing                 |
| RESET         | GPIO2.15        | active-low pulse (see below)                 |

- **One bus byte = 3 writes** to GPIO0 SWPORTA_DR (0x40160000): value
  with WR=0, again with WR=0, then with WR=1. The reference keeps the
  remaining DR control bits in a RAM shadow (0x03006030) and ORs the
  byte/D/C bits in; `panel_io_rknanoc.c` does the same.
- **Direction setup** (extracted): GPIO0 DDR bits 0x03FF9000 = pins 12,
  15, 16..25 as outputs.
- **Reset pulse** (extracted helper 0x03051cb0): GPIO2.15 high 50 ms,
  low 150 ms, high, then a settle delay (30 ms first call, 120 ms
  second call). The reference pulses reset twice, around its ID probe;
  `panel_io_init()` reproduces both pulses back-to-back with the
  extracted settle times.
- **ID probe** (extracted, NOT implemented): GPIO0.12 toggled low/high,
  then the input port 0x40160050 bits 16..23 read 4 times, assembled
  big-endian into the ID word. Would need a `panel_io` bus-read
  primitive to implement.
- **`panel_io_send_pixel`** is two byte writes, high byte first - the
  reference's flush loop sends each halfword as exactly two byte writes.
- **Delay**: the reference uses a timer-based delay (0x03051b04 loops
  over a 1000-us primitive); not decoded yet. `panel_io_delay_ms()` is
  an uncalibrated busy loop (TODO) - affects init timing margins only.
- The reference's bring-up also pokes GPIO(2,9) (reads it for the
  variant strap) and a 12-entry GPIO config table (0x030600A5..0x030600B0:
  `10 11 12 13 14 15 16 17 18 19 0F 0C`); those pins are NOT decoded
  (probably power/backlight). Unverified, see section 8.

## 5. Target pixel path (verified against the extracted facts)

Reference primitives and their verified semantics (this is the contract
`display.c` + `panel_io_rknanoc.c` implement):

| Reference addr | Role | Verified semantics |
|---|---|---|
| 0x03051c6c | byte write (the "pixel write") | one byte to the GPIO0 bus (section 4); D/C = data for pixels |
| 0x03051c8c | command write | `cmd, ptr, len`: command byte (D/C=0) then len data bytes (D/C=1) |
| 0x03051D40 | flush_rows(y0,y1) | clamp y1<=169 / y0>=0; CASET `00 00 01 3F` (cols 0..319); RASET `00 (y0+23) 00 (y1+23)`; 0x2C; pixels from FB, halfword -> **high byte first** |
| 0x03051EC0 | fill_rect | clip against a 4-word rect, 16-bit stores, FB base + y*640 + x*2 |
| 0x03052244 | draw_text | 6x11 glyphs (stride 0xB) from base literal 0x03052310 (= 0x030600B1), FB base literal 0x03052314 (= 0x03006038), row pitch 640 B |
| 0x030503EC | draw_init | once-flag, clip reset (0x03051EA8), fill_rect(0,0,319,169,0), flush all |
| 0x03051dd0 | flush all | tail-call flush_rows(0, 169) |

Framebuffer contract (verified): base 0x03006038 in the reference,
**640 bytes per row** (320 px x 2 B), RGB565 stored little-endian in
RAM and sent high byte first on the wire.

**Bugs found and fixed in our code** (written from docs before this
extraction):

1. **Row mapping**: `display_flush_rows()` sent RASET `y0..y1`; the
   reference sends `y0+35..y1+35`. Fixed via `PANEL_ROW_OFFSET` (35) in
   `display.h`, applied in `display.c`, modeled by the sim (which maps
   the physical band back to logical rows), and asserted byte-exactly in
   `tests/test_display.c` (full-screen RASET = `00 23 00 CC`).
2. **Target transport**: `panel_io_rknanoc.c` targeted the SDK VOP MCU
   registers (0x60070000); the reference's proven byte path is the
   GPIO0 bit-bang above. Rewritten to the extracted GPIO0 protocol.
3. Verified correct and unchanged: CASET full width (0..319), RGB565
   high-byte-first pixel order, 320px/640B row pitch, inclusive rect
   coordinates, flush clamping (matches 0x03051D40), draw_init =
   init + clear + flush.

## 6. Display contract address map (as given -> where it lands)

| Given fact | Verified at | Our code |
|---|---|---|
| command write 0x03051c8c | `cmd,ptr,len` -> GPIO0 byte writes | `panel_io_send_cmd/data` (both backends) |
| pixel write 0x03051c6c | one GPIO0 byte write, D/C=1 | `panel_io_send_pixel` (2 bytes, hi first) |
| draw_init 0x030503EC | clear+flush, no DCS bytes | `display_init()` |
| fill_rect 0x03051EC0 | 640 B/row FB fills | `display_fill_rect()` |
| draw_text 0x03052244 | 6x11 font, FB literals above | `display_draw_text()` (our 8x8 font) |
| flush_rows 0x03051D40 | CASET/RASET/RAMWR, row+35 | `display_flush_rows()` |
| FB pointer 0x03052314 | literal -> 0x03006038, 640 B/row | static `display_fb` (pitch `DISPLAY_ROW_BYTES`) |
| font pointer 0x03052310 | literal -> 0x030600B1, stride 0xB | `font8x8.c` (own data, different metrics) |

## 7. Verification results (2026-10-09)

### 7.1 Host build + tests - `firmware/drivers/tests/run_tests.sh`

```
test_display: 69 checks, 0 failures
test_panel_init: 204 checks, 0 failures
test_font_ppm: 358 checks, 0 failures
all host tests passed (PPM artifact: .../font_render.ppm)
```

Compiled with `cc -std=c99 -Wall -Werror`. Coverage now includes:

- geometry contract: `DISPLAY_W==320`, `DISPLAY_H==170`, row pitch 640 B,
  `PANEL_ROW_OFFSET==35`
- `display_init` wire trace: starts with 0x11 sleep-out (init stream),
  ends with one 0x2A/0x2B/0x2C flush triple whose CASET bytes are
  `00 00 01 3F` and RASET bytes `00 23 00 CC`, window + pixels + zero
  overflow
- **init streams: both variants compared byte-for-byte** (command
  bytes, every parameter byte, the 120 ms sleep-out delay, nothing
  emitted for bad variant numbers; the compiled default must match one
  of the two extracted streams)
- `fill_rect`: inclusive corners, all four clip edges, reversed rect and
  off-screen rects draw nothing, single-pixel rect, full-screen clear
- row math: negative `y0` clamps to 0, huge `y1` clamps to 169, wire
  RASET parameters verified at the byte level and as physical windows
  (rows + 35), unflushed rows stay untouched, empty/reversed ranges emit
  zero command bytes
- font: glyph bytes, `?` fallback, 95 glyphs, pixel-exact blit at
  negative offsets, canary padding, PPM artifact byte-verified

### 7.2 Cross build - `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -Wall -Werror`

```
display.c font.c font8x8.c panel_init_rknanoc.c panel_io_rknanoc.c
  -> all clean

   text    data     bss     dec     filename
    328       0  108800  109128     display.thumb.o      (bss = framebuffer)
    192       0       0     192     font.thumb.o
    760       0       0     760     font8x8.thumb.o
    960       0       0     960     panel_init_rknanoc.thumb.o
    252       0       4     256     panel_io_rknanoc.thumb.o
```

Host `cc -std=c99 -Wall -Werror -c` is equally clean for every TU
including both panel_io backends.

## 8. Open items / hardware-unverified

1. **Which init variant the Echo Mini panel needs (the main unknown).**
   The reference probes a panel ID at runtime; we pick at compile time.
   Default is variant B (`PANEL_INIT_VARIANT 1`): its 0x2B window
   (rows 35..204) matches this device's extracted +35 band exactly, and
   it is the reference's fallback path. If hardware shows no image,
   wrong colors/gamma, or an inverted image, try variant A (or call
   `panel_init_run_variant(0)` from bring-up code). Implementing the ID
   probe (GPIO0 bus read via 0x40160050) would remove the guess.
2. **Register meanings are inferred** (ST7789-family); the bytes are
   exact. 0xDF and 0xA4 semantics in particular are unknown. MADCTL
   0x60 = landscape orientation is an inference; if the image comes up
   rotated/mirrored on hardware, that byte is the knob.
3. **GPIO bring-up not fully decoded**: the reference touches GPIO2.9
   (reads it; forces the fallback variant when 0) and a 12-pin GPIO
   config table (power/backlight pins, probably). If the panel stays
   dark after the init sequence on hardware, check power/backlight
   enable pins first.
4. **`panel_io_delay_ms()` calibration**: busy loop scaled for 200 MHz;
   init timing margins (sleep-out 120 ms, reset pulse widths) are
   approximate. Replace with the timer delay once decoded.
5. **Reset pulse ordering**: we run the reference's two reset pulses
   back-to-back (50/150/30 ms then 50/150/120 ms) because the ID probe
   between them is not implemented. If bring-up misbehaves, a single
   pulse + 120 ms settle is the conservative variant to try.
6. **Framebuffer RAM budget**: 170 * 640 B = 108,800 B of `.bss`;
   placement belongs to the startup/linker workstream.
7. **Panel real size**: 320x170 is the visible band of what is likely a
   320x240 landscape controller (35 rows hidden top and bottom).
   `DISPLAY_H`/`PANEL_ROW_OFFSET` are the two knobs if measurement says
   otherwise.
8. **Text formatting** (`printf`-style) stays out of drivers scope (see
   the architecture doc); dirty-row tracking in `fill_rect` and 6x11
   font data remain future work.
