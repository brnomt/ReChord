#!/usr/bin/env python3
"""RKnano firmware trailer CRC.

The HIFIEC*.IMG container stores a 4-byte little-endian trailer at EOF. The
device computes CRC-32 with polynomial 0x04C10DB7 (NOT the standard
0x04C11DB7; bit 0x1000 is missing), non-reflected, init 0, xorout 0 over
IMG[:-4].  Verified against HIFIEC37.IMG (trailer 0x1EA1C309) and further
stock images; see docs/re/route-b-minimum.md section 4b.

This module is shared by tools/make_fw1.py and tools/fw1_5m.py so the
standalone path uses exactly the same primitive as the legacy packer.
"""

from __future__ import annotations

CRC_POLY = 0x04C10DB7


def _crc_table() -> list[int]:
    table = []
    for i in range(256):
        c = i << 24
        for _ in range(8):
            if c & 0x80000000:
                c = ((c << 1) ^ CRC_POLY) & 0xFFFFFFFF
            else:
                c = (c << 1) & 0xFFFFFFFF
        table.append(c)
    return table


_CRC_TABLE = _crc_table()


def rknano_crc32(data: bytes) -> int:
    """CRC-32(poly 0x04C10DB7, non-reflected, init 0, xorout 0)."""
    crc = 0
    table = _CRC_TABLE
    for byte in data:
        crc = (table[((crc >> 24) ^ byte) & 0xFF] ^ (crc << 8)) & 0xFFFFFFFF
    return crc
