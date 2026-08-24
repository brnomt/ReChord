# AP Mailbox & Event Dispatch — Ghidra RE Guide (Echo Mini / RKnanoC)

> Goal: locate and understand, in the **stock AP (fw1)**, (a) the mailbox
> send/receive path, (b) the ISRs and wait-loops that block on BB replies,
> and (c) the key/menu event dispatch — using the Ghidra MCP server.
>
> Everything in **bold** below was verified in this repo (SDK source +
> stock section_3 loaded in Ghidra as `section_3_0x00081A14.bin`,
> ARM:LE:32:v8-m, image span 0x02FE0000–0x04F427E3).

## 0. Known facts (do not re-derive)

| Item | Value | Source |
|---|---|---|
| Mailbox MMIO base | **0x40110000** | `driver/hw_memap.h`; confirmed in stock disasm @0x03073b7e |
| Mailbox struct | **0x50 bytes**, 4 channels per direction; Cmd n at `+0x08+8n` (A2B) / `+0x30+8n` (B2A), Data at Cmd+4; `IntEn` at +0x00/+0x28; `Status` at +0x04/+0x2C, **write-1-to-clear** | `hw_mailbox.h`; stock disasm (`add r0,r0,lsl#2; add r0,r2,r0,lsl#4` = base + id*0x50) |
| Channel assignment | **ch0 = system, ch1 = decode, ch2 = file, ch3 = debug** | `Main2.c`, `BBSystem.c`, `AudioControl.c` |
| IRQ numbers | MAILBOX0..3 = **INT_ID 25..28 = NVIC IRQ 9..12 = vector slot 25..28** | `system/os/interrupt.h` |
| Boot handshake | BB sends **`MSGBOX_CMD_SYSTEM_START_OK` (=1)** on B2A ch0; AP's `BBSystemAIsr` sets `BbSystemStartOK=1`; `StartBBSystem` waits ≤ **200,000** iterations (~200 ms) | `driver/BB/BBSystem.c` |
| Hold handshake | AP→BB **`BB_HOLD`(2)** on ch0; BB must reply **`BB_HOLD_ACK`(3)**, then poll for **`BB_HOLD_EXIT`(4)** (5 ms) | `BSP2.c:BBSystemBIsr` |
| Decode protocol | AP sends `DEC_OPEN`(1) on ch1, waits for `DEC_OPEN_ERR`(2) **or** `DEC_OPEN_CMPL`(3) — either breaks the wait (`gOpenDone` set on both, AudioControl.c ~2786/2797); wait loop counts down from **20,000,000** (0x1312D00) × DelayUs(1) ≈ 20 s | `audio_main.h`, `AudioControl.c` |
| File protocol | BB→AP requests `FILE_SEEK(105)`, `FILE_READ(107)`, `FILE_WRITE(109)`, `FILE_TELL(111)`, `FILE_GET_LENGTH(113)`, `FILE_CLOSE(115)`; AP replies `*_CMPL` (106/108/110/112/114/116). AP→BB on ch2 carries `FILE_OPEN_CMPL(101)` with a `FILE_HANDLE_t*` in the data reg | `filesys/file.h`, `Main2.c` |
| Shared data | `chip_freq` struct copied by AP to **0x01010000** (0x01020000 for AAC module) before releasing the BB; UI framebuffer **0x03024868** | `BBSystem.c:StartBBSystem`, segment table |
| BB boot | ROM/loader jumps to `firmware_entry` @ **0x03000010** (section_3 +0x10) with boot params in r0; stock entry is an HW-init callback that tail-calls `rom_hw_init2(0x18f|0x191)` and **returns to the ROM** | `docs/dispatch-map.md` |
| ROM dispatch table | The ROM calls **fixed offsets inside section_3**: +0x010 entry, +0x162/0x16C/0x24E USB-MSC, +0x296/0x4F4/0xA72 unknown, +0x546 RKDev_Close, +0xA74, +0xABA MSC dispatcher, **+0x70FA (0x0300710a) = status-bar/text draw**, +0x1020C (0x0301020c) = MainUI_KeyHandler | `docs/dispatch-map.md`, `entry_stubs.S` |
| AP code base | stock fw1 SYS_CODE @ **0x03060000**, SYS_DATA @ 0x03000000+; fw1 header SP = 0x03050000. (0x03060000 is *AP code*, not a BB vector table) | `docs/fw1-packing.md` |

> Note: vector slots reference the **RAM** vector table each core builds
> (`exceptions_table2` for the BB, `exceptions_table` for the AP) — the
> only "vector table" at a fixed address is the 16-byte RKnanoFW header
> (word[2] = initial SP, code entry at +0x10).

## 1. Load fw1 into Ghidra the useful way

fw1 is scatter-loaded, but **flash is XIP-mapped with IMG offset N ↔
address 0x03000000+N** (verified in `docs/fw1-packing.md`). Therefore:

1. Import the **entire stock `HIFIEC37.IMG`** (33,554,436 bytes) as a raw
   binary at base **0x03000000**, language `ARM:LE:32:v8-m` (or v7-m;
   Cortex-M3 Thumb-2). Every fw1 scatter-table `src` and every XIP pointer
   then resolves statically.
2. Alternatively import just the fw1 payload (`0x7B8..0x57820`) at
   0x03000000+0x7B8 — smaller, but XIP pointers past the payload dangle.
3. Run analysis, then **disable the "ARM Aggressive Instruction Finder"**
   re-runs on data ranges if disassembly goes sideways.

You already have stock section_3 (stock BB) loaded — that is the reference
implementation for the *other* side of every transaction.

## 2. Find the mailbox MMIO accessors

1. **Byte search** for the base constant: pattern `00 00 11 40`.
   In stock section_3 this hits dozens of literal-pool entries — each codec
   overlay module embeds its own copy of the `MailBox*` API.
2. Open a hit's xref (`get_xrefs_to`) and disassemble ~0x60 bytes around it.
   The signature is unmistakable:
   ```asm
   ldr   r2, [pc, #..]        ; literal = 0x40110000
   add.w r0, r0, r0, lsl #2   ; r0 = MailBoxID * 5
   add.w r0, r2, r0, lsl #4   ; r0 = base + MailBoxID * 0x50
   ldr/str ..., [r0, #off]    ; off per the table in §0
   ```
3. Classify each function by the offsets it touches:
   `+0x00` RMW = `MailBoxEnableA2BInt`, `+0x04` plain store = `ClearA2BInt`,
   `+0x08+8n` store = `MailBoxWriteA2BCmd`, load = `ReadA2BCmd`, etc.
   Name them (`rename_function_by_address`) — xrefs will then lead you to
   every IPC call site in that module.
4. Read-side functions with a `switch(CmdPort)` over 4 cases compile to a
   small jump table on 0..3 — that's your confirmation.

## 3. Find the RAM vector table and the registered ISRs

1. **Byte search** for the VTOR literal `08 ED 00 E0` (0xE000ED08). Every
   `IntRegister`-style function loads it before `str vt, [.., #0xD08]`-
   style stores (the SDK writes `nvic->VectorTableOffset`).
2. The RAM vector table itself = a run of ~57 near-identical Thumb pointers
   (all `IntDefaultHandler`), with slots 25–28 patched at runtime. Find it
   by searching data for a repeated function pointer value spaced 4 bytes
   apart, or by following the VTOR store operand.
3. `IntRegister(irq, handler)` compiles to `str handler, [table + irq*4]`
   plus (first time) the VTOR init. **Xrefs to the table base = every ISR
   registration site.** Slots 25–28 = mailbox; slot = 16 + IRQn.
4. Enable/clear sites: `NVIC ISER0` (0xE000E100) store with bits 9–12 =
   mailbox IRQs; the SDK also writes priorities via IPR (0xE000E400).

## 4. AP boot handshake — `StartBBSystem` / `BBSystemAIsr`

SDK source `driver/BB/BBSystem.c` is the exact blueprint; find its compiled
form in fw1 by anchors:

1. **String anchor**: `"StartBBSystem: timeout!!!"` (and `DEBUG` strings
   near it). `list_strings`/`search_strings` → xref → containing function.
2. Inside, expect in order:
   `ScuSoftResetCtr(CAL_CORE_SRST, 1)` → `ModuleOverlay(id, ALL)` →
   `memcpy(0x01010000, &chip_freq, ...)` → `ScuSoftResetCtr(.., 0)` →
   the wait loop `while(!BbSystemStartOK){ WatchDogReload(); BBDebug(); DelayUs(1); }`
   with **200,000 (0x30D40)** countdown.
3. `BBSystemAIsr` = the ISR registered on slot 25: clears B2A int 0
   (store 1 to 0x4011002C), reads B2A Cmd0, and its `switch` sets
   `BbSystemStartOK=1` on **cmd==1** and `BBsystemHoldState=1` on
   **cmd==3**. Those two flag stores identify the handshake flags —
   xrefs to the flags give you every waiter.

## 5. AP decode/file blocking waits (the "freeze" loops)

1. **Constant anchor**: 20,000,000 = `0x1312D00` in a literal pool near a
   `DelayUs(1)` call — that's the codec-open wait (`while (!gOpenDone)` in
   PCMFunction/CodecOpen). Any DEC_OPEN_ERR/CMPL breaks it; *nothing* = 20 s
   hang then watchdog.
2. The AP's decode ISR (`AudioDecodingGetOutBuffer`, AudioControl.c ~2883)
   registers on **slot 26**; the file ISR (`AudioDecodingInputFileBuffer`,
   ~3006) on **slot 27**. In fw1, slots 26/27 handlers read A2B Cmd/Data —
   wait, careful with direction: the *AP* reads **B2A** regs (the BB's
   replies) and writes **A2B** regs (its commands). The naming is from the
   BB's perspective ("A to B"); on the AP side the same registers appear
   with swapped roles.
3. `FILE_HANDLE_t` (passed by pointer in the data reg on `FILE_OPEN_CMPL`):
   `handle1` (FILE*), `filesize`, `curfileoffset[3]`, `codecType`,
   `handle2`/`handle3` (AAC/ALAC/FLAC aux) — layout from `Main2.c`.
4. Every `while (!flag)` + `DelayUs` pair near a `MailBoxWriteA2BCmd` is a
   candidate deadlock — enumerate them via xrefs to the write-API functions.

## 6. Key / menu event path

1. Physical keys = SAR-ADC matrix (`LADC_AIN[0:2]`, 3 button modes A/B/C —
   `docs/HARDWARE.md`). Find the scan loop via `GetAdcData` /
   `CheckAdcState` call sites in fw1.
2. The loader/ROM input API: **`0x02ff813a`** (ROM "get input event",
   used by our key-handler hook). Calls into the 0x02FE0000–0x02FFFFFF
   ROM API region are direct BLs — grep the disasm for `bl 0x02ff8`.
3. **Fixed-offset dispatch**: the ROM also calls section_3 at +0x1020C
   (`MainUI_KeyHandler`) and +0x70FA (status-bar draw). In a *stock AP*
   trace, key events flow: SAR-ADC ISR → OS message queue → UI window task
   → mailbox commands to BB (on media actions) or direct redraw.
   When only BB is replaced, the crash point is the **mailbox command**
   stage — the AP-side UI code is intact.

## 7. Ghidra MCP cookbook (commands used to verify all of the above)

```
get_current_program_info                     # confirm what's loaded
search_byte_patterns  pattern:"00 00 11 40"  # mailbox base literal
search_byte_patterns  pattern:"08 ED 00 E0"  # VTOR literal
get_xrefs_to           address:0x03073ba4    # who uses this literal
disassemble_bytes      start:0x03073b7e end:0x03073bd0   # verify struct math
decompile_function     address:0x0300710a    # status-bar draw callback
search_functions       name_pattern:"MailBox"
search_strings         search_term:"StartBBSystem"
list_functions_enhanced / get_bulk_xrefs     # batch follow-up
```

Workflow that works: **byte-pattern → xref → 0x60-byte disasm → rename →
bulk xrefs to the renamed accessor**. Do not trust auto-generated names
(Ghidra matched libc/net signatures — `dhcpd_add_option`, `Http_Close`…);
only code and xrefs are reliable.

## Appendix A — why the top status/menu text is missing (stock AP + custom BB)

Three verified mechanisms, in order of impact:

1. **The text renderer lives in section_3 (BB image).** The stock loader/ROM
   draws the cassette *bitmap* from the preserved resource section
   (0x04F00000+), then calls **section_3 offset 0x70FA (0x0300710a)** to
   render the top status text. Decompiled stock code there
   (`application_start`) formats time/status via ROM getters
   (`FUN_02ff2e00(0x40/0x3f/0x49/0x4f)`), measures strings with
   `FUN_02fed6a6`, and blits via `FUN_02feda18`. The custom section_3 has an
   entry stub (`bx lr`-style trampoline) at that offset → the call returns
   instantly → **bitmap shows, text never draws**. Same story for the fonts
   (`Font12_CompiType @ 0x03010f98`) and string tables that live in the
   section_3 image.
2. **Zero-padded flash tail corrupts AP load-time data.** `pack_img.py`
   (before `--keep-stock-tail`) zero-pads the 9.3 MB section_3 flash region
   beyond the custom image. But fw1's memory-map table **XIP-copies data
   from inside that region** (verified entry 88 → IMG 0x8A5A4 ≈ 35 KB in —
   squarely inside the custom code). The AP then loads custom-BB code bytes
   as string/pointer tables → blank fields, garbage pointers, crashes on
   use. `--keep-stock-tail` (now default for `pack-bb-stub-img`) fixes this
   class entirely.
3. **Runtime mailbox deadlocks** (not a render bug but the same symptom
   family): first button press → AP sends `DEC_OPEN` → nothing replies →
   20 s `gOpenDone` wait → watchdog power-off. The V0.20 build registers
   the SDK ISRs but `rechord_main` replaced the `Main2()` service loop, so
   `pcb.audio_decode_status` flags are set and never consumed — **no reply
   is ever sent**. The bb_stub (below) replies directly from the ISRs.

## Appendix B — the minimal BB stub

`firmware/bb_stub/bb_stub.c` (build: `make bb-stub`, flash:
`make pack-bb-stub-img` → `build/ReChord_BB_Stub.IMG`):

- option-B boot (stock ROM HW init) then a WFI loop — no UI, no framebuffer;
- full 57-entry RAM vector table in `bb_vect` (256-aligned), VTOR set,
  mailbox clock gate enabled defensively, A2B ints 0–3 + NVIC IRQ 9–12 on;
- sends `SYSTEM_START_OK` on B2A ch0;
- ISR-context replies: `BB_HOLD→HOLD_ACK` (+ exit poll), `DEC_OPEN→DEC_OPEN_ERR`,
  `DECODE→DECODE_ERR`, `GETBUFFER/GETTIME/SEEK/CLOSE→*_CMPL(0)`,
  file `SEEK/READ/WRITE/TELL/GET_LENGTH/CLOSE→*_CMPL`;
- telemetry in RAM @ 0x03000118: `[0]='BOTS' [1] enabled [2] START_OK sent
  [3..6] per-channel reply counts [7] last cmd [8] last channel`.

Expected behavior when flashed: stock cassette UI boots, **buttons/menus
navigate without freezing** (AP gets its replies), audio playback obviously
doesn't work (every open returns ERR), and the top status text is still
blank until the draw callback (mechanism A-1) is re-implemented or the
ReChord UI replaces the loader screen.

**Hardware result (2026-08-24 23:29 build):** boots to the cassette UI,
menu navigation works, no boot freeze — the mailbox wiring, RAM vector
table, IRQ 9–12 enable and START_OK handshake are CONFIRMED working.
Media Library and Music crash/freeze: **expected** — those features need
the real BB services (media DB queries, codec pipeline, file streaming),
which the dummy stub intentionally does not implement. The packed artifact
was verified byte-exact: stub code at IMG 0x81A14, tail 100% stock
(`--keep-stock-tail`). Next step toward "music plays on custom BB": make
`rechord_main` run the SDK's real `Main2()` service loop (V0.20 registers
the ISRs but never consumes `pcb.audio_decode_status`, so no replies are
ever sent), or extend the stub with the file/DB protocol.
