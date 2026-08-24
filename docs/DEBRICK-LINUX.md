# ReChord — Echo Mini Debrick / Recovery (bare-metal Ubuntu)

> How to recover a bricked Snowsky Echo Mini from **maskrom mode** using a
> **bare-metal Ubuntu** machine (no WSL, no Windows tooling). The Windows
> factory tool (`RKDevelopTool.exe`) cannot do this — it only handles the
> SDK's own `Firmware.img` over USB mass storage, which needs a *booting*
> device and rejects the FiiO `HIFIEC*.IMG` container. Maskrom recovery is a
> hardware USB path that works even when the device won't boot at all.

---

## 0. What you need

- **Bare-metal Ubuntu** (any recent x86-64 release).
- The ReChord repo (contains the flasher binary + the Echo Mini loader).
- A known-good stock image: `stock/3.7.0/ECHO MINI V3.7.0/HIFIEC37.IMG`.
- A **pin / paperclip** for the RST (reset) hole.

```bash
git clone https://github.com/brnomt/ReChord.git
cd ReChord
```

> The `.IMG` and the loader are under `stock/` and `community/rkbin/`, which
> are gitignored — if you cloned on a fresh machine they won't be present.
> Copy `HIFIEC37.IMG` and the `community/` folder over from your working
> machine first (they're local-only by design).

---

## 1. Install dependencies

```bash
sudo apt-get update
sudo apt-get install -y libusb-1.0-0 libusb-1.0-0-dev usbutils
```

`rkdeveloptool` needs `libusb-1.0.so.0`; `usbutils` gives `lsusb`.

---

## 2. (One-time) Build the merged Echo Mini loader

The Echo Mini's maskrom loader is **two blobs merged into one**:

| Component | File | Role |
|---|---|---|
| FlashData (DDR init) | `community/rkbin/bin/rk30/rk3036_ddr_300MHz_v1.11.bin` | initializes DRAM |
| FlashBoot (miniloader/SPL) | `community/rkbin/bin/rk30/rk303x_echo_miniloader_v2.36.bin` | USB loader |

They're merged by the recipe `community/rkbin/RKBOOT/RK3036_ECHOMINIALL.ini`
into a single loader (`rk3036_echo_loader_v1.11.236.bin`):

```bash
cd community/rkbin
./tools/boot_merger RKBOOT/RK3036_ECHOMINIALL.ini
cd ../..
```

This writes `rk3036_echo_loader_v1.11.236.bin` into the current directory
(run it from `community/rkbin/`, since the `.ini` paths are relative to it).

> The prebuilt `rkdeveloptool` binary is at
> `community/rkbin/tools/rkdeveloptool` (64-bit Linux ELF). No compilation
> needed. `upgrade_tool` in the same dir is a *different* tool that expects
> RKAF update.img — do **not** use it for the FiiO image.

---

## 3. Enter maskrom mode

1. Press the **RST hole** (paperclip) to fully power off — *not* the power button.
2. **Hold all buttons except the power key.**
3. **Plug in USB while holding those keys.** The screen stays black — that's correct.

The device now enumerates as Rockchip maskrom:
```
Bus 00x Device 00x: ID 2207:262d Fuzhou Rockchip Electronics Company
```

Confirm:
```bash
lsusb | grep -i rockchip
```

---

## 4. (One-time) USB permissions

Either run every `rkdeveloptool` command with `sudo`, or install the udev rule
so your user can talk to the device without root:

```bash
sudo cp community/rkdeveloptool/99-rk-rockusb.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
# unplug + replug the device after this
```

---

## 5. Flash the recovery image

```bash
RKTOOL=community/rkbin/tools/rkdeveloptool
LOADER=rk3036_echo_loader_v1.11.236.bin
STOCK=stock/3.7.0/ECHO\ MINI\ V3.7.0/HIFIEC37.IMG

# 1. Confirm maskrom device is seen
sudo $RKTOOL ld

# 2. Download the merged loader (initializes DRAM + USB)
sudo $RKTOOL db $LOADER

# 3. Write the full stock image starting at sector 0
sudo $RKTOOL wl 0 "$STOCK"

# 4. Reboot into the restored firmware
sudo $RKTOOL rd
```

After `rd`, the device should boot normally (FiiO cassette UI, working).

---

## 6. What if it fails?

| Symptom | Cause / fix |
|---|---|
| `ld` shows nothing | not in maskrom — redo §3; check cable; `lsusb` |
| `db` errors / times out | loader not merged — redo §2; try the standalone miniloader `community/rkbin/bin/rk30/rk303x_echo_miniloader_v2.36.bin` |
| `wl` writes but no boot | image truncated/wrong — verify `HIFIEC37.IMG` is 33,554,436 bytes and unmodified |
| "permission denied" | redo §4 (udev) or use `sudo` |

---

## 7. Important caveats (read before flashing)

1. **The `wl 0 <full image>` sequence is the community-documented "flash the
   working IMG" step, not a command I've tested against a live device.** The
   exact LBA layout of the Echo Mini's SPI NAND is not fully mapped. If it
   doesn't recover on the first try, stop and compare notes rather than
   experimenting (each attempt rewrites flash).

2. **Flash the whole image, never the extracted sections.** The sections
   (`fw1_AP`, `sec2_bootloader`, `section3_BB`, `resources`) that
   `tools/unpack_rk.py` dumps are for analysis, not flashing — they aren't
   individually flashable and aren't 512-byte aligned.

3. **The Echo Mini IMG is NOT a Rockchip RKAF update.img.** It's an RKnanoD
   player container. That's why `upgrade_tool uf` and the Windows factory tool
   reject it — and why maskrom + `wl 0` is the correct recovery path.

---

## 8. Reference

- `community/debrick-livetrack.md` — the original community guide this is based on.
- `community/rkdeveloptool/main.cpp` — the open-source tool's `usage()` (ld/db/wl/rd).
- `community/rkbin/RKBOOT/RK3036_ECHOMINIALL.ini` — the Echo Mini loader recipe.
- `tools/unpack_rk.py` — why the FiiO files are RKnanoD containers, not RKAF.
