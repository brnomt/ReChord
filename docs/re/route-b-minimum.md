# Route B — minimum viable ReChord CFW (hybrid image)

> Strategy chosen 2026-10-09: instead of reproducing the Mask ROM's full
> 91-entry scatter contract (Route A — `pack_fw1.py` bricked hardware
> 2026-08-25), boot OUR code inside the proven RefCFW skeleton. RefCFW boots on
> hardware with a 6-row scatter table + its own vector table, so the ROM's
> requirements are satisfiable without replicating stock's table.
>
> Source of truth: `docs/re/refcfw-analysis.md` (RefCFW RE), Ghidra project
> `ReChordV2:/refcfw_ramimg.bin` @ `0x0304F490`.

## 1. Milestone M-B1 — "ReChord boots and paints its name"

Hybrid flash image = RefCFW container/scatter/vector-table skeleton with OUR
entry code swapped in, calling RefCFW's own display primitives (kept in the
image). Flash via the proven safe path (`pack_img.py` + stock backup).

Success criteria: device boots the hybrid IMG, screen shows a ReChord boot
line, no brick, stock backup restores.

## 2. Memory model (byte-verified)

- RAM image = `IMG[0x1F8..0x500000]` loaded at **`0x0304F490`** (RAM addr =
  `0x0304F298` + IMG offset).
- Vector table at **`0x03050000`** (img file 0xB70): SP `0x03004000`,
  Reset `0x0306296F` → code at `0x0306296E`.
- Reset = standard C startup: IRQs off → capture 4 boot params (ROM handoff,
  stored via `DAT_0305015c` block — contents still to be identified) →
  `.data` copy → `.bss` zero → DSB/ISB → call main → hang.

## 3. Display contract (extracted from RefCFW, decompiled)

| Primitive | Addr | Semantics |
|---|---|---|
| `panel_init` | `0x03051EA8` | one-shot panel bring-up (called from draw_init) |
| `draw_init` | `0x030503EC` | one-shot: panel_init + clear screen + flush |
| `fill_rect(x0,y0,x1,y1,color)` | `0x03051EC0` | clip against `DAT_03051f0c` rect, fill RGB565 |
| `draw_text(x,y,str,color,..)` | `0x03052244` | glyph blit: `(ch-0x20)*0x0B` into font base `DAT_03052310` (6x11 font), rows into FB `row*0x280 + DAT_03052314` |
| `flush_rows(y0,y1)` | `0x03051D40` | DCS `0x2A/0x2B/0x2C` (CASET/RASET/RAMWR) via `FUN_03051c8c`, pixels via `FUN_03051c6c(port 0x2000000-ish)` |
| `vsnprintf` | `0x0305BD04` | stdarg formatter (also used by CFG serializer) |
| `status_line(id,color,fmt,...)` | `0x03050550` | id*12px rows; clear row, draw text, flush — the boot-screen line API |

Screen geometry observed: width **320 px** (x1=0x13F), boot area 170 rows
(0xA9), RGB565, 640 bytes/row framebuffer. Line height 12 px, text y offset +1.
Colors seen in boot screen: `0x8410`, `0xFA47`, `0xF79E` (RGB565).

## 4. Hybrid build plan (minimum)

1. Compile our entry/main for the corrected link addresses (RAM image base
   `0x0304F490`; code that RefCFW keeps at s3 file 0x244E = Reset region).
2. Splice our objects into a copy of the RefCFW RAM image (replace RefCFW's
   main at `0x03050B08` region or Reset region — keep display/font/panel
   code untouched).
3. Rebuild the IMG: RefCFW container header + scatter rows verbatim + our RAM
   image + trailer (keep `0xEFDC758B`).
4. Flash with stock backup in place; verify boot line; restore if not.

## 4b. Image checksum contract (cracked 2026-10-09, byte-verified 4/4)

The 4-byte trailer is a **content CRC** of the whole image body:

    trailer = CRC32(poly 0x04C10DB7, non-reflected MSB-first, init 0, xorout 0)
              over IMG[:-4]

- The poly is **non-standard** (0x04C10DB7 — standard CRC-32 is 0x04C11DB7,
  bit 0x1000 missing; matches RefCFW's checksum fn `FUN_03054284`, whose table
  init is the same MSB-first loop).
- Verified against HIFIEC37/HIFIEC39/MINIV390/HIFIEC90 — exact match on all.
- Consequence for patching: ANY byte change requires recomputing the trailer.
  The device rejects wrong-trailer images **pre-write** with `"bad CRC Image"`
  (observed live on-device 2026-10-09) — a safe failure: nothing is flashed.
  The module checksum (`mod: mNN crc %08x, want %08x`, `sum 62ccbf5f` in the
  log for m78) uses the same primitive.

## 5. Open contracts (next extractions)

- Boot params: identity of the 4 words captured at Reset (ROM handoff ABI).
- Input: key ADC scanning (strings `keys: adc ch %d ...`, `keys: chord vol+-`).
- Storage: mount/FAT path for REFCFW.CFG-style config (RefCFW reads
  `\REFCFW.CFG`, `ECHOLOG.TXT`).
- Scatter row semantics as consumed by the ROM (needed later to grow beyond
  RefCFW's exact layout windows).

## 6. Suggested ReChord CFW shape (final target)

Rockbox-style modules, all C (decision 2026-10-09): resident core (startup,
display, input, FS, config, logger) + overlay modules (player, settings,
browser, DSP) loaded via a module format like RefCFW's (`magic/id/checksum/
len/bss`), custom DSP on the proven `rechord_dsp_core.c`, RKNano SDK reused
underneath.
