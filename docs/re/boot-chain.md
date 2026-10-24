# Boot-chain map and Phase 0 evidence (BRNOMT-82)

**Status:** Phase 0 partial; Phase 1a (standalone 5 MB container) done.
**Baseline:** `build/ReChord_AP5M.IMG`, CRC `0x2264CCBA`, SHA-256
`c2238cb388b14d6bfd984da9328ca0005831b90d42cf4b5be77617b48babb93d`.
**No hardware flash in this phase.**

## 1. Why this document exists

BRNOMT-82 requires a clean `git clone` to build a flashable image without a
FiiO stock image, a reference CFW image or external blobs. Today `stock/` and
`community/` are gitignored, and the legacy packers read both. This document
records the stock boot-chain map, the first standalone container milestone,
and the open questions that block a custom bootloader.

## 2. Build-time dependency inventory

- `.gitignore:7` ignores `stock/` (1.1 GB of `HIFIEC*.IMG`); `.gitignore:33`
  ignores `community/` (438 MB of SDKs, `refcfw`, `rkbin`, `rkdeveloptool`).
  `git ls-files stock community` is empty.
- `tools/pack_img.py:52` `DEFAULT_STOCK`; `pack_section3()` and
  `pack_full()` start from that stock image and preserve its section_3 tail
  and trailer.
- `tools/make_fw1.py` (legacy path) uses `pack_img.pack_full()` and
  `community/refcfw/HIFIEC90.IMG` as the 5 MB "ref-shape" template.
- `Makefile:224` `SDK_LIB` and `:228` `BB_CODEC_LIBS`: `make release` links
  proprietary codec `.lib` archives from `community/sdks`.
- `firmware/rockchip/manifests/ap.mk` references SDK sources for the legacy
  AP build.
- The new `make standalone-ap5m` path uses `tools/fw1_5m.py` and
  `tools/rknano_crc.py`; it reads no stock/ref image and generates the 0x1F8
  descriptor from the field table in `docs/fw1-packing.md` section 10.

## 3. Stock fw1 memory map (HIFIEC37, v3.7.0)

`tools/HIFIEC37_unpacked/fw1_memory_map.bin` is 91 x 16 B. The entries
alternate LOAD and REG shapes. Rows relevant to the AP layout:

| row | shape | fields | meaning |
|-----|-------|--------|---------|
| 0 | LOAD | dest `0x030795CC`, src `0x03050000`, size `0x026568`, end `0x030A3B34` | stock AP code + loader code destination |
| 15 | REG | `0x0301E778`, obj `0x10000` | FAT cache |
| 17 | REG | `0x0301E794`, obj `0x4000` | main stack |
| 39 | REG | `0x030247A8`, obj `0xFB20` | UI framebuffer (loaded from flash) |
| 73 | REG | `0x0304AE64`, size `0xC834`, end `0x03057698` | loader data/BSS |

The row-0 destination spans:

- stock AP code block: `0x030795CC..0x03088000` (0xEA34 = 59,956 B)
- **stock loader code: `0x03088000..0x030A3B34` (0x1BB34 = 113,460 B)**

Our current AP:

- vector table/code: `0x03050000..0x03087F8C`; latest row-0 end `0x03087900`
- `_bss_end`: `0x03049BE0`; the known-safe bss ceiling is `0x0304A000`
- the `0x03088000` gate is load-bearing (DFIU `51cc212d`: a row crossing it
  overwrote the loader and bricked the device).

### 3.1 What a custom bootloader can reclaim

- The stock AP block `0x030795CC..0x03088000` is already overlapped by our
  AP code; it is not new space.
- The loader data row `0x0304AE64..0x03057698` overlaps our AP code from
  `0x03050000`. The clean part below our code is `0x0304AE64..0x03050000`
  (0x51A0 = 20,896 B). The gap `0x0304A000..0x0304AE64` (0xE64 = 3,684 B)
  is not proven; do not use it without a hardware test.
- **The loader code window `0x03088000..0x030A3B34` (113,460 B) is the main
  new window.** It can only be used after replacing the stock loader/boot
  chain, which is Phase 2. A bootloader alone does not create RAM; **overlays
  are still required** for the 224 KiB resident code limit.

## 4. sec2 bootloader REA (Phase 0)

- Extract: `tools/HIFIEC37_unpacked/sec2_bootloader`, 172,532 B =
  `IMG[0x57820..0x81A14]`; Ghidra base `0x03000000`; runtime mapping is
  documented in `docs/bootloader-analysis.md` (`RAM = Ghidra + 0x0301e2b0`).
- Reproducible import (avoids the Ghidra ET_REL +0x10000 shift):

  ```sh
  arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm \
      tools/HIFIEC37_unpacked/sec2_bootloader /tmp/sec2.o
  cat > /tmp/flat.ld <<'LD'
  SECTIONS { . = 0x03000000; .text : { *(.data) } }
  LD
  arm-none-eabi-ld -T /tmp/flat.ld -e 0x03000001 -o /tmp/sec2.elf /tmp/sec2.o
  ```

- REA/Ghidra 12.1.4 evidence: 184 procedures, 55 strings; `fw1valid = %d
  fw2valid = %d` at `0x0300008C`; `fw1 compare error!` at `0x03000028`.
- The `check_fw` function at the documented `0x030000AC` was not created by
  the current automatic analysis (Thumb/analysis split); the pseudocode in
  `docs/bootloader-analysis.md` remains the reference. It calls two ROM
  validators at `0x0306FDA8` / `0x0306FDD0` (populated by the chip ROM,
  invisible in the image).
- The reference image zeroes the sec2 header so the loader skips the fw1
  signature stage; the legacy `make_fw1.py` does the same. The standalone
  5 MB image is fw1-only and contains no sec2/section_3.
- Open questions that gate Phase 2:

  1. What is the minimum fw1 header/table/entry contract the Mask ROM
     requires? Can a first-stage be loaded with a minimal table?
  2. Can the sec2 validation gate be skipped reliably, and what does the ROM
     require immediately after?
  3. Who executes the loader code at `0x03088000`, what does it initialize,
     and can it be replaced without breaking recovery/update paths?
  4. What is the exact XIP vs. contiguous load model (the docs record a
     correction to the contiguous model), and what is the A/B boot-pair
     behavior?
  5. Which stock RAM regions are ROM/loader-required versus stock-app-only?
     Which of the loader rows can be reclaimed?
  6. Are the stock section_3 tail tables and ROCK resources required by a
     clean-room AP? The 5 MB fw1-only path does not use them; the legacy
     33 MB path does.

### 4.1 Loader-source correction (2026-10-20)

The stock row 0 is `dest 0x030795CC, src 0x03050000`. The computed source
for the `0x03088000` destination is:

```text
0x03050000 + (0x03088000 - 0x030795CC) = 0x0305EA34
```

Ghidra has no procedure at `0x0305EA34`; `read_bytes` returns a data table
(`7f 8b f0 8c f4 8c 12 8d ...`), not Thumb/ARM code. Therefore the
`0x03088000` region is **not** a straightforward copy of fw1 AP code.

A comparison of the fw1 tables settles the practical question:

| image | row 0 dest range | loads `0x03088000`? | boots |
|---|---|---|---|
| stock 3.7.0 | `0x030795CC..0x030A3B34` | yes | yes |
| stock 3.9.0 | `0x030795CC..0x030A3C38` | yes | yes |
| HIFIEC90 (ref) | `0x03050B68..0x030771D4` | **no** | yes |
| MB1_ECHOMINI | `0x03050B68..0x030771D4` | **no** | yes |
| our AP5M | `0x03050B68..0x03087900` | **no** | yes |

The fw1-only reference, MB1 and our AP5M all boot without loading
`0x03088000`. The stock 3.7/3.9 images do load through it because their
stock AP/loader uses that region. So `0x03088000` is not a stock loader
that the AP5M needs; it is a window that must not be touched by the
**initial fw1 scatter row** (the loader/stock chain is still using it during
that copy), and it is a candidate **runtime overlay window** after the
loader hands control to the AP.

Consequences for Phase 2 (revised):

- The initial fw1 row gate at `0x03088000` stays.
- Do not put code in the initial scatter row above `0x03088000`.
- **Probe result (2026-10-20): the window is NOT free at runtime.** The
  AP_MEM_PROBE image booted through `display_init` (backlight on, brief
  panel flicker) then stayed black before the splash. The first write at
  `0x03088000` is live loader/ROM memory; it hung the AP. Do not write or
  copy overlays there.
- The 113,460 B reclaim now requires a bootloader that owns/replaces the
  loader (or an update-path redesign), not a runtime write. The custom
  bootloader is back as the path for the 113 KB; it remains optional for
  the AP5M build/update independence.
- Next runtime-window candidates, to probe one word at a time:
  `0x0304AE64..0x03050000` (20,896 B, loader data below the AP code),
  `0x0304A000..0x0304AE64` (3,684 B gap), `0x030A3B34+` (unproven).

### 4.2 Stock section_3 at 0x03088000 is app code, not the loader

REA on the stock 3.7.0 section_3 (loaded at `0x03000000`) shows real Thumb
code at `0x03088000`:

- Function `FUN_03087fd0`, body `0x03087FD0..0x0308805F` (144 B), called by
  `FUN_03085f4e` at `0x03086128`; no direct callees.
- It walks an object array (count at `param+0x188`, index array at
  `param+0x18C`, base at `param+0xD8`, object stride `0xFC`), reads
  `+0x84`/`+0x86`, and writes geometry/layout fields
  `+0x54/+0x58/+0x5C/+0x60/+0x64` from the table at `DAT_0103ccec`
  (`0x0103ccec`). This is stock UI/app code, not a loader.
- Stock 3.9.0 section_3 also has code at `0x03088000`; HIFIEC90's section_3
  has data/zeros there; our AP5M has no section_3 at all.

Therefore the probe hang on AP5M does **not** come from the stock app code
at that address. `0x03088000` is owned at runtime by the ROM/second stage
(or by our own loader path), and it cannot be reclaimed by copying or
replacing the stock section_3. The 113,460 B window needs a bootloader that
owns/replaces the ROM loader/update path; the stock app is a red herring.

## 5. Phase 1a — standalone 5 MB container (done)

- `tools/fw1_5m.py` builds the fw1-only 5 MB image directly from a `.fw1`
  container. It reads no stock/ref image and creates no 33 MB intermediate.
- Descriptor: 0x1F8 bytes generated from the documented field table
  (`docs/fw1-packing.md` section 10), with our build date, brand, build tag
  and `fw1_end` patched in. Marker container: `RKnanoFW` + 8 zero bytes at
  `0x44C1F8`. Self-consistency copy at `0x44C000`. Trailer CRC is computed
  by `tools/rknano_crc.py` (shared with the legacy packer).
- Build command:

  ```sh
  make standalone-ap5m
  # or
  python3 tools/make_fw1.py build/cfw/rechord_ap.elf --standalone-5m \
      -o build/ReChord_AP.IMG
  ```

- Verification (`tools/verify_fw1_5m.py`): the result is **byte-identical**
  to the boot-confirmed image: size `0x500004`, CRC `0x2264CCBA`, SHA-256
  `c2238...babb93d`.
- The legacy `cfw-img` target still uses the stock 33 MB image and the
  reference header; the new target bypasses both. Only
  `build/ReChord_AP5M.IMG` is flashed; the 33 MB `ReChord_AP.IMG` is not.

### 5.1 Clean-clone check (PASS)

`make clean-clone-check` creates **two independent** detached worktrees at
`HEAD` containing only tracked files (so `stock/` and `community/` are absent
by construction), asserts both directories are missing in each, runs
`make standalone-ap5m` in both, and requires the two 5 MB images to be
**byte-identical**. That is the property that matters after a `git clone`:
the build is standalone and the packer is deterministic.

A byte-exact boot-confirmed image can still be demanded explicitly:

```sh
EXPECT_SHA256=<sha256> make clean-clone-check
```

The golden is not hardcoded because every firmware commit changes the image;
it is updated only after a flash is confirmed on hardware. The old check
pinned `c2238...babb93d` from commit `02482c1`, which made every later commit
fail even though the build was correct.

### 5.2 Remaining Issue 82 dependencies

- `0x03088000` is live loader/ROM memory at runtime (the probe hung there).
  The AP5M does not load it in the initial row, but it is not a free overlay
  window. The 113 KB needs a loader-owning bootloader or a different layout.
- The legacy `cfw-img`/33 MB path still reads `stock/` and `community/refcfw`;
  it is not on the standalone target and should stay quarantined as a
  reference/fallback path.
- The `make release` BB still links proprietary `community/sdks` `.lib`
  codecs (Phase 3). The clean-room AP already uses `BB_CORE_SRCS`
  (WAV/MP3/FLAC).
- Flashing/update path (Phase 4). The stock updater/maskrom is a device
  component, not a build input; a custom updater is a separate decision.
- The 0x1F8 descriptor field table is RE-derived from the reference and the
  section 10 map is incomplete. It is a documented compatibility spec, not a
  stock-image read, but Phase 2 should validate or replace the remaining
  fields.

## 6. Go / no-go

- **Phase 1a: PASS.** Standalone 5 MB container reproduces the known-good
  image with no stock/ref reads.
- **Phase 1b: PASS.** `make clean-clone-check` builds twice from independent
  detached worktrees with only tracked files (no `stock/`, no `community/`)
  and requires byte-identical images. Pass `EXPECT_SHA256=<sha>` to also
  compare a boot-confirmed golden. The legacy `cfw-img` path still reads
  stock/ref; the standalone target bypasses it. Validate the descriptor on
  hardware when a flash is scheduled.
- **Phase 2 (revised after the probe):** `0x03088000` is live at runtime;
  the runtime overlay at that address is retracted. Probe
  `0x0304AE64..0x03050000` (20,896 B) and other candidates one word at a
  time, or build a bootloader that owns/replaces the loader for the 113 KB.
  The fw1-only AP5M still boots through the ROM; a custom bootloader is
  optional for the AP5M build but is the path for the 113 KB window.
- Current images keep the DFIU rule: loaded code must end at or below
  `LOADER_CODE_BASE 0x03088000`. A runtime overlay is a deliberate new
  invariant, not a bypass of the initial-row gate.

## 7. Flash safety

- Flash only `build/ReChord_AP5M.IMG`; never the 33 MB `ReChord_AP.IMG`.
- The probe image `build/probe/ReChord_AP5M.IMG` is a failed test; do not
  reuse it. Recover with the stock 33 MB image, then flash the normal AP5M.
- Keep the known-good image and maskrom recovery (`docs/DEBRICK-LINUX.md`).
- Current baseline: CRC `0x2264CCBA`, SHA-256 `c2238...babb93d`.
- Any new boot-chain image needs a hardware ladder (known-good -> first-stage
  hello -> minimal AP -> real AP) and a DFIU preflight.

## 8. Hardware test plan (not yet executed)

The user accepts brick risk and has maskrom recovery. The tests are ordered
from zero-flash evidence to runtime code execution.

### 8.1 UART boot log (no flash)

Capture the boot log on PB5 with a 3.3 V USB-UART adapter (GND common),
matching the pin/baud notes in `docs/bootloader-analysis.md`. Compare:

1. stock 3.7.0 (`HIFIEC37.IMG`) — the stock AP/loader path.
2. HIFIEC90 or MB1 — the fw1-only reference path.
3. our AP5M — the ReChord path.

Look for `fw1valid = %d fw2valid = %d`, `fw1 Sign error!`, `fw1 compare
error!`, `fw2 compare error!`, and any loader/update output. This is the
decisive evidence for what owns `0x03088000` and for the sec2 gate. No flash,
no brick risk.

### 8.2 Runtime memory probe (first flash)

Build an AP5M with a one-shot probe that runs after the loader, reads a few
words at `0x03088000`, writes a pattern, reads it back and restores the
original values. The initial fw1 scatter row stays below `0x03088000`.

- If the UI boots normally, the window is free for runtime overlays.
- If it hangs/bricks, the window is still owned at runtime; restore via
  maskrom and keep overlays out of it.

Keep the normal known-good AP5M available. The probe must not be part of the
default image; use an explicit build flag/output directory.

**Probe image (2026-10-20):**

```sh
make standalone-ap5m AP_MEM_PROBE=1 CFW_BUILD_DIR=build/cfw-probe \
    AP5M_OUT=build/probe/ReChord_AP.IMG
```

| field | value |
|---|---|
| image | `build/probe/ReChord_AP5M.IMG` |
| CRC | `0x6FE0ACB3` |
| SHA-256 | `377c5df853ca54c6d46f767046c529ddd175f6e468db00af4c196194812d1b4d` |
| row 0 end | `0x03087A34` (1,484 B below `0x03088000`) |
| normal image | unchanged, CRC `0x2264CCBA` |

The probe reads/writes/restores three words at `0x03088000`, `0x03095998`
and `0x030A3B30`, then shows `RAM window: free` (pass) or
`RAM window: used` (mismatch) on the boot splash for ~2 s. The initial fw1
row is still `0x03087A34`, so the loader gate passes.

**Result (2026-10-20): FAIL / soft brick.** The device booted through
`display_init` (backlight on, brief panel flicker) and then stayed black
before the splash; the UI never ran. The first write at `0x03088000` hit
live loader/ROM memory and hung the AP. The normal AP5M image recovered
the device. Do not flash this probe image again; do not use `0x03088000`
for overlays or runtime code. DFIU incident
`cf076cf2-e6cc-4dd9-8037-9d27e8001a51`.

**Recovery confirmed (2026-10-20, on-device):** the user flashed the normal
AP5M image (SHA-256 `c2238...babb93d`, CRC `0x2264CCBA`) and ReChord
booted. DFIU incident verified `pass` (`operator_review`); rule
`R-29b085f95328` is verified. The probe image SHA `377c5df8...` was not
reused.

Recovery: maskrom with the stock 33 MB image, then the normal AP5M via the
updater; see `docs/DEBRICK-LINUX.md`.

**Second probe (2026-10-20):** target the loader-data window below the AP
code, `0x0304AE64..0x03050000` (20,896 B). It reads/writes/restores three
words at `0x0304AE64`, `0x0304D734` and `0x0304FFFC` and shows
`RAM 0304AE64: free` / `used`; it does not touch `0x03088000`.

```sh
make standalone-ap5m AP_MEM_PROBE=1 CFW_BUILD_DIR=build/cfw-probe \
    AP5M_OUT=build/probe/ReChord_AP.IMG
```

| field | value |
|---|---|
| image | `build/probe/ReChord_AP5M.IMG` |
| CRC | `0x9691EE3F` |
| SHA-256 | `bc91a6ef1ee3f2be4faad8b623092044628322fc3e69dc97029dd761ab0ff8bb` |
| row 0 end | `0x03087A18` (1,512 B below `0x03088000`) |
| window | `0x0304AE64..0x03050000` (20,896 B) |
| normal image | unchanged, CRC `0x2264CCBA` |

Expected: splash shows `RAM 0304AE64: free` (window usable) or `used`
(loader data still owned). A black/hung boot means the write/restore
crashed; recover with the normal AP5M/stock image.

**Result (2026-10-20): PASS.** The user flashed SHA
`bc91a6ef...` and the splash showed `RAM 0304AE64: free`; the UI booted.
The `0x0304AE64..0x03050000` window (20,896 B) is free at runtime and the
write/read/restore did not hang the AP. The `0x03088000` region remains
live and untouched (`R-29b`). Next: a small code overlay in this window,
then the CDC console as an overlay/debug build.

### 8.3 Code overlay (second flash)

The 20,896 B window at `0x0304AE64..0x03050000` is free (probe PASS), so
the code overlay uses it. The demo overlay is 8 B, linked at `0x0304AE64`:

```text
0304ae64: 4800       ldr r0, [pc, #0]   ; 0x0304AE68
0304ae66: 4770       bx  lr
0304ae68: c0de1234   .word 0xC0DE1234
```

It is embedded in the AP image below the initial-row gate, copied into the
window at runtime and called through a Thumb function pointer. The AP shows
`Overlay: OK` for 2 s when the magic matches, `Overlay: FAIL` otherwise.

```sh
make standalone-ap5m AP_MEM_OVERLAY=1 CFW_BUILD_DIR=build/cfw-overlay \
    AP5M_OUT=build/overlay/ReChord_AP.IMG
```

| field | value |
|---|---|
| image | `build/overlay/ReChord_AP5M.IMG` |
| CRC | `0x6F38B64D` |
| SHA-256 | `69977945e56753b394c3735354088181075f4049c906d90fd085930e9f7e04ca` |
| row 0 end | `0x030879A8` (1,624 B below `0x03088000`) |
| normal image | unchanged, CRC `0x2264CCBA` |

A black/hung boot means code execution from the window is not safe; recover
with the normal AP5M/stock image. If it shows `Overlay: OK`, the window is
validated for code and the CDC console can move there as an overlay/debug
build.

**Result (2026-10-20): PASS.** The user flashed SHA `69977945...` and the
splash showed `Overlay: OK`; the 8-byte overlay executed from `0x0304AE64`
and returned the magic. The 20,896 B window is validated for code. Next:
move the CDC console into this overlay window as a USB debug build.

### 8.4 CDC console overlay (file-loaded)

The CDC driver + console is 1,736 B, linked at `0x0304AE64` with the AP USB
symbols as absolute addresses (`usb_cable`, `usb_dwc2_init`,
`usb_dwc2_stop`). It is loaded from the FAT file `/.rechord/cdc.bin`; the
AP image only contains the loader.

| field | value |
|---|---|
| AP image | `build/cdc/ReChord_AP5M.IMG` |
| AP CRC | `0xBF72CFF5` |
| AP SHA-256 | `fff75187d9addb9e05081a3e36d14b54b86292042ff9d5cad85d5f89da14e593` |
| CDC blob | `build/cfw-cdc/cdc/cdc_overlay.bin` |
| CDC blob size | 1,736 B |
| CDC blob SHA-256 | `82bc342265d2cbe55abe1d7d6eae4fd4da4454608e322e1881c7a728efc38443` |
| row 0 end | `0x030879F0` (1,552 B below `0x03088000`) |
| normal image | unchanged, CRC `0x2264CCBA` |

Steps: copy `cdc_overlay.bin` to the device as `/.rechord/cdc.bin` (the
host sees the ECHO MINI FAT root; the AP addresses it as
`/Internal/.rechord/cdc.bin`), flash the AP image, boot. The CDC is now a
menu action: **System → CDC overlay**. It first calls `usb_dwc2_stop()`
(release the MSC/USB core), then loads the blob after the UI and the
`Internal` volume are ready, then runs the CDC console; the host should see
`/dev/ttyACM0`, a banner and an echo console. Type `q` to return to the UI.
If the file is missing, the menu action returns to the UI.

Note: `fs_target` paths are `/<volume>/...`; the AP mounts `Internal`
(eMMC, contains `.rechord`) and `SD` (music card). The loader must use
`/Internal/.rechord/cdc.bin`; `/.rechord/...` resolves `.rechord` as a
volume, and `/SD/.rechord/...` looks on the music card.

### 8.5 When a custom bootloader is actually needed

A custom bootloader is now the only path to reclaim the 113,460 B window:
it must own/replace the loader (and the update path), not merely write to
`0x03088000` at runtime. The fw1-only reference/MB1 and our AP5M already
boot through the ROM without one; the bootloader is a memory/update
project, not a prerequisite for the AP5M build.
