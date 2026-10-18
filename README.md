# ReChord

> **Re-Chord** — re-harmonize the FiiO Echo Mini. A **free, from-source custom
> firmware (CFW)** for the Echo Mini.

[!WARNING]
Work in Progress (WIP)
ReChord is still experimental, but no longer silent: the from-source fw1 boots,
the UI/library work, and audio plays from 16/44.1 up to 24-bit/96 kHz — Core B
decodes and the AP drives the CS43131 DAC. Gapless across tracks, the 192 kHz
edge case (FLAC block 16384) and the DSP chain are still in progress.
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

- **ReChord ships a from-source fw1 (AP) plus its own Core B image.** fw1 is
  our boot + UI + services + player; the Core B image (linked at
  `0x01000000`, `firmware/bb/`) is embedded in fw1 and started by our own CRU
  sequence. The AP keeps the DAC/I2S bring-up, serves the track bytes over a
  shared file ring and drains the PCM ring in its I2S TX ISR; **Core B
  decodes** (FLAC/MP3/WAV) and produces the ring. See
  [`docs/rewrite/core-b-phase3.md`](docs/rewrite/core-b-phase3.md).
- **section_3 is left stock** (preserved) as a fallback path; the boot ROM /
  sec2 is stock forever.
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
| **fw1 (AP) boots standalone** (ROM → our reset → our Main) | ✅ on hardware |
| **UI**: menus, keys, file browser (eMMC + SD, FAT16/32/exFAT) | ✅ on hardware |
| **USB mass storage** (LUN0 = eMMC r/w, LUN1 = SD r/o) | ✅ on hardware |
| **Self-updater** (copy HIFIEC90.IMG, eject/power → install) | ✅ on hardware |
| **microSD driver** (DW-MMC, 4-bit, read-only) | ✅ on hardware |
| **DAC renderer** (I2S1 → CS43131) | ✅ on hardware — audible |
| **Core B bring-up** (own image at `0x01000000`, CRU start, mailbox) | ✅ on hardware |
| **Audio on Core B** (phase 3: B decodes, AP feeds the DAC from the ring) | ✅ on hardware |
| **Hi-res**: 24-bit at 44.1/48/88.2/96 kHz, int32 left-justified path | ✅ on hardware |
| **Library**: recursive index + Album/Artist/Genre/Year/Label/… + All Songs | ✅ on hardware |
| **Playback list** (album/genre/folder queue, "i/N", shuffle, repeat) | 🟡 implemented; hard boundary per track (~2–3 s) |
| **Gapless between tracks** | ⬜ next (B-side track switch, BRNOMT-73) |
| **192 kHz / FLAC block 16384** | ⬜ needs a bigger decode window (~143 KB) |
| **384 kHz** | ⬜ not in the rate table / untested |
| **Player module** (state machine, ReplayGain, gapless trim on B, codecs) | ✅ linked, host-tested |
| **Codecs**: WAV (ours), MP3 (minimp3), FLAC (dr_flac) | ✅ vendored + host-tested |
| **DSP effects** (`EffectProcess` hook) | 🟡 core written + host-tested (`firmware/dsp/`), not in the 3c path |
| **BB (section_3) SDK build** | ✅ compiles + links (`make build-bb`), stock preserved |

- Audio flows: `Core B decodes -> shared PCM ring (0x01030000) -> AP I2S TX
  ISR -> CS43131`. The AP serves the file through the A→B file ring; `bfs_poll`
  keeps it filled. Tested on device: 16/44.1 and 24/96 clean; a slight lag and a
  per-track gap remain.
- Flash safety: images are packed behind a scatter-table gate (DFIU rules)
  and always installed via the official updater flow; stock/restorable via
  maskrom (`docs/DEBRICK-LINUX.md`).

Full details: [`docs/HANDOVER.md`](docs/HANDOVER.md) (start here) ·
[`docs/STATUS.md`](docs/STATUS.md) · [`docs/rewrite/audio.md`](docs/rewrite/audio.md)

## Quick build check

```bash
# host tests: player (codecs, playlist, gapless, ReplayGain), UI, services
sh firmware/player/tests/run_tests.sh
sh firmware/ui/tests/run_tests.sh
sh firmware/services/tests/run_tests.sh

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
  ├── player/               # engine: player.c, playlist.c, ipc_b*, codecs/
  │   └── codecs/           #   minimp3 + dr_flac vendored (CC0 / MIT-0)
  ├── bb/                   # Core B image: decoder + codec_io over the file ring
  ├── audio/                # DAC/I2S HAL: CS43131 + rknano_i2s + TX-IRQ feeder
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
- [x] **DAC renderer**: I2S1 → CS43131 — first audible sound
- [x] **Two-core audio**: our Core B image started by our CRU sequence; B
      renders the ring, then (phase 3c) decodes and produces it
- [x] **Hi-res path**: int32 left-justified end-to-end, source depth to the
      DAC (24-bit at 44.1/48/88.2/96 kHz verified)
- [x] **Library**: recursive media index + category navigation + All Songs
- [x] **Playback lists**: album/genre/folder queue, "i/N", shuffle, repeat
- [ ] **Gapless on Core B**: switch tracks without stopping the ring (BRNOMT-73)
- [ ] **192 kHz** (FLAC block 16384 needs a ~143 KB window) and **384 kHz**
- [ ] **DSP effects**: custom EQ / bass in the playback path

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
