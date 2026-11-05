#!/usr/bin/env python3
"""eq_asm_check.py — the Thumb-2 band_block (firmware/dsp/eq_bank.c) must be
bit-identical to its C reference.

Builds eq_bank.c twice for Cortex-M3 (asm, and -DEQ_BANK_NO_ASM), runs both
band_block()s on an emulated M3 (Unicorn) over the same random and edge-case
blocks, and compares the output samples and the saved history.

    python3 -m venv /tmp/uvenv && /tmp/uvenv/bin/pip install unicorn
    /tmp/uvenv/bin/python tools/eq_asm_check.py
"""
import os
import random
import struct
import subprocess
import sys
import tempfile

from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS
from unicorn.arm_const import (UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2,
                               UC_ARM_REG_SP, UC_ARM_REG_LR)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CODE, RAM, RET = 0x00000000, 0x20000000, 0x0000FFF0
F_ADDR, P_ADDR = RAM + 0x100, RAM + 0x1000

HARNESS = r'''
#include "%s/firmware/dsp/eq_bank.c"
__attribute__((used, section(".text.entry")))
void entry(Biquad *f, int32_t *p, int frames) { band_block(f, p, frames); }
'''


def build(tmp, asm):
    src = os.path.join(tmp, "h.c")
    out = os.path.join(tmp, "asm.bin" if asm else "c.bin")
    elf = out + ".elf"
    with open(src, "w") as fh:
        fh.write(HARNESS % ROOT)
    cmd = ["arm-none-eabi-gcc", "-mcpu=cortex-m3", "-mthumb", "-Os",
           "-ffreestanding", "-nostdlib", "-ffunction-sections",
           "-fdata-sections", "-Wl,--gc-sections", "-Wl,-e,entry",
           "-Wl,-Ttext=0", "-I" + os.path.join(ROOT, "firmware/dsp"),
           "-I" + os.path.join(ROOT, "firmware"), src, "-o", elf]
    if not asm:
        cmd.insert(1, "-DEQ_BANK_NO_ASM")
    subprocess.run(cmd, check=True)
    subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", elf, out],
                   check=True)
    nm = subprocess.run(["arm-none-eabi-nm", elf], check=True,
                        capture_output=True, text=True).stdout
    entry = next(int(l.split()[0], 16) for l in nm.splitlines()
                 if l.endswith(" entry"))
    with open(out, "rb") as fh:
        return fh.read(), entry


def run(image, entry, coeffs, hist, pcm, frames):
    uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
    uc.mem_map(CODE, 0x10000)
    uc.mem_map(RAM, 0x40000)
    uc.mem_write(CODE, image)
    uc.mem_write(F_ADDR, struct.pack("<9i", *(coeffs + hist)))
    uc.mem_write(P_ADDR, struct.pack("<%di" % len(pcm), *pcm))
    uc.reg_write(UC_ARM_REG_R0, F_ADDR)
    uc.reg_write(UC_ARM_REG_R1, P_ADDR)
    uc.reg_write(UC_ARM_REG_R2, frames & 0xFFFFFFFF)
    uc.reg_write(UC_ARM_REG_SP, RAM + 0x40000)
    uc.reg_write(UC_ARM_REG_LR, RET | 1)
    uc.emu_start(entry | 1, RET, count=50_000_000)
    out = struct.unpack("<%di" % len(pcm), uc.mem_read(P_ADDR, 4 * len(pcm)))
    st = struct.unpack("<9i", uc.mem_read(F_ADDR, 36))
    return out, st


def s32(rng):
    return rng.randint(-2**31, 2**31 - 1)


def main():
    rng = random.Random(97)
    cases = []
    # Edge cases: exact -2^31 result (C clamps to -(2^31-1)), +/- overflow,
    # empty and single-frame blocks.
    cases.append(([1 << 30, 0, 0, 0, 0], [0] * 4, [-2**31, 7], 1))
    cases.append(([2**31 - 1] * 3 + [-(2**31 - 1)] * 2, [2**31 - 1] * 4,
                  [2**31 - 1, 0] * 4, 4))
    cases.append(([2**31 - 1] * 3 + [2**31 - 1] * 2, [-2**31] * 4,
                  [-2**31, 0] * 4, 4))
    cases.append(([s32(rng) for _ in range(5)], [s32(rng) for _ in range(4)],
                  [1, 2, 3, 4], 0))
    # Realistic: Q2.30 coefficients within +/-2, 24-bit-ish audio.
    for _ in range(40):
        frames = rng.randint(1, 1024)
        co = [rng.randint(-2**31 + 1, 2**31 - 1) for _ in range(5)]
        hi = [rng.randint(-2**28, 2**28) for _ in range(4)]
        pcm = [rng.randint(-2**31, 2**31 - 1) >> rng.choice((0, 4, 8))
               for _ in range(2 * frames)]
        cases.append((co, hi, pcm, frames))
    # Fully random words (lots of saturation).
    for _ in range(40):
        frames = rng.randint(1, 512)
        cases.append(([s32(rng) for _ in range(5)],
                      [s32(rng) for _ in range(4)],
                      [s32(rng) for _ in range(2 * frames)], frames))

    with tempfile.TemporaryDirectory() as tmp:
        a_img, a_ent = build(tmp, True)
        c_img, c_ent = build(tmp, False)
        if a_img == c_img:
            sys.exit("FAIL: asm build equals C build (asm path not compiled)")
        bad = 0
        for i, (co, hi, pcm, frames) in enumerate(cases):
            ra = run(a_img, a_ent, co, hi, pcm, frames)
            rc = run(c_img, c_ent, co, hi, pcm, frames)
            if ra != rc:
                bad += 1
                print("MISMATCH case %d (frames %d)" % (i, frames))
    total = sum(c[3] for c in cases)
    print("eq_asm_check: %d cases, %d frames, %d mismatches"
          % (len(cases), total, bad))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
