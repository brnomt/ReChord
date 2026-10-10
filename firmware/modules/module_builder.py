#!/usr/bin/env python3
"""module_builder.py — offline RMF1 module packer (ReChord Module Format v1).

Packs an ELF or a raw code blob plus parameters into an RMF1 module file, and
inspects existing module files. Pure python3 stdlib, relative paths only.

The byte layout and the checksum must match firmware/modules/module_format.h
exactly:

  header  = <IHHIIIIII  magic, version, id, flags, code_len, bss_start,
                        bss_len, entry_offset, crc32>   (32 bytes, LE)
  payload = code_len bytes of code (initialized image: Thumb-2 code plus any
            const/initialized data the linker placed in the same load span)
  crc32   = non-reflected MSB-first CRC-32, poly 0x04C10DB7, init 0, xorout 0
            over the payload (the device's documented checksum primitive)

Usage:
  module_builder.py pack --code code.bin --id 7 --entry 0 --bss-len 20220 \
      -o m07.rmf1
  module_builder.py pack --elf module.elf --id 44 -o m44.rmf1
  module_builder.py inspect m07.rmf1

Exit codes: 0 success, 1 usage/validation error, 2 file/parse error.
"""

import argparse
import struct
import sys

MAGIC = 0x31464D52  # 'R' 'M' 'F' '1' little-endian
VERSION = 1
HEADER_SIZE = 32
HEADER_STRUCT = struct.Struct("<IHHIIIIII")

POLY = 0x04C10DB7  # note: NOT the common 0x04C11DB7 (bit 0x1000 missing)

# ELF32 little-endian constants (just enough to extract a module image).
ELF_MAGIC = b"\x7fELF"
ELFCLASS32 = 1
ELFDATA2LSB = 1
SHT_NOBITS = 8
SHF_ALLOC = 0x2
SHF_EXECINSTR = 0x4


def rmf_crc32(data, init=0):
    """Non-reflected, MSB-first CRC-32 (poly 0x04C10DB7, init 0, xorout 0)."""
    crc = init
    for byte in data:
        crc ^= byte << 24
        for _ in range(8):
            if crc & 0x80000000:
                crc = ((crc << 1) ^ POLY) & 0xFFFFFFFF
            else:
                crc = (crc << 1) & 0xFFFFFFFF
    return crc


def align4(n):
    return (n + 3) & ~3


class BuilderError(Exception):
    pass


def parse_elf(blob):
    """Extract (code, entry_offset, bss_start, bss_len) from an ELF32 LE file.

    code = zero-filled image span covering every SHF_ALLOC initialized
    section, relative to the lowest allocated address. bss comes from the
    SHT_NOBITS allocated sections (must sit after the initialized span).
    """
    if len(blob) < 52 or blob[:4] != ELF_MAGIC:
        raise BuilderError("not an ELF file")
    if blob[4] != ELFCLASS32 or blob[5] != ELFDATA2LSB:
        raise BuilderError("only ELF32 little-endian is supported")

    (_e_type, _e_machine, _e_version, e_entry, _e_phoff, e_shoff, _e_flags,
     _e_ehsize, _e_phentsize, _e_phnum, e_shentsize, e_shnum,
     _e_shstrndx) = struct.unpack_from("<HHIIIIIHHHHHH", blob, 16)

    if e_shoff == 0 or e_shnum == 0:
        raise BuilderError("ELF has no section headers")
    if e_shentsize < 40 or e_shoff + e_shentsize * e_shnum > len(blob):
        raise BuilderError("corrupt section header table")

    sections = []
    for i in range(e_shnum):
        off = e_shoff + i * e_shentsize
        (_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size) = \
            struct.unpack_from("<IIIIII", blob, off)
        sections.append((sh_type, sh_flags, sh_addr, sh_offset, sh_size))

    alloc = [s for s in sections if s[1] & SHF_ALLOC and s[4] > 0]
    if not alloc:
        raise BuilderError("ELF has no allocated sections")

    base = min(s[2] for s in alloc)
    init_secs = [s for s in alloc if s[0] != SHT_NOBITS]
    nobits = [s for s in alloc if s[0] == SHT_NOBITS]
    if not init_secs:
        raise BuilderError("ELF has no initialized sections to load")

    init_end = max(s[2] + s[4] for s in init_secs) - base
    code = bytearray(init_end)  # zero-filled gaps between initialized secs
    for (_t, _f, addr, off, size) in init_secs:
        rel = addr - base
        if off + size > len(blob):
            raise BuilderError("section data past end of file")
        code[rel:rel + size] = blob[off:off + size]
    code = bytes(code)

    if nobits:
        bss_start = min(s[2] for s in nobits) - base
        bss_end = max(s[2] + s[4] for s in nobits) - base
        bss_len = bss_end - bss_start
        if bss_start < init_end:
            raise BuilderError("bss overlaps initialized image in ELF layout")
        if bss_start & 3:
            raise BuilderError("bss is not 4-byte aligned in ELF layout")
    else:
        bss_start = align4(init_end)
        bss_len = 0

    entry_raw = e_entry - base
    if entry_raw < 0 or entry_raw >= init_end:
        raise BuilderError("ELF entry point is outside the loaded image")
    # ARM ELF e_entry carries the Thumb bit (addr|1); RMF1 stores a clean
    # even offset and the loader re-applies the Thumb bit at deploy time.
    entry_offset = entry_raw & ~1
    return code, entry_offset, bss_start, bss_len


def build_header(code, module_id, flags, entry_offset, bss_start, bss_len):
    """Validate parameters and serialize the 32-byte RMF1 header."""
    if not 0 <= module_id <= 0xFFFF:
        raise BuilderError("id must fit in u16 (0..65535)")
    if not 0 <= flags <= 0xFFFFFFFF:
        raise BuilderError("flags must fit in u32")
    code_len = len(code)
    if code_len == 0:
        raise BuilderError("code is empty")
    if not 0 <= entry_offset < code_len or entry_offset & 1:
        raise BuilderError("entry offset must be even and inside the code")
    if bss_start < code_len or bss_start & 3:
        raise BuilderError("bss_start must be >= code_len and 4-aligned")
    if bss_len < 0 or bss_start + bss_len > 0xFFFFFFFF:
        raise BuilderError("bss length out of range")

    crc = rmf_crc32(code)
    header = HEADER_STRUCT.pack(MAGIC, VERSION, module_id & 0xFFFF, flags,
                                code_len, bss_start, bss_len, entry_offset,
                                crc)
    return header, crc


def cmd_pack(args):
    if (args.code is None) == (args.elf is None):
        raise BuilderError("give exactly one of --code or --elf")

    if args.elf is not None:
        with open(args.elf, "rb") as f:
            code, elf_entry, elf_bss_start, elf_bss_len = parse_elf(f.read())
        entry = args.entry if args.entry is not None else elf_entry
        if args.bss_len is not None:
            bss_len = args.bss_len
            bss_start = align4(len(code))
        else:
            bss_len = elf_bss_len
            bss_start = elf_bss_start
    else:
        with open(args.code, "rb") as f:
            code = f.read()
        entry = args.entry if args.entry is not None else 0
        bss_len = args.bss_len if args.bss_len is not None else 0
        bss_start = align4(len(code))

    header, crc = build_header(code, args.id, args.flags, entry,
                               bss_start, bss_len)
    with open(args.out, "wb") as f:
        f.write(header)
        f.write(code)

    print("RMF1 packed: id=%u code=%u bss=%u..%u entry=%u crc=%08x -> %s"
          % (args.id, len(code), bss_start, bss_start + bss_len, entry, crc,
             args.out))
    return 0


def cmd_inspect(args):
    with open(args.path, "rb") as f:
        blob = f.read()
    if len(blob) < HEADER_SIZE:
        print("error: file shorter than a header (%u bytes)" % len(blob),
              file=sys.stderr)
        return 2
    (magic, version, module_id, flags, code_len, bss_start, bss_len,
     entry_offset, crc) = HEADER_STRUCT.unpack_from(blob, 0)

    print("RMF1 inspect: %s" % args.path)
    print("  magic        %08x %s" % (magic,
          "ok" if magic == MAGIC else "BAD (want %08x)" % MAGIC))
    print("  version      %u %s" % (version,
          "ok" if version == VERSION else "BAD (want %u)" % VERSION))
    print("  id           %u" % module_id)
    print("  flags        %08x" % flags)
    print("  code_len     %u" % code_len)
    print("  bss          %08x..%08x (%u bytes)"
          % (bss_start, bss_start + bss_len, bss_len))
    print("  entry_offset %u" % entry_offset)
    print("  crc32        %08x" % crc)

    if len(blob) < HEADER_SIZE + code_len:
        print("  payload      TRUNCATED (file %u bytes, want %u)"
              % (len(blob), HEADER_SIZE + code_len))
        return 2
    want = rmf_crc32(blob[HEADER_SIZE:HEADER_SIZE + code_len])
    ok = want == crc
    print("  payload crc  %08x %s" % (want, "ok" if ok else "BAD"))
    return 0 if ok and magic == MAGIC and version == VERSION else 2


def main(argv):
    ap = argparse.ArgumentParser(
        description="Pack or inspect ReChord RMF1 overlay modules.")
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("pack", help="pack a code blob or ELF into RMF1")
    p.add_argument("--code", help="raw code blob (binary)")
    p.add_argument("--elf", help="ELF32 little-endian ARM object/image")
    p.add_argument("--id", type=int, required=True, help="module id (u16)")
    p.add_argument("--entry", type=int, default=None,
                   help="entry byte offset from code base, even "
                        "(default: 0 for --code, ELF e_entry for --elf)")
    p.add_argument("--bss-len", type=int, default=None,
                   help="bss bytes after the code (default: 0 for --code, "
                        "ELF NOBITS span for --elf)")
    p.add_argument("--flags", type=int, default=0,
                   help="RMF1 flags word (default 0; v1 defines no bits)")
    p.add_argument("-o", "--out", required=True, help="output module file")
    p.set_defaults(func=cmd_pack)

    q = sub.add_parser("inspect", help="dump and verify an RMF1 file")
    q.add_argument("path", help="module file to inspect")
    q.set_defaults(func=cmd_inspect)

    args = ap.parse_args(argv)
    try:
        return args.func(args)
    except BuilderError as e:
        print("error: %s" % e, file=sys.stderr)
        return 1
    except OSError as e:
        print("error: %s" % e, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
