# ReChord

> **Re-Chord** — re-harmonize the FiiO Echo Mini. A **free, from-source custom
> firmware (CWF)** for the Echo Mini — a **"Rockbox for the FiiO"**.

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
| **fw1 (AP)** | `0x0007B8 – 0x057820` | **UI** — menus, navigation, fonts, I2C, file browser | **RKnanoD SDK** — App/UI build (`main.c` + `SDK_160_128/UI/` 45 `.c`) |
| **section_3 (BB)** | `0x081A14 – 0x9BAA0E` | **Audio** — codecs, DSP/EQ, playback engine | **RKnanoD SDK** — BB build (`Main2.c` + codecs) |

- **Both firmwares are two builds of the same Rockchip RKnanoD SDK**
  (confirmed by string match: fw1 = 223 RKnanoD_MP3 strings vs only 32
  rk3399-table). The AP is the `main.c` + `UI/` build; the BB is the
  `Main2.c` + codecs build.
- We compile both SDK builds from source and write **our own app layer** on
  top (our UI for the AP, our DSP/effects for the BB).
- The **mask ROM** (`0x00000000–0x02FFFFFF`) is the fixed hardware HAL
  (LCD, I2S, DAC, USB, keys) — it is *not* firmware we replace; it is the
  "BIOS" our code calls.

> **This is NOT a decompilation project, and NOT a patch project.** The
> reverse-engineering phase (2026) produced the knowledge (addresses, SDK
> integration, hardware map) that makes the from-source build possible. See
> `docs/RE-HISTORY.md`.

## Status (Aug 2026)

ReChord is currently under active development and should be considered experimental compared to established alternatives

| Layer | Status |
|-------|--------|
| **Rockchip SDK (BB: kernel + audio + codecs)** | ✅ compiles (53 `.c`) |
| **BB custom boot** (`firmware_entry` → `rechord_main` on the SDK) | ✅ builds, links, QEMU-verified |
| **BB display** (our menu → LCD) | 🟡 in progress (framebuffer → LCD DMA is the open problem) |
| **AP / UI from scratch** (menus, navigation, fonts) | ⬜ next — needs fw1 mapped |
| **DSP effects** (`Effect.c` / `EffectProcess`) | ⬜ hook ready, effects to write |
| **Flash** (pack IMG → copy + reboot) | ✅ pipeline verified (byte-exact header/resources) |

- Ghidra project: `FIIO-3.7.0-Decomp`, language **`ARM:LE:32:v8-m`** (both
  fw1/AP and section_3/BB imported; section_3 loads at `0x03000000`).
- `tools/pack_img.py` replaces section_3 (BB) and preserves bootloader +
  resources. Replacing **fw1 (AP)** is the missing piece for the UI.

Full details: [`docs/STATUS.md`](docs/STATUS.md) · [`docs/dispatch-map.md`](docs/dispatch-map.md) · [`docs/community.md`](docs/community.md)

## Quick build check

```bash
# any SDK .c compiles (see docs/STATUS.md for full include list)
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

## Repository layout

```
firmware/rockchip/          # Rockchip RKnanoD SDK (source + codec .lib)
  ├── bbsystem/             # kernel base (Main2, CRU, DMA, interrupt, ...)
  ├── system/               # OS, fileseek, module_overlay, sysservice
  ├── audio/                # AudioControl, Effect (EQ/DSP), codecs
  └── include/              # ReChord integration headers (armcc compat, etc.)
firmware/                   # our from-source layer (rechord_app, UI, stubs)
tools/                      # build, analyze, pack scripts
docs/                       # STATUS, dispatch-map, community, hardware, flashing
stock/                      # official firmware backups (not distributed)
```

## Roadmap

- [x] Rockchip SDK compiles (kernel + audio + codecs) — the BB base
- [x] BB custom boot (`firmware_entry` → our own `rechord_main`)
- [x] QEMU smoke test (boot + framebuffer menu + text rendering)
- [x] **AP identified**: fw1 = RKnanoC SDK (`rk3399-table-RKNanoC`, 103 `.c`)
- [ ] **BB display**: get our framebuffer to the LCD (find the ROM DMA/transfer)
- [ ] **AP (fw1) UI**: compile the RKnanoC SDK (drivers + FileSys + NANO_OS) + our menus
- [ ] **DSP effects**: custom EQ / bass / reverb in `EffectProcess`
- [ ] **Flash the full CWF** (AP + BB) and verify on hardware

## Safety / flashing

- Flashing uses the **official upgrade method** (copy IMG + reboot) — stock is
  always restorable by copying the original back.
- No signature verification in hardware (confirmed by the community —
  `docs/community.md`).
- `tools/pack_img.py` preserves bootloader + resources; it replaces the code
  section(s) cleanly.
- See [`docs/FLASHING.md`](docs/FLASHING.md) for the safe procedure.

## Disclaimer

ReChord is an independent project. It is not affiliated with FiiO, SNOWSKY,
or Rockchip. It uses the publicly-available Rockchip RKnanoD SDK and
independent reverse-engineering notes. Firmware images are not distributed;
obtain official firmware from FiiO. Flashing modified firmware carries risk —
use at your own risk.

## Alternatives
https://recho.atzkey.workers.dev/ by atzkey.
An existing tool serving a similar purpose. ReChord is an independent project developed from scratch with a different architecture and focus
