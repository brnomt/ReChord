# ReChord

> **Re-Chord** — re-harmonize the FiiO Echo Mini. A **free, from-source custom
> firmware (CFW)** for the Echo Mini.

[!WARNING]
Work in Progress (WIP)
ReChord is currently in an early experimental stage and is not functional for everyday use yet (audio playback and low-level drivers are still under active development)
Contributions, issues, and PRs are very welcome! However, please note that any driver or hardware-related changes must be tested and verified on physical hardware before being merged

## Objective (the whole point)

ReChord is a **complete custom firmware, written from source** — not a patch,
not a byte-mod of the stock binary. The goal is to have **all the firmware
source code available and modifiable**, covering **both** halves of the device:

- **UI** — custom menus, screens, navigation, theme (what you see).
- **DSP / audio** — custom effects, EQ, codecs, playback (what you hear).

Everything compiles from source and is flashed as a normal firmware update.
No binary patching of the stock firmware. No stock app layer left over.

## Architecture (the key insight)

The Echo Mini runs **two firmwares side by side**, communicating over a
hardware mailbox (`MailBox A2B/B2A`):

| Firmware | IMG offset | Role | SDK source |
|----------|-----------|------|-------------|
| **fw1 (AP)** | `0x0007B8 – 0x057820` | **UI + system** — menus, fonts, storage, USB, updates | **RKnanoD SDK** — App/UI build (`main.c` + `SDK_160_128/UI/`) |
| **section_3 (BB)** | `0x081A14 – 0x9BAA0E` | **Audio** — codecs, DSP/EQ, playback engine | **RKnanoD SDK** — BB build (`Main2.c` + codecs) |

- **Both firmwares are two builds of the same Rockchip RKnanoD SDK**
  (confirmed by string match: fw1 = 223 RKnanoD_MP3 strings vs only 32
  rk3399-table). The AP is the `main.c` + `UI/` build; the BB is the
  `Main2.c` + codecs build.
- **ReChord's current product is a from-source fw1 (AP)**: our own boot,
  UI, storage, and player — the stock AP layer is gone. The BB/section_3
  remains in the image (stock or SDK-built) for the hybrid-era paths; the
  reference two-core audio protocol is being reimplemented clean-room
  (`firmware/player/ipc_b.h`).
- The **mask ROM** (`0x00000000–0x02FFFFFF`) is the fixed hardware HAL
  (LCD, I2S, DAC, USB, keys) — it is *not* firmware we replace; it is the
  "BIOS" our code calls.

> **This is NOT a decompilation project, and NOT a patch project.** The
> reverse-engineering phase (2026) produced the knowledge (addresses, SDK
> integration, hardware map) that makes the from-source build possible. See
> `docs/RE-HISTORY.md`.

## Status (Oct 2026)

ReChord is currently under active development and should be considered
experimental compared to established alternatives.

| Layer | Status |
|-------|--------|
| **fw1 (AP) boots standalone** (ROM → our reset → our Main) | ✅ on hardware (verdict #20 lineage) |
| **UI**: menus, keys, file browser (eMMC + SD, FAT16/32/exFAT) | ✅ on hardware (m5a/m5b) |
| **USB mass storage** (LUN0 = eMMC r/w, LUN1 = SD r/o) | ✅ on hardware (m3c) |
| **Self-updater** (copy HIFIEC90.IMG, eject/power → install) | ✅ on hardware (verdict #24) |
| **microSD driver** (DW-MMC, 4-bit, read-only) | ✅ on hardware (verdict #26) |
| **Player module** (state machine, gapless, ReplayGain, fs-backed codec_io) | ✅ linked into fw1, host-tested (~340 asserts) |
| **Codecs**: WAV (ours), MP3 (minimp3), FLAC (dr_flac) | ✅ vendored + host-tested; MP3/FLAC decoded losslessly in tests |
| **DAC renderer** (I2S → CS43131) | ⬜ **next — no sound until this lands** |
| **DSP effects** (`EffectProcess` hook) | 🟡 core written + host-tested (`firmware/dsp/`), waiting on the renderer |
| **BB (section_3) SDK build** | ✅ compiles + links (`make build-bb`) |

- The player decodes WAV/MP3/FLAC and applies gapless trimming +
  ReplayGain, but the PCM ring currently has **no consumer** in our fw1
  (no B core in the image) — the DAC renderer is the next milestone
  (`docs/rewrite/audio.md` step 7).
- Flash safety: images are packed behind a scatter-table gate (DFIU rules)
  and always installed via the official updater flow; stock/restorable via
  maskrom (`docs/DEBRICK-LINUX.md`).

Full details: [`docs/HANDOVER.md`](docs/HANDOVER.md) (start here) ·
[`docs/STATUS.md`](docs/STATUS.md) · [`docs/rewrite/audio.md`](docs/rewrite/audio.md)

## Quick build check

```bash
# host tests (codecs, gapless, ReplayGain, player): ~340 assertions
sh firmware/player/tests/run_tests.sh

# UI host tests
make -C firmware/ui test

# clean-room AP image (UI + player + codecs) → flashable IMGs
make cfw-ap                 # build/cfw/rechord_ap.elf
make cfw-img                # build/ReChord_AP.IMG + build/ReChord_AP5M.IMG

# SDK builds (BB/section_3) — see docs/STATUS.md for the full include list
make build-bb && make link-bb
```

Flash: copy `build/ReChord_AP5M.IMG` to the device root as
**`HIFIEC90.IMG`**, then **eject** (or unplug — power key also scans since
m6b) → on-device prompt → confirm → power-cycle. See
[`docs/FLASHING.md`](docs/FLASHING.md).

## Repository layout

```
firmware/rockchip/          # Rockchip RKnanoD SDK (source + codec .lib)
  ├── bbsystem/             # kernel base (Main2, CRU, DMA, interrupt, ...)
  ├── system/               # OS, fileseek, module_overlay, sysservice
  ├── audio/                # AudioControl, Effect (EQ/DSP), codecs
  └── include/              # ReChord integration headers (armcc compat, etc.)
firmware/                   # our from-source layer
  ├── startup/              # fw1 boot (ap_startup, vectors, UI loop)
  ├── ui/                   # screens (menu, browser, player, settings)
  ├── services/             # settings, log, input, FAT/exFAT fs, fw_update
  ├── drivers/              # panel, eMMC, SD (DW-MMC), USB MSC, input
  ├── player/               # playback engine: codec_t + codecs/ (wav/mp3/flac)
  │   └── codecs/           #   minimp3 + dr_flac vendored (CC0 / MIT-0)
  ├── dsp/                  # effect modules (EQ, bass, volume) — host-tested
  └── theme/                # theme data + loader
tools/                      # build, analyze, pack scripts
docs/                       # HANDOVER (start here), STATUS, re/, rewrite/
stock/                      # official firmware backups (not distributed)
```

## Roadmap

- [x] Rockchip SDK compiles (kernel + audio + codecs) — the BB base
- [x] BB custom boot (`firmware_entry` → our own `rechord_main`)
- [x] QEMU smoke test (boot + framebuffer menu + text rendering)
- [x] **AP (fw1) boots standalone with our own from-source UI**
- [x] Storage: eMMC + microSD read, FAT/exFAT browser, USB MSC, self-update
- [x] **Player linked into the AP**: gapless, ReplayGain, WAV/MP3/FLAC decode
- [ ] **DAC renderer**: I2S out to the CS43131 DACs — first audible sound
- [ ] **DSP effects**: custom EQ / bass / reverb in the render path
- [ ] **Two-core audio**: reference AP↔BB protocol re-impl (`ipc_b`) or
      single-core renderer, whichever the bring-up proves simpler

## Safety / flashing

- Flashing uses the **official upgrade method** (copy IMG + reboot) — stock is
  always restorable by copying the original back.
- No signature verification in hardware (confirmed by the community —
  `docs/community.md`); images are CRC-validated by our own updater.
- `tools/pack_img.py` / `tools/make_fw1.py` validate the scatter table
  against the reference boot model before anything is flashable (DFIU rules).
- See [`docs/FLASHING.md`](docs/FLASHING.md) for the safe procedure and
  [`docs/DEBRICK-LINUX.md`](docs/DEBRICK-LINUX.md) for maskrom recovery.

## Disclaimer

ReChord is an independent project. It is not affiliated with FiiO, SNOWSKY,
or Rockchip. It uses the publicly-available Rockchip RKnanoD SDK and
independent reverse-engineering notes. Firmware images are not distributed;
obtain official firmware from FiiO. Flashing modified firmware carries risk —
use at your own risk.

## Acknowledgments 
- https://recho.atzkey.workers.dev/ by atzkey.
The pioneering FiiO Echo Mini CFW. ReChord is a **completely independent project** in implementation and codebase from RECHO, but we gratefully acknowledge RECHO as prior inspiration for exploring custom firmware on this hardware.

- https://github.com/berettavexee/echo-mini-opus by berettavexee.
 Valuable reference implementation and technical insights for fixed-point LibOpus porting and memory-constrained audio decoding on the Rockchip RKnano platform
