# ReChord — Flashing Guide (Echo Mini)

How to get a modified firmware onto your device, what works today, and what is still missing.

## How Fiio Upgrades Work (Official)

From the stock `Read me.txt`:

1. Copy `HIFIEC37.IMG` (or `HIFIECxx.IMG` for your version) to the **root** of the player's **internal storage** (USB mass-storage mode).
2. **Remove the TF/micro-SD card** if one is inserted.
3. **Reboot** the player — it upgrades automatically.

No PC flashing tool is required. The player reads the IMG from internal flash and applies it on boot.

> **Warning:** Some major upgrades (e.g. v2.4.0) **format internal storage**. Back up music and data before flashing anything custom.

> **Verificación de firma (confirmado por la comunidad):** el IMG **NO tiene
> verificación de firma en el hardware**. Hay un CRC de 8 bits (Rockchip
> **RKCRC**) y un bloque tipo hash al final, pero **ningún código lo
> verifica** en el dispositivo (la verificación, si existe, estaría en la
> herramienta de flasheo oficial, que el Echo Mini no usa). La comunidad
> (blog RSE de Takobin) flasheó firmwares modificados sin ninguna
> interceptación. Ver `docs/community.md`.

---

## Build pipeline (current)

```bash
make                # = make standalone-ap5m
                    # -> build/ReChord_AP5M.IMG  (5 MB, the only image to flash)
make cfw-ap         # ELF only -> build/cfw/rechord_ap.elf
make clean-clone-check   # reproducibility: two clean worktrees, same SHA-256
```

Everything is built from tracked source: our clean-room AP (`firmware/ap.mk`),
our Core B decoder (`firmware/bb`, embedded as a blob) and the from-source
codecs. `make` does **not** read `stock/`, does **not** link `community/` SDK
blobs and does **not** use proprietary `.lib` archives.

The legacy targets (`make release`, `make all`, `make cfw-img`, the SDK
BB/AP pipeline and the stock splice packers) were removed in BRNOMT-82. The
historical packers stay in `tools/` for reference only.

> **⚠️ AP code span: stay below 0x03088000.** The stock loader code lives
> there and the AP image must not overwrite it. The linker caps `FW_CODE` at
> `0x03050000 + 0x38000 = 0x03088000` and `tools/make_fw1.py` fails the pack
> when a row crosses the loader base (DFIU `51cc212d`). Flash only
> `build/ReChord_AP5M.IMG`.

---

## Flash procedure

### Step 1 — Build

```bash
make
sha256sum build/ReChord_AP5M.IMG
```

### Step 2 — Copy to device

1. Connect Echo Mini via USB → enters storage mode.
2. Copy `build/ReChord_AP5M.IMG` to the **root** of internal storage as
   `HIFIEC90.IMG`. The updater accepts `HIFIECnn.IMG` (two digits) or
   `ECHOMINI.IMG`; do not overwrite the stock `HIFIEC37.IMG`.
3. Eject safely (our AP offers the update prompt; the stock/RECHO updater
   installs silently on reboot).
4. **Remove the TF/micro-SD card.**
5. Power-cycle — the player applies the upgrade.

### Step 3 — Verify

- **If it boots:** ReChord UI (cassette home by default). The update result is
  in `/.rechord/LOG`.
- **If it does not boot / loops:** wait ~10 s, then restore stock (below).

---

## Recovery (if something goes wrong)

1. Keep a stock image: `stock/3.7.0/ECHO MINI V3.7.0/HIFIEC37.IMG` (local,
   gitignored) or the official download.
2. Connect device → storage mode. If the device won't enter storage mode,
   power off and retry; the bootloader A/B fallback may also auto-restore.
3. Copy the **stock** IMG to root as `HIFIEC37.IMG`, remove TF card, reboot.
4. If the device won't boot at all, use **maskrom recovery** (a hardware
   path that works even when normal boot fails):
   - Press the **RST hole** (paperclip) to power off — NOT the power button.
   - Hold **all buttons except power** while plugging in USB (screen stays black).
   - Confirm maskrom in `dmesg`: `idVendor=2207` (Rockchip), `idProduct=262d`.
   - Flash a known-good `HIFIEC37.IMG` with **rkdeveloptool**
     (`community/rkdeveloptool/`).
   - Full guide: `community/debrick-livetrack.md` (saved from livetrack.club/debrick).

---

## What was verified on hardware so far

| Test | Result |
|------|--------|
| Resource mod (boot animation colors) | ✅ Flashed, device booted, reversible |
| Clean-room AP5M (our AP + Core B) | ✅ Boots on hardware (verdict #20 lineage) |
| AP5M + cassette/standby/WAV/MP3/EQ | ✅ Boots and plays (2026-10-24) |

---

## IMG File Layout (Reference)

```
HIFIEC37.IMG (~32 MB)
├── Outer header          @ 0x000000
├── Section 1             @ 0x0001F8   Relocation / segment table
├── Section 2             @ 0x057820   Bootloader (~173 KB)
├── Section 3             @ 0x081A14   Main firmware (ARM Thumb-2)  ← REPLACED
├── Section 4             @ 0x1FC41F8  Padding
└── EOF trailer           last 4 bytes  0x1EA1C309 (LE)
```

Part 5 (inside section 3 region): `ROCK26IMAGERES` — 1617 RGB565 UI bitmaps.

### RKnanoFW section_3 header (16 bytes @ 0x03000000)

```
[0:8]  "RKnanoFW"  magic
[8:12] 0x0301E794  initial SP (main stack base)
[12:16]0x00000052  count/flags
```

`firmware_entry` is at 0x03000010 (bootloader jumps here, r0 = boot params).

---

## Related Docs

- [STATUS.md](STATUS.md) — build status and roadmap
- [HARDWARE.md](HARDWARE.md) — memory map, registers, segment table
- [RE-HISTORY.md](RE-HISTORY.md) — decompilation → SDK pivot history
- [re/](re/) — archived RE docs + decompiled spec (`re/decomp/`)
