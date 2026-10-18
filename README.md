<div align="center">

# ReChord

**Re-harmonize the FiiO Echo Mini.**

A free, **from-source** custom firmware for the Echo Mini — not a patch, not a
byte-mod of the stock binary.

`dual-core` · `from source` · `GPL-3.0`

[What works](#what-works-today) · [Architecture](#architecture) · [Build](#build--test) · [Flash](#flash) · [Docs](#documentation)

</div>

> [!WARNING]
> **Work in progress.** ReChord is experimental, but no longer silent: the
> from-source fw1 boots, the UI/library work, and audio plays from 16/44.1 up to
> **24-bit / 96 kHz** — Core B decodes and the AP drives the CS43131 DAC.
> Gapless across tracks, the 192 kHz edge case (FLAC block 16384) and the DSP
> chain are still in progress. Driver/hardware changes must be verified on
> physical hardware before merge.

## What works today

- **Boots standalone** — ROM → our reset → our `Main`; the stock AP layer is gone.
- **UI** — menus, keys, file browser over eMMC + microSD (FAT16/32/exFAT), themes.
- **Storage & USB** — eMMC read/write, microSD read, USB mass storage, self-updater.
- **Audio** — Core B decodes **FLAC / MP3 / WAV**; the AP owns the DAC/I2S
  bring-up and feeds the CS43131 from a shared PCM ring.
- **Hi-res** — int32 left-justified end-to-end, source bit depth reported to the
  DAC; 24-bit at 44.1/48/88.2/96 kHz verified on device.
- **Library** — recursive media index + Album / Artist / Genre / Year / Label /
  Album Artist / Composer and **All Songs**.
- **Playback lists** — playing a song inside a list queues that list, shows
  **`i/N`**, auto-advances, with Rockbox-style **shuffle** and
  **repeat off/all/one**.

## Architecture

The Echo Mini is a **dual Cortex-M3** (RKnanoD). ReChord ships a from-source
**fw1 (AP)** plus its **own Core B image**, started by our own CRU sequence:

```
                shared memory (0x0102xxxx..0x0106xxxx)
  ┌───────────────┐   A→B file ring   ┌──────────────────┐
  │  fw1  (AP)    │ ───────────────▶ │  Core B (CAL_M3) │
  │  UI/services  │                   │  FLAC/MP3/WAV    │
  │  DAC / I2S    │ ◀─────────────── │  decoder         │
  └───────┬───────┘   PCM ring        └──────────────────┘
          │
          ▼
     CS43131 DAC  →  headphones
```

| Part | Role |
|------|------|
| **fw1 (AP)** | Boot, UI, services, filesystem, player state machine, DAC/I2S bring-up, I2S TX-IRQ feeder |
| **Core B** | Decodes the track (our codecs) and produces the PCM ring; data/pool live in its own memory region |
| **Mask ROM** | Fixed hardware HAL (LCD, I2S, DAC, USB, keys) — not replaced; it is the "BIOS" our code calls |

Details: [`docs/rewrite/core-b-phase3.md`](docs/rewrite/core-b-phase3.md) ·
[`docs/rewrite/audio.md`](docs/rewrite/audio.md) ·
[`docs/rewrite/player.md`](docs/rewrite/player.md).

> **This is not a decompilation project and not a patch project.** The
> reverse-engineering phase produced the addresses, the SDK integration and the
> hardware map that make a from-source build possible — see
> [`docs/RE-HISTORY.md`](docs/RE-HISTORY.md).

## Status

| Area | State |
|------|:-----:|
| fw1 boots standalone | ✅ |
| UI, file browser, storage, USB, self-updater | ✅ |
| DAC renderer (I2S1 → CS43131) | ✅ audible |
| Core B bring-up + audio (B decodes, AP feeds) | ✅ |
| Hi-res 24-bit @ 44.1/48/88.2/96 kHz | ✅ |
| Library index + category navigation | ✅ |
| Playback lists, `i/N`, shuffle, repeat | 🟡 hard boundary per track (~2–3 s) |
| Gapless between tracks | ⬜ BRNOMT-73 |
| 192 kHz (FLAC block 16384) | ⬜ needs a ~143 KB decode window |
| 384 kHz | ⬜ not in the rate table / untested |
| DSP effects in the playback path | 🟡 core written + host-tested |

## Build & test

```bash
# Host tests (player: codecs/playlist/gapless/ReplayGain · UI · services)
sh firmware/player/tests/run_tests.sh
sh firmware/ui/tests/run_tests.sh
sh firmware/services/tests/run_tests.sh

# Clean-room AP image (UI + player + Core B decoder) → flashable IMGs
make cfw-ap        # build/cfw/rechord_ap.elf
make cfw-img       # build/ReChord_AP.IMG + build/ReChord_AP5M.IMG

# SDK builds (section_3) — see docs/STATUS.md for the include list
make build-bb && make link-bb
```

## Flash

Copy `build/ReChord_AP5M.IMG` to the device root as **`HIFIEC90.IMG`**, then
**eject** (or unplug — the power key also scans) → confirm on the device prompt →
power-cycle. Full procedure: [`docs/FLASHING.md`](docs/FLASHING.md). Recovery:
[`docs/DEBRICK-LINUX.md`](docs/DEBRICK-LINUX.md).

## Repository layout

```
firmware/rockchip/            # Rockchip RKnanoD SDK (source + codec .lib)
  ├── bbsystem/               # kernel base (Main2, CRU, DMA, interrupt, ...)
  ├── system/                 # OS, fileseek, module_overlay, sysservice
  ├── audio/                  # AudioControl, Effect (EQ/DSP), codecs
  └── include/                # ReChord integration headers (armcc compat, etc.)
firmware/                     # our from-source layer
  ├── startup/                # fw1 boot (ap_startup, vectors, UI loop)
  ├── ui/                     # screens (menu, browser, player, settings)
  ├── services/               # settings, log, input, FAT/exFAT fs, fw_update
  ├── drivers/                # panel, eMMC, SD (DW-MMC), USB MSC, input
  ├── player/                 # engine: player.c, playlist.c, ipc_b*, codecs/
  │   └── codecs/             #   minimp3 + dr_flac vendored (CC0 / MIT-0)
  ├── bb/                     # Core B image: decoder + codec_io over the file ring
  ├── audio/                  # DAC/I2S HAL: CS43131 + rknano_i2s + TX-IRQ feeder
  ├── dsp/                    # effect modules (EQ, bass, volume) — host-tested
  └── theme/                  # theme data + loader
tools/                        # build, analyze, pack scripts
docs/                         # HANDOVER (start here), STATUS, re/, rewrite/
stock/                        # official firmware backups (not distributed)
```

## Roadmap

- [x] Rockchip SDK compiles (kernel + audio + codecs)
- [x] BB custom boot (`firmware_entry` → our own `rechord_main`)
- [x] **AP (fw1) boots standalone with our own from-source UI**
- [x] Storage: eMMC + microSD, FAT/exFAT browser, USB MSC, self-update
- [x] **DAC renderer**: I2S1 → CS43131 — first audible sound
- [x] **Two-core audio**: our Core B image started by our CRU sequence
- [x] **Core B decodes** (phase 3c) — the AP keeps the DAC and file service
- [x] **Hi-res path**: int32 end-to-end, source depth to the DAC
- [x] **Library**: recursive index + category navigation + All Songs
- [x] **Playback lists**: queue, `i/N`, shuffle, repeat
- [ ] **Gapless on Core B** — switch tracks without stopping the ring
- [ ] **192 kHz** (bigger decode window) and **384 kHz**
- [ ] **DSP effects** in the playback path

## Documentation

- [`docs/HANDOVER.md`](docs/HANDOVER.md) — session log and decisions (**start here**)
- [`docs/STATUS.md`](docs/STATUS.md) — build status and history
- [`docs/rewrite/`](docs/rewrite/) — architecture: audio, player, DAC, Core B
- [`docs/re/`](docs/re/) — reverse-engineering notes and decompilation
- [`docs/HARDWARE.md`](docs/HARDWARE.md) · [`docs/TRM-notes.md`](docs/TRM-notes.md) — SoC map
- [`docs/FLASHING.md`](docs/FLASHING.md) · [`docs/DEBRICK-LINUX.md`](docs/DEBRICK-LINUX.md) — flash / recovery

## Safety

- Flashing uses the **official upgrade method**; stock is always restorable by
  copying the original image back.
- Images are validated by our packer (scatter-table gate, CRC trailer) before
  they can be flashed; recovery is via maskrom.
- No signature verification exists in hardware (community-confirmed). Flashing
  modified firmware carries risk — use at your own risk.

## License

GPL-3.0 — see [`LICENSE`](LICENSE). Vendored decoders keep their own licenses
(minimp3 CC0, dr_flac MIT-0).

## Disclaimer

ReChord is an independent project, not affiliated with FiiO, SNOWSKY or Rockchip.
It builds on the publicly available Rockchip RKnanoD SDK and independent
reverse-engineering notes. Firmware images are not distributed; obtain official
firmware from FiiO.

## Acknowledgments

- [recho.atzkey.workers.dev](https://recho.atzkey.workers.dev/) by **atzkey** —
  the pioneering Echo Mini CFW. ReChord is a completely independent project in
  implementation and codebase, but we gratefully acknowledge RECHO as prior
  inspiration for custom firmware on this hardware.
- [echo-mini-opus](https://github.com/berettavexee/echo-mini-opus) by
  **berettavexee** — reference implementation and technical insights for
  fixed-point LibOpus porting and memory-constrained audio decoding on the
  Rockchip RKnano platform.
