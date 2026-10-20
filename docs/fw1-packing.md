# fw1 (AP) packing — RKnanoFW container format & status

> Consolidated reverse-engineering of the **fw1/AP** image container so the
> "pack both halves" step can be completed correctly. The BB (section_3)
> container is a *flat* 16-byte header + code (already handled by
> tools/pack_img.py); **fw1 is different** — it is scatter-loaded by the
> Mask ROM through a memory-map table.

## 1. IMG layout (stock HIFIEC37.IMG, v3.7.0)

| IMG offset | Content |
|---|---|
| 0x000000 | outer header / part table |
| 0x0001F8 | **RKnanoFW header #1 (fw1/AP)** — magic + 0x03050000 + count 0x5B |
| 0x000208 | **memory-map table** — 91 entries x 16 B (0x208..0x7B8) |
| 0x0007B8 | fw1/AP payload: starts with a pointer table, then code/data |
| 0x057820 | **RKnanoFW header #2 (sec2 bootloader)** + strings (fw1 Sign error!) |
| 0x081A14 | **RKnanoFW header #3 (section_3/BB)** — magic + 0x0301E794 + count 0x52 |
| 0x081A24 | section_3 code (flat, firmware_entry @ 0x03000010) |
| 0x09BAA0E | ROCK resources |
| 0x02000000 | 4-byte trailer 0x1EA1C309 |

The three RKnanoFW sections are independent stages: the **Mask ROM loads fw1
via the memory-map table**, fw1/second-stage validates and then loads
**section_3 flat to 0x03000000** and jumps to firmware_entry @ 0x03000010.
That is why pack_img.py (flat section_3 splice) works, while fw1 needs the
scatter table.

### 1.1 Hard limit: LOADER_CODE_BASE 0x03088000

The SDK scatter map (`docs/community.md` §4.2) pins `LOADER_CODE_BASE` at
`0x03088000`. That loader code is part of the boot chain: an AP image whose
loaded code crosses it overwrites the loader and **bricks the boot**, even if
every other size check passes. On 2026-10-20 AP5M `0xEF0C1BFC` did exactly
that: its fw1 row 0 ended at `0x03088EB0`, while the previous working image
ended at `0x03087F8C` (116 B below the loader base).

- `firmware/firmware_ap.ld` caps `FW_CODE` at `0x03050000 + 0x38000 =
  0x03088000`.
- `tools/make_fw1.py` prints `loader code base ... [OK]` and fails the pack
  when any scatter row load end crosses `0x03088000`.
- The fixed image ends at `0x03087900` (1,792 B of margin), `_bss_end
  0x03049BE0`.

Do not raise the `FW_CODE` length back to `0x3C000`. A pack that does not
show `loader code base ... [OK]` must not be flashed.

> **Phase 0 correction (BRNOMT-82, 2026-10-20):** the fw1 row-0 source that
> maps to the `0x03088000` destination is
> `0x03050000 + (0x03088000 - 0x030795CC) = 0x0305EA34`. Ghidra finds no
> procedure there and the bytes are a data table, not code. Do not assume
> the loader is a copy of fw1 AP code or that a fw1 row can replace it;
> treat `0x03088000` as ROM/second-stage-owned until proven. See
> `docs/re/boot-chain.md` §4.1.

## 2. RKnanoFW container format (confirmed against SDK)

Byte-exact, seen in both the Echo Mini IMG and the prebuilt
rk3399-table-RKNanoC SDK Firmware.img (offset 0xC2D7 there):

    [0:8]   "RKnanoFW"          magic (ASCII)
    [8:12]  uint32 LE           load base / initial SP
    [12:16] uint32 LE           entry count N
    [16..]  N x 16-byte entries (memory-map table)

- Echo Mini header #1: base **0x03050000**, count **0x5B** (91).
- Echo Mini header #3: base **0x0301E794**, count **0x52** (82, but NO table —
  section_3 is flat; the ROM treats count as flags there).
- SDK RKnanoC header: base **0x01000000** (its IRAM), count **0x3D** (61).

## 3. Memory-map entry format (two interleaved types)

Each entry is 4 x uint32 LE. The table alternates two shapes:

**Type B — load/copy** (even indices mostly), {dest, src, size, dest_end}:

| field | meaning | example (entry 2) |
|---|---|---|
| w0 | RAM destination | 0x030A61F4 |
| w1 | XIP source (flash) | 0x03079CC8 |
| w2 | byte size | 0x1604 |
| w3 | RAM end (== w0 + w2) | 0x030A77F8 |

**Type A — data/BSS region** (odd indices), {ram_start, size, ram_end, flash_size}:

| field | meaning | example (entry 1) |
|---|---|---|
| w0 | RAM start | 0x03004000 |
| w1 | region size | 0x1AFC |
| w2 | RAM end (== w0 + w1) | 0x03005AFC |
| w3 | total object size (may exceed the span) | 0x18C50 |

XIP mapping: **flash is XIP-mapped with IMG offset N <-> address 0x03000000+N**
(verified: entry 88's src 0x0308A5A4 -> IMG 0x8A5A4, which is section_3 code;
several src values land on erased/zero flash used to zero-fill data). The
scatter destinations (0x030A61F4 .. 0x0311EE04) are the *copy-to-RAM* window,
above the XIP image.

## 4. Why fw1 cannot simply be spliced like section_3

1. **Scatter table.** The Mask ROM loads fw1 through the 91-entry table; a
   flat binary has no way to express it. The table must be regenerated from
   the build's real section layout (what Rockchip's RKnanoTool.exe /
   My_tool.exe / RKImageMaker.exe do from the Keil AXF).
2. **Module overlay.** Stock fw1 is the RKnanoD *A_CORE* build
   (Scatter/BuildAll.sct): resident SYS_CODE @ 0x03060000 (256 KB) +
   SYS_DATA @ 0x03000000 (384 KB), with USB/Video/UI-windows/audio/BT/FM/
   record/codecs as **overlay modules** swapped into the same RAM on demand.
   Our flat AP build does not reproduce this; it links everything at once.
3. **Size mismatch.** Current AP ELF: .text 0x42814 (272 KB) @ 0x03200000,
   .data 0x3329C + .bss 0x93C18 (~814 KB) @ 0x03000000. The code base
   is 0x03200000 (not the stock 0x03060000) precisely because data+bss
   (~814 KB) overflows the stock 384 KB data window.

## 5. Path forward

1. **Resident A_CORE only** — restrict the AP manifest to the resident
   SysCode+SysData object set (kernel + UI core), link it at
   SYS_CODE 0x03060000 / SYS_DATA 0x03000000, and build each overlay
   module separately (mirroring the uvproj's A_CORE + per-codec targets).
2. **Generate the scatter table** from that build's section map (implement in
   tools/pack_fw1.py), or run Rockchip RKnanoTool.exe on the AXF and
   transplant its RKnanoFW section into the FiiO IMG.
3. **Validate on hardware UART** (fw1valid = %d fw2valid = %d) — the only
   authoritative check, since the ROM loader format is not fully documented.

See also docs/dispatch-map.md, docs/bootloader-analysis.md, docs/HARDWARE.md.

## 6. Size analysis (flat AP build, after .bss NOLOAD fix)

`firmware_ap.ld` now marks `.bss (NOLOAD)`; the ARMCC `zero_init` attribute is
ignored by GCC, so every `_ATTR_*_BSS_` section (e.g. `RecordControlBss`,
271 KB) lands in the output `.bss` (NOBITS) and is correctly zero-fill — this
cut the scatter payload from 1.30 MB to **694 KB**.

Remaining payload = RW FileSiz 0x3BA00 (244 KB `.data`+stack) + RX FileSiz
0x6DEE3 (450 KB `.text`+`.rodata`+ARM unwind tables). The fw1 region is
0x57068 (356 KB), so the flat build is still ~338 KB over.

Largest object contributors (`arm-none-eabi-size`, text+data+bss):

| group | total | note |
|---|---:|---|
| audio/RecordControl | 335 KB | 271 KB BSS (now zero-fill) + 57 KB data |
| audio/ID3 | 167 KB | ID3/Unicode tables — all code+rodata |
| audio/AudioControl | 127 KB | mostly `.data` |
| display/LcdInterface | 111 KB | LCD framebuffer/config `.data` |
| filesys/nFAT | 67 KB | FAT structures |
| bt/lwbt + radio/FMControl | 60 KB | BT + FM |

To fit 356 KB the AP must be split into the resident A_CORE (kernel +
drivers + filesystem + LCD) and on-demand overlay modules (ID3, RecordControl,
BT, FM, image codecs, UI windows) — the exact structure the Keil A_CORE +
per-codec targets already model. A flat GCC build of all 187 sources cannot
share their data the way the overlay scatter does.

## 7. Result — AP slims to 273 KB and packs (DONE)

The codec `.lib` (AP_CODEC_LIBS) and 40 overlay sources (audio codec
wrappers, ID3/AsicToUnicode, RecordControl, image, BT, FM, usbcontrol) were
removed from `ap.mk`; 72 weak stubs were added to `firmware/stubs.c` for their
entry points (they belong in the BB / overlay modules, not the resident UI).
The AP now links with 0 undefined symbols and its scatter payload is
**273,084 bytes** (< 356 KB), producing `build/ap/fw1_custom.img`.

`pack_img.py --pack-full` then emits `build/ReChord_APBB.IMG`: fw1 header
(SP 0x03050000, 3 entries) + payload @ 0x7B8, sec2 preserved, section_3
(SP 0x0301E794, count 82) @ 0x81A14, trailer 0x1EA1C309 — 33,554,436 bytes.

Remaining before claiming a *verified* boot: the AP UI now has stubbed
audio/BT/FM/image/record — it is the resident A_CORE, not the full player.
Hardware-UART validation (`fw1valid = %d fw2valid = %d`) is the authoritative
check that the scatter table + payload are accepted by the Mask ROM.

## 8. make_fw1.py - one-command AP packing pipeline (milestone 1c, DONE)

`tools/make_fw1.py` chains the whole AP path in one command:

    ELF (or flat bin)
      -> fw1 RKnanoFW slot image   (tools/pack_fw1.py conventions, imported)
      -> splice into stock HIFIEC*.IMG (tools/pack_img.py pack_full, imported)
      -> RECOMPUTE the 4-byte trailer CRC
      -> verification report

Neither `pack_fw1.py` nor `pack_img.py` was modified; `make_fw1.py` imports
their readers/tables/constants and drives them.

### 8.1 Exact invocations (run from the repo root)

    # real AP ELF -> build/ReChord_AP.IMG  (the deliverable image)
    python3 tools/make_fw1.py build/ap/rechord_ap.elf

    # dry-run proof: tiny placeholder ELF (vector table + reset loop at the
    # documented base 0x03050000), built with arm-none-eabi-gcc inline
    python3 tools/make_fw1.py --placeholder -o build/ReChord_AP_dryrun.IMG

    # flat binary input (placed at --load-addr, entry defaults to load-addr)
    python3 tools/make_fw1.py code.bin --load-addr 0x03050000

    # optional: also splice a custom section_3 (BB) instead of keeping stock
    python3 tools/make_fw1.py build/ap/rechord_ap.elf --bb build/section3_custom.bin

Outputs: `<output>` (default `build/ReChord_AP.IMG`) and `<output>.fw1` (the
intermediate fw1 slot image, kept for inspection). The tool refuses to write
anything named `ReChord_BB.IMG` (safe fallback) or over the stock IMG, and it
never touches the mounted device.

### 8.2 Verification steps (the report covers all of these)

1. Slot limits: scatter rows <= 91, payload <= 356,456 B (0x57068).
2. Scatter src XIP window: every load row's src must lie in
   0x030007B8..0x03057820 (IMG offset = addr - 0x03000000).
3. Entry contract: ELF e_entry must fall inside a loaded segment. NOTE the
   RKnanoFW header has NO entry field: field 0 is the load base / initial SP
   (stock 0x03050000 = vector-table base, word0 = SP, word1 = reset, per
   docs/re/route-b-minimum.md section 2). The ELF entry is informational.
4. IMG wrap round-trip: re-read the output and check the fw1 header @ 0x1F8,
   the scatter rows @ 0x208 (zero-padded to 0x7B8), the payload @ 0x7B8
   (zero-padded to 0x57820), the section_3 region @ 0x81A14, byte-for-byte.
5. Trailer CRC: self-check the primitive against the stock image body (must
   reproduce the stock trailer 0x1EA1C309), then report CRC before (carried
   from stock) / CRC after (recomputed over IMG[:-4], stored LE at 0x2000000).
   The device rejects wrong-trailer images pre-write with "bad CRC Image".

Independent re-check of the trailer (bit-loop form of the same primitive):

    python3 - <<'EOF'
    import struct
    data = open("build/ReChord_AP.IMG", "rb").read()
    crc = 0
    for b in data[:-4]:
        crc ^= b << 24
        for _ in range(8):
            crc = ((crc << 1) ^ 0x04C10DB7) & 0xFFFFFFFF \
                if crc & 0x80000000 else (crc << 1) & 0xFFFFFFFF
    print(hex(crc), hex(struct.unpack_from("<I", data, len(data) - 4)[0]))
    EOF

### 8.3 Format limits and contracts (confirmed while building the pipeline)

| item | limit / contract |
|---|---|
| scatter rows | max **91** (table region 0x208..0x7B8 = 91 x 16 B) |
| fw1 payload | max **356,456 B** (0x57068: payload window 0x7B8..0x57820) |
| fw1 slot total | 16 (header) + 16*rows + payload |
| XIP mapping | flash addr = 0x03000000 + IMG offset; load-row src must be inside the fw1 payload window 0x030007B8..0x03057820 |
| row types | Type B load `{dest, src, size, dest_end}` (w3 = w0 + w2); Type A zero-fill `{start, len, start+len, 0}` (w3 = 0) |
| entry | no entry field in the container; load base 0x03050000 doubles as the vector-table address (SP at word 0, reset at word 1) |
| IMG size | fixed 33,554,436 B (0x2000004); trailer CRC at 0x2000000, LE |
| trailer CRC | CRC32 poly **0x04C10DB7** (NOT 0x04C11DB7), non-reflected, init 0, xorout 0, over IMG[:-4] |

The packer concatenates PT_LOAD file images with NO padding, so segment
alignment inside the payload is only what the ELF file offsets give; link
segments 4-aligned (the scatter copy is word-oriented).

### 8.4 Dry-run proof (2026-10-09, both chains PASS)

Placeholder run (`--placeholder`): 1 scatter row, 20 B payload, entry
0x03050011 (Thumb, reset at 0x03050010), fw1 payload landed at IMG 0x7B8,
output 33,554,436 B, trailer recomputed 0x1EA1C309 -> 0xD58A726A.

Real AP run (`build/ap/rechord_ap.elf`): entry 0x03060045 (Thumb); segments
0x03000000 (filesz 0x194E4, memsz 0x60000) + 0x03060000 (0x295FC) - note the
AP now links at stock SYS_DATA/SYS_CODE (section 4's 0x03200000 note is
obsolete); 3 scatter rows (2 load + 1 zero-fill), payload 273,120 B
(76.6% of 356,456), fw1 slot 273,184 B, payload landed at IMG
0x7B8..0x43298, trailer recomputed 0x1EA1C309 -> 0x13379CAA. The emitted
fw1 slot is byte-identical to `build/ap/fw1_custom.img`.

### 8.5 Deviations from this doc's earlier plan

1. **Output naming:** the pipeline emits `build/ReChord_AP.IMG` (custom fw1 +
   STOCK section_3 preserved verbatim), not the "pack both halves"
   `build/ReChord_APBB.IMG`. A custom BB is only spliced with explicit
   `--bb`; `build/ReChord_BB.IMG` stays the untouched safe fallback.
2. **pack_full section_3 semantics:** `pack_img.pack_full` zero-pads the
   section_3 region past a shorter custom BB; to keep the stock BB intact
   `make_fw1.py` passes the stock region bytes themselves (an identity
   splice), so no stock tail is lost.
3. **The trailer is RECOMPUTED, not preserved.** `pack_full` preserves the
   stock trailer; `make_fw1.py` then rewrites it with the CRC of the new
   body (required: any byte change invalidates it).
4. Still unverified: the custom scatter table on hardware (2026-08-25 brick;
   see pack_fw1.py header). Offline analysis only until the UART check
   (`fw1valid = %d fw2valid = %d`) passes.


## 9. CORRECTION (2026-10-11): the load model is CONTIGUOUS, not XIP

Section 3's "XIP mapping (IMG offset N <-> 0x03000000+N)" is REFUTED by
cross-audit against the stock tables and the reference image (incident
5667c9c4, hard brick 2026-10-11):

- Under XIP the entry address 0x03050000 reads IMG 0x50000 = all zeros (no
  vector table anywhere); under contiguous (RAM = 0x0304F298 + IMG offset)
  IMG 0xD68 holds a real vector table (SP 0x03004000, Reset 0x0306296F).
- Under XIP 0/46 of the stock v2.5.0 Type-B rows source from fw1's own
  payload window (0x7B8..0x57820); under contiguous 46/46 do.

Correct model:
1. The loader splices the fw1 payload at IMG 0x7B8 and loads it contiguously
   into RAM at RAM = 0x0304F298 + IMG offset (payload[0] -> 0x0304FA50).
2. Scatter rows copy from addresses IN THAT LOADED IMAGE (Type B w1 = src =
   0x0304F298 + IMG offset of the source bytes) and zero/declare regions
   (Type A).
3. Entry = the vector table at RAM 0x03050000 = IMG 0xD68 (0x5B0 bytes into
   the payload; the reference prefixes its payload with a 0x5B0-byte pointer
   table, we pad the same computed span). word0 = initial SP (0x03004000),
   word1 = reset.
4. RAM 0x03000000..0x03004000 = the AP MAIN STACK (SP 0x03004000, grows
   down). NEVER a row target and never data (the 2026-08-25 brick put .data
   there).

Our build (tools/pack_fw1.py) now emits: payload = [computed prefix][chunks
placed at their LMA offsets (in-place load)], rows = [B identity for the code
chunk][A for the RW region], and tools/make_fw1.py byte-checks the vector
table at IMG 0xD68 on every pack. Residual uncertainties (hardware-unproven):
the count field (we emit the real row count, stock/reference emit 91) and the
reference's extra third row (purpose unknown).

## 10. Outer header / IDB part-table map (2026-10-11)

The first 0x1F8 bytes of the IMG are the RKnano firmware descriptor (family of
the eMMC "SAVE"/IDB record the loader reads at boot - see the recovered boot
log in docs/HANDOVER.md). Field 0x00 = build date (MM DD 20 YY), 0x10 =
brand name, and a part table follows. Two images give simultaneous equations:

| off | STOCK | REFERENCE | meaning |
|-----|-------|-----------|---------|
| 0x000 | 0x06222026 (Jun 22 2026) | 0x10072026 (Oct 7 2026) | build date |
| 0x010 | "Rockchip" | (brand) | brand name |
| 0x030 | "RKnano SDK 1.0" | "7e287ea" | build hash/string |
| 0x1F4 | 0x01FC4000 | 0x0044C000 | trailing-marker container base (header at +0x1F8) |
| 0x148-0x150 | (1, 0x009BAA0E, 0x016069E8) | 0,0,0 | optional part: sec3 end, resources size |
| 0xC8-0xD0 | (1, 0x0059931C, 0x001CA34A) | 0,0,0 | optional part group 2 |
| 0xF4-0xFC | (1, 0x00763666, 0x002573A8) | 0,0,0 | optional part group 3 |
| 0x080 | 0x00387716 | 0x00239476 (= its fw1 end) | unidentified (bounds?) |

Status: map INCOMPLETE - the fw1 extent fields are not yet pinned (0x80 is the
leading candidate). The reference zeroed every optional group; its image is
fw1 + marker only. Remaining decode targets: fields 0x40-0x6C and 0x70-0x84
(tag values 0x00201C06 / 0x00211C06).

Full non-zero field dump (stock vs reference):
| off | STOCK | REFERENCE | meaning |
|-----|-------|-----------|---------|
| 0x000 | 0x06222026 | 0x10072026 |  |
| 0x004 | 0x00000000 | 0x00090000 |  |
| 0x008 | 0x00000000 | 0x00000002 |  |
| 0x00C | 0x76543210 | 0x76543210 |  |
| 0x010 | 0x6B636F52 | 0x48434552 |  |
| 0x014 | 0x70696863 | 0x0000004F |  |
| 0x030 | 0x616E4B52 | 0x38326537 |  |
| 0x034 | 0x53206F6E | 0x00616537 |  |
| 0x038 | 0x31204B44 | 0x00000000 |  |
| 0x03C | 0x0000302E | 0x00000000 |  |
| 0x050 | 0x00000004 | 0x00000004 |  |
| 0x054 | 0x00000200 | 0x00000200 |  |
| 0x05C | 0x00000200 | 0x00000200 |  |
| 0x064 | 0x00000200 | 0x00000200 |  |
| 0x06C | 0x00000200 | 0x00000200 |  |
| 0x070 | 0x00185910 | 0x00037670 |  |
| 0x074 | 0x00000002 | 0x00000003 |  |
| 0x078 | 0x00185B10 | 0x00037870 |  |
| 0x07C | 0x00201C06 | 0x00201C06 |  |
| 0x080 | 0x00387716 | 0x00239476 |  |
| 0x084 | 0x00211C06 | 0x00211C06 |  |
| 0x088 | 0x00000000 | 0x0044B07C |  |
| 0x08C | 0x00000000 | 0x0000039A |  |
| 0x0C8 | 0x00000001 | 0x00000000 |  |
| 0x0CC | 0x0059931C | 0x00000000 |  |
| 0x0D0 | 0x001CA34A | 0x00000000 |  |
| 0x0F4 | 0x00000001 | 0x00000000 |  |
| 0x0F8 | 0x00763666 | 0x00000000 |  |
| 0x0FC | 0x002573A8 | 0x00000000 |  |
| 0x148 | 0x00000001 | 0x00000000 |  |
| 0x14C | 0x009BAA0E | 0x00000000 |  |
| 0x150 | 0x016069E8 | 0x00000000 |  |
| 0x19C | 0x00000001 | 0x00000001 |  |
| 0x1F4 | 0x01FC4000 | 0x0044C000 |  |

## 11. Standalone 5 MB path (Phase 1a, BRNOMT-82)

The legacy `make_fw1.py` path still wraps the fw1 into the stock 33 MB image
and clones the 5 MB outer descriptor from
`community/refcfw/HIFIEC90.IMG`. Both inputs are gitignored, so that path is
not available in a clean clone.

`tools/fw1_5m.py` builds the fw1-only 5 MB image directly:

```sh
make standalone-ap5m
# or
python3 tools/make_fw1.py build/cfw/rechord_ap.elf --standalone-5m \
    -o build/ReChord_AP.IMG
```

It writes only the fw1 container and `ReChord_AP5M.IMG`; it does not create
the 33 MB image and does not read `stock/` or `community/`. The 0x1F8
descriptor is generated from the field table in section 10, with our build
date/brand/tag and fw1 end patched in. The trailing marker container is
`RKnanoFW` plus eight zero bytes at `0x44C1F8`; the self-consistency copy is
at `0x44C000`; the trailer is the CRC from `tools/rknano_crc.py`.

The output is byte-identical to the boot-confirmed AP5M image:

| field | value |
|---|---|
| size | `0x500004` |
| trailer CRC | `0x2264CCBA` |
| SHA-256 | `c2238cb388b14d6bfd984da9328ca0005831b90d42cf4b5be77617b48babb93d` |
| row-0 end | `0x03087900` |
| `_bss_end` | `0x03049BE0` |

`tools/verify_fw1_5m.py` rebuilds from the current `.fw1` and compares
against a saved known-good image. Run it before `make standalone-ap5m` if
the known-good file is the same path (the build target overwrites it), or
pass `GOLDEN=/path/to/saved/ReChord_AP5M.IMG`.

`make clean-clone-check` is the stronger acceptance test: it creates a
detached worktree at `HEAD` with tracked files only (no `stock/`, no
`community/`), runs `make standalone-ap5m` there and compares the image
SHA-256. Verified on `02482c1`: PASS, output
`c2238...babb93d`.

This does **not** complete Issue 82: the stock loader/ROM contract at
`0x03088000`, the legacy 33 MB stock/ref path, the SDK `.lib` codec path and
the update/flash path remain. See `docs/re/boot-chain.md` for the Phase 0
map, the loader windows and the Phase 2 gate.
