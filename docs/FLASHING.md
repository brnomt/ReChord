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

## Build pipeline (current status)

```bash
mingw32-make release        # SAFE default: custom BB + STOCK AP
                            # → build/ReChord_BB.IMG (hardware-verified to boot)
```

> **⚠️ DO NOT flash `ReChord_APBB.IMG` (custom AP + custom BB).** It
> **bricked a device on 2026-08-25** (maskrom recovery required). The custom
> fw1 memory-map table does not reproduce the stock 91-entry table, which
> lays out the whole system RAM (UI framebuffer, audio buffers, FAT cache,
> stacks) for the Mask ROM. See `make apbb-experimental` and
> `docs/fw1-packing.md` before touching fw1.

Legacy (still works):

```bash
mingw32-make all            # = make release
python tools/pack_img.py --pack build/bb/section3_custom.bin --keep-stock-tail -o build/custom.IMG
```

**Status:**
- ✅ SDK compiles (44 objects) + links with stubs
- ✅ `section3_custom.bin` has byte-correct RKnanoFW header:
  `"RKnanoFW"` + SP `0x0301E794` + count `0x52` (matches stock v3.7.0)
- ✅ `pack_img.py --identity-test` PASS (byte-identical repack)
- ✅ packaged IMG preserves bootloader + resources + trailer
- ⬜ **Drivers are stubs** — the linked firmware boots the SDK kernel (Main2)
  but hardware init is stubbed (globals zero, functions return 0)

> **What this first code flash validates:** the full pipeline — compile,
> link, pack, bootloader acceptance of our section_3. The device may show
> a black screen or not fully boot (stubs). Stock is always restorable.

---

## Flash procedure

### Step 1 — Build + pack (done)

```bash
mingw32-make all
python tools/pack_img.py --pack build/section3_custom.bin -o build/custom.IMG
```

Backup of the stock IMG is saved as `build/stock_restore_HIFIEC37.IMG`.

### Step 2 — Copy to device

1. Connect Echo Mini via USB → enters storage mode.
2. Copy `build/custom.IMG` to the **root** of internal storage as
   `HIFIEC37.IMG` (overwrite the existing one if present).
3. Eject safely.
4. **Remove the TF/micro-SD card**.
5. Reboot the player — it applies the upgrade automatically.

### Step 3 — Verify

- **If it boots:** device powers on. (With stubs, expect at most a
  boot screen or black screen — this milestone is about pipeline
  validation, not function.)
- **If it does not boot / loops:** wait ~10s, then restore stock (below).

---

## Recovery (if something goes wrong)

1. Keep `build/stock_restore_HIFIEC37.IMG` (or the stock download).
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
| `pack_img.py --identity-test` | ✅ Byte-identical IMG |
| First code flash (stub kernel) | ⬜ Next step |

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
