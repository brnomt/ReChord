# RefCFW CFW (HIFIEC90.IMG) — Initial Black-Box Analysis

> Source: `community/refcfw/HIFIEC90.IMG` (community CFW "RefCFW", author unknown,
> **no source available** — binary-only RE). Unpacked artifacts:
> `community/refcfw/unpacked/` (gitignored, like the rest of `community/`).
> Analysis date: 2026-10-09. Tools used: `tools/unpack_rk.py`, strings, header
> parsing vs `tools/HIFIEC37_unpacked/` (stock v3.7.0).

> **⚠ CORRECTIONS (2026-10-09, after independent audit + re-verification).**
> The initial round's address model was WRONG; the byte-verified model is:
>
> - **One contiguous RAM image**: `IMG[0x1F8..0x500000]` loads at **`0x0304F490`**
>   (equivalently: RAM addr = `0x0304F298` + IMG offset). There are NOT three
>   separately-loaded parts: only IMG 0x1F8 is a real `RKnanoFW` section (base
>   0x03050000, 91 scatter rows); the `RKnanoFW` bytes at 0x10CC3/0x11288 are
>   **strings**, and `unpack_rk.py`'s fw1/sec2/section3 split is an artifact.
>   File offsets map as: `fw1_AP X ↔ 0x0304F490+X`, `sec2 X ↔ 0x0305FF5B+X`,
>   `section3_BB X ↔ 0x03060520+X`.
> - **Vector table at `0x03050000`** (fw1 file 0xB70): initial SP `0x03004000`,
>   Reset `0x0306296F` → section3 file **0x244E** (the code start). So the header
>   field 0x03050000 is the vector-table/load base — NOT "initial SP".
> - Superseded: §2/§3 base claims (`0x03050B68`, `0x03000000`), §5c pool
>   attributions (the pool near 0x83BC actually owns `player: B …` strings —
>   `FUN_03058D6C` is the **player/B-core supervisor**, not the update writer),
>   and the "5x 0x03000020" string proof (those hits are unaligned bitmap noise).
> - The `0x030610C3/C8/CC…` pool entries resolve to codec extensions
>   (`flac`,`wav`,`mp3`,`m4a`…) at section3 0xBA3+, not to `7e287ea`.
> - REFCFW.CFG keys also include **`cursor`** (key-name table ~0x17F8); `browse`
>   is in that table, not the template string.
> - Corrected Ghidra import: project **`ReChordV2`**, program `refcfw_ramimg`
>   @ 0x0304F490 (the old `ReChord` project keeps the wrong-base v1 programs).
> - Methodology lesson: pointer→string proofs must land on **exact string
>   starts at aligned offsets**; verify at least 3 independent anchors and try
>   to falsify with alternative bases before accepting a load base.

## 1. What it is

A **custom firmware written from scratch** (not a patched stock image), branded
**"REFCFW 0.9.2"** with build hash `7e287ea` embedded in the image header.
Internal codename string: `RefCore`. Every part differs from stock in code,
strings, and structure; only the RKnanoD container/SDK conventions are shared.

## 2. Container layout (RKnanoD player container, same family as stock)

```
0x00000000  header: version code 26 20 07 10 00 00 09 00,
            magic 0x76543210 @ 0x0C, vendor "REFCFW", build "7e287ea"
0x000001F8  fw1_AP            68,299 B   (stock: 357,928 B)
0x00010CC3  sec2_bootloader    1,477 B   (stock: 172,532 B)
0x00011288  section3_BB    5,172,600 B   (stock: 9,670,650 B)
0x0044C1F8  RKnanoFW end marker (zero pad to 0x500000)
0x00500000  trailer 0xEFDC758B            (stock v3.7.0: 0x1EA1C309)
Total: 5,242,884 B = 5 MiB + 4 (stock IMGs are 32 MiB + 4).
```

- **No `ROCK26IMAGERES` resource blob** — RefCFW ships no stock UI resources.
  Either it renders its own UI (evidenced by UI strings, see below) or reuses
  whatever is already in flash beyond the 5 MiB region. On-device check needed.
- Parts are contiguous (no gaps); layout is clean and self-packed — the author
  has their own packer, like our `pack_fw1.py`/`pack_img.py`.

## 3. fw1 memory map (verified 2026-10-09)

Same 91-entry format as stock (RKnanoD SDK convention). Row[1]
(`0x03004000 0x00001AFC 0x03005AFC 0x00018C50`, ROM region) is **byte-identical
to stock** — same chip ROM. Only **6 rows are non-zero** (stock fills the table):
row[0], row[1], row[2], row[88], row[89], row[90] — a lean loader.

```
[ 0] 0x03050B68  0x03050000  0x0002666C  0x030771D4   image: load/base 0x03050B68, SP 0x03050000
[ 1] 0x03004000  0x00001AFC  0x03005AFC  0x00018C50   ROM region descriptor (== stock)
[ 2] 0x03078CD0  0x03079DCC  0x00000004  0x00000000   data chunk: XIP -> RAM
[88] 0x03078CD4  0x0307A000  0x00000BA4  0x00000000   data chunk: XIP -> RAM
[89] (bss)                      0x0307ABA4  0x00004EFC   bss window
[90] 0x03079878  0x0307A000  0x000008E8  0x00000000   data chunk: XIP -> RAM
```

**Verified load bases (literal-pool evidence):**
- **fw1 file offset X ↔ 0x03050B68 + X** — its own code references its strings
  and code with this base (e.g. `REFCFW 0.9.2` @ file 0x10558 is referenced as
  0x030610C0; a format string at file 0x1075B is referenced as 0x030612C3).
  The header field 0x03050000 is the **initial SP**, not the load base.
- **section3 file offset X ↔ 0x03000000 + X** (flat "firmware window", same as
  our own `firmware.ld` convention) — e.g. `usb: bus reset` @ file 0x20 is
  referenced 5x as 0x03000020.
- section3 code calls into fw1's RAM window via absolute 0x0305xxxx pointers
  (fw1 is the resident library; section3 is the big app).

## 4. Evidence it is custom-built (not stock patched)

- Header strings are custom: `read error`, `copy 1`, `copy 2`, `repair`,
  `usb: bus reset` — stock has `fw1 Sign error!`, `fw1 compare error!`,
  `fw2 compare error! 0x%x`.
- Version string `REFCFW 0.9.2` appears in both fw1 and section3.
- No stock SDK banners (`RKnano SDK 1.0` is replaced by the build hash).

## 5. Feature surface (from strings)

### 5a. fw1 = boot manager / updater (102 strings, full inventory 2026-10-09)

| Area | Evidence |
|------|----------|
| Update flow | `no HIFIECnn.IMG / ECHOMINI.IMG` — picks update images by name from the user volume; `Firmware update`, `do not power off or unplug`, `Update done`, `rebooting...`, `image deleted (as stock does)` (stock-compatible install: copy IMG, reboot) |
| A/B slots | `FW1 written and verified (no room for FW2)`, `FW1 and FW2 written and verified`, `FW1 %s  FW2 %s`, `fw1_only %d` — dual slots written to flash with readback verify |
| IDB / disk layout | parses Rockchip IDBlock: `IDB%d: sys %u%s MiB, FW2 @%x, user @%x`, `idb copy %d: P %x R %x user_base %x (stock's %x)`, `volume @%x, NOT at stock's place %x`, guard `FW2 would overwrite stock's disk` |
| Recovery | `recovery: result %d cause %d`, `no FW1 repair`, `copy a firmware image over USB`, `Recovery failed` |
| Clock safety | `pll: the last 200 MHz boot did not clear its marker: staying at 96 MHz` — crash-safe clock marker; `pll: switching to %u MHz`, `2M-iteration loop`, `hiram: %08x..%08x, %u bad words` (RAM diagnostics) |
| USB MSC | `USB disk`, `waiting for the computer`, exit reasons `ejected`/`cable removed`/`host suspended`/`charger (no host)`/`key pressed` |
| Storage | own FAT stack: `mount: %d type FAT%d...`, `exFAT (not supported here)`, `superfloppy`, `in MBR p0`, `eMMC %u MiB` |
| Fonts | `bitmap fonts from selected slot`, `not found, built-in 6x11`, `font: %s, %u icons` |
| Key scripts | `KEYS    TXT`, `script: start`, `script: done, font misses %u` |
| Boot capture / backup | `Boot capture`, `System backup`, backup format checks: `backup header mismatch`, `CRC mismatch (bad copy?)`, `size / layout not accepted`, `flash verify failed` |
| Boot banner | `boot: build %s, cpu %u Hz, panel %08x`, `boot: resources @%x (%s) are %s` (inspects the ROCK26 resources region in flash) |

### 5c. fw1 function map (literal-pool clustering, round 2)

fw1 references its strings through per-function literal pools (pointer
tables); clustering the u32 pointers maps features to code:

| Function region | Strings owned | Feature |
|---|---|---|
| `FUN_03058D6C` (pool @`0x03058F24`) | `FW1 written and verified (no room for FW2)`, `FW1 and FW2 written and verified`, `image deleted (as stock does)`, `image cleanup failed` | **update writer/verifier + state machine** (states 0-4, 30 s timeout, shared block at `0x01020xxx`, callback hook, `FUN_03057C28` = verify) |
| `FUN_0305850C` area (pool @`0x030590C4`) | `image kept`, `no HIFIECnn.IMG / ECHOMINI.IMG` | update image finder |
| `~FUN_03059168` (pool @`0x03059168-0x03059430`) | `update: found %s (%s), %u bytes`, `found %s, %u bytes`, `after update`, `remove update image before restarting` | update orchestrator |
| `FUN_03059588` (pool @`0x03059814`) | `Update failed`, `Boot capture`, `System backup`, `rebooting...`, `tests module: error %d`, `source loader` | status/UI + capture + tests dispatcher |
| `0x0305AC5C-0x0305B330` | `IDB +16/+20 %08x  +4/+6 %08x`, `volume @%x, NOT at stock's place %x`, `idb copy %d: P %x R %x user_base %x (stock's %x)`, `emmc: %u sectors...` | IDB parser / eMMC init |
| `0x0305B804-0x0305B9BC` | `recovery: result %d cause %d`, `no FW1 repair: %s (%d)` | recovery |
| `0x0305BD10-0x0305BF7C` | `pll: %d, cpu %u Hz, con0...`, `pll: 2M-iteration loop took %u ms`, `hiram: %08x..%08x, %u bad words` | PLL / RAM diagnostics |
| `0x0305C27C-0x0305C2AC` | `font: %s, %u icons`, `not found, built-in 6x11` | font loader |
| `0x0305C4B0-0x0305C74C` | `usb: exit %s...`, `script: start`, `script: done, font misses %u` | USB mode + KEYS.TXT script engine |
| `0x0305CC6C-0x0305CED4` | `CRC mismatch (bad copy?)`, `backup header mismatch`, `FW2 would overwrite stock's disk`, `flash write/verify failed`, `size / layout not accepted` | backup format validator / flash writer |
| `FUN_030571E4` | (used with all fmt strings) | **the logger** (`log(fmt, ...)`) |

Runtime notes:
- The update state machine coordinates via a **shared block at `0x01020xxx`**
  (handoff with ROM/loader — cf. string `invalid loader slot/geometry handoff`).
- `FUN_03057C28(image)` = image verification; `FUN_0305D0D0(1,1,slot)` = commit.

### 5d. section3 = main app (strings)

| Area | Evidence |
|------|----------|
| Player / IPC | `player: B ...` family: peer "B" core streamed via ring buffer (`ring rd %u wr %u`), clock negotiation (`B clock /%u: no ack`), state machine (`B is gone`, `B drained`, `B off`) |
| Config | `\REFCFW.CFG` — `key=value, one per line. Unknown keys are ignored` |
| Dual firmware | `FW1 %s  FW2 %s  settings \REFCFW.CFG`, `FW1 may be damaged: reinstall the`, `firmware over USB` |
| Modules | `mod: m%02u header: magic %08x id %u key %08x/%08x len %u/%u bss %08x..%08x`, `MODULE m%02u did not load` — loadable module format (magic/id/key/len/bss) |
| Recovery/dump | `ECHONANO SYSTEM BACKUP v1`, `ECHONANO BOOT CAPTURE v1`, `BOOTDUMPREQ`/`BOOTDUMPBIN`/`BOOTDUMPOK` protocol |
| Logging | `ECHOLOG.TXT` on storage, ring log with drop counter |
| USB MSC | SCSI trace strings, INQUIRY `ECHO    MINI            1.00`, bulk timing telemetry |
| FAT | own FAT driver with cache telemetry (`FAT cache %u %% hits`) |
| Audio/DAC | tone test (jack 3.5/4.4 mm), direct DAC/I2S/CRU/DMA bring-up log, dual-DAC status (`dac0 ... dac1 ...`) |
| Keys | ADC keys, chords (`keys: chord vol+-`), arming (`keys: armed`) |
| UI | `Music`, `Now playing`, `Nothing playing`, `no playable files in there`, `no shortcut on that key`, `PLAYER` |
| Faults | `B FAULT kind %u pc %08x lr %08x cfsr %08x` (Cortex-M fault dump) |

## 6. Open questions (hypotheses, not yet verified)

1. **Core role of each part.** fw1 loads at the stock AP base `0x03050000`.
   But the "player: B ..." strings (AP-side behavior: talks *to* the B core)
   live in section3. Consistent with our earlier note
   (`dispatch-verification`: "section_3 contains UI code, model unresolved").
   Needs Ghidra to settle which core executes which part.
2. **FW1/FW2 slots** — A/B update banks in flash? Or just the two core
   firmwares? The 5 MiB image size may be one slot size.
3. **Module storage** — where do `m%02u` modules live (flash offsets vs a
   storage folder), and is the `key` field encryption or just a check?
4. **Resources/UI** — custom rendering (fonts/framebuffers) vs in-flash reuse.
5. **Trailer `0xEFDC758B`** — CRC or version magic (stock is per-version).

## 6b. Boot chain (round 3, decompiled — corrected bases)

```
Reset @ 0x0306296E (s3 file 0x244E)   — standard C runtime startup:
  IRQs off → save 4 boot params (ROM handoff) → .data copy → .bss zero
  → DSB/ISB → call FUN_03050B08 → hang
FUN_03050B08 (fw1 region)             — boot manager main:
  early-init x4 → enable IRQs → phase calls → PLL settle (100 ms)
  → boot status screen via FUN_03050550(id, flags, fmt, ...) draws
    (ids 0,1,2,4,8,9,10 = build/SD/eMMC/IDB/pll/hiram/resource lines)
  → storage + IDB validation (FUN_030527AC/FUN_03052A98/FUN_03052D44,
    retry loop, geometry checks field+0x18+0x2000 vs field+0x0C)
  → readback verify via FUN_0305478C(region, 0/size) (0 = verified)
  → summary logs via FUN_03055B0C (the logger) → next stage
```

Key service functions (renamed in ReChordV2 when applied):
- `FUN_03055B0C` = **logger** `log(fmt, ...)`
- `FUN_0305CDF8` = **module header dump** (logs `mod: m%02u header: magic %08x
  id %u key %08x/%08x len %u/%u bss %08x..%08x`) — its CALLER is the loader
  (next target)
- `FUN_0305D63C` = **REFCFW.CFG serializer** — iterates a 12-slot key table over
  the settings struct (fields at +0x0C count, +0x40, +0x48, +0x4D flags) —
  gives the settings struct layout
- `FUN_03057694` = **player/B-core supervisor** (state 0-4, 30 s timeout,
  shared block @ 0x01020000, indirect callback)
- `FUN_03050550` = boot/status screen draw (id, flags, fmt, ...)

## 6c. Runtime ground truth (device log, 2026-10-09)

`community/refcfw/device/ECHOLOG.TXT` (1.5 MB rolling log) + `REFCFW.CFG`,
pulled from the live device (runs **REFCFW v0.9.2 build `7c3d4cf`, 2026-10-05** —
newer than the `7e287ea` IMG we hold). Key facts, all previously predicted:

- **Module system in action**: `mod: m44 loaded, 2980 B + bss 20220 B`,
  `m45` 2280 B, `m47` 1648 B, `m48` 2288 B (UI/feature modules, swapped
  in/out per screen), and **`mod: m78 loaded into HIGHRAM0, 41196 B in
  8193 us`** — 41196 = **0xA0EC**, the exact length the loader decompilation
  validates (`FUN_0305d0c0`), immediately followed by **`player: B start 0 at
  400 MHz`** → **m78 IS the B-core firmware**, loaded into HIGHRAM0
  (`hiram: 0307a000..0309f000`) and the second core started from it.
- **IDB layout**: `idb copy 0: P 22000 R 11000 user_base 88000 (stock's 24000)
  user_sectors e08000` — the user volume sits at sector 0x88000, **not** at
  stock's 0x24000 (matches `volume @%x, NOT at stock's place`).
- Boot telemetry: CPU starts at 24 MHz → `pll: switching to 200 MHz` (pclk
  50 MHz, 2M-iteration loop 180 ms), fonts from slot (29 icons), `update: no
  image`, key ADC calibration (ch1-3), `keys: armed`, `sd: init -5` (no card).
- `fw1 1 fw2 1` — both firmware slots populated on-device.
- `cfg: load -4` — REFCFW.CFG parse returned -4 (its 12 keys are present with
  sane values; the return code needs interpretation).
- REFCFW.CFG real values: `play=repeat_track brightness=8 screen_off=30
  auto_off=15 usb=ask resume=folder strip=off tags=on cpu=auto volume=-23
  browse= cursor=0` — exactly the 12-key schema (incl. `cursor`).
- B-core clock negotiation is live: `player: B /1 -> /4 (100 MHz; busy 10%,
  out min 5576)`.

## 7. Ghidra scan status (round 2, 2026-10-09 — corrected)

**Current working program: `ReChordV2` project, `refcfw_ramimg.bin`** — the
contiguous RAM image `IMG[0x1F8:0x500000]` @ **`0x0304F490`**, one program
(fw1+sec2+section3 together, as they really load), 499 BL-target seed
functions + full auto-analysis, **730 functions**, saved in the project.

The old `ReChord` project holds the wrong-base v1 programs (`fw1_AP` @
0x03050B68, `section3_BB` @ 0x03000000, `sec2_bootloader`) — kept only as
backup; do not trust their absolute addresses.

Round-2 lessons (from the audit + re-verification):
- Load bases must be proven by **multiple aligned pointer→string-start anchors
  and falsification tests**; single u32 hits can be bitmap noise or table data
  that merely looks like a pointer.
- The vector table is the ground truth for the memory model (fw1 0xB70 →
  RAM 0x03050000: SP 0x03004000, Reset 0x0306296F).
- Reliable function seeds = BL targets from offline objdump sweeps, converted
  to runtime addresses with the CORRECT base.
- `run_script_inline`/`run_ghidra_script` are broken in this Ghidra 12.1.4 +
  ghidra-mcp 7.0.0 GUI combo (Felix bundle class-loading: "class not found by
  <loader>"); `analyzeHeadless` scripts work fine — do heavy lifting headless.

Round-1 lessons:
- Raw imports get no entry points, so auto-analysis finds only heuristic hits.
  Reliable function seeds = **BL targets from an offline objdump sweep**
  (`arm-none-eabi-objdump -D -b binary -marm -Mforce-thumb`, parse `bl`/`b`
  targets inside file bounds). Top targets (call count): `0x0301B01C` (56),
  `0x0301C7D8` (37), `0x0301E7C4` (35), `0x03003034` (20), `0x030243F4` (16),
  `0x0301C810` (14), `0x03024550` (14), `0x03002620` (12), `0x0301C934` (12),
  `0x030028DC` (11), `0x0301D348` (11), `0x03002C20` (10), `0x03002E92` (10),
  `0x0301A95A` (10), `0x03002452` (8).
- The 20 odd "pointers into section3" found in fw1 are **data references**
  (tables, pixel data, zeros), not code entries — do not seed functions there.
- section3 layout: string/data blob `0x0..~0x2450`, code from `~0x244C`;
  64KB-bank structure with recurring offsets `0xEA23`/`0xF0xx` (module banks?)
  — some are code, some data; unresolved.
- Deep pass needs `GHIDRA_MCP_ALLOW_SCRIPTS=1` on the MCP server (batch
  function creation from BL targets + `tools/run_auto_analysis.py`-style
  string-xref renaming). The Ghidra project is versioned and programs are not
  checked out — checkout before editing or changes are lost on close.

## 8. RE plan

1. ~~Import into Ghidra~~ — DONE (round 1 above). Next: with scripts enabled,
   batch-seed function starts from the offline BL-target list and re-run
   `tools/run_auto_analysis.py`-style string-xref renaming.
2. Map the boot chain: tiny sec2 stub → fw1 → section3; identify the module
   loader and the module header format.
3. Map the AP↔B protocol (ring buffer + mailbox registers) and compare with
   the stock IPC we already decompiled (`docs/re/decomp/...`, `Main2.c`).
4. RE the `REFCFW.CFG` parser (all keys) and the `ECHOLOG.TXT` writer — a live
   device log is the cheapest ground truth we can get.
5. On-device: boot RefCFW, pull `ECHOLOG.TXT`; use `BOOT CAPTURE`/`SYSTEM BACKUP`
   to dump boot ROM / stock regions (likely the easiest way to get the ROM API).
6. Feed findings back into ReChord: RefCFW proves which hardware paths work
   (B-core streaming, DAC bring-up order, USB MSC); its bring-up log strings
   are effectively a checklist of the register init sequence.
