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
`0x03088000` loader is **not** a straightforward copy of fw1 AP code. It is
likely populated by the ROM/second stage, or the row-0 mapping is not the
loader source.

Consequences for Phase 2:

- Do not design a first-stage that merely replaces a fw1 code row at
  `0x03088000`. The RAM region is reserved by a component whose source is
  not in the fw1 AP image.
- Treat `0x03088000` as ROM/second-stage-owned until UART/hardware evidence
  proves otherwise.
- The `0x03088000` gate stays for current images.
- The next evidence step is to locate the actual loader code/source (ROM is
  invisible; compare sec2, the 5 MB reference and a UART boot log) before
  changing the memory layout.

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

`make clean-clone-check` creates a detached worktree at `HEAD` containing
only tracked files (so `stock/` and `community/` are absent by construction),
asserts both directories are missing, runs `make standalone-ap5m`, and
compares the resulting image SHA-256 against the boot-confirmed value.

Verified on commit `02482c1`: **PASS**, `stock/` absent, `community/`
absent, output SHA-256 `c2238...babb93d`. This proves the AP5M build path
(AP + clean-room BB core + fw1 + 5 MB container) is independent of the
gitignored stock/reference directories.

### 5.2 Remaining Issue 82 dependencies

- **Stock loader/ROM contract at `0x03088000`** (Phase 2). The 5 MB image
  still boots through the stock boot chain.
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
- **Phase 1b: PASS.** `make clean-clone-check` builds from a detached
  worktree with only tracked files (no `stock/`, no `community/`) and
  reproduces the known-good SHA. The legacy `cfw-img` path still reads
  stock/ref; the standalone target bypasses it. Validate the descriptor on
  hardware when a flash is scheduled.
- **Phase 2:** only after the sec2/loader/ROM questions above are answered
  and a first-stage + memory layout + recovery ladder is defined. Do not
  flash a custom fw1 table or overwrite `0x03088000` before that.
- Current images keep the DFIU rule: loaded code must end at or below
  `LOADER_CODE_BASE 0x03088000`. A custom layout is a deliberate new
  invariant, not a bypass.

## 7. Flash safety

- Flash only `build/ReChord_AP5M.IMG`; never the 33 MB `ReChord_AP.IMG`.
- Keep the known-good image and maskrom recovery (`docs/DEBRICK-LINUX.md`).
- Current baseline: CRC `0x2264CCBA`, SHA-256 `c2238...babb93d`.
- Any new boot-chain image needs a hardware ladder (known-good -> first-stage
  hello -> minimal AP -> real AP) and a DFIU preflight.
