#!/usr/bin/env python3
"""
unpack_rk.py — unpack Rockchip firmware images (pure Python 3, no mmap,
no C deps, Windows-native).

Supports FOUR container formats (auto-detected):

  1. RKAF  — Rockchip update.img  (the classic Android/update.img with
            "RKAF" magic and a file table: boot.img, kernel.img,
            parameter.txt, MiniLoaderAll.bin, system, misc, recovery...)
  2. RKFW  — loader image that embeds a Boot/LDR stage + an inner RKAF.
  3. RKFP  — GPT-like partition format (sector table).
  4. RKnanoD player container — the format FiiO ships (stock/*.IMG). It is
            NOT RKAF; it starts with a version code + "Rockchip" + "RKnano
            SDK 1.0" and contains "RKnanoFW" sections (fw1/AP, sec2
            bootloader, section_3/BB) plus a ROCK26IMAGERES resource blob.

The RKAF/RKFW/RKFP layouts are transcribed from the reference rkunpack.c
(tools/rkflashtool), which uses mmap() and is POSIX-only; this is a clean
port that reads the file into memory instead, so it runs on Windows.

CLI:
    python unpack_rk.py <input_firmware.img> [-o OUTDIR]

It prints a partition/section table with byte offsets, 512-byte sector
offsets (what rkdeveloptool's `db`/`wl` commands take), and sizes, then
writes each partition to OUTDIR.
"""

from __future__ import annotations

import argparse
import os
import struct
import sys
from pathlib import Path

SECTOR = 512


def u32(b: bytes, off: int) -> int:
    return struct.unpack_from("<I", b, off)[0]


def cstr(b: bytes, off: int, maxlen: int) -> str:
    raw = b[off:off + maxlen]
    nul = raw.find(b"\x00")
    if nul >= 0:
        raw = raw[:nul]
    return raw.decode("utf-8", "replace").strip()


class Unpacker:
    def __init__(self, data: bytes, src_path: Path):
        self.d = data
        self.src = src_path
        self.outdir = Path(".")
        self.table: list[tuple[str, int, int, int]] = []  # (name, off, size, fsectors)

    def emit(self, name: str, off: int, size: int, relpath: str | None = None):
        if off < 0 or off + size > len(self.d):
            print(f"  [skip] {name}: bad range off=0x{off:X} size=0x{size:X}")
            return
        sector = off // SECTOR
        size_sectors = (size + SECTOR - 1) // SECTOR
        self.table.append((name, off, size, sector))
        print(f"  {off:08X}  {size:>10}  sector {sector:>8}  {name}")
        if relpath is None:
            relpath = name
        out = self.outdir / relpath
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(self.d[off:off + size])

    # ---- RKAF: classic update.img -------------------------------------
    def unpack_rkaf(self) -> int:
        print("Format: RKAF (Rockchip update.img)")
        total = u32(self.d, 4) + 4
        if total != len(self.d):
            print(f"  note: header says {total} bytes, file is {len(self.d)} bytes")
        print(f"  model:        {cstr(self.d, 0x08, 34)!r}")
        print(f"  manufacturer: {cstr(self.d, 0x48, 30)!r}")
        count = u32(self.d, 0x88)
        print(f"  files: {count}")
        n = 0
        p = 0x8C
        for _ in range(count):
            name = cstr(self.d, p, 32)
            path = cstr(self.d, p + 0x20, 32)
            ioff = u32(self.d, p + 0x60)
            noff = u32(self.d, p + 0x64)
            isize = u32(self.d, p + 0x68)
            fsize = u32(self.d, p + 0x6C)
            p += 0x70
            if path.startswith("SELF"):
                print(f"  [skip] SELF entry {name}")
                continue
            out_off, out_size = ioff, fsize
            # parameter.txt is wrapped in an 8-byte header + 4-byte CRC footer
            if name.startswith("parameter"):
                out_off += 8
                out_size -= 12
            safe = path.replace("/", os.sep) if path else name
            self.emit(name or path, out_off, out_size, safe)
            n += 1
        return n

    # ---- RKFW: loader with embedded RKAF -----------------------------
    def unpack_rkfw(self) -> int:
        print("Format: RKFW (loader + embedded update.img)")
        ver = f"{self.d[9]}.{self.d[8]}.{(self.d[7] << 8) + self.d[6]}"
        print(f"  version: {ver}")
        family = {
            0x50: "rk29xx", 0x60: "rk30xx", 0x70: "rk31xx",
            0x80: "rk32xx", 0x41: "rk3368", 0x36: "rv1106", 0x38: "rk35xx",
        }.get(self.d[0x15], f"unknown (0x{self.d[0x15]:02X})")
        print(f"  chip family: {family}")

        ioff = u32(self.d, 0x19)
        isize = u32(self.d, 0x1D)
        if self.d[ioff:ioff + 4] == b"BOOT":
            first = "boot.bin"
        elif self.d[ioff:ioff + 4] == b"LDR ":
            first = "MiniLoaderAll.bin"
        else:
            print("  error: cannot find BOOT/LDR signature")
            return 0
        self.emit(first, ioff, isize)

        eoff = u32(self.d, 0x21)
        esize = u32(self.d, 0x25)
        if self.d[eoff:eoff + 4] != b"RKAF":
            print(f"  error: embedded RKAF not found at 0x{eoff:X}")
            return 1
        print(f"  embedded RKAF @ 0x{eoff:X} ({esize} bytes)")
        sub = Unpacker(self.d[eoff:eoff + esize], self.src)
        sub.outdir = self.outdir
        sub.unpack_rkaf()
        self.table += sub.table
        return 1

    # ---- RKFP: GPT-like sector table ---------------------------------
    def unpack_rkfp(self) -> int:
        print("Format: RKFP (GPT-like partition table)")
        pss = u32(self.d, 0x10)
        peo = u32(self.d, 0x14)
        pbeo = u32(self.d, 0x18)
        pes = u32(self.d, 0x1C)
        pec = u32(self.d, 0x20)
        print(f"  sector size: {pss}")
        print(f"  entry offset: {peo} sectors, backup: {pbeo} sectors, "
              f"entry size: {pes}, count: {pec}")
        n = 0
        for i in range(pec):
            p = pss * peo + i * pes
            if p + 44 > len(self.d):
                break
            name = cstr(self.d, p, 36)
            off = u32(self.d, p + 36) * pss
            size = u32(self.d, p + 40) * pss
            fsize = u32(self.d, p + 44)
            self.emit(name, off, fsize)
            n += 1
        return n

    # ---- RKnanoD player container (FiiO stock/*.IMG) ------------------
    def unpack_rknano(self) -> int:
        print("Format: RKnanoD player container (FiiO stock .IMG)")
        ver = self.d[:8].hex(" ")
        print(f"  version code: {ver}")
        print(f"  vendor: {cstr(self.d, 0x10, 16)!r} {cstr(self.d, 0x28, 16)!r}")

        # Locate RKnanoFW section headers (magic = "RKnanoFW").
        sections = []
        pos = 0
        while True:
            i = self.d.find(b"RKnanoFW", pos)
            if i < 0:
                break
            sp = u32(self.d, i + 8)
            cnt = u32(self.d, i + 12)
            sections.append((i, sp, cnt))
            pos = i + 1

        # Locate resource blob.
        res_off = self.d.find(b"ROCK26IMAGERES")
        trailer_off = len(self.d) - 4

        if not sections:
            print("  error: no RKnanoFW sections found")
            return 0

        # The last RKnanoFW hit is an end marker (SP==0). Drop it.
        if sections and sections[-1][1] == 0:
            marker = sections.pop()
            print(f"  end marker @ 0x{marker[0]:08X}")

        # Role assignment: first = fw1/AP (has a memory-map table, count>0),
        # second = sec2 bootloader, third = section_3/BB.
        roles = ["fw1_AP", "sec2_bootloader", "section3_BB"]

        boundaries = [s[0] for s in sections]
        if res_off > 0:
            boundaries.append(res_off)
        boundaries.append(trailer_off)
        boundaries = sorted(set(boundaries))

        # Map each section header to the nearest following boundary.
        for idx, (soff, sp, cnt) in enumerate(sections):
            end = next((b for b in boundaries if b > soff), trailer_off)
            # section_3 (last code section) ends at resources, not the marker
            name = roles[idx] if idx < len(roles) else f"section_{idx}"
            self.emit(name, soff, end - soff)

        # Resource blob (ROCK26IMAGERES ... trailer)
        if res_off > 0:
            self.emit("resources_ROCK26IMAGERES", res_off, trailer_off - res_off)

        # Trailer (last 4 bytes)
        trailer = u32(self.d, trailer_off)
        self.table.append(("trailer", trailer_off, 4, trailer_off // SECTOR))
        print(f"  {trailer_off:08X}  {4:>10}  sector {trailer_off // SECTOR:>8}  "
              f"trailer (0x{trailer:08X})")

        # Also dump fw1's memory-map table (count entries after header+8).
        if sections:
            fw1_off, fw1_sp, fw1_cnt = sections[0]
            if 1 <= fw1_cnt <= 0x400:
                tbl = fw1_off + 16
                mmap_out = self.outdir / "fw1_memory_map.bin"
                mmap_out.write_bytes(self.d[tbl:tbl + fw1_cnt * 16])
                print(f"  fw1 memory-map table: {fw1_cnt} entries -> "
                      f"fw1_memory_map.bin")
        return len(sections) + (1 if res_off > 0 else 0)

    def run(self, outdir: Path):
        self.outdir = outdir
        outdir.mkdir(parents=True, exist_ok=True)
        print(f"Input:  {self.src.name}  ({len(self.d):,} bytes)")
        print(f"Output: {outdir}")
        print()

        if self.d[:4] == b"RKAF":
            self.unpack_rkaf()
        elif self.d[:4] == b"RKFW":
            self.unpack_rkfw()
        elif self.d[:4] == b"RKFP":
            self.unpack_rkfp()
        elif b"RKnanoFW" in self.d[:0x200000] or b"RKnano SDK" in self.d[:0x200]:
            self.unpack_rknano()
        else:
            print("error: unrecognized firmware signature "
                  f"({self.d[:4].hex(' ')!r})")
            return 1

        print()
        print("Partition / section table (offsets in bytes; sector = 512 B):")
        print(f"  {'offset':>10}  {'size':>10}  {'sector':>8}  name")
        for name, off, size, sector in self.table:
            print(f"  0x{off:08X}  {size:>10}  {sector:>8}  {name}")
        return 0


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Unpack Rockchip update.img / RKAF / RKFW / RKFP / "
                    "RKnanoD firmware containers (pure Python, Windows-safe).")
    ap.add_argument("image", type=Path, help="input firmware image (.img)")
    ap.add_argument("-o", "--output", type=Path, default=None,
                    help="output directory (default: <image stem>_unpacked)")
    args = ap.parse_args()

    if not args.image.is_file():
        print(f"error: not a file: {args.image}", file=sys.stderr)
        return 1

    data = args.image.read_bytes()
    outdir = args.output or Path(args.image.stem + "_unpacked")

    u = Unpacker(data, args.image)
    return u.run(outdir)


if __name__ == "__main__":
    raise SystemExit(main())
