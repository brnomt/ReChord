#!/usr/bin/env bash
# Apply Echo Mini (2207:262d) fixes to an upstream rockchip-linux/rkdeveloptool tree.
# Run from inside the rkdeveloptool source directory (after clone / inside community/rkdeveloptool).
set -euo pipefail

if [[ ! -f main.cpp || ! -f RKScan.cpp || ! -f RKComm.h ]]; then
  echo "error: run this from the rkdeveloptool source root (needs main.cpp, RKScan.cpp, RKComm.h)" >&2
  exit 1
fi

python3 <<'PY'
from pathlib import Path

def read(p):
    return Path(p).read_text(encoding="utf-8", errors="replace").replace("\r\n", "\n")

def write(p, s):
    Path(p).write_text(s, encoding="utf-8")

# --- RKComm.h: finite USB timeout (upstream CMD_TIMEOUT is 0 = hang forever) ---
h = read("RKComm.h")
old = "#define CMD_TIMEOUT 0"
new = "#define CMD_TIMEOUT 5000"
if old not in h and "#define CMD_TIMEOUT 5000" not in h:
    raise SystemExit("RKComm.h: expected '#define CMD_TIMEOUT 0' not found")
h = h.replace(old, new, 1)
write("RKComm.h", h)

# --- main.cpp: max 32 sectors per rockusb transfer (64+ wedges Echo Mini) ---
m = read("main.cpp")
if "#define DEFAULT_RW_LBA 32" not in m:
    if "#define DEFAULT_RW_LBA 128" not in m:
        raise SystemExit("main.cpp: expected '#define DEFAULT_RW_LBA 128' not found")
    m = m.replace(
        "#define DEFAULT_RW_LBA 128",
        "/* Echo Mini (2207:262d): bulk writes >32 sectors time out and wedge USB. */\n"
        "#define DEFAULT_RW_LBA 32",
        1,
    )
    write("main.cpp", m)

# --- RKScan.cpp: classify 2207:262d via bcdDevice (real bcdUSB 2.01 looks like Loader) ---
s = read("RKScan.cpp")
needle = "desc.usbcdUsb = descriptor.bcdUSB;"
if "idProduct == 0x262d" not in s:
    if needle not in s:
        raise SystemExit("RKScan.cpp: expected bcdUSB assignment not found")
    repl = """/* Rockchip usually overloads bcdUSB LSB as Maskrom(0)/Loader(1).
			 * Echo Mini (2207:262d) reports real USB 2.01 (LSB=1) while still
			 * in maskrom — use bcdDevice (0x0100) for that PID instead. */
			if (descriptor.idVendor == 0x2207 && descriptor.idProduct == 0x262d)
				desc.usbcdUsb = descriptor.bcdDevice;
			else
				desc.usbcdUsb = descriptor.bcdUSB;"""
    s = s.replace(needle, repl, 1)
    write("RKScan.cpp", s)

print("ok: Echo Mini patches applied (CMD_TIMEOUT=5000, DEFAULT_RW_LBA=32, bcdDevice for 262d)")
PY
