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
> Track-to-track **gapless works within one format** (same rate / channels /
> depth); a format change stays a **hard boundary by design**, because the DAC
> has to re-lock (an album that mixes rates, e.g. 96k and 44.1k, will show a
> short pause there). Still open: the 192 kHz edge case (FLAC max_block 16384
> needs a ~143 KB decode window) and the DSP chain. Driver/hardware changes must
> be verified on physical hardware before merge.

## What works today

- **Boots standalone** — ROM → our reset → our `Main`; the stock AP layer is gone.
- **UI** — monochrome house look (black/white + one grey, inverted selection,
  1 px rules, no debug clutter outside the System view); menus, keys, file
  browser over eMMC + microSD (FAT16/32/exFAT), themes, Now Playing with
  title/artist, position/duration, stream format, `i/N` and `SHUF`/`REP1`, plus
  list wrap-around and cursor memory.
- **Storage & USB** — eMMC read/write, microSD read, USB mass storage,
  self-updater. With a cable in, "off" parks the player (dark + silent + still
  charging) and a power press wakes it — the rail cut only works on battery.
- **Audio** — Core B decodes **FLAC / MP3 / WAV**; the AP owns the DAC/I2S
  bring-up and feeds the CS43131 from a shared PCM ring.
- **Hi-res** — int32 left-justified end-to-end, source bit depth reported to the
  DAC; 24-bit at 44.1/48/88.2/96 kHz verified on device.
- **Library** — recursive media index (v3, dual `INDEX.A`/`INDEX.B` slots with a
  CRC-checked commit) with Album / Artist / Genre / Year / Label / Album Artist
  / Composer and **All Songs**; names sort case-insensitively, paths up to 208
  chars. A reindex is only needed when the layout changes.
- **Playback lists** — playing a song inside a list queues that list, shows
  **`i/N`**, auto-advances, with Rockbox-style **shuffle** (reorders the list)
  and **repeat off/all/one**, both persisted in `rechord.cfg`.
- **Resume** — the `resume` setting (off/folder/track) restores the last track
  at its position plus the browser folder/row from `/.rechord/state`, written
  in place and deferred. It opens **paused** (never autoplays).
- **Diagnostics** — a RAM log that persists itself (append + rotate at 32 KiB,
  written when there is news, at most every 30 s) so a session can be read back
  without plugging the USB cable.

## Architecture

The Echo Mini is a **dual Cortex-M3** (RKnanoD). ReChord ships a from-source
**fw1 (AP)** plus its **own Core B image**, started by our own CRU sequence:

```
                shared memory (0x0102xxxx..0x0106xxxx)
  ┌───────────────┐   A→B file rings  ┌──────────────────┐
  │  fw1  (AP)    │ ───────────────▶ │  Core B (CAL_M3) │
  │  UI/services  │   (current +     │  FLAC/MP3/WAV    │
  │  DAC / I2S    │    pre-opened)   │  decoder         │
  └───────┬───────┘ ◀─────────────── └──────────────────┘
          │           PCM ring (64 KiB)
          ▼
     CS43131 DAC  →  headphones
```

| Part | Role |
|------|------|
| **fw1 (AP)** | Boot, UI, services, filesystem, player state machine, DAC/I2S bring-up, I2S TX-IRQ feeder that drains the PCM ring |
| **Core B** | Decodes the track (our codecs) and produces the PCM ring; data/pool/stack live in its own memory region |
| **Mask ROM** | Fixed hardware HAL (LCD, I2S, DAC, USB, keys) — not replaced; it is the "BIOS" our code calls |

At a track boundary the AP hands Core B the next file **without stopping the
stream**: the next file is pre-opened into a second file ring ~3 s before the
end (with its codec opened and its FLAC prescan geometry published), and the
boundary is a `NEXT` command over the live ring. Details:
[`docs/rewrite/core-b-phase3.md`](docs/rewrite/core-b-phase3.md) ·
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
| Playback lists, `i/N`, shuffle, repeat | ✅ |
| Gapless between **same-format** tracks (Core B, BRNOMT-73) | ✅ verified on device |
| **Format-change** boundary | hard by design (DAC re-lock, ~200 ms) |
| 96 kHz margins (ring cover, FIFO refill) | 🟡 every time budget halves at 96k; occasional crackles under investigation |
| 192 kHz (FLAC block 16384) | ⬜ rate is in the table; needs a ~143 KB decode window (the workbuf is 80,896 B) |
| 384 kHz | ⬜ not in the rate table / untested |
| DSP effects in the playback path | 🟢 5–10 band EQ on Core B with runtime profiles (`docs/rewrite/eq.md`) |
| RMF1 module loader | 🟡 host-tested; later milestone |

## Build & test

```bash
# Host tests (player: codecs/playlist/gapless/ReplayGain · UI · services ·
# drivers · modules)
sh firmware/player/tests/run_tests.sh
sh firmware/ui/tests/run_tests.sh
sh firmware/services/tests/run_tests.sh
sh firmware/drivers/tests/run_tests.sh
sh firmware/modules/tests/run_tests.sh

# Clean-room product image (our AP + Core B + from-source codecs)
make               # = make standalone-ap5m -> build/ReChord_AP5M.IMG
make cfw-ap        # ELF only -> build/cfw/rechord_ap.elf

# Reproducibility acceptance: two clean worktrees, byte-identical image
make clean-clone-check
```

`make` needs no stock image, no `community/` SDK and no proprietary `.lib`.
The legacy `release`/`all` and `cfw-img` targets were removed (BRNOMT-82).

## Flash

Copy `build/ReChord_AP5M.IMG` to the device root as **`HIFIEC90.IMG`**, then
**eject** (or unplug — the power key also scans) → confirm on the device prompt →
power-cycle. Full procedure: [`docs/FLASHING.md`](docs/FLASHING.md). Recovery:
[`docs/DEBRICK-LINUX.md`](docs/DEBRICK-LINUX.md).

## Repository layout

```
firmware/rockchip/            # Rockchip RKnanoD SDK (source + codec .lib)
  ├── bbsystem/               # kernel base (Main2, CRU, DMA, interrupt, ...)
  ├── system/                 # OS, fileseek, module_overlay, sysservice, debug
  ├── audio/                  # AudioControl, Effect (EQ/DSP), codecs
  └── include/                # ReChord integration headers (armcc compat, etc.)
firmware/                     # our from-source layer
  ├── startup/                # AP boot: ap_startup/ap_main/ap_ui + vectors + UI loop
  ├── app/                    # clean-room app/registry glue (host-tested wiring)
  ├── ui/                     # screens (menu, browser, player, settings, library)
  ├── services/               # settings, log, input, FAT/exFAT fs, media index,
  │                           #   store, fw_update
  ├── drivers/                # panel, eMMC, SD (DW-MMC), USB MSC/CDC, I2S, DAC
  ├── player/                 # engine: player.c, playlist.c, bfs.c, ipc_*, codecs/
  │   └── codecs/             #   wav + minimp3 + dr_flac vendored (CC0 / MIT-0)
  ├── bb/                     # Core B image: decoder + codec_io over the file rings
  ├── bb_stub/                # minimal section_3 stub image (bb-stub target)
  ├── audio/                  # DAC/I2S HAL: CS43131 + rate table + TX-IRQ feeder
  ├── dsp/                    # effect modules (EQ, bass, volume) — not linked yet
  ├── modules/                # RMF1 module loader/registry — later milestone
  ├── theme/                  # theme data + loader
  ├── libc/                   # freestanding libc (mem/str/stdio/math shims)
  └── (dev trees: hook, qemu, mb1, config, util)
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
- [x] **Playback lists**: `i/N`, shuffle, repeat (+ persistence)
- [x] **Gapless between same-format tracks** on Core B — verified on device; a
      format change stays a hard boundary (the DAC must re-lock)
- [x] **Resume**: last track/position + browser folder/row in `/.rechord/state`
      (`resume` off/folder/track), restored paused
- [ ] **96 kHz margins** — 96k halves the ring cover and the TX-FIFO refill
      margin; occasional crackles there are still being chased
- [ ] **192 kHz** (bigger decode window) and **384 kHz**
- [x] **EQ** 5–10 bands with runtime profiles on Core B (`docs/rewrite/eq.md`)
- [x] **ReplayGain** track/album/auto normalization (`docs/rewrite/audio.md`)
- [x] **Backlight brightness** 1–10 (PWM on GPIO2.4)
- [ ] **RMF1 modules** (loader host-tested; later milestone)

## Documentation

- [`docs/HANDOVER.md`](docs/HANDOVER.md) — session log and decisions (**start here**)
- [`docs/THEMING.md`](docs/THEMING.md) — theme package format, skin language and runtime variables
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
- [EchoPod](https://github.com/BasilSkyWalk/echopod) by **BasilSkyWalk**
  (GPL-3.0) — the 1–10 backlight brightness mapping and the 10 kHz GPIO2.4
  PWM approach; ReChord's `panel_io_rknanoc.c` backlight implementation is
  adapted from it.
