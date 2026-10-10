# ReChord rewrite — DRIVERS workstream

> Clean-room drivers for the FiiO Echo Mini (Rockchip RKNanoC, dual
> Cortex-M3). Scope of this phase: display + font. All code here is ours,
> written from the documented hardware contracts
> (`docs/re/route-b-minimum.md` §3) and the vendor SDK's documented
> register maps. No third-party CFW code is copied or paraphrased.
>
> Status: **display + font landed, host-tested, cross-compiles.** The one
> blocking hardware unknown (panel power-on init sequence) is isolated
> behind a documented no-op hook — see *Open items*.

## 1. What exists

```
firmware/drivers/
├── display.h            public display API (contract below)
├── display.c            RGB565 framebuffer driver, DCS row-flush path,
│                        panel_init_sequence() hook (TODO inside)
├── font.h               font renderer API (8x8 cell, ASCII 0x20..0x7E)
├── font.c               renderer: glyph lookup + clipped opaque blitting
├── font8x8.c            glyph data (generated, our own designs)
├── panel_io.h           transport abstraction: cmd/data/pixel/delay
├── panel_io_rknanoc.c   TARGET backend: VOP MCU register interface
├── panel_io_sim.c/.h    HOST backend: simulated DCS panel + PPM writer
├── tools/gen_font8x8.py regenerates font8x8.c from the glyph designs
└── tests/
    ├── test_display.c   geometry, fill_rect bounds/clip, row math
    ├── test_font_ppm.c  glyph contract, font clipping, PPM rendering
    ├── test_util.h      tiny CHECK harness
    └── run_tests.sh     cc -std=c99 -Wall -Werror build + run
```

## 2. API contract

### 2.1 display.h (stable — other workstreams code against this)

```c
#define DISPLAY_W 320            /* hardware contract: 320 px wide       */
#define DISPLAY_H 170            /* SINGLE KNOB: observed boot-area rows */
#define DISPLAY_ROW_BYTES 640    /* hardware contract: 640 B/row pitch   */

void display_init(void);
void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t rgb565);
void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg);
void display_flush_rows(int y0, int y1);
void display_clear(uint16_t rgb565);
void panel_init_sequence(void);  /* power-on init hook — no-op for now   */
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
  `0x2A CASET (0 .. W-1)`, `0x2B RASET (y0 .. y1)`, `0x2C RAMWR` + pixels
  (RGB565, high byte first), exactly the documented contract.
- `display_init()` = transport init + `panel_init_sequence()` + clear to
  black + full flush (mirrors the observed one-shot `draw_init` contract
  "panel_init + clear screen + flush").
- `DISPLAY_H` (170) is the **observed boot-area height** (rows 0..0xA8,
  `docs/re/route-b-minimum.md` §3). It is one define; the framebuffer,
  clipping and flush math all derive from it. Raise it if the panel turns
  out to be taller (see *Open items*).

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

### 2.3 panel_io.h (transport abstraction)

```c
void panel_io_init(void);
void panel_io_send_cmd(uint8_t cmd);
void panel_io_send_data(uint8_t data);
void panel_io_send_pixel(uint16_t rgb565);   /* 2 wire bytes, HIGH first */
void panel_io_delay_ms(uint16_t ms);
```

Two interchangeable backends:

- **`panel_io_rknanoc.c`** (target): the VOP MCU-panel interface at
  `0x60070000` (register map per vendor SDK `driver/vop/hw_vop.h`,
  base per `driver/hw_memap.h`). Mirrors the SDK's documented register
  semantics: one command byte per `VopMcuCmd` write, one parameter byte
  OR one whole RGB565 pixel per `VopMcuData` write. Bring-up TODOs are
  marked in the file (clock/pinmux, FIFO pacing, delay calibration).
- **`panel_io_sim.c`** (host tests): models a DCS panel — CASET/RASET
  window registers, RAMWR cursor with right-edge wrap, byte-pair pixel
  assembly, strict bounds checking with a fault counter
  (`panel_io_sim_overflow()`), a command log for wire-level assertions,
  and `panel_io_sim_write_ppm()` to dump the simulated panel as PPM.
  Simulated geometry equals `DISPLAY_W/DISPLAY_H` (enforced with #error).

## 3. Verification results (2026-10-09)

### 3.1 Host build + tests — `firmware/drivers/tests/run_tests.sh`

```
test_display: 57 checks, 0 failures
test_font_ppm: 358 checks, 0 failures
all host tests passed (PPM artifact: .../host-tests/font_render.ppm)
```

Compiled with `cc -std=c99 -Wall -Werror` (gcc 15.2.0). Coverage:

- geometry contract: `DISPLAY_W==320`, `DISPLAY_H==170`, row pitch 640 B
- `display_init` wire trace: exactly one `2A/2B/2C` triple, window
  `0,0..319,169`, zero out-of-window pixels
- `fill_rect`: inclusive corners, all four clip edges, reversed rect and
  off-screen rects draw nothing, single-pixel rect, full-screen clear
- row math: negative `y0` clamps to 0, huge `y1` clamps to 169, wire
  RASET parameters verified (`ys/ye`), unflushed rows stay untouched,
  empty/reversed ranges emit zero command bytes
- font: glyph bytes for `'A'`/`'!'`/space, `?` fallback for 0x00/0x7F/0xFF,
  all 95 glyphs present and non-empty except space
- font blitting: pixel-exact render at offset (-3,-3) checked against the
  glyph bitmaps, canary padding proves no out-of-surface writes
- PPM artifact: exact `P6` header + size, RGB565→RGB888 spot pixels
  (foreground from the `'R'` glyph, background at (300,160)) verified
  byte-level in the file and on the simulated panel

The PPM render (headline `ReChord 0.9.2` + full ASCII sheet) was also
inspected visually: all glyphs legible.

### 3.2 Cross build — `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c`

```
display.c  font.c  font8x8.c  panel_io_rknanoc.c   -> all clean
(also compiled with -Wall -Werror on top of the required flags)

   text    data     bss     dec     filename
    316       0  108800  109116     display.thumb.o   (bss = framebuffer)
    192       0       0     192     font.thumb.o
    760       0       0     760     font8x8.thumb.o
     60       0       0      60     panel_io_rknanoc.thumb.o
```

Host `cc -std=c99 -Wall -Werror -c` is equally clean for every TU
including both panel_io backends.

## 4. Open items

1. **Panel power-on init sequence (the one real unknown).** The exact
   DCS sequence for the Echo Mini panel (typically `0x11` sleep-out +
   delay, vendor power/gamma commands, `0x3A` pixel format `0x55`,
   `0x29` display-on) must still be **extracted from the reference
   firmware's `panel_init` @ 0x03051EA8** (see `route-b-minimum.md` §3).
   Policy: the extraction yields a hardware *contract* (a command/parameter
   list), which we then encode as rows of `panel_init_table[]` in
   `display.c` — no reference code is copied. Until then
   `panel_init_sequence()` is a **safe no-op** (the table holds only its
   terminator): the driver works against an already-initialized panel
   (hybrid boot path) and the host simulator. The hook is wired end-to-end
   (cmd + params + per-step delay).
2. **DISPLAY_H 170 vs full panel height.** 170 rows is the observed boot
   area; the controller's real GRAM height stays unknown until item 1 is
   answered or measured on hardware. Single define to change.
3. **VOP pacing on target.** The SDK paces transfers with
   `VopMcuStatus`/`VopMcuFIFOWaterMark` and VOP "split" mode around rows;
   whether bare register writes are safe at 200 MHz needs hardware
   validation. If pacing is needed, add a row-boundary hook to
   `panel_io.h` (sim would make it a no-op). Also: VOP clock/pinmux
   bring-up currently assumes earlier boot stages did it
   (`panel_io_rknanoc.c` TODO), and `panel_io_delay_ms()` is an
   uncalibrated busy loop.
4. **Frame buffer RAM budget.** The full framebuffer is 170 * 640 B =
   108,800 B of `.bss` (measured above). The clean-room link must place it
   in a RAM window with that much room (the reference's own framebuffer
   contract suggests this is expected, but placement belongs to the
   startup/linker workstream).
5. **Text formatting.** `display_draw_text` takes plain strings only;
   `printf`-style formatting (`vsnprintf`) belongs to the services layer
   (it is explicitly out of drivers scope per the architecture doc).
6. **Build wiring.** `drivers.mk` fragment / Makefile integration is
   outside this workstream's file ownership (root Makefile is off-limits);
   tests are self-contained via `tests/run_tests.sh` and the module layout
   follows `docs/rewrite/architecture.md` §2.
7. **Future (not blocking):** dirty-row tracking in `fill_rect` so
   `display_flush_rows` callers can flush less; 6x11 or proportional fonts
   as `font_surface_t`-compatible alternatives (theme workstream).
