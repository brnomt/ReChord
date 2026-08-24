# ReChord — Echo Mini Debrick (bare-metal Ubuntu)

Recover a bricked **Snowsky Echo Mini** when the screen is black / it will not
boot. Method matches [livetrack.club/debrick](https://livetrack.club/debrick):
put the device in **maskrom**, then write a known-good firmware `.img` with
`rkdeveloptool`.

**Do not run `db`.** On this device (`USB ID 2207:262d`) the BootROM already
speaks rockusb; `DownloadBoot` (`db`) is not used and will fail. Only
`ld` → `td` → `wl` → `rd`.

Verified working on Ubuntu (2026-08-24).

---

## Prerequisites

| Item | Detail |
|---|---|
| Host | Bare-metal Ubuntu x86-64 (not WSL, not a phone) |
| Firmware image | `stock/3.7.0/ECHO MINI V3.7.0/HIFIEC37.IMG` (33 554 436 bytes). `stock/` is gitignored — copy it onto this machine if missing. |
| Tool | Open-source `rkdeveloptool` **with Echo Mini patches** (below). **Never** use `community/rkbin/tools/rkdeveloptool`. |
| Hardware | Paperclip for the RST hole; USB data cable |

---

## One-time setup

```bash
sudo apt-get update
sudo apt-get install -y libusb-1.0-0 libusb-1.0-0-dev libudev-dev \
  usbutils dh-autoreconf pkg-config git

# From the ReChord repo root:
cd /path/to/ReChord

# Prefer the local tree if present; otherwise clone upstream next to tools/
if [ ! -d community/rkdeveloptool ]; then
  git clone https://github.com/rockchip-linux/rkdeveloptool.git community/rkdeveloptool
fi

cd community/rkdeveloptool
sed -i 's/\r$//' autogen.sh configure.ac Makefile.am 2>/dev/null || true
chmod +x ../../tools/patch-rkdeveloptool-echo-mini.sh
../../tools/patch-rkdeveloptool-echo-mini.sh
./autogen.sh && ./configure && make
cd ../..

# Persist USB autosuspend disable (also required every boot — see flash section)
echo 'usbcore.autosuspend=-1' | sudo tee /etc/modprobe.d/usb-autosuspend.conf

# Optional: non-root USB access
sudo cp community/rkdeveloptool/99-rk-rockusb.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

Resulting binary (use this path everywhere below):

```text
community/rkdeveloptool/rkdeveloptool
```

Patches the script applies (required):

1. `RKComm.h` — `CMD_TIMEOUT 5000` (upstream `0` hangs forever)
2. `main.cpp` — `DEFAULT_RW_LBA 32` (chunks &gt;32 sectors time out / wedge USB)
3. `RKScan.cpp` — for VID/PID `2207:262d`, use `bcdDevice` so `ld` reports **Maskrom** (not Loader)

---

## Enter maskrom (every recovery)

1. Press **RST** with a paperclip (fully power off). Do **not** use the power button.
2. Hold **all buttons except the power key**.
3. While still holding, plug USB into the PC.
4. Screen stays **completely black** — that is expected.

Confirm:

```bash
lsusb | grep -i rockchip
# Expect: ID 2207:262d Fuzhou Rockchip Electronics Company
```

If that line is missing, repeat the RST / hold / plug sequence.

---

## Flash (exact commands)

Run from the **ReChord repo root**. Do not insert `db`, `cs`, or other probes.

```bash
cd /path/to/ReChord

sudo sh -c 'echo -1 > /sys/module/usbcore/parameters/autosuspend'

RKTOOL=./community/rkdeveloptool/rkdeveloptool
STOCK='stock/3.7.0/ECHO MINI V3.7.0/HIFIEC37.IMG'

test -x "$RKTOOL" || { echo "missing $RKTOOL — build it in One-time setup"; exit 1; }
test -f "$STOCK"  || { echo "missing $STOCK"; exit 1; }

sudo "$RKTOOL" ld
# Expect a line like:
#   DevNo=1  Vid=0x2207,Pid=0x262d,LocationID=...  Maskrom

sudo "$RKTOOL" td
# Expect: Test Device OK.
# If this fails: USB session is wedged — unplug, re-enter maskrom, start again from ld.

sudo "$RKTOOL" wl 0 "$STOCK"
# Expect progress up to: Write LBA from file (100%)

sudo "$RKTOOL" rd
# Expect: Reset Device OK.
```

After `rd`, `lsusb | grep rockchip` should show nothing (maskrom gone). Power on /
wait: cassette UI should return.

---

## Troubleshooting

| Symptom | Action |
|---|---|
| `creating comm object failed` / `libusb_open` EIO | `sudo sh -c 'echo -1 > /sys/module/usbcore/parameters/autosuspend'`, re-enter maskrom |
| `command is invalid!` on `ld` | Wrong binary (`rkbin/tools/...`). Use `./community/rkdeveloptool/rkdeveloptool` |
| `ld` shows `Loader` instead of `Maskrom` | Patches not applied — re-run `tools/patch-rkdeveloptool-echo-mini.sh` and `make` |
| `td` / `wl` fails after earlier errors | Re-enter maskrom (failed USB ops wedge the session) |
| `wl` hangs near 0% | Unpatched tool (LBA chunk too large) — apply patch, rebuild |
| Tempted to run `db` | **Don't.** It fails on this BootROM and can wedge USB |

---

## Notes for automation / other agents

- Order is mandatory: autosuspend → `ld` → `td` → `wl 0` → `rd`.
- Never run `db` / `ul` / `cs` / opcode experiments during recovery.
- Flash the **entire** `HIFIEC37.IMG` only — not files from `tools/unpack_rk.py`.
- Image format is RKnanoD player container, not RKAF (`upgrade_tool uf` is wrong).
- Community overview: `community/debrick-livetrack.md`
