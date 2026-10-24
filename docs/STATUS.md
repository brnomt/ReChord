# ReChord — Build Status (Aug 2026)

> **Goal:** a **complete custom firmware written from source** for the FiiO
> Echo Mini (RKnanoC) — a **"Rockbox for the FiiO"**. Not a patch, not a
> byte-mod: **all the firmware source available and modifiable**, covering
> **both** halves of the device.
>
> **ReChord** = re-harmonize: compile the Rockchip SDK from source (the audio
> side) + write our **own UI layer** from scratch (the front end).
>
> **Architecture (key):** the device runs **two firmwares** over a mailbox,
> **both with SDK source available**:
> - **fw1 (AP) @ IMG `0x7B8–0x57820`** → **UI** — built on the **RKnanoC SDK**
>   (`rk3399-table-RKNanoC`: UI MainMenu/MusicWin, drivers I2C/AD_KEY/DAC,
>   FileSys FAT, NANO_OS — 103 `.c`). We rebuild our own UI on top.
> - **section_3 (BB) @ IMG `0x81A14–0x9BAA0E`** → **audio/DSP** — the
>   **RKnanoD SDK** covers this (already integrated).
>
> **Current:** SDK (BB) compiles and our own BB code boots in QEMU. The open
> problems are: (1) **BB display** — get our framebuffer to the LCD (the DMA
> transfer `Lcd_BuferTranfer` lives in the ROM/FiiO layer, not the SDK), and
> (2) **AP/UI** — map fw1 and rebuild the menus/navigation from scratch.
> See `docs/dispatch-map.md` (M0) and `docs/community.md` (community findings).

---

## Current status (2026-10-18) — supersedes the historical sections below

The project is now a **from-source fw1 + our own Core B image** that boots and
plays audio on hardware:

- **Audio**: Core B decodes FLAC/MP3/WAV; the AP does the DAC/I2S bring-up,
  serves the track bytes over the A→B file rings (`bfs`) and drains the PCM ring
  in its I2S TX ISR. Verified: 16/44.1 and 24-bit 88.2/96 kHz clean. Note that
  **96 kHz halves every time budget** (64 KiB PCM ring ≈ 74 ms of cover, TX-FIFO
  refill margin ≈ 0.15 ms) — occasional crackles on 96k material are still being
  chased.
- **UI/library**: menus, browser, iterative media index (depth-capped, one
  scratch claim), Album/Artist/Genre/Year/Label/Album Artist/Composer, All
  Songs; Now Playing shows bit depth/rate (`16/44.1kHz`), the list position
  (`i/N`) and the repeat/shuffle mode (`R:one`, ` S`). Names sort
  **case-insensitively** and paths are stored whole up to **208 chars**. The
  index format is **v3** (BRNOMT-92: global CRC covering the header, strict
  UTF-8 text, dual `INDEX.A`/`INDEX.B` slots) — an index built by an older
  firmware is ignored, so **rebuild it once from the menu** after flashing.
- **Playback lists**: album/genre/folder queue with Rockbox-style shuffle and
  repeat off/all/one (config keys `shuffle`, `repeat_mode`; 15-key schema),
  applied to the player immediately and preserved when a new list starts.
  Settings is grouped into **Playing** (Volume, Shuffle, Repeat, Resume,
  Fade) and **Interface** (Theme); `fade` is off/150/300/500 ms and applies
  to play/pause/resume.
- **Resume**: the last track + position and the browser folder/row live in
  `/.rechord/state` (separate from the config, rewritten in place); the
  `resume` setting (off/folder/track) decides what is restored at boot. It
  opens paused — never autoplays. The playback queue is persisted by origin
  (folder, or media category + value) and rebuilt from the media index at
  boot, so next/prev keep working. The folder/row are restored as browser
  state (Files opens there), but boot stays on the active theme's home
  (BRNOMT-102).
- **Album art (read-only, no decoding)**: `tools/artpack` (PC, Pillow+mutagen)
  precomputes a 120x120 and a 48x48 RGB565 cover plus a palette per track and
  writes `/Internal/.rechord/ARTPACK.BIN`; the firmware only reads it
  (`firmware/services/art_pack.c`). Themes get the `art` meter (streamed
  through one sector buffer, clipped) and the `art_ok`/`art_c1`/`art_c2`/
  `art_mean`/`art_accent` variables; missing art falls back to a placeholder.
  Format and costs: `docs/ART_PACK.md`.
- **Cassette theme**: `themes/cassette/` recreates the cassette-deck mockup
  (`docs/themes/cassette/`): full-screen artwork gradient, cream label with
  title/artist/stripes, reels whose tape packs and rotation follow `pos_ms`
  (frozen on pause), the home bar, and a Now Playing card with the 120x120
  cover, a mini cassette and a progress strip. It uses the ARTPACK v2 palette
  (`art_bg_a/bg_b/glow/panel/label/ink/ink_dim`); without a cover it falls back
  to the theme.cfg warm grey/cream/amber palette and draws a cassette glyph.
  Engine additions: `gradient` meter, `sqrt`, text `maxc`/`align=right`, and
  the section cap raised 32 -> 52 (parsed, 0 B RAM).
- **Quick settings**: MENU opens an overlay with Shuffle, Repeat and Playlist
  (BRNOMT-105). The playlist view is the live queue, marks the current track
  and SELECT jumps to/plays the chosen entry. MENU or BACK closes it; MENU
  acts once per physical press (LONG/RELEASE are ignored, so a long press
  neither flickers nor leaves the overlay open).
- **System menu**: the diagnostics info rows (Index, Keys, eMMC, SD, USB)
  stay. The DAC test and USB CDC console are debug-only
  (`make ... AP_DEBUG_TOOLS=1`) because they cost ~3 KB of code and the AP
  code must end below `LOADER_CODE_BASE 0x03088000` (DFIU `51cc212d`).
- **Memory** (vendor `B_Core.sct` + TRM): B code in HIGHRAM0
  `0x01000000..0x01020000`; control block `0x01020000`; A→B file ring
  `0x01021000` (32 KiB) + **pre-open ring** `0x01029000` (16 KiB); PCM ring
  `0x01030000` (64 KiB); B data/pool/stack `0x01040000..0x01060000`. The
  authoritative layout is `firmware/player/ipc_shared.h`.
- **Gapless on B (BRNOMT-73)**: **verified on device for same-format tracks**.
  At B's decoder EOF the AP hands the next track to B on the live ring
  (`ipc_b_next` → `IPC_C_NEXT`, no DAC/ring restart) with no AP-side storage
  access on the boundary: the next file is pre-opened ~3 s earlier, its codec is
  opened and kept as `pending`, its index→path resolution is cached, and its
  FLAC prescan geometry is published so B skips its own header walk. A **format
  change** stays a hard boundary by design (the DAC must re-lock) — expect a
  short pause on albums that mix rates/depths.
- **Power**: on battery the power hold cuts the PMIC rail (a real off); with
  USB/VBUS the host keeps the SoC powered, so "off" **parks** (dark + silent +
  charging) and a power press wakes the UI.
- **Battery**: an approximation from the stock SDK constants — SARADC channel 0,
  `((adc<<1)*2500)>>10` mV, 5-step ladder (3350/3500/3600/3800/4000 mV) → 25 %
  steps, polled every 15 s and shown in the root menu's title row.
- **Equalizer (BRNOMT-76)**: 5–10 band EQ applied on Core B
  (`firmware/dsp/eq_bank.c`) to the decoded int32 block, with profiles loaded
  at runtime from `/Internal/.rechord/eq*` (no recompilation). The AP publishes
  the curve over the shared block; Settings → Interface → Equalizer edits it.
  Contract, format and budgets: `docs/rewrite/eq.md`.
- **Open**: 96k margins (above), 192 kHz / FLAC block 16384 (~143 KB window),
  384 kHz, and EQ skin support (the EQ screens use the compiled layout).
- **History**: everything below is dated notes from the reverse-engineering and
  bring-up phases; kept for provenance (some statements are superseded).

---


### fw1 (AP/UI) = build App/UI del SDK RKnanoD — identificado 2026-08-12
> **Corrección tras el match de strings**: fw1 (AP) **NO** es el SDK
> `rk3399-table-RKNanoC` (solo 32 strings). Es el **SDK RKnanoD** (el mismo
> que el BB): 223 strings de RKnanoD_MP3_V1.3, 205 de RKnanoD_Wireless_V1.5.
> La UI del AP está en `RKnanoD_MP3_V1.3/SDK_160_128/UI/` (45 `.c`: MainMenu,
> MusicWin, SetMenu, Browser…) + `main.c`. El BB es `Main2.c` + codecs.
> → **Ambos firmwares son dos builds del mismo SDK RKnanoD.**

### RKnanoD SDK (BB/audio) — integrado (ver abajo)

## What compiles today (RKnanoD = BB)

| Layer | Files | Status |
|-------|------:|--------|
| **Kernel** (bbsystem + system/os + fileseek + module_overlay + sysservice) | 29 | ✅ all compile |
| **Audio** (AudioControl, HoldonPlay, Pcm, audio_file_access, audio_track_control, pCODECS, **Effect**, RecordControl) | 8 | ✅ all compile |
| **Codec wrappers** (AAC, DSDIFF, DSF, ALAC, APE, FLAC, MP3, OGG p*.c) | 16 | ✅ all compile |
| **Codec .lib binaries** (FLAC/AAC/DSD/APE/ALAC + EQ/FADE) | 22 | 📦 ready to link |

Compile check (any SDK .c):
```bash
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -fsyntax-only \
  -include firmware/rockchip/include/armcc_compat.h \
  -Ifirmware/rockchip/include -Ifirmware/rockchip \
  -Ifirmware/rockchip/audio/Include -Ifirmware/rockchip/audio/AudioControl \
  -Ifirmware/rockchip/audio/Common -Ifirmware/rockchip/audio/RkEQ/Effect \
  -Ifirmware/rockchip/audio/RecordControl -Ifirmware/rockchip/audio/ID3 \
  -Ifirmware/rockchip/audio/Wav/WAV_LIB -Ifirmware/rockchip/audio/SSRC/resampler \
  -Ifirmware/rockchip/system/os -Ifirmware/rockchip/system/fileseek \
  -Ifirmware/rockchip/system/module_overlay -Ifirmware/rockchip/system/sysservice \
  -Ifirmware/rockchip/bbsystem \
  firmware/rockchip/bbsystem/Main2.c
```

## The integration headers (all in firmware/rockchip/include/)

Created to make the armcc-written SDK build with GCC:

| Header | Purpose |
|--------|---------|
| `armcc_compat.h` | Keil keywords → GCC (`__packed`, `__irq`, `_ATTR_*` sections, `__asm`) |
| `SysConfig.h` | Module selection (MUSIC/RADIO/RECORD/PICTURE/BT/USB_HOST), MODULE_ID_*, SYSTEM_DEFAULT_PARA_T, FIRMWARE_INFO_T/CODE_INFO_T, MEMDEV_INFO |
| `driverlib_def.h` | SoC registers: CRU @0x20000000, DMA @0x40010000, INTC @0x400B0000, GRF @0x400C0000, SAR-ADC @0x400D0000, I2S/PMU, SysTick/NVIC (RKnano layout), PLL_ARG_t, chip_freq_t |
| `freq_enums.h` | eFREQ_APP enum (shared; PowerManager.h redefines EXT) |
| `mailbox.h` | CPU↔DSP GOODE mailbox protocol (IDs/channels/MSGBOX_CMD_*) |
| `service_globals.h` | Audio service globals (AudioPtr, AudioPlayState, DmaTransting...) |
| `FileInfo.h` / `File.h` / `fsinclude.h` | File info struct, file I/O API, FAT/find types |
| `battery.h` / `backlight.h` / `lowpower.h` / `pmu.h` | Power/battery/backlight |
| `I2S.h` / `Spectrum.h` / `Dma.h` / `pcm.h` | Audio/USB interfaces |
| `RecordWin.h` / `MusicWin.h` / etc. | UI window externs (30+ stubs) |

## Build status (link works)

| Item | Status |
|------|--------|
| **Linker script** | ✅ `firmware/firmware.ld` — captures all SDK sections (AudioCode/Bss, SysCode, FindFileCode, FlacDecCode, driver_code, bb_vect) + buffers at segment-table addresses |
| **Link** (section_3 binary) | ✅ `make all` → `build/rechord_full.elf` (998 KB: 67 KB text, 183 KB data, 747 KB bss) + `build/section3_custom.bin` (50 KB) |
| **Codec .lib integration** | ⬜ 22 binaries ready; the linker must place them at their segment addresses |
| **App layer (FiiO UI)** | ⬜ The Ghidra-decompiled layer (archived in `docs/re/decomp/`) — the SDK covers kernel/audio/codecs, the UI is FiiO-specific |
| **Flash test** | Resource mods flashed OK (boot animation). Code replacement not yet flashed — stubs make the current build a bootable kernel without working drivers |

### What the current build is

The linked firmware boots the SDK kernel (Main2 = the BB/audio side) on real
hardware, but the FiiO UI layer is not ours — the stock AP side draws the
cassette UI. Pressing a menu item makes the AP send a mailbox command to our
BB, which (being stubs) never answers → freeze + power-off.

**The next milestone is the mailbox handshake** (§7 in HANDOVER.md): if we
reply to the AP's commands, menus stop freezing and we get a working
navigation → then real drivers + DSP.

### Makefile targets

> **SUPERSEDED — see "Current status" above.** The live build is the clean-room
> AP image: `make cfw-ap` → `build/cfw/rechord_ap.elf`, `make cfw-img` →
> `build/ReChord_AP.IMG` + **`build/ReChord_AP5M.IMG`** (the one to flash),
> `make build-bb` / `make link-bb` for section_3, and `make release` for the
> stock-AP + custom-BB fallback. Everything below is the older hybrid path, kept
> for provenance only.

```
make build-ap      # compile AP (fw1) 187 objects -> build/ap/objs/
make link-ap       # link AP -> build/ap/rechord_ap.elf + fw1_custom.bin
make build-bb      # compile BB (section_3) 44 objects -> build/bb/objs/
make link-bb       # link BB -> build/bb/section3_custom.bin
make all           # both halves
make pack-img      # splice section_3 into HIFIEC37.IMG (identity test)
python tools/pack_fw1.py build/ap/rechord_ap.elf -o build/ap/fw1_custom.img
python tools/pack_img.py --pack-full --fw1 build/ap/fw1_custom.img \
                        --bb build/bb/section3_custom.bin -o build/ReChord_APBB.IMG
```

Excluded from the manifests (need project-layer defines): `systick2.c`,
`pCODECS2.c`, `RecordControl.c`, `PowerManager.c`, `AsicToUnicode.c`,
`cue.c`, `ID3.c`, `AsicToUnicodeTable.c`. Linked via prebuilt .o or stubs.

## Packing / flashable-IMG status

| Half | Container | Status |
|---|---|---|
| **BB (section_3)** | flat 16-byte RKnanoFW header + code @ 0x81A14 | ✅ packs → `build/ReChord_BB.IMG` |
| **AP (fw1)** | RKnanoFW header + scatter table @ 0x1F8 + payload @ 0x7B8 | ✅ slims to **273 KB** (fits the 356 KB region) → `build/ap/fw1_custom.img` |
| **AP+BB combined** | both halves spliced into stock IMG | ✅ `build/ReChord_APBB.IMG` (33,554,436 bytes, headers/trailer verified) |

To make fw1 fit, the codec `.lib` (AP_CODEC_LIBS) and 40 overlay sources
(audio codec wrappers, ID3, RecordControl, image, BT, FM, usbcontrol) were
removed from `ap.mk`; their entry points are 72 weak stubs in `firmware/stubs.c`
(these belong in the BB / overlay modules, not the resident UI).

`pack_img.py --pack-full` splicing is byte-exact (re-packing stock fw1 +
section_3 reproduces the stock IMG). Full detail: `docs/fw1-packing.md`.

## The DSP-effects mod target (your goal)

`firmware/rockchip/audio/RkEQ/Effect/Effect.c` compiles clean:

```c
long EffectInit(void);                          // init
long EffectProcess(EQ_TYPE *pBuffer, long PcmLen); // ← per-frame hook
long EffectAdjust(void);                        // adjust
long RKEQAdjust(RKEffect *pEft);                // apply coefficients
```

`effect.h` defines `RKEffect` (5-band `dbGain[5]`) + 8 presets
(`EQ_HEAVY/POP/JAZZ/CLASS/BASS/ROCK/USER/NOR`).

**To add DSP effects:** modify `EffectProcess()` (e.g. multiply samples for
bass boost, add delay taps for reverb) and rebuild.

## Next steps (recommended order)

1. **Linker script** — place kernel/audio/codecs at segment addresses
   (`docs/memory-map.md` + section_1 table). The 22 `.lib` codecs go at their
   `RkNanoD_*` addresses; `pack_img.py` splices the result into section_3.
2. **Link test** — produce a flat binary, verify it loads at 0x03000000 and
   `firmware_entry` @ 0x03000010 runs (QEMU cortex-m3 smoke test).
3. **App layer** — the Ghidra-decompiled UI (852 named, 327 compile) fills
   the FiiO-specific windows/menus on top of the SDK.
4. **Flash** — pack_img.py + the safe flashing method (stock backup always).

## SDK compile pipeline hardening (Oct 2026)

The SDK symbol-extraction pipeline (`tools/compile_check.py`) went from
**5/220 to 73/220** `.c` files compiling (544 functions extracted) via:

1. `tools/fix_include_case.py` (new) — rewrites `#include` directives to match
   on-disk header names: the vendor tree is Windows-authored, so Linux broke on
   case mismatches (`Macro.h` vs `macro.h`), backslash paths
   (`..\ImageInclude\...`) and ambiguous basenames (resolves to the nearest
   candidate). ~300 include lines fixed across ~180 files. Re-runnable.
2. `compile_check.py` now force-includes `armcc_compat.h` (the Keil keyword
   shim already existed but was never injected) and compiles with `-std=gnu89`
   + implicit-declaration leniency — the SDK is armcc-era C90 and we only need
   objects for symbol extraction.
3. Structural fix: `struct _FIND_DATA` is defined twice with different layouts
   (`filesys/FDT.h` = search cursor, `include/fsinclude.h` = file handle);
   both are now wrapped in a shared `_FIND_DATA_DEFINED` guard (first-wins).
   **TODO(unify):** give them distinct tags.

Remaining compile failures (~147) are mapped by cause: missing cross-subsystem
type/enum includes (CodecMode_en_t, CLK_SYS_CORE_GATE, MENU ids), the
duplicate `driverlib_def.h` (ours in `include/` vs vendor in `driver/` — same
name, different content), and a handful of section-attribute conflicts.

## Related docs

- `docs/dispatch-map.md` — **M0: mapa de despacho del ROM** (entry points fijos + ROM API)
- `docs/community.md` — **hallazgos de la comunidad** (RSE blog, FlameOcean, SDKs leakeados)
- `docs/HARDWARE.md` — SoC addresses, segment table, ROM API, fuentes
- `docs/FLASHING.md` — safe flashing + recovery
- `docs/c-cleanup-status.md` — decompiled .c tree cleanup (327/394 compile)


## BB milestone 2: the full SDK builds from source (2026-10-10)

`make release` is GREEN end to end: every SDK subsystem the BB needs compiles
from `.c` and links into `build/ReChord_BB.IMG` (33,554,436 B, custom trailer
recomputed; section_3 = 193,224 B). This is the freeze-fix milestone: the BB
now runs the REAL filesys (FAT/exFAT/nFAT), MemDev (eMMC), mailbox, systick,
power, display, USB device and recorder services instead of weak stubs.

**What it took (integration taxonomy, all documented inline):**

- Config is king: dozens of feature gates (`_USB_`, `_SBC_ENCODE_`, `_RK_ID3_`,
  `ENCODE`, `CODEC_CONFIG=CODEC_ROCKC`, BT UART wiring) live in
  `include/SysConfig.h`. Values copied from the stock SDK config where the
  board matters (BT UART = `UART_CH1_PA`/`INT_ID_UART1`/`INT_ID_UART5`).
- Include order is load-bearing: `include/` was shadowing 52 vendor headers
  (Fat.h/Fdt.h/FileInfo/AddrSaveMacro/driverlib_def...). Vendor dirs now win;
  our `include/` sits before `community/sdks` so OUR `SysConfig.h` beats the
  leaked SDK's sample config (that one silently won for a while).
- Our synthesized headers became wrappers or lost their duplicates
  (`driverlib_def.h`, `File.h`, `mainmenu.h`, `FunUSBInterface.h`); enum
  members beat same-named `#define`s (FS_TYPE, MEDIA_FILE_TYPE_*, I2S_*,
  CHARGE_CURRENT_*, FS_* sample rates).
- Keil-isms neutralized in `armcc_compat.h`: ~600 `_ATTR_*`/section placements
  map to empty (GCC section-type conflicts), `typedef.h` is force-included,
  `-fcommon` restores legacy tentative definitions, and the linker takes
  `--allow-multiple-definition` because the prebuilt .lib archives carry their
  own copies of SDK functions (first = our source wins).
- `firmware/libc` grew the missing C surface: `__aeabi_mem*` family (required
  by the codec libs), `strncasecmp`/`strcasecmp`, wide-string and UTF-16
  helpers.
- A `PMU.C` (uppercase!) compiled as C++ and mangled `PmuPdLogicPowerDown`;
  renamed to `PMU.c`.

**Memory map change (TODO to revert):** FW_RAM was extended into
`0x03060000..0x030E0000` (HARDWARE.md's "Codec implementations" overlay area)
to fit the statically linked services. Bring-up codec set is MP3 + WAV +
HIFI-FLAC + SBC/SSRC/EQ/FADE; APE, ALAC, DSF, DSDIFF, Ogg, AAC and the image
decoders return later as overlay modules (their resident .bss alone is
hundreds of KB). TODO(overlay): move codec modules to the overlay area and
shrink FW_RAM back to `0x5A504`.

**Next:** flash `build/ReChord_BB.IMG` and verify the menu-freeze is gone
(the AP's file calls now reach a real BB filesys), then wire the clean-room UI
and the module system.


## Strategy pivot (2026-10-10): from stock-AP hybrid to full clean-room CFW

The Route-B hybrid (our BB + the stock AP) reached its ceiling: the stock AP
is a black box (private mailbox handshake, no log drain, no UI control), each
step needs another reverse-engineered handshake, and even the BB-side
breadcrumb channel dies in context. Decision: **pivot to the reference-CFW
architecture** - which was always the end goal (no FiiO code):

- our own fw1 (AP + UI + services) with the verified boot recipe (bounded
  .data copy, bounded .bss zero, VTOR=table, main) extracted from the
  reference image startup at 0x030500E4
- our BB (the SDK-built baseband) delivered as the core-B module, loaded to
  HIGHRAM0 and started at 400 MHz (the reference ships its core-B as module
  "m78" the same way)
- our own AP<->B protocol (firmware/player/ipc_b.h contract, host-tested)
- our own image packing (tools/pack_fw1.py + pack_img.py) and CRC trailer

Pieces already in place: clean-room UI (host-tested), display/font/panel
drivers, services (settings/input/fs/log), player, DSP chain, module system
(RMF1 + loader), libc, BB from source. Milestone 1: fw1 image that boots to
OUR screen (status screen is enough). The stock-AP BB build stays as the
safe fallback image.
