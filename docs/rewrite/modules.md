# Modules — ReChord Module Format v1 (RMF1) + loader + registry

> Workstream: overlay module system (`firmware/modules/`). Goal: features
> (player, browser, recorder…) are **loadable units**, not one monolith.
> Clean-room: the format is OURS; only documented FORMAT FACTS from the
> device runtime are treated as requirements (no third-party CFW code).
>
> Facts used (docs/re/route-b-minimum.md §4b/§6c, docs/re/refcfw-analysis.md):
> modules are small units (device log: `mod: m44 loaded, 2980 B + bss 20220 B`,
> `mod: m78 loaded into HIGHRAM0, 41196 B in 8193 us`), loaded on demand into
> named RAM windows (`hiram: 0307a000..0309f000` = HIGHRAM0), verified with
> the device checksum (non-reflected MSB-first CRC-32, poly 0x04C10DB7,
> init 0, xorout 0 — same primitive as the IMG trailer), against a header of
> the shape `{magic, id, key/crc, len, bss_start..bss_end}`. Loader failure
> classes observed: header invalid / verify fail / checksum fail.

## 1. Files

| File | Role |
|---|---|
| `firmware/modules/module_format.h` | RMF1 header struct, magic/version, result codes |
| `firmware/modules/module_loader.h/.c` | `rmf_crc32()`, `rmf_load_module()` |
| `firmware/modules/module_registry.h/.c` | loaded-module table |
| `firmware/modules/module_builder.py` | offline packer (raw blob or ELF → `.rmf1`) |
| `firmware/modules/modules.mk` | build fragment for the root Makefile |
| `firmware/modules/tests/` | host tests + `run_tests.sh` driver |

## 2. RMF1 byte layout

All fields little-endian. Header is exactly 32 bytes, packed, no padding.

```
 0               32                32+code_len
 +----------------+--------------------------+
 | header (32 B)  | code (code_len bytes)    |
 +----------------+--------------------------+
   off size field
     0    4  magic          'R' 'M' 'F' '1'  (u32 = 0x31464D52 LE)
     4    2  version        1
     6    2  id             module id (u16)
     8    4  flags          RMF1_FLAG_* (0 in v1)
    12    4  code_len       bytes of code after the header
    16    4  bss_start      bss offset from the LOAD BASE (window base)
    20    4  bss_len        bss bytes (0 = no bss)
    24    4  entry_offset   entry offset from the load base (even)
    28    4  crc32          rmf_crc32() over the code_len code bytes
```

Runtime window layout produced by the loader (the header is NOT copied):

```
 window base                                          window end
     | code (code_len) | [pad] | bss (bss_len) |  ... unused ... |
     0              code_len  bss_start      bss_start+bss_len
```

### Field rationale

- **magic** — `'RMF1'`; 4 ASCII bytes make files self-identifying in a hex
  dump and are the cheapest reject of untrusted data. Deliberately not any
  observed device magic: interop with the stock loader is NOT a goal.
- **version** — bumped on any layout/semantic change; separate from magic so
  future versions stay "RMF" but old loaders refuse them (RMF_ERR_VERSION)
  instead of misparsing.
- **id** — small integer identity (device logs show m44/m45/m47/m48/m78).
  Registry key. u16 is plenty for the `%02u`-style ids seen on device.
- **flags** — forward-compat bitfield. v1 defines NO bits; the builder
  writes 0 and the loader rejects unknown bits (RMF_ERR_FLAGS), reserving
  room for compression / relocation / signing without old loaders
  mis-executing new modules.
- **code_len** — size of the initialized image after the header: Thumb-2
  code plus const/initialized data in the same load span (the device's
  "2980 B code" figures are the same idea). Bounds the file tail and the
  CRC input, so the three can never disagree.
- **bss_start / bss_len** — the zero-filled range, stored window-relative
  (`bss_start` is an offset from the load base) because our loader is
  window-agnostic; the device log prints absolute addresses (`bss
  %08x..%08x`) since its loader knows the window. Constraints:
  `bss_start >= code_len`, `bss_start` 4-aligned, `bss_len >= 0`.
  Length-form (not end-address) keeps every size field overflow-checkable
  uniformly.
- **entry_offset** — even byte offset of the entry from the load base
  (Thumb code is halfword-aligned). The loader computes the callable
  address as `base + entry_offset | 1` (Thumb state bit), so the file
  stores a clean offset. ARM ELF `e_entry` carries the Thumb bit — the
  builder strips it when packing `--elf`.
- **crc32** — `rmf_crc32()` over exactly the `code_len` code bytes: the
  device's documented checksum primitive (see §3), so one verifier can
  later cover flash images too. Only code is covered; the header is fully
  validated structurally, and a corrupted header field is caught by the
  checks or by the code CRC disagreeing with the payload.

## 3. Checksum: `rmf_crc32()`

CRC-32, **non-reflected, MSB-first**, polynomial **0x04C10DB7** (note: NOT
the common 0x04C11DB7 — bit 0x1000 is missing; byte-verified on device
against 4 stock images), **init 0, xorout 0**. Implemented bitwise from the
definition in `module_loader.c` (no table, no libc).

Known-answer vectors (also asserted in the host tests):

| Input | CRC-32 |
|---|---|
| `"123456789"` | `0x889A9615` |
| empty | `0x00000000` |
| bytes `0x00..0xFF` | `0x141E59FC` |
| 2980-byte pattern `(i*7+3)&0xFF` | `0x33B4999C` |

The C loader and `module_builder.py` implement the same algorithm
independently; the builder→loader round-trip test proves they agree.

## 4. Loader

`rmf_load_module(blob, blob_len, window, window_size, out)` —
address-window agnostic: no fixed RAM address is assumed anywhere, so the
identical object runs on the Cortex-M3 target (window = e.g. HIGHRAM0 at
`0x0307A000`, 151552 B) and in host tests (window = `malloc`).

State machine (any failure returns immediately and leaves the window
untouched — a bad module is never partially deployed):

```
        +------+   arg ok    +-----------+  magic ok   +-----------+
 blob-->| ARG  |----------->|  STRUCT   |------------>| HEADER    |
        +------+             | hdr bytes|             | magic/ver |
          | NULL arg         | present  |             | /flags    |
          v                  +-----------+             +-----------+
       RMF_ERR_ARG              | len < 32                | bad
                                v                         v
                          RMF_ERR_LENGTH          RMF_ERR_MAGIC /
                                                  RMF_ERR_VERSION /
                                                  RMF_ERR_FLAGS
                             (header ok)
                                  |
                                  v
                            +-----------+  lengths ok  +-----------+
                            | LENGTHS   |------------->|  WINDOW   |
                            | code/entry|              | fit check |
                            | /bss sane |              +-----------+
                            +-----------+                   | fits
                                | bad                       v
                                v                     +-----------+
                          RMF_ERR_LENGTH              |  VERIFY   |
                                                      | CRC-32    |
                                                      +-----------+
                                                           | match
                                                           v
                                                     +-----------+
                                                     |  DEPLOY   |
                                                     | copy code |
                                                     | zero bss  |
                                                     | entry |1  |
                                                     +-----------+
                                                           |
                                                           v
                                                        RMF_OK
```

Check order and invariants:

1. **STRUCT/HEADER** — `blob_len >= 32`, then `magic`, `version`, `flags`.
2. **LENGTHS** — `code_len > 0`; `entry_offset < code_len` and even;
   `bss_start >= code_len` and 4-aligned; `blob_len >= 32 + code_len`.
   All sums computed in 64-bit so hostile u32 fields cannot wrap past a
   check on the 32-bit target.
3. **WINDOW** — `code_len <= window_size` and
   `bss_start + bss_len <= window_size`.
4. **VERIFY** — CRC over the code bytes in the blob (before any write).
5. **DEPLOY** — copy code to the window base; zero `[bss_start, +bss_len)`;
   `entry = base + entry_offset | 1`. The gap `[code_len, bss_start)` (0–3
   bytes from the builder's 4-alignment) is left untouched — modules must
   not place live data there.

### Error codes (`rmf_result_t`, in `module_format.h`)

| Code | Meaning |
|---|---|
| `RMF_OK` | loaded |
| `RMF_ERR_ARG` | NULL pointer argument |
| `RMF_ERR_MAGIC` | magic ≠ 'RMF1' |
| `RMF_ERR_VERSION` | version ≠ 1 |
| `RMF_ERR_FLAGS` | unknown flag bits set |
| `RMF_ERR_LENGTH` | truncated file or inconsistent field lengths |
| `RMF_ERR_WINDOW` | module does not fit the target RAM window |
| `RMF_ERR_CRC` | code CRC-32 mismatch |
| `RMF_ERR_DUPLICATE` | registry: id already registered |
| `RMF_ERR_FULL` | registry: no free slot |
| `RMF_ERR_NOT_FOUND` | registry: id not registered |

`rmf_strerror()` maps codes to log strings.

## 5. Registry

`module_registry.h/.c`: fixed static table (16 slots, no allocation) of
`(id, name[16], base, size, state)`. Pure bookkeeping over caller-owned
windows — it never allocates, copies code, or frees windows, and it does not
depend on the loader (wire them at the app level: `rmf_load_module()` then
`rmf_registry_add()`).

- `rmf_registry_init()` — reset all slots (call at boot).
- `rmf_registry_add(id, name, base, size)` — first free slot; name copied
  and truncated safely; `RMF_ERR_DUPLICATE` / `RMF_ERR_FULL` / `RMF_ERR_ARG`.
- `rmf_registry_find(id)` — lookup, NULL if absent.
- `rmf_registry_at(slot)` + `rmf_registry_capacity()` — iteration over
  slots (NULL = empty slot); `rmf_registry_count()` = loaded entries.
- `rmf_registry_remove(id)` / `rmf_registry_unload_all()` — forget entries
  (slot state → EMPTY; window memory is untouched and stays the owner's to
  reuse or power-gate).

## 6. Builder (`module_builder.py`)

Offline packer, python3 stdlib only, relative paths only. Two subcommands:

```
# raw blob (code + explicit params)
python3 firmware/modules/module_builder.py pack \
    --code code.bin --id 7 --entry 0 --bss-len 20220 -o m07.rmf1

# ELF32 little-endian (sections extracted automatically)
python3 firmware/modules/module_builder.py pack \
    --elf mod_player.elf --id 44 -o m44.rmf1

# dump + verify a module file
python3 firmware/modules/module_builder.py inspect m44.rmf1
```

- `--code`: `bss_start = align4(len(code))`, `bss_len` from `--bss-len`
  (default 0), `entry` from `--entry` (default 0).
- `--elf`: all `SHF_ALLOC` initialized sections become the code image
  (address-relative, zero-filled gaps); `SHT_NOBITS` sections define
  `bss_start..bss_start+bss_len`; entry from `e_entry` minus the image base
  (Thumb bit stripped). `--bss-len`/`--entry` override when given.
- `inspect` recomputes the payload CRC and exits non-zero on any mismatch —
  usable as a CI sanity gate on built modules.

## 7. Tests & verification

`sh firmware/modules/tests/run_tests.sh` (run 2026-10-09, all green):

- `test_loader.c` — CRC known-answer vectors; pack→load round trip at
  device scale (2980 B code + 20220 B bss); bss-zeroing proof on a poisoned
  window (code exact, gap untouched, bss zero, tail untouched); CRC
  corruption rejection (1-byte code flip and corrupt crc field); bad
  magic/version/flags; length sanity (truncated header/payload, zero code,
  bad entry, overlapping/unaligned bss, hostile u32 lengths); window
  overflow (code, bss, huge bss_len without wrap); NULL arguments.
- `test_registry.c` — full lifecycle (init/add/find/iterate/remove/
  unload-all), duplicate/full/not-found errors, name truncation, and an
  end-to-end load→register→unload pass.
- `test_builder.c` — builder→loader round trip for files produced by
  module_builder.py (raw and `--elf`), proving the Python and C CRC/header
  serializations agree byte-for-byte; 1-byte flip rejection on builder
  output.

Compile standard: host `cc -Wall -Werror` on every test; sources also
compile with `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c` (the driver
runs this check when the toolchain is present).

## 8. Open items (target integration)

1. **Relocation** — RMF1 v1 has NO relocation table: a module must be
   linked position-dependent for its target window (like the device's own
   modules, which are placed at fixed HIGHRAM windows). Either (a) link
   each module per window with a dedicated linker script, or (b) add a
   reloc-table flag bit + `RMF1_FLAG_RELOC` payload in v2 so one image can
   land in any window. Decision deferred until the app wiring starts.
2. **Overlay interplay / window reuse** — the device swaps small modules in
   and out per screen ("m44/m45/m47/m48 … swapped per screen"); the
   registry's `remove`/`unload_all` are bookkeeping only. We still need the
   eviction policy (who may unload what while a screen is active) and the
   cache-flush/I-cache considerations on Cortex-M3 (none architecturally,
   but DSB/ISB after deploy is cheap insurance) at the app level.
3. **Window table** — the loader is window-agnostic by design; the app must
   own the window inventory (e.g. HIGHRAM0 `0x0307A000..0x0309F000`,
   151552 B) and enforce exclusive use per slot. Multiple named windows
   (the device log suggests at least HIGHRAM0) need a small window manager
   before multi-module-per-boot scenarios.
4. **Module storage & id allocation** — where `.rmf1` files live (flash
   region vs user volume) and the id map (device uses m44 player-UI-ish,
   m78 = B-core firmware). ReChord will keep a static id table in `app/`;
   RMF1 ids are opaque to the loader.
5. **key/`%08x/%08x` question** — the device header log shows `key
   %08x/%08x` (encryption? double check?). RMF1 v1 uses a plain CRC and
   reserves the flags word for future signing; revisit if modules ever come
   from untrusted storage.
6. **B-core images** — m78-style second-core firmware (41196 B into
   HIGHRAM0, then `player: B start`) is a *loadable unit plus a core-start
   action*, i.e. RMF1 load + an app-level "start core" hook. Not part of
   the loader; tracked with the IPC workstream.
7. **App wiring** — `app/` will own: window table, id→name table, load-on-
   demand from the UI, and logging in the device's spirit (`mod: m%02u
   loaded, %u B + bss %u B`) via `rmf_strerror()`.
