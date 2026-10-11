# ReChord — Handover / Debug Notes for AI Agents

> **Read this first.** This file captures hard-won knowledge from building
> and flashing ReChord custom firmware on the FiiO Echo Mini (RKnanoC).
> It exists so a fresh AI session can pick up where the last one left off
> without repeating months of RE work or re-discovering bugs the hard way.
>
> Companion docs: [STATUS.md](STATUS.md) (build status), [HARDWARE.md](HARDWARE.md)
> (memory map), [FLASHING.md](FLASHING.md) (flash/recovery), [RE-HISTORY.md](RE-HISTORY.md)
> (decomp → SDK pivot), [re/](re/) (archived RE corpus + decompiled spec).

---

## 1. TL;DR — where the project stands

- **Goal:** free compile-from-source firmware for the FiiO Echo Mini so the
  user can add DSP effects (bass boost / reverb / EQ). One device, used daily —
  **must not brick it.** Always flash via the official method (copy IMG + reboot),
  stock is always restorable.
- **Approach:** compile the real Rockchip **RKnanoD SDK** from source
  (`firmware/rockchip/`), link it with a GNU linker script, splice the result
  into a stock IMG via `tools/pack_img.py`.
- **Working:** SDK compiles (44 objects), links to `build/rechord_full.elf`
  (998 KB), produces `build/section3_custom.bin`, packs into a flashable IMG.
- **Flashed on hardware:** V0.1 (stub kernel) and V0.2 (fault-dbg) **both boot**.
  The device shows the FiiO "cassette" UI, buttons/navigation work, **but every
  menu item press freezes then powers off** (no fault screen shown yet).
- **Next big open question:** *why no fault screen?* — see §7. Working theory:
  it is a mailbox deadlock, not a CPU fault.

### Pipeline (all working, `make all`)

```
make build-bb       → firmware/rockchip/**/*.c → build/bb/objs/*.o (45)
make link-bb        → build/bb/rechord_bb.elf + build/bb/section3_custom.bin
make pack-bb-img    → build/ReChord_BB.IMG (spliced into stock IMG)
copy ReChord_BB.IMG → device root as HIFIEC37.IMG, remove TF, reboot
```

Windows note: use `/c/winlibs/mingw64/bin/mingw32-make`.

---

## 2. Architecture discovered (critical)

The Echo Mini is effectively a **two-side system** (Rockchip "AP + BB" split),
not one monolithic app:

- **BB side = `bbsystem/Main2.c`** (what we compile and flash). `Main2()` is a
  service loop: `BSP_Init2()`, registers mailbox ISRs (`RegHifiDecodeServer`,
  `RegHifiFileServer`), then spins waiting on the mailbox for decode/file
  commands from the AP. It owns audio decode (codecs), file I/O.
- **AP side = the FiiO UI** (the cassette UI, menus, MusicWin/MainMenuWin).
  This is the ~10% FiiO layer, **not in our SDK build** — the stock binary has
  it, and it is what draws the screen and sends mailbox commands to the BB.
- **Mailbox** (`mailbox.h`, `main2_msgbox.h`): the inter-process channel.
  `MailBoxWriteB2ACmd/Data` (BB→AP) and `MailBoxReadA2BCmd/Data` (AP→BB).
  Channels: 0 = sys status, 1 = decode, 2 = file. Commands: `MEDIA_MSGBOX_CMD_*`
  (FILE_OPEN 0x0102, DECODE 0x0103, etc.).

**Implication:** our firmware is only the BB/audio half. When the user presses a
menu item, the AP (stock UI) sends a mailbox command to our BB — and our stubs
don't answer → the AP waits forever → watchdog/power-off. **This is a deadlock,
not a hard fault** (see §7).

### V0.20 boot path: Option B (ROM HW init + from-source main)

`firmware_entry` (entry_stubs.S @ 0x03000010) now does:
1. `bl rechord_hw_init` — runs the stock ROM init sequence (boot_param_layout,
   rom_alloc, rom_hw_init, rom_early_init) so PLLs, clocks, and the LCD
   controller are properly configured.
2. `b rechord_main` — our from-source app: ScatterLoader2 (zero BSS),
   BSP_Init2 (board init), mailbox handshake (SYSTEM_START_OK), then our own
   UI loop with the ReChord menu drawn into the LCD framebuffer at 0x03024868.

**Critical fix:** the ROM HW init macros (ROM_ALLOC, ROM_HW_INIT, etc.) now
have the Thumb bit (`|1`) on their addresses. Without it, calling these ROM
functions through function pointers (`BLX`) enters ARM state on Cortex-M3 and
faults immediately. This was the hidden root cause of V0.15 crashing on real HW.

### Why the cassette UI appears even though we replaced all the code

The UI framebuffer is at `0x03024868` (64,288 bytes, 320×100 visible per
`LCD_WIDTH/HEIGHT` in `SysConfig.h`). The LCD displays it by **hardware/DMA**
(`Lcd_BuferTranfer` is `#ifdef _FRAME_BUFFER_` — not a software path). So any
code that writes bitmaps there is visible. The AP UI (stock code or resources)
keeps drawing the cassette; our section_3 only replaced the BB half.

---

## 3. RKnanoFW section_3 header (byte-exact — do not "fix" it)

16 bytes at section_3 offset 0 (load address 0x03000000):

```
[0:8]  0x52 0x6B 0x6E 0x61 0x6E 0x6F 0x46 0x57   "RKnanoFW" magic
[8:12] 0x94 0xE7 0x01 0x03                       0x0301E794 LE = initial SP
[12:16]0x52 0x00 0x00 0x00                        0x52 = count/flags
```

`pack_img.py` does **splice only** — it does NOT rewrite the magic. The header
must be correct in `firmware/startup/startup.c`. The bootloader validates it;
a wrong header = boot rejected.

`firmware_entry` is at `0x03000010`; the bootloader jumps there with boot
params in r0. Stock prologue is `push {r4} / mov r4,r0 / bl ...` — mirror it.

---

## 4. The linker (`firmware/firmware.ld`) — what we learned

- SDK sections: `AudioCode/AudioBss/AudioData/AudioInitCode`, `SysCode/SysBss/
  SysData/SysInitCode`, `FindFileCode`, `FlacDecCode`, `driver_code`, `bb_vect`,
  `BBSysData`, `BBMainStack`, `SysReservedCode/Data`. All emitted by `_ATTR_*`
  macros in `armcc_compat.h`.
- Put `.text*`, `.rodata*` + all named code sections in `FW_CODE` (0x03000000).
- Put `.data*`, `.bss*`, `BBSysData`, `SysData`, `AudioData` in `FW_RAM`.
- Buffer regions (audio buffer 0x03005AFC, FAT cache, MP3 frame buf, AAC
  decoder, UI fb 0x03024868…) are `NOLOAD` at their stock segment-table
  addresses so absolute pointers in code match.
- Keil scatter symbols the SDK references: `Image$$BB_MAIN_STACK$$ZI$$Limit`,
  `Image$$BB_SYS_DATA$$ZI$$Base/Length` — define them in the .ld.
- Gotchas: `--gc-sections` silently drops nearly everything (stubs look dead);
  `.vectors` with 256-byte align breaks section ordering (separate it, NOLOAD);
  ALIGN(4) between SDK sections fixes "ordered sections with incompatible
  alignments".

---

## 5. stubs.c — the trap that caused the "freeze + power off"

Auto-generated link stubs live in `firmware/stubs.c`. **Original bug:** the
generator declared *function* symbols as `uint32_t name;` VARIABLES (zeroed).
The SDK calls them as functions:

- `__CPU_IntDefaultHandler2` — the vector table's fault handler target
  (`IntDefaultHandler2` does `b __CPU_IntDefaultHandler2`). As a variable, a
  fault made the CPU execute RAM zeros → immediate crash/power-off.
- `__SETPRIMASK2` / `__RESETPRIMASK2` / `__SETFAULTMASK2` /
  `__RESETFAULTMASK2` / `__WFI2` — ARM intrinsics called from `interrupt2.c`.
  As variables, IRQ enable/disable was executing zeros.

**Fixed:** `firmware/fault.c` now provides the real `__CPU_IntDefaultHandler2`
(naked, reads PC from exception frame at SP+24, renders `RECHORD FAULT /
VEC= / PC= / CFSR=` into the UI framebuffer, loops forever) and stubs.c got
real intrinsics (`cpsid i / cpsie i / cpsid f / cpsie f / wfi`).

**Rule for regenerating stubs.c:** classify undefined symbols by how the SDK
*uses* them — called with `()` → function; only address-taken → variable.
A quick heuristic: name ends in Set/Get/Init/Start/Stop/Enable/Disable/Read/
Write/Enter/Exit/Register → function. Then check the header decl for `(...)`.

> ⚠️ **THE SAME BUG HIT AGAIN (V0.2–V0.5, root-caused in the "stub-fix"
> pass):** the mailbox/SysTick/codec function stubs were STILL zeroed
> variables, so the boot path crashed silently:
> `BSP_Init2()` → `MailBoxEnableA2BInt()` (variable @ bss) → CPU executes
> zeros → hard fault before Main2's loop. That's why no heartbeat ever
> showed and the menu freeze never changed. **Fixed in commit `stub-fix`:**
> 57 symbols converted to `__attribute__((weak))` FUNCTIONS (MailBox*A2B/
> B2A, SysTick*2, Codec*2, bb_printf1, TaskID2Str, FindFirst/NextFile,
> GetCurFileNum, GetGlobeFileNum, GetLongFileName, GetTotalFiles,
> BuildDirInfo, PowerOff, GetAdcData, ScuClockGateCtr, SetPllDefault,
> System_Power_On, MDDeInitAll, HostGetChannelInfo, Gpio_SetPortDirec,
> Grf_*, SFlashGetBluetoothMac, CheckAdcState, AdcPowerDown,
> BluetoothReConnectResult, Codec_*, MusicWinPaint, bt_a2dp_connect,
> rk_print_string2). **Audit method that works:** scan every SDK .c for
> `name(` (→ function) vs `&name`/data use (→ variable). `SetPowerOffFlag`
> is a real UINT8 VARIABLE despite the `Set` prefix — always verify call
> sites, never trust the name heuristic alone.
>
> ⚠️ Stub-return-0 is NOT a fix for the freeze itself — the AP still waits
> for real mailbox replies. V0.6 (stub-fix) is a *diagnostic*: if the
> fullscreen red/black heartbeat shows, Main2 reaches its loop and the
> mailbox ROM-API register addresses are the only remaining blocker.

`IntDefaultHandler2` is `interrupt("IRQ") naked` and branches to
`__CPU_IntDefaultHandler2`; our replacement is plain naked (the branch target
must not return).

---

## 6. SDK compile facts (what works / what's excluded)

Compile (18 include paths, see Makefile `SDK_CFLAGS`):

```
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -ffunction-sections -fdata-sections
  -include firmware/rockchip/include/armcc_compat.h ...
```

- **Duplicate *2 variants:** the SDK ships both `audio_file_access.c` and
  `audio_file_access2.c` etc. (11 pairs: Delay, SysTickHandler,
  audio_file_access, pAAC, pDSDIFF, pDSF, pMP3, pOGG, p_hifi_Ape,
  p_hifi_alac, p_hifi_flac + `interrupt.c`). Only the `*2` (RKnano2) versions
  link. Excluded in Makefile `NON2_EXCLUDE`.
- **Project-layer files excluded** (need FiiO defines not in SDK):
  `systick2.c`, `pCODECS2.c`, `RecordControl.c`, `PowerManager.c`,
  `AsicToUnicode.c`, `cue.c`, `ID3.c`, `AsicToUnicodeTable.c`. Their symbols
  are covered by stubs. (`AsicToUnicode` also needs `FONT_CODEPAGE_*` which we
  added to `armcc_compat.h`.)
- Newlib syscalls needed for `memcpy/memset/printf`: `_sbrk _write _read
  _close _lseek _fstat _isatty _getpid _kill _exit` (in stubs.c).
- Link with `arm-none-eabi-gcc` (not bare ld) so newlib resolves.
- `IntMasterDisable/IntMasterEnable` (non-*2 names) are real `cpsid/cpsie` in
  stubs.c.

---

## 7. The current mystery: why no fault screen? (NEXT AGENT, START HERE)

**INCIDENT (2026-08-25, `ReChord_APBB.IMG` @ 00:35): HARD BRICK — do not repeat.**
Flashing a custom fw1 (AP) bricked the device (no boot, no USB-storage
fallback; recovered via maskrom). Root cause: the fw1 RKnanoFW header's
memory-map table is the **whole-system RAM layout** — the stock 91 entries
cover the UI framebuffer load from flash (0x03024868), audio buffers
(0x03005AFC), FAT cache (0x0301E778), stacks and the code copies — which the
Mask ROM sets up at boot. `pack_fw1.py` emits only 3 entries from our flat
ELF (data copy / bss zero / text copy), destroying that layout, AND copies
0x194E4 bytes of AP `.data` over 0x03000000. Until the stock table format is
fully reverse-engineered and reproduced (all 91 entries), a custom fw1 is
unflashable. Fixes landed: `make release` now = custom BB + STOCK AP only
(`ReChord_BB.IMG`, hardware-proven); the AP+BB pack is gated behind
`make apbb-experimental` with loud warnings in the Makefile, pack_fw1.py and
pack_img.py --pack-full.


**UPDATE (2026-08, mailbox session): see `docs/re/AP-MAILBOX-GUIDE.md`** —
verified mailbox map (base 0x40110000, struct 0x50, IRQ 9–12, real command
IDs), the three causes of "blank top text + freeze" (ROM text-draw callback
@0x0300710a lives in section_3; zero-padded section_3 tail corrupts fw1
scatter data — use `pack_img.py --keep-stock-tail`; V0.20 registers ISRs but
never runs the Main2 reply loop → AP blocks 20 s on DEC_OPEN), and a
minimal ACK-everything dummy BB: `firmware/bb_stub/bb_stub.c`
(`make bb-stub` / `make pack-bb-stub-img`). Also fixed: the old
`include/mailbox.h` shim had wrong command IDs (real ones are sequential:
BB_HOLD=2/ACK=3/EXIT=4, FILE_* from 100).

**UPDATE (round 12): the AP<->BB handshake mechanism is now CONFIRMED from the
SDK** (`driver/BB/BBSystem.c`). The AP side (`StartBBSystem`) does:
`ScuSoftResetCtr(CAL_CORE_SRST, TRUE)` → `ModuleOverlay(...)` →
`ScuSoftResetCtr(CAL_CORE_SRST, FALSE)` (releases the CAL_M3/core1) → then
`while(!BbSystemStartOK)` with a ~200 ms timeout. core1's reset vector IS
`firmware_entry` (0x03000010). The BB must reply
`MSGBOX_CMD_SYSTEM_START_OK` (0x0001) on B2A channel 0; the AP's
`BBSystemAIsr` sets `BbSystemStartOK=1`. Our Main2.c already sends that
reply correctly. So the ~23 s freeze is NOT the boot handshake (that only
waits 200 ms) — it is the AP codec's `while(!gOpenDone)` wait
(20,000,000 × DelayUs(1) ≈ 20 s, in PCMFunction/CodecOpen) after it sends
DEC_OPEN. The AP's decode ISR (AudioControl.c ~2786) sets gOpenDone=1 on
BOTH DEC_OPEN_ERR and DEC_OPEN_CMPL, so ANY reply breaks the wait; our
CodecOpen2 stub returning 0 → DEC_OPEN_ERR is therefore a VALID answer. The
freeze only happens when the BB's decode/file ISRs
(MailBoxDecService/MailBoxFileService) never fire — because the **vector
table was misaligned** (fixed: `exceptions_table2` is now 256-aligned at
0x03021600).

**UPDATE (V0.15, fresh-instance session): ROOT CAUSE FOUND via Ghidra** —
the stock `firmware_entry` @ 0x03000010 is an **HW-init callback that RETURNS**
to the ROM. Stock disasm: `push {r4,lr}; mov r4,r0; bl boot_param_layout;
rom_alloc(0x1dc); [rom_hw_init(0x1dc/0x16f/0x16f) rom_hw_init2(0x171)
rom_hw_init(0x170)]; rom_early_init(); ldrh r0,[r4]; cmp r0,#0xb;
pop {r4,lr}; movw r0,#0x18f/0x191; b.w rom_hw_init2` — the final
`b.w rom_hw_init2` is a TAIL CALL whose return goes back to the ROM.

Our V0.1-V0.14 entries did `b rechord_app` -> infinite loop -> **the ROM
boot sequence never continued**: cassette stays (loader boot screen), no
text, no app, and a ~23s poweroff (ROM-side timeout, NOT our fault handler
looping — that V0.13 inference was wrong). The whole V0.8-V0.14 display
test series was invalidated by the hung boot; that's why NOTHING custom
was ever visible even though code likely ran.

V0.15 (`build/ReChord_V0.15_bootfix.IMG`) mirrors stock exactly:
`rechord_firmware_entry()` (rechord_app.c) runs the stock init and returns
0x18f/0x191; entry_stubs.S does `pop {r4,lr}; b.w 0x02feee7c` (tail call).
The draw test moved to the `application_start` stub (0x0300710a). Also
learned: the loader draws via `fef124(0x19b) wait -> fea848(1)/fea824(2)
save -> feb0f6(color) -> fea8f4(x,y,w,h,a5,a6) -> feabea(1) refresh ->
restore`. **fea8f4 takes SIX args** (r0=x,r1=y,r2=w,r3=h,sp[0]=2,sp[4]=var)
— the V0.10 packed-2-arg and V0.14 4-arg calls were wrong. Known-good
rects: top bar (0,3,320,16,2,0x58), bottom (0,120,320,15,2,0x6c).

Legacy theories below (mailbox deadlock etc.) were never proven; the boot
hang explains everything observed.

Symptom: **every** menu item press → screen freezes → device powers off.
V0.2 added a real hard-fault handler that should show `PC=0x…` on the
framebuffer, but the user reports the freeze still happens and **no fault
screen appears**.

Leading theories (in order):

1. **Mailbox deadlock (most likely).** The visible UI is the AP side (stock).
   Pressing a menu item makes the AP send a `MEDIA_MSGBOX_CMD_*` to our BB
   (`Main2`). Our mailbox handlers (`MailBoxDecService`, `MailBoxFileService`)
   or their callees are stubs → no reply or a corrupt reply → AP waits forever
   → watchdog powers off. **No CPU fault occurs**, so no fault screen. This
   fits "freeze then power off" exactly (a fault would be near-instant, not a
   freeze).
   - **Test:** stub `MailBoxWriteB2ACmd` to always return OK / or make
     `MailBoxDecService`/`MailBoxFileService` reply with a canned
     `MEDIA_MSGBOX_CMD_*_CMPL` so the AP stops waiting. If the UI stops
     freezing, theory confirmed.
2. **Hard fault but handler not reached.** If the fault happens on the AP side
   (stock code, separate vector table) or before our vector table is
   installed, our framebuffer render won't run. Also: if the fault is in the
   AP, VTOR points elsewhere.
3. **Watchdog kills before render.** If the fault handler runs but the watchdog
   (or PMU) resets the chip faster than we can DMA a frame, the screen never
   updates. Our handler disables IRQ but not the watchdog. Consider disabling
   WDT in the handler (find WDT regs in `driverlib_def.h` — PMU 0x400E0000).
4. **Our firmware isn't the one drawing.** Confirm which side is actually
   running our section_3 (see §8 instrumentation ideas).

### Concrete next experiments (pick cheapest first)

- **A. Visual BB heartbeat:** in `Main2()` loop, periodically flip a color in
  the UI framebuffer (e.g. a 10×10 px square at 0,0 every N ms, with a
  `DelayMs`-ish loop). If the user sees the square blink, our BB is running and
  drawing. If not, our code isn't the one driving the screen.
- **B. Canned mailbox replies:** make `MailBoxWriteB2ACmd/Data` no-ops that
  return 0 AND make `MailBoxDecService`/`MailBoxFileService` immediately send
  a `*_CMPL` back. If menus stop freezing → deadlock confirmed.
- **C. UART debug:** PB5/TXD + PB6/RXD (per HARDWARE.md). Implement a tiny
  `debug_puts` on UART0 (regs in `driverlib_def.h`) and log BB boot +
  mailbox traffic. Needs a USB-UART adapter (3.3V).
- **D. Distinguish fault vs hang:** have the fault handler ALSO flash the
  backlight (via Backlight.c hooks) in a pattern. If backlight flashes but no
  screen → handler runs, render path issue. If nothing at all → not our
  handler / not a fault.

---

## 8. Repo layout (after the big cleanup)

```
firmware/
├── rockchip/            # RKnanoD SDK — THE source we compile
│   ├── audio/           #   AudioControl, RkEQ/Effect (DSP target), codecs
│   ├── bbsystem/        #   Main2 (BB entry), BSP2, cru2, dma2, interrupt2,
│   │                    #   Delay2, Debug2, SysTickHandler2
│   ├── system/          #   os (Task/Thread/Msg/Win), fileseek,
│   │                    #   module_overlay, sysservice (battery, backlight…)
│   └── include/         #   armcc_compat.h + integration headers
├── startup/startup.c    # RKnanoFW header
├── rechord_app.c        # rechord_hw_init (ROM init) + rechord_main (UI loop)
├── entry_stubs.S        # firmware_entry → rechord_hw_init → rechord_main
├── fault.c              # real hard-fault handler + framebuffer renderer
├── stubs.c              # link stubs: globals, weak funcs, ARM intrinsics,
│                        #   newlib syscalls
├── firmware.ld          # linker script
├── resource/            # resource_api.h
└── INDEX.md
docs/
├── STATUS.md            # build status (link works; stubs = kernel-only boot)
├── HARDWARE.md          # memory map, registers, segment table, ROM APIs
├── FLASHING.md          # flash + recovery procedure
├── HANDOVER.md          # ← this file
├── RE-HISTORY.md        # decompilation → SDK pivot story
└── re/                  # archived RE docs + decompiled spec (not compiled)
    └── decomp/          #   apps/codecs/drivers/… FiiO-layer spec (410 files)
tools/
├── pack_img.py          # splice section_3 into stock IMG (identity-tested)
├── compile_check.py     # now scans only firmware/rockchip
└── … (RE-era scripts archived in git history)
```

Deleted (recoverable from git): `firmware/unidentified/` (2597 unnamed Ghidra
exports), `firmware/rockchip_wireless/` (259 MB — wrong SDK, Wireless V1.5).

---

## 8b. QEMU GUI dev harness (use this before flashing!)

Iterate the ReChord GUI in QEMU instead of flash-testing every build.

**Why:** QEMU 11 mps2-an385/an511 have the CLCD/LCD as "unimplemented"
(no GUI); vexpress-a9 PL111 is real but bare-metal boot is painful;
the lm3s6965evb OLED (ssd0323) is wired via a pl022 whose SPI master
mode QEMU does not implement. **The working path is framebuffer-in-RAM:**

1. Harness (`firmware/qemu/qemu_gui_menu.c`) draws into RAM @ 0x20000000
   (128x64 RGB565) on lm3s6965evb.
2. Run in QEMU with a TCP monitor.
3. `tools/render_qemu_fb.py` reads the RAM via monitor `x` and renders PNG.

```bash
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -nostdlib   -T firmware/qemu/qemu_ld_test.ld firmware/qemu/qemu_gui_menu.c   -o build/qemu_gui_menu.elf
arm-none-eabi-objcopy -O binary build/qemu_gui_menu.elf build/qemu_gui_menu.bin
python tools/render_qemu_fb.py --bin build/qemu_gui_menu.bin --out build/gui.png
```

**Key gotchas:**
- lm3s6965evb RAM is 64KB @ 0x20000000-0x2000FFFF — SP must be inside
  (use 0x2000FC00), not 0x20010000 (overflows -> code never runs).
- `-kernel` loads the vector table at 0x00000000 (flash). QEMU boots M3
  from the vector table: [0]=SP, [1]=reset, rest=handlers.
- The monitor `x` output is "ADDR: 0xVAL ..." — parse only after ':'.
- Font tables as C arrays get optimized into memcpy — provide a local
  memcpy or compile with -fno-builtin.
- Windows newline trap: writing `
` inside a C string via python
  heredoc produces a literal LF and breaks the string. Fix with byte
  replace (b"\n").

**Harness files** (`firmware/qemu/`): qemu_gui_fb.c (color boxes, works),
qemu_gui_menu.c (menu w/ title bar + cursor + status bar, works),
qemu_ui_lm3s.c (pl022/OLED attempt — blocked by QEMU model),
qemu_test1.c/qemu_probe.c (mps2 probes), qemu_ld_test.ld.

## 9. Flash safety rules (device is the user's ONLY one)

1. **Official method only:** copy IMG to internal storage root, remove TF,
   reboot. Bootloader A/B (fw1/fw2) + stock backup = reversible.
2. Keep `build/stock_restore_HIFIEC37.IMG` around; to restore, copy it back.
3. Bootloader has **no crypto** — "sign error" strings are structural checks.
4. First code flash already done (V0.1, V0.2) and the device recovered fine.
5. Never flash "for science" without a way back; test on the bench flow first.

---

## 10. The DSP goal (end game)

`firmware/rockchip/audio/RkEQ/Effect/Effect.c` compiles clean:

```c
long EffectInit(void);
long EffectProcess(EQ_TYPE *pBuffer, long PcmLen);  // per-frame hook
long EffectAdjust(void);
long RKEQAdjust(RKEffect *pEft);
```

`RKEffect` = 5-band `dbGain[5]`; presets EQ_HEAVY/POP/JAZZ/CLASS/BASS/ROCK/
USER/NOR. Path to a *useful* custom firmware: get the mailbox handshake
working (so the stock UI can play audio through our BB), then modify
`EffectProcess()` and flash.

---

## 11. Gotchas / traps (read before changing anything)

- **pack_img.py does not write the RKnanoFW magic** — startup.c must emit the
  byte-exact header (§3).
- **`__CPU_IntDefaultHandler2` and friends must be FUNCTIONS, not variables**
  (§5). Regenerate stubs.c with the call-vs-address classification.
- **`_FRAME_BUFFER_` is not defined** → `Lcd_BuferTranfer()` compiles to
  nothing; the LCD is refreshed by hardware/DMA from 0x03024868.
- **`EXT` macro** gets redefined by PowerManager.h/Service.h — include those
  headers directly in their .c, not via SysInclude.
- **`eFREQ_APP`** shared enum lives in `freq_enums.h`; mailbox ENCODE/REC
  macros in `main2_msgbox.h` (separate from RecordControl.h enum — they clash).
- **`RKFIO_FOpen`** is old-style `()` to satisfy both .c variants.
- **Windows:** files use CRLF; `$(cat file)` in bash needs `tr -d '\r'`.
- **Ghidra:** project `FIIO-3.7.0-Decomp` — 852/2776 named (30.7%); 327 named
  decompiled files compile. The `docs/re/decomp/` tree is the FiiO-layer spec.


---

# SESSION 2026-10-10 (night) - Pivot to full clean-room CFW; agents 1a/1b/1c + scatter fix

START HERE NEXT SESSION. Everything below is verified this session unless
marked otherwise.

## 0. Decisions made

- Strategy PIVOT (user decision, recorded in docs/STATUS.md): stop extending
  the stock-AP hybrid (black-box mailbox protocol, no log drain, no UI). Go
  to the reference-CFW architecture: our own fw1 (AP+UI) + our BB as a
  core-B module + our own protocol. Milestone 1: fw1 that boots to OUR
  screen.
- The stock-AP BB build (build/ReChord_BB.IMG) stays as the SAFE FALLBACK
  image. It boots, navigation works, but entering a menu item still freezes
  (AP<->BB file/decode handshake). Do not spend more time on that freeze.

## 1. HARD-BRICK CORRECTION (read before touching fw1)

HANDOVER section 7 says a custom fw1 bricked a device on 2026-08-25 and
concluded "all 91 scatter entries must be reproduced". NEW EVIDENCE
(byte-level extraction from the reference image community/refcfw/HIFIEC90.IMG)
corrects the root cause:

- Reference fw1 header @ IMG 0x1F8: w0=0x616E4B52 w1=0x57466F6E (magic
  "RKnanoFW"), w2=0x03050000 (load base / initial SP / vector-table base),
  w3=0x0000005B (91 = table capacity field, NOT used row count).
- Reference scatter table @ IMG 0x208, THREE rows then terminator:
  [0] dest=0x03050B68 size=0x2666C dest_end=0x030771D4 src=0x03050000 (code)
  [1] dest=0x03004000 size=0x1AFC  dest_end=0x03005AFC src=0x18C50 (.data)
  [2] 0x03078CD0, 0x4, 0x03079DCC, 0x0 (zero-fill-ish; shape to confirm)
  [3] zeros = end
  Row decode so far: w0=dest, w1=size, w2=dest+size, w3=src (row 0 src is a
  RAM/XIP base, row 1 src looks IMG-relative - UNCLEAR, was being decoded
  against the stock table by agent bg-12).
- Reference memory roles (from its startup at RAM 0x030500E4, literals
  0x03050178-0x03050194): .data copy LMA 0x03074B70 -> 0x03004000..0x03005AFC
  (6908 B); .bss zero 0x03005AFC..0x03039128 (205,868 B); VTOR=0x03050000;
  dsb/isb; bl main (0x03050B08). NOTHING of its own below 0x03004000.

=> The 2026-08-25 brick was caused by the flat-ELF pack placing .data over
0x03000000 (ROM-owned AP fast mem / workspace) plus rows that did not
describe the real memory roles - NOT by having fewer than 91 rows (the
reference boots with 3). RULE: never let a scatter row target
0x03000000..0x03004000.

## 2. Agent deliverables landed this session (all committed? NO - uncommitted!)

- bg-9 (link): firmware/firmware_ap.ld (fw1 layout + ASSERTs), firmware/ap.mk
  (AP sources/flags), firmware/startup/ap_main.c (Main() test screen + weak
  stubs). AP links clean: 0 undefined, text 20,912 B, bss 120,572 B.
  NOTE: its first link put data/bss at 0x03000000 (brick pattern) - being
  corrected by bg-12.
- bg-10 (panel): firmware/drivers/panel_init_rknanoc.c (byte-exact DCS init,
  2 variants selected by panel ID probe GPIO0 bits 16..23: id 0x00E80000 ->
  variant A, else B; default PANEL_INIT_VARIANT=1 (B)). Found+fixed 3 real
  bugs: PANEL_ROW_OFFSET 35 (visible band rows 35..204), target transport is
  a GPIO0 bit-banged 8080 bus (GPIO0_BASE 0x40160000, byte in DR bits 16..23,
  D/C pin 25, WR pin 24, reset GPIO2.15) - NOT the SDK VOP block, and window
  mapping verified. tests: 69+204+358 checks green. docs/rewrite/drivers.md
  rewritten with the full sequence tables.
- bg-11 (packing): tools/make_fw1.py (ELF -> fw1 -> IMG + trailer CRC, self
  verifying; refuses to write ReChord_BB.IMG). Format limits: max 91 scatter
  rows (table 0x208..0x7B8), fw1 payload max 356,456 B (0x7B8..0x57820), XIP
  window 0x030007B8..0x03057820 (flash = 0x03000000 + IMG offset), IMG fixed
  33,554,436 B. Trailer CRC: poly 0x04C10DB7 (bit 0x1000 missing), non-
  reflected, init 0, xorout 0 over IMG[:-4], LE. docs/fw1-packing.md section 8.
- bg-12 (scatter fix, RUNNING at handoff time): reworking firmware_ap.ld to
  the reference memory roles (vectors @0x03050000, code after, .data VMA
  0x03004000 with LMA copy, .bss from 0x03005AFC), adding the bounded .data
  copy to ap_startup.c (copy bounded / zero bounded / VTOR / dsb/isb / Main),
  and making make_fw1.py emit reference-shaped scatter rows after decoding
  the row semantics against the STOCK table. Its final report is appended
  below when it lands.

## 3. Where things stand

- build/ReChord_AP.IMG (23:47) = the BRICK-PATTERN image (3 flat rows, .data
  over 0x03000000). DO NOT FLASH. bg-12 must re-emit it.
- Device state: running the instrumented hybrid BB build (trailer 0x7B3F34E3,
  with bb_trace breadcrumbs). Last test: menu navigation works, entering an
  item freezes; BBTRACE.TXT never appeared (the BB-side FileOpenA breadcrumb
  is itself unsafe/silent under the stock AP - expected, do not chase).
- Recovery tooling ready if ever needed: docs/DEBRICK-LINUX.md (maskrom +
  rkdeveloptool, USB ID 2207:262d, sequence ld -> td -> wl -> rd, NEVER db;
  verified 2026-08-24).

## 4. Next session TODO (in order)

1. Collect bg-12 report (agent id bg-12-ac84791c) and verify its output:
   side-by-side header+rows vs the reference table, safety asserts (no row
   below 0x03004000, vectors at 0x03050000 256-aligned), CRC recomputed.
2. Decide the flash test path with the user (their call): direct flash of the
   fixed ReChord_AP.IMG vs UART validation first (fw1valid/fw2valid prints
   over hardware UART are the authoritative check per docs/fw1-packing.md).
   Debrick procedure ready as the safety net.
3. Milestone 1 acceptance: device boots to OUR screen (ap_main test screen).
4. Then: real UI wiring (boot_screen -> ui_main loop), replace weak stubs
   (settings_view_load/save, theme file loader fopen/fgets/fgetc/feof/fclose),
   add strchr/strrchr to firmware/libc/str.c + host test (libc policy).
5. Small fix: firmware/ui/Makefile ui_tests link line misses
   ../theme/theme.c ../theme/themes_builtin.c ../theme/theme_file.c (pre-
   existing harness gap found by bg-9; host test otherwise passes: 250/250).
6. Milestone 2: our BB (the SDK-built one) as the core-B module to HIGHRAM0
   + our AP<->B protocol (firmware/player/ipc_b.h).

## 5. Tree state: UNCOMMITTED work (commit in the morning)

Everything from this session is uncommitted: freeze fixes to the BB build
(firmware.ld ZI bounds = real .bss span; _ATTR_VECTTAB_BB_ restored to
section bb_vect aligned(256) and the local empty define in
bbsystem/interrupt2.c removed - exceptions_table2 now at a 256-aligned
address; bb_trace breadcrumbs), the4 agent output sets, and doc updates.
Working tree also has the earlier-verified build system state. User wanted
progress preserved; commit+push when they say so (their rule: no push
without explicit authorization).


---

# RESUME 2026-10-11 (PC powering off mid-session - read this to continue)

## State at shutdown

- Device: was hard-bricked by a custom fw1 flash (SECOND brick incident). The
  user is recovering it via maskrom + rkdeveloptool (docs/DEBRICK-LINUX.md).
  Last seen: 2207:262d in maskrom; `td` failed once ("creating comm object
  failed" = wedged USB session -> re-enter maskrom, retry the chained block).
  Stock restore target: stock/3.7.0/ECHO MINI V3.7.0/HIFIEC37.IMG.
- build/ReChord_AP.IMG (10:47) = the image that BRICKED (row src semantics
  were wrong). DO NOT FLASH IT. It will be rebuilt by the fix below.
- Agent bg-1-eae22aaa (boot-model audit + fw1 fix) was RUNNING at shutdown;
  its in-memory audit is lost unless it wrote files. RELAUNCH with the same
  brief if needed.

## The boot model to verify (derived 2026-10-11 morning, byte evidence)

Claims (agent bg-1 was auditing exactly these):
1. The loader loads the fw1 IMG body contiguously into RAM:
   RAM = 0x0304F298 + IMG_offset (IMG 0x1F8 -> RAM 0x0304F490).
   Evidence: reference .data LMA literal 0x03074B70 -> IMG 0x258D8 (inside
   the image) under this model; under the XIP model (IMG N <-> 0x03000000+N)
   it lands outside the payload window (impossible).
2. Scatter rows copy FROM addresses in that loaded RAM image (not XIP), and
   zero-fill regions. (Row shape stays: even = Type B {dest,src,size,dest_end},
   odd = Type A {ram_start,size,ram_end,total_obj}.)
3. Entry = vector table at RAM 0x03050000 = IMG 0xD68: word0 = initial SP
   0x03004000, word1 = Reset 0x0306296F (verified bytes: 0x03004000,
   0x0306296F, 0x03050155, 0x03050155).
4. RAM 0x03000000..0x03004000 = the AP MAIN STACK (SP=0x03004000 growing
   down). Never place data there (that was brick cause B).
5. Brick cause A (this build): row B src=0x030007B8 (XIP semantics) = garbage
   source under the real model -> garbage copied over the vector table.
6. Reference payload: vector table at IMG 0xD68 = 0x5B0 bytes into the
   payload (window starts 0x7B8) = the "pointer table" prefix in
   docs/fw1-packing.md.

NOTE: docs/fw1-packing.md section 3 currently documents the XIP model
("entry 88 src 0x0308A5A4 -> IMG 0x8A5A4"). The audit must explain why that
observation exists (likely section_3 stage vs fw1 stage mapping) and correct
the doc.

## The fix (implement after verification)

1. Payload layout: place our vector table at IMG 0xD68 (0x5B0-byte prefix,
   padding acceptable).
2. Scatter row src = 0x0304F298 + IMG offset of the source bytes.
3. word0/SP = 0x03004000; stack zone 0x03000000..0x03004000 unused by
   data/bss. (firmware_ap.ld currently has __stack_top = 0x03050000 - fix.)
4. Keep the pack_fw1.py safety gate (no row below 0x03004000).
5. Rebuild: make cfw-ap && make cfw-img; verify side-by-side vs the
   reference (header words, rows, vector bytes at IMG 0xD68).

## Also pending (from the milestone-1 work, all uncommitted)

- All milestone-1 artifacts uncommitted (Makefile cfw-* targets, firmware_ap.ld,
  ap.mk, ap_main.c, ap_startup.c, pack_fw1.py safety gate + parity checks in
  make_fw1.py, docs). Commit+push when the user says so.
- The reference-image facts (header/rows/memory roles) are recorded in the
  session section above ("SESSION 2026-10-10 (night)").

---

# PHASE A AUDIT RESULT (2026-10-11, independent byte audit - PERSISTED)

Auditor re-derived every claim from the binaries (reference IMG, stock 2.5.0
and 3.7.0 IMGs, the bricked build/ReChord_AP.fw1). Verdicts:

1. **CONTIGUOUS RAM LOAD - CONFIRMED.** The fw1 body is loaded contiguously
   with RAM = 0x0304F298 + IMG offset (IMG 0x1F8 -> 0x0304F490). Proven by:
   (a) the vector table sits at IMG 0xD68 and enters at RAM 0x03050000 (the
   header base); (b) the startup at IMG 0xE4C disassembles coherently at RAM
   0x030500E4 (literal pool: .data LMA 0x03074B70 -> IMG 0x258D8 holds the
   .data init words 0x03004000/0x030500E5/0x03050155..., including a pointer
   to the startup itself = 0x030500E5); (c) under the XIP model the VT would
   sit at IMG 0x50000 = all zeros and the .data source at IMG 0x74B70 = font
   data - both impossible. Generalization (from stock): the load base is
   "0x03050000 minus the vector table's IMG offset" - stock's VT is at IMG
   0x297CC (base 0x03026834) and its default handler resolves there as a
   `b .` loop; for any image with the VT at IMG 0xD68 (ours, per the fix)
   the constant 0x0304F298 applies exactly.
2. **ROW SRC = LOADED RAM IMAGE (not XIP) - CONFIRMED; zero-fill UNCLEAR.**
   Row 0 src 0x03050000 = the loaded VT address; rows 2/88/90 srcs map inside
   the loaded body. Stock row 2 src 0x03079CC8 resolves to real Thumb code at
   IMG 0x2AA30 under this model and to ALL ZEROS at IMG 0x79CC8 under XIP.
   Nuance: row 0 {dest 0x03050B68, src 0x03050000, size 0x2666C} cannot be a
   destructive pre-entry copy - post-copy bytes at the reset address
   0x0306296E would be string data (blob+0x11E06), and main (0x03050B08)
   straddles the dest boundary. Rows behave as an object/module registry
   (they chain into the payload prefix records); Type A rows are region
   descriptors (row 1 == exactly the .data window the startup copies into).
   Our startup zeroes .bss itself, so the loader zero-fill question is moot.
3. **ENTRY = VT AT IMG 0xD68 - CONFIRMED BYTE-EXACT.** IMG 0xD68 =
   0x03004000, 0x0306296F, 0x03050155, 0x03050155. 0x0306296E is a veneer
   `b.w 0x030500e4` to the startup; 0x03050154 is `mrs r0, IPSR; b.w ...`.
4. **0x03000000..0x03004000 = AP MAIN STACK - CONFIRMED.** VT word0 = SP =
   0x03004000; lowest data row in reference AND stock = 0x03004000 (stock
   3.7.0 row 1 is byte-identical to the reference row 1); the reference's
   startup writes nothing below 0x03004000. (2026-08-25 brick cause B:
   .data copied over 0x03000000 - documented earlier.)
5. **BRICK CAUSE A - CONFIRMED (facts), mechanism note.** The bricked
   build/ReChord_AP.fw1 row 0 = {dest 0x03050000, src 0x030007B8, 0x51D0,
   0x030551D0}: src is the old XIP formula, which under the real model points
   BELOW the load window (unmapped garbage). SECOND, independent defect: the
   build had no 0x5B0 prefix, so its VT sat at RAM 0x0304FA50 and RAM
   0x03050000 held code bytes = garbage vector table at entry. Either defect
   bricks; the fix (prefix + RAM src formula) removes both.
6. **0x5B0 PREFIX - CONFIRMED.** 0xD68-0x7B8 = 0x5B0 bytes of 16-byte
   allocation/module records (chain 0x03078CD0 -> 0x03078CD4 -> 0x03079878
   -> 0x0307A160 -> ... continues into table rows 2/88/90).

STOCK CROSS-CHECK: both stock tables are self-consistent ONLY under the
contiguous model - all 37 Type B srcs land inside the fw1 payload window
(fw1-payload x37); under XIP they would source from the sec2 region (29) and
section_3 (8). The old "entry 88 src 0x0308A5A4 -> IMG 0x8A5A4 = section_3"
XIP reading is MISLEADING twice over: stock row 88 (and rows 70/90) have
SIZE 0 - no byte is ever copied from that src under any model - and the XIP
arithmetic (IMG N <-> 0x03000000+N) is the SECTION_3/BB stage's flat-load
mapping (a different stage), not fw1's. The --keep-stock-tail rule stays in
force (R-1f1c5d829375) on empirical grounds (2026-10-10 hardware note), but
its old "entry 88 XIP" mechanism is wrong: candidates are ROM/sec2 region
validation ("fw2 compare error!") or the running AP's module loader.

PHASE B plan (implementing now): payload = 0x5B0-byte zero prefix + blob
(VT at IMG 0xD68, code, .data LMA image); rows mirror the reference's shapes
[0] Type B {dest 0x03050B68, src 0x03050000, blob_len, end}, [1] Type A
{0x03004000, .data span, +end, total obj}, [2] Type B {dest 0x03004000,
src 0x0304F298+IMG(.data LMA), .data span, end}; count field = 0x5B (table
capacity, as both stock and reference); __stack_top -> 0x03004000; .data
gets a real LMA in the code image (AT> FW_CODE); safety gate kept.


## RECOVERED BOOT LOG (2026-10-11) - the loader speaks (FSCK0003.REC)

fsck.vfat recovered deleted FAT chains from the user volume as FSCK0003.REC
(1,646,592 B, preserved in the session scratchpad) = the reference CFW's own
boot log. Decisive lines:

    1.015 idb copy 0: P 22000 R 11000 user_base 88000 (stock's 24000)
    1.015 sector 24000: 45564153 08242026 .. p0 00000000 end 00000000; idb +16/+20 04060001 +4/+6 00000000
    1.016 fw1 1 fw2 1
    1.016 boot: resources @0 (legacy) are <refcfw> 7c3d4cf v0.9.2 2026-10-05
    1.017 mount: 0 type FAT32 part 64 spc 8 clusters 1835506 fat@32 x2 root_clus 2 data@28720
    1.321 update: no image

Implications:
- The boot reads an IDB/"SAVE" structure (magic 45 56 41 53) at raw eMMC
  sectors (copies reported at P 0x22000 / R 0x11000) that carries PART BOUNDS
  ("p0 ... end") plus fields at +16/+20 and +4/+6. This is NOT in the IMG.
- The reference REWROTE that area (user_base 0x88000 sectors vs stock
  0x24000) = a repartitioning; the boot reports it as "(stock's 24000)".
- The boot also VALIDATES the resources region identity ("resources @0
  (legacy) are ..."): the reference replaced the stock "ROCK26IMAGERES"
  resources with its own branded ones. Our images keep the stock resources.
- The stock IMG contains one "SAVE" magic at 0x570E4 (fw1 payload, near its
  end); the reference contains none in its 5MB image.

Hypothesis for the 7 bricks (now with the loader's own words): the loader
maps/validates fw1 using the IDB part bounds + the outer header values
(IMG 0x0..0x1F8 is the same structure family; 0x1F4 = marker base, 0x14C =
sec3 end were identified). Our images carry the STALE STOCK bounds while the
containers differ -> the loader reads the wrong extents -> halt.

NEXT TEST (one cycle, max info): flash the pending build; if it bricks, enter
maskrom and read BOTH regions:
    rl 0x10000 0x3000 idb_dump.bin     # IDB/SAVE area (copies at 0x11000/0x22000)
    rl 0 65536 store_dump.bin          # whole firmware store
Then decode the IDB fields using the log's own names (p0/end, +16/+20, +4/+6)
and patch them for our image (plus the outer header 0x0..0x1F8).


## SESSION CLOSE 2026-10-11 (night) - day-2 outcomes, state for tomorrow

DECISIVE RESULT OF THE DAY: the update-image validator is DECOMPILED (REA on
the reference image; the update orchestrator FUN_03060774 -> validator
FUN_03064474). Checks decoded: -2 magic 'RKnanoFW' at IMG 0x1F8; -3 memcmp
(first 0x200 bytes vs bytes at fw_end, fw_end = LE32(IMG[0x1F4])); -5 trailer
CRC (our algorithm is correct; it must sit at (fw_end + 0x104000) & ~0xFFFFF);
-4 size bounds; -1 read errors. THE HEADER SELF-COPY AT fw_end was missing
from EVERY image we ever built (and from our patched clones) = the "result -3"
seen in the recovered reference boot log (FSCK0003.REC). Implemented in
tools/make_fw1.py for both the 33MB and 5MB builders with a byte-check.

Hardware verdict #12 (the first FAIR test: our fw1 + all decoded checks
satisfied, md5 5cf037931c18a9d1b87887a0062b1cdf): device BRICKED. Remaining
explanations: (a) another branch of the updater's write/A-B phase, (b) our
fw1 content crashes before visible output (structure now validated; the CODE
side is untested), (c) an undecoded check.

State at shutdown:
- Device is BRICKED (verdict #12). Debrick via maskrom (docs/DEBRICK-LINUX.md).
- DFIU: incident 5667c9c4 (the brick line, 12 verdicts) + incident 3a318ade
  (the validator rules, rule R-9f6e141101cb) + R-70b34b8f2528 hypothesis
  (boot model) + R-980571f4f9cc (sec2 neutralization). None activated yet -
  nothing activates until a hardware-verified boot.
- The reference CFW's updater runs on the device whenever the reference fw1
  boots - its ECHOLOG.TXT is the oracle (it logs 'update: result %d').
- Test asset built: build/ReChord_REFVERBATIM.IMG (byte-exact reference,
  boots - md5 4c333e193739512bb4bb706495523955) = the golden control and the
  source for envelope comparisons.

TOMORROW'S LEAD ORDER:
1. Read the decompiled validator's WRITE phase (rest of FUN_03064474) for any
   check we mapped wrong (the fw_end-based write bounds / A-B slot logic).
2. Compare the reference's fw1 CONTENT vs ours beyond structure: the reference
   runs its init stub from the un-relocated zone and its body relocated;
   verify our word1/thunk pattern matches exactly (REA: reset_handler was a
   thunk to init_stub in the reference).
3. If the envelope is exonerated: instrument our fw1's first instructions (a
   GPIO/register breadcrumb readable without UART) to see how far boot gets.


## DFIU USAGE RUTINE (retrospective 2026-10-11 - apply from now on)

The failure KB proved its worth as an evidence ledger + discipline (failed
hypotheses stay in 'hypothesis', operator_review never counts as a pass), but
it did NOT stop idea-retries because preflight_check was used only 1 of 12
hardware tests. Routine from tomorrow:
1. BEFORE every hardware test / variable change: run preflight_check with a
   ChangeSummaryV1 of the planned delta. If the history already covers the
   hypothesis (rule match), combine the delta or drop it - do not re-flash.
2. Every hardware outcome: verify_resolution with method=reproduction and
   honest preconditions (pass|fail) - no verdict without hardware.
3. First successful boot: verify_resolution (reproduction) + activate_rule +
   compile_artifacts - invariants.json becomes the mandatory gate for future
   builds.


---

# SESSION 2026-10-11 (afternoon) - static re-audit after verdict #12; START HERE

Device: RECOVERED by the user (maskrom), back on stock. Nothing was flashed
this session. No code changed; only this doc + a Ghidra import.

## Tools state
- Ghidra GUI MCP works (project ReChord, socket in /run/user/1000/ghidra-mcp).
  NEW program `/ref_fw1_ram0304F490.bin` = reference IMG[0x1F8:0x37670]
  (md5 4b0e957c..., copy in community/refcfw/ghidra_input/), language
  ARM:LE:32:Cortex, image base 0x0304F490 (= contiguous-load model, so the
  addresses match the device). Functions created: ref_startup @0x030500E4,
  ref_main @0x03050B08. Saved. Auto-analysis was running - re-run
  `reanalyze` if functions are missing.
- The OLD `/fw1_AP` program is MISALIGNED (imported from a 68 KB unpack,
  addresses off by ~0x10) - do not trust its addresses.
- REA MCP: Ghidra provider available but needs `open_binary` first (batch
  calls fail with target_unavailable otherwise). The Ghidra GUI MCP was
  enough this session.

## New facts (byte-verified this session)

1. OUTER HEADER 0x70..0x8C IS AN (offset,size) PART CHAIN, not fw1 bounds.
   REF: 0x70=0x37670 (0x200-byte part header), 0x74=3 parts,
   (0x78 off 0x37870, 0x7C size 0x201C06) -> 0x239476 = 0x80 (next off),
   0x84 size 0x211C06 -> 0x44B07C = 0x88, 0x8C size 0x39A -> 0x44B416
   (= last non-zero byte of the ref image). STOCK uses the same chain
   (0x185910 hdr, parts continue in 0xC8/0xF4/0x148 groups up to
   0x1FC13F6). The two 0x20xxxx parts are the bitmap fonts ("font: bitmap
   fonts from selected slot"); 0x44B07C starts with "EchoIcon" (icons).
   => fw1 occupies IMG 0x1F8..0x37670 in the reference.
   => OUR verdict-#12 image (build/ReChord_AP5M.IMG) copied the ref's
   chain but (a) has NO font/icon parts at those offsets (zeros) and
   (b) make_fw1.py build_ref_shape_5m() OVERWROTE 0x80 with our fw1 end
   (0x5F38), breaking the chain. Fix in make_fw1.py: never patch 0x80;
   either carry the ref's parts verbatim or set the chain to our real
   parts. (Our fw1 itself does not read fonts, so this may not be THE
   brick cause, but it is a real envelope bug.)

2. WHY #9/#10 "FELL BACK" AND #12 BRICKED (consistent theory):
   REFCLONE/RefV1/V2/V3 all modified header bytes (0x10/0x30) WITHOUT
   updating the -3 self-copy at fw_end 0x44C000 (checked: selfcopy_ok
   False for all four) -> updater rejected them -> old firmware stayed
   = "fallback". #12 had the self-copy -> updater ACCEPTED and wrote it
   -> ROM jumped into our fw1 -> our fw1 never produced a picture -> no
   USB MSC either -> looks bricked. So the envelope is probably fine and
   the failure is in OUR fw1 code path. RefV1/V2/V3 are invalid tests
   (no self-copy) - do not flash them as-is.

3. REFERENCE STARTUP (0x030500E4) does more than ours (ap_startup.c):
   cpsid i; stmia ip,{r0-r3} (SAVES ROM boot params r0-r3 to 0x03039128);
   mov sp,0x03004000; SysTick CTRL (0xE000E010)=0; NVIC ICER/ICPR
   (0xE000E180/184, 0xE000E280/284) = 0xFFFFFFFF (disable+clear all IRQs);
   0xE000ED24 (SHCSR)=0; 0xE000ED0C (AIRCR)=0x05FA0002 (VECTCLRACTIVE);
   then .data copy 0x03074B70->0x03004000..0x03005AFC, .bss zero
   0x03005AFC..0x03039128, VTOR=0x03050000, dsb/isb, bl main.
   Ours does NONE of the IRQ/SysTick/AIRCR cleanup and no cpsid: any IRQ
   left enabled/pending by the ROM fires into our Default_Handler loop
   right after VTOR is written = silent hang.

4. REFERENCE main (0x03050B08) boot order: 0x03051C2C, 0x030514C0,
   0x030518D8, 0x030515B8, cpsie i, 0x030513C4(1), delay 0x03051B04(20),
   0x030513C4(2), 0x03051F14() (display bring-up per drivers.md),
   0x03051DD8(100) (likely BACKLIGHT level 100 - UNVERIFIED, decompile
   it), 0x030506F0(0x24DF,"<refcfw> 0.9.2") (title), 0x03050550(line,color,
   fmt,...) (prints a status text line - "build %s panel %08x"),
   then eMMC init etc.
   OUR ap_main.c NEVER TURNS ON THE BACKLIGHT and never does whatever
   0x03051C2C/514C0/518D8/515B8/513C4 do (clocks/pinmux/PMU?). Even with a
   perfect panel driver the screen would stay black. Strongest
   explanation for "#12 = black screen".
   Our panel delays assume 200 MHz; the boot CPU runs at 24 MHz (log
   "cpu 24000000 Hz") -> delays ~8x longer (seconds), harmless.
   Boot log "panel 66666666" => panel init VARIANT B (our default) is right.

## NEXT STEPS (in order, cheapest + least brick risk first)

1. Decompile in Ghidra (ref_fw1_ram0304F490.bin): 0x03051DD8 (backlight?),
   0x03051C2C, 0x030514C0, 0x030518D8, 0x030515B8, 0x030513C4, 0x03051F14.
   Write down which registers they touch (CRU 0x40180000, GRF, GPIO, PWM).

2. HOOK TEST (recommended next flash, low brick risk, max info): take the
   VERBATIM reference image, patch ONLY its payload:
   - at 0x03050B3C (`bl 0x03051DD8`, IMG = addr-0x0304F298 = 0x18A4)
     -> `bl our_hook`;
   - place our_hook in the zero run IMG 0x14FE8..0x258D8
     (RAM 0x03064280..0x03074B70), linked at e.g. 0x03064300, with its
     own tiny .data/.bss INSIDE that region (do NOT touch 0x03004000..
     0x03039128 = ref .data/.bss, IRQs are live at that point);
   - our_hook: call 0x03051DD9(100) (original call), then
     0x03050551(3, 0xFFFF, "ReChord hook OK") (ref text printer = proof
     our code ran, independent of our driver), ~2 s delay, then
     panel_io_init + panel_init_sequence + raw CASET/RASET/RAMWR colour
     bars via panel_io (NO 108 KB framebuffer - display.c's fb does not
     fit), ~3 s delay, return.
   - Header unchanged -> the -3 self-copy stays valid; recompute ONLY the
     trailer CRC at 0x500000.
   Outcomes: ref UI boots + our text/bars = everything proven; ref UI
   boots, no bars = our panel driver wrong; ref UI boots, nothing of ours =
   hook not reached; brick = payload is checksummed (then test a
   1-byte-in-zero-run change alone).
   Run DFIU preflight_check before flashing; verify_resolution after.

3. Fix our own fw1 startup/main (only after step 2 passes): copy the ref
   startup recipe from fact 3 (cpsid, save r0-r3, SysTick off, NVIC
   clear, AIRCR, then data/bss/VTOR) and call our equivalents of the
   ref's pre-display init + backlight before display_init(). Then rebuild
   with make_fw1.py (with the 0x80 fix from fact 1) and flash.

4. Commit all uncommitted work when the user says so (no push without
   explicit authorization).


---

# SESSION 2026-10-11 (afternoon) - static re-audit after verdict #12; START HERE

Device RECOVERED (maskrom), on stock. Nothing flashed this session; no code
changed. Only this doc + a new Ghidra import.

## Tools state
- Ghidra GUI MCP works (project ReChord). NEW program
  `/ref_fw1_ram0304F490.bin` = reference IMG[0x1F8:0x37670] (copy in
  community/refcfw/ghidra_input/, md5 4b0e957c...), ARM:LE:32:Cortex, image
  base 0x0304F490 (contiguous-load model, addresses = device RAM).
  Functions: ref_startup @0x030500E4, ref_main @0x03050B08. Saved; re-run
  `reanalyze` if functions are missing.
- OLD `/fw1_AP` program is MISALIGNED (68 KB unpack, ~0x10 off) - ignore it.
- REA: needs `open_binary` before any analysis call.

## New facts (byte-verified)
1. Outer header 0x70..0x8C = (offset,size) PART CHAIN, not fw1 bounds.
   REF: 0x70=0x37670 (0x200 part header), 0x74=3, (0x37870,0x201C06)
   -> 0x239476 (=0x80), (0x239476,0x211C06) -> 0x44B07C (=0x88),
   size 0x39A -> 0x44B416 (= ref last non-zero byte). Parts = 2 bitmap
   fonts + "EchoIcon" icons. So ref fw1 = IMG 0x1F8..0x37670.
   Our #12 image copied that chain with NO parts present (zeros) and
   make_fw1.py build_ref_shape_5m() OVERWROTE 0x80 with our fw1 end
   (0x5F38) -> broken chain. Fix: never patch 0x80; carry the ref parts
   verbatim (or rebuild the chain for our real parts).
2. RefCLONE/RefV1/V2/V3 all changed header bytes WITHOUT the -3 self-copy
   at 0x44C000 (verified: selfcopy False) -> updater rejected -> "fallback".
   #12 had the self-copy -> accepted -> ROM ran our fw1 -> no picture, no
   USB -> "brick". => envelope probably OK; failure is in OUR fw1 code.
   RefV1/V2/V3 are invalid tests; do not flash as-is.
3. Ref startup 0x030500E4 does what ours (ap_startup.c) does NOT:
   cpsid i; save ROM r0-r3 to 0x03039128; sp=0x03004000; SysTick
   CTRL=0; NVIC ICER/ICPR (0xE000E180/184/280/284)=0xFFFFFFFF;
   SHCSR(0xE000ED24)=0; AIRCR(0xE000ED0C)=0x05FA0002; then .data
   0x03074B70->0x03004000..0x03005AFC, .bss 0x03005AFC..0x03039128,
   VTOR=0x03050000, dsb/isb, bl main. Without the IRQ cleanup any ROM IRQ
   lands in our Default_Handler loop = silent hang.
4. Ref main 0x03050B08 order: 0x03051C2C, 0x030514C0, 0x030518D8,
   0x030515B8, cpsie i, 0x030513C4(1), delay 0x03051B04(20),
   0x030513C4(2), 0x03051F14() display bring-up, 0x03051DD8(100)
   (probably BACKLIGHT 100 - verify), 0x030506F0(0x24DF,"<refcfw> 0.9.2")
   title, 0x03050550(line,color,fmt,..) = on-screen status text printer.
   OUR ap_main.c NEVER enables the backlight nor runs the pre-display
   init -> black screen even if the panel driver is right. Prime suspect.
   Boot log "panel 66666666" -> panel init variant B (our default) is right.
   Our panel delays assume 200 MHz; boot CPU is 24 MHz (slow, harmless).

## NEXT STEPS (in order)
1. Ghidra: decompile 0x03051DD8, 0x03051C2C, 0x030514C0, 0x030518D8,
   0x030515B8, 0x030513C4, 0x03051F14; note registers touched
   (CRU 0x40180000, GRF, GPIO, PWM).
2. HOOK TEST (next flash, low risk, max info): VERBATIM ref image, patch
   payload only: `bl 0x03051DD8` at 0x03050B3C (IMG 0x18A4) -> `bl our_hook`.
   our_hook lives in zero run IMG 0x14FE8..0x258D8 (RAM 0x03064280..
   0x03074B70), linked ~0x03064300, its .data/.bss inside that region
   (never touch 0x03004000..0x03039128; IRQs are live). our_hook:
   0x03051DD9(100); 0x03050551(3,0xFFFF,"ReChord hook OK"); ~2 s;
   panel_io_init + panel_init_sequence + raw CASET/RASET/RAMWR colour
   bars (no 108 KB framebuffer); ~3 s; return. Header untouched (self-copy
   stays valid); recompute only the trailer CRC @0x500000.
   Outcomes: ref UI + our text + bars = all proven; text but no bars =
   our panel driver wrong; nothing of ours = hook not reached; brick =
   payload checksummed (then test 1 byte in the zero run alone).
   DFIU: preflight_check before, verify_resolution after.
3. After step 2 passes: port the ref startup recipe (fact 3) into
   ap_startup.c, call our equivalents of the pre-display init + backlight
   before display_init(), apply the 0x80 fix (fact 1), rebuild, flash.
4. Commit the uncommitted work when the user says so (no push without OK).


---

# SESSION 2026-10-11 (afternoon, cont.) - ref pre-display init DECODED; hook image BUILT

Nothing flashed. Device on stock.

## Decoded (Ghidra, /ref_fw1_ram0304F490.bin; literals byte-read)
- 0x030514C0 = GPIO2.4 -> GPIO output HIGH (BACKLIGHT pin), 2nd call in main.
- 0x03051DD8(level) = PWM backlight: IOMUX GPIO2.4 func 1, CRU gate 0x90 on
  (CRU+0xA4 bit0), soft-reset pulse 0x17 (CRU+0xC4 bit7), PWM @0x400F0000:
  [0]=0, [1]=period=pclk/10000 (pclk global 0x0300410C = 24 MHz), [2]=duty=
  period*level/100, [3]=0x10003. level 0/out of range -> GPIO2.4 low.
- 0x03051458(port,pin,func) = IOMUX: *(0x50010000+port*0x20+(pin>>3)*4) =
  (func|0x30000)<<((pin&7)*2). 0x03051478(port,pin,v) = mux GPIO + DR/DDR
  out (GPIO0 0x40160000, GPIO1 0x40170000, GPIO2 0x50030000).
- 0x03051F14 display bring-up muxes GPIO0 16..25,15,12 to GPIO FIRST
  (0x03051CE8), reset = GPIO2.15 (0x03051CB0), ID probe, DCS variants A/B.
- 0x030515B8 = clocks: CRU+0x10/14/18, sys/pclk globals = 24 MHz, SysTick
  1 ms with IRQ (CTRL=7). 0x030518D8 = broad CRU gates (+0x80..A4) / soft
  resets (+0xC0..CC) / GRF (0x50010000) setup + NVIC ICER bit5 - NOT ported.
- Watchdog @0x40120000: armed by the ref itself (0x03051A38, AFTER display);
  kicked (WDT+0xC=0x76) in every 1 ms delay tick. Likely off at entry.
- 0x03051C2C fills 0x03000000..sp with a stack-paint pattern (harmless).

=> Verdict #12 black-screen suspects in OUR fw1: no backlight (GPIO2.4),
   no IOMUX for panel pins (FIXED now in panel_io_init), no IRQ cleanup.

## Built: build/ReChord_HOOK.IMG (`make hook-img`)
md5 3e0480206eda2f41daf00f45583d00e8. Verbatim HIFIEC90.IMG + 751 bytes:
`bl ref_hook` at IMG 0x18A4, 1496 B blob at RAM 0x03064300 (zero run);
header/self-copy untouched, CRC recomputed; script self-verifies all of it.
ref_hook: ref backlight(100); ref print(3,0xFFFF,"ReChord hook OK"); 2 s;
OUR panel_io_init (incl. new IOMUX) + panel_init_sequence + 8 colour bars;
3 s; return. DFIU preflight: no matching failures.
Outcomes: text+bars = our panel driver proven -> port backlight/IRQ cleanup
to our fw1; text only = our driver wrong; neither = hook not reached;
brick = payload integrity-checked.

## HARDWARE VERDICT #13 (2026-10-11 ~13:45): HOOK TEST PASSED - START HERE

Flashed build/ReChord_HOOK.IMG (md5 3e0480206eda2f41daf00f45583d00e8) as
ECHOMINI.IMG (NOTE: the device runs the REFERENCE CFW, not stock; its
updater only accepts HIFIECnn.IMG / ECHOMINI.IMG - the stray HIFIEC.IMG on
the root is ignored). User saw: "ReChord hook OK" -> 8 colour bars (OUR
panel driver) -> reference UI booted normally.

Proven on hardware:
- OUR panel stack works: GPIO0 8080 transport, IOMUX, reset GPIO2.15,
  variant-B init, CASET/RASET(+35)/RAMWR. (Exact bar-row alignment not
  inspected closely.)
- Updater: payload is NOT integrity-checked beyond the trailer CRC; the
  self-copy + CRC model is right. Patching the reference payload is a SAFE
  test vehicle for any further bring-up experiment.

DFIU: f5e3b421 (panel) + 3a318ade (validator) -> verified_resolved; rules
R-d9fb94121b9c + R-9f6e141101cb ACTIVE; artifacts compiled (6 active rules).

=> The verdict-#12 black screen is in OUR fw1's pre-display boot, not the
panel driver and not the envelope. NEXT: port to ap_startup.c/ap_main.c
(1) the ref startup IRQ cleanup (cpsid, SysTick off, NVIC ICER/ICPR,
SHCSR, AIRCR), (2) backlight GPIO2.4 high (mux GPIO, DR|=0x10, DDR|=0x10)
before display_init. Cheapest de-risking first: a 2nd hook test that runs
OUR ap_startup+Main-equivalent init from the hook is not possible (it
re-inits RAM); instead flash our fixed fw1 directly (debrick ready).
Device ECHOLOG.TXT after the hook boot not yet read (device unplugged).

## VERDICTS #14 + #15 (2026-10-11 ~14:00): OUR fw1 Main() RUNS ON HARDWARE

- #14 EARLY HOOK (build/ReChord_HOOK_EARLY.IMG, `make hook-early-img`):
  patch `bl 0x030514C0` at RAM 0x03050B12 (IMG 0x187A) = ref main's 2nd
  call, state = ROM handoff + ref startup only. OUR backlight_on() +
  panel init + bars -> visible. No ref init (CRU/clock/SysTick) needed.
- #15 TRIAL (build/ReChord_TRIAL.IMG, `make trial-img`, md5 ea2dda4b...):
  firmware/hook/trial_entry.c + trial.ld run OUR fw1 Main() (all AP_SRCS
  except ap_startup.c/vectors.c, -DAP_TRIAL) from the same patch point:
  code in the ref zero run (0x03064300..0x03074B70, 67 KB), our .data/.bss
  inside the ref .bss (0x03005B00.., wiped back to zero before returning).
  User saw the ReChord screen (accent band, "ReChord AP alive", theme name)
  ~6 s, then the reference UI booted normally.

THE TRIAL VEHICLE IS BRICK-FREE: the reference always resumes, so USB +
updater stay available. Develop ReChord features on it (`make trial-img`,
copy as ECHOMINI.IMG, exit USB). Limits: 67 KB code, ~205 KB RAM, IRQs off
(ref has not enabled them yet), 24 MHz CPU, must return.

Code changes: display_init() now calls backlight_on() (GPIO2.4 high);
panel_io_init() muxes panel pins to GPIO; ap_startup.c got the ref IRQ
quiesce (cpsid, SysTick off, NVIC ICER/ICPR, SHCSR, AIRCR) - standalone
build only, NOT hardware-tested yet.

DFIU: 5667c9c4 got the trial evidence (inconclusive: standalone fw1 not
re-flashed). 3a318ade/R-9f6e141101cb revalidated + re-activated.

NEXT OPTIONS: (a) keep building ReChord on the trial vehicle (keys via
ADC - the ref log shows adc ch0..3 key ladder, back ~500, vol- 0; UI
loop; settings) - zero risk; (b) retry STANDALONE fw1 (now with backlight +
IRQ quiesce; and make_fw1.py 5M builder 0x80 chain fix still pending) -
brick risk, debrick ready.

## VERDICTS #16-#18 (2026-10-11 ~14:30): THE REAL fw1 ENTRY FOUND - ROM -> OUR CODE

`make entry-img` builds ENTRY/ENTRY2/ENTRY3 = verbatim reference + our fw1
(ap_startup.c + Main, -DAP_TRIAL -DAP_CHAIN_REF, firmware/hook/entry.ld:
code at 0x03064300 in the zero run, RAM roles = firmware_ap.ld) + patched
reset word(s) -> ChainEntry (saves ROM r0-r3 @0x03003FF0, runs OUR
PowerOn_Reset -> Main ~6 s -> restores r0-r3 -> ref reset 0x0306296F).
- #16 ENTRY  (VT @0x03050000 word1, IMG 0xD6C)          -> ignored.
- #17 ENTRY2 (VT heading the .data image, IMG 0x258DC)  -> ignored.
- #18 ENTRY3 (all three incl. IMG 0x273D8)              -> RECHORD SCREEN,
  then the reference UI. => ROM -> our startup -> our Main WORKS.

BOOT ENTRY RULE (hardware-verified, DFIU R-5add7c251c3f active):
  the ROM enters through the vector table at
      IMG = row0.dest_end - 0x03050000 + 0x200
  (ref 0x030771D4 -> IMG 0x273D4; stock 0x030A3B34 -> IMG 0x53D34, also a
  VT: SP 0x03004000, reset 0x03056245). row0.dest maps the same way to the
  code VT (ref 0xD68, stock 0x297CC).
ROOT CAUSE OF VERDICT #12: its row0 end 0x03055D38 -> IMG 0x5F38 = zeros
  -> ROM jumped to 0 -> black screen, no USB.

The ENTRY3 image is effectively a DUAL BOOT: ReChord from the first
instruction, then the reference (USB/updater) - brick-free iteration on
our startup as well as our app.

STANDALONE TODO: make the packer emit a VT copy (SP 0x03004000, reset =
PowerOn_Reset|1) at row0.dest_end (IMG mapping above). A standalone fw1
has no USB/updater yet -> leaving it needs maskrom (docs/DEBRICK-LINUX.md).

## VERDICT #19 (2026-10-11 ~14:15): standalone fw1 -> BLACK (maskrom) - REAL BOOT CONTRACT

Flashed build/ReChord_AP5M.IMG (md5 ee103862..., our startup + Main, VT
appended after the row-0 block) -> updater accepted, black screen, no USB.
DFIU incident 0968d8bb, rule R-fcc7ba7fc504.

ROOT CAUSE (byte-verified on reference AND stock): the "entry VT at
row0.dest_end" is really scatter ROW 1. The ROM copies row1
{0x03004000, init_size, init_end, total} from the IMG bytes right after the
row-0 block to RAM 0x03004000 and enters via the VT at its start.
  ref: row1 {0x03004000, 0x1AFC, 0x03005AFC, 0x18C50}; ref[0x273D4:+0x1AFC]
  == ref[0x258D8:+0x1AFC] (its .data image), starting SP 0x03004000,
  reset 0x030500E5, handlers. Stock: same row1, same layout (0x53D34).
  Ours (#12 and #19): .data empty -> row1 init_size 0 -> nothing copied.
  (ENTRY3 worked because it patched the ref's .data VT reset word.)

FIX (built, not flashed yet): ap_startup.c `ap_ram_vectors` {0x03004000,
PowerOn_Reset} in .data.ram_vectors, first in .data (linker ASSERT ==
0x03004000); pack_fw1.py emits ONE RW row1 {0x03004000, .data filesz,
end, RW span} and asserts the .data image follows the row-0 block;
make_fw1.py refuses an image whose row1 init_size < 8 or whose bytes after
row 0 are not SP 0x03004000 + odd reset. New AP5M row1 =
{0x03004000, 0x8, 0x03004008, 0x1D704}, IMG 0x5FCC = 0x03004000/0x03050145.

## VERDICT #20 (2026-10-11 ~14:30): STANDALONE fw1 BOOTS - ReChord IS RUNNING

build/ReChord_AP5M.IMG (md5 5e4477a85c40249436edf50b7e2c40a1, `make
cfw-img`, copy kept as build/ReChord_FW1_v2.IMG) installed by the STOCK 3.7
updater as HIFIEC90.IMG -> cold boot -> "ReChord AP alive" + theme name,
stable. No reference code anywhere in the boot path: ROM -> row0 code copy
-> row1 .data copy (VT) -> our PowerOn_Reset -> our Main.
DFIU: incidents 0968d8bb + 5667c9c4 verified_resolved; R-fcc7ba7fc504 and
R-70b34b8f2528 active.

Device is now ON OUR fw1: no USB/updater -> to change firmware, maskrom +
stock (docs/DEBRICK-LINUX.md), then copy the next HIFIEC90.IMG.
NEXT: USB MSC (or a key combo that reboots into maskrom/loader) in our fw1
so iteration stops needing the paperclip; then keys, menus, audio.

## VERDICT #21 (2026-10-11 ~15:00): MILESTONE 2 - KEYS + NAVIGABLE UI ON HARDWARE

build/ReChord_AP5M.IMG md5 2dddb029f64c8709293d0bc973d44edc (copy
build/ReChord_FW1_m2.IMG) -> splash, then the ReChord root menu (Files /
Now Playing / Settings / Keys); vol+/vol- move, play selects, back/menu
return. All keys confirmed by the user.

Key hardware (reference key scan 0x03057eb0 + its ECHOLOG, now in our code):
- SAR-ADC 0x400D0000: [0] data (10-bit), [1] status, [2] ctrl =
  ch | 0x08 power | 0x10 start (0x40 irq status). Read the previous result,
  then start the next conversion (keys_adc in panel_io_rknanoc.c).
- Ladder on channel 1 (measured): vol- 0, vol+ 121..177, play 245..299,
  menu 321..395, back 483..549, idle >= 1001 (bands in services/input.c).
- Power key: GPIO2.5 EXT_PORTA bit, 1 = pressed (keys_power).
- input_scan(): 2-sample debounce, one event per press, no auto-repeat yet.
UI loop: firmware/startup/ap_ui.c (root menu + live "Keys" diagnostic row,
KEY_MENU = back to root, ~5 ms busy-wait scan period).
Also fixed: firmware/ui `make test` link (theme sources), 250 checks pass.

NEXT candidates: SysTick timebase + key repeat; storage (eMMC/FAT) for
the browser and settings; USB MSC so updates stop needing maskrom.

## VERDICT #22 (2026-10-11): READ-ONLY eMMC DRIVER WORKS ON HARDWARE

build/ReChord_FW1_m3a.IMG (md5 1c15eca4a355094f67efed246a525700) -> root
menu "eMMC" row showed `7456M u24000 n[o bs]`: init (CMD0/1/2/3/9/7/16,
EXT_CSD via CMD8, CMD6 8-bit bus, CMD13 tran), CMD17/CMD18 reads and the
IDB parse all work. 7456 MiB = 15269888 sectors, IDB system size 0x22000
sectors -- identical to the reference's own ECHOLOG on this unit.

`no bs` is correct: on this unit LBA 0x24000 (stock position, right after
the firmware area) holds a "SAVE" block; the user volume is at 0x88000.
The reference finds it by scanning forward in 0x800-sector steps for an
exFAT/FAT boot sector or MBR (0x03052a1c); ported into emmc_layout()
(not yet flashed; expected row: `7456M u88000`).

Driver: firmware/drivers/emmc_rknanoc.c (clean-room from reference init
0x030527ac, cmd 0x0305257c, data 0x030523d4, clock 0x0305235c, read
0x03052694, layout 0x03052a98). Facts worth keeping:
- DW-MMC 0x60030000, polled FIFO at +0x200; 24 MHz source, CRU clksel
  +0x30 = 0x9F008000 | (prediv-1)<<8, CLKDIV = cd>>1; update-clock CMD
  0x80202000.
- CRU gates: 0x19 (bus) on at init; 0x6C (card clock) on only around
  each transfer. Reset 0x11 deasserted. GRF mux [0]=0xFFFF5555,
  [1]=0x3F0015, [9]=0xC00040 (GPIO0.0-10 + GPIO1.11; no panel pins).
- IDB copies at LBA 0x40 + n*0x400 (n<5): word0 0xFCDC8C3B, "NANO" at
  +0x20A, u16 system size (MiB) at +0x202. Base device reads are raw LBA.
- Writes (reference 0x0305271c): CMD24/CMD25 (+CMD12 0x4140), then CMD13
  wait for tran; FIFO depth from FIFOTH reset value. Not ported yet.

## VERDICT #23 (2026-10-11): USB MASS STORAGE IN OUR fw1 - READ/WRITE WORK

build/ReChord_FW1_m3c.IMG (md5 617612d17b8777a8f57954a371eeab0e), flashed
via stock updater as HIFIEC90.IMG (the name must be HIFIECnn.IMG; a plain
HIFIEC.IMG is ignored). Plugging the cable into the PC opens our USB screen;
the host enumerates 071b:3203 "ReChord ECHO MINI" (full speed, forced),
14712832 sectors = stock MSC window (user volume at 0x88000). All baseline
file md5s match; a 4 MiB random file written, remounted and re-read
identically (~1 MB/s, full-speed ceiling). udisks Drive.Eject (SCSI START
STOP with LoEj) ends the session by itself (row `x1`); power key ends it
(`x6`).

Root cause of m3b's "config descriptor too short / length 39": our startup
re-copied .data from _data_load_start (0x03056AEC) = exactly row-0 end, a
region the ROM never loads -> every initialized global was junk since #20.
The ROM already copies .data via scatter row 1 (the reference copies from
inside its row-0 span instead). Fix: no .data copy in ap_startup.c.

Open:
- `udisksctl power-off` (port disable, no LoEj) does NOT end the session:
  suspend after configuration isn't detected. m3d (md5 2fabfbdf...) adds a
  "USB diag" row: last opcode, last START STOP byte 4, GINTSTS bits 10..13
  seen. Rows now report MiB (`x1 r12M w4M`).
- Updates still need maskrom: nothing in our fw1 installs a copied image.
- Drop forced full speed once the PLL is set up.

Device state at end of session: m3c installed, stock HIFIEC.IMG leftover on
the volume was overwritten by mistake (no copy; updater ignores that name).
Volume files backed up to ~/echo_backup (ECHOLOG.TXT, RECHO.CFG).

## VERDICT #24 (2026-10-11): OUR fw1 UPDATES ITSELF - NO MORE MASKROM

m4a (installed via stock) -> m4b and m4b -> m4c installed by OUR updater on
hardware: copy HIFIEC90.IMG over our USB, eject (or unplug), screen shows
"Firmware update detected", name, KiB, SHA-256 (= `sha256sum`), PLAY = yes.
Both slots written and verified, image renamed to HIFIEC90.OLD, new build
boots after a power-cycle (title shows RC_BUILD in ap_ui.c).

Code: firmware/services/fw_update.c (clean-room from reference 0x03054474):
- FAT32 only (MBR p0 or superfloppy), root dir, contiguous files only
  (FWU_EFRAG otherwise: delete and copy again).
- Checks: "RKnanoFW" @0x1F8, header copy at fw size (@0x1F4), CRC
  0x04C10DB7 over [0, fw_end) == trailer @fw_end, fits one slot.
- Firmware area = LBA 0x2000 + [0, IDB sysMiB*0x800); slot 2 at half.
  Order: slot2 header := 0xFF, slot2 body, slot2 header, slot1 header :=
  0xFF, slot1 body copied from slot2, slot1 header. 8 KiB blocks, each
  read back. emmc_fw_write() is bounded to the firmware area.
- Host check: `python3 tools/test_fw_update.py [IMG]` (mkfs.fat volume,
  install, fsck, corrupted-image rejection).

Open:
- Warm reboot does NOT work: neither SYSRESETREQ nor a port of the
  reference sequence (0x03051b40: "NanoCRebootFlag" at 0x0309FFF0, SysTick/
  NVIC off, CRU gates 0x80..0xA4 = 0xFFFF0000 then 0xA0 = 0x80008000, resets
  0xC0..0xCC = 0xFFFF0000, GRF+0xC4 = 0x01000000, VTOR 0, AIRCR 0x05FA0007).
  The screen now says "power-cycle the player". Likely missing: what the
  reference does before calling it (clock tree / PMU state).
- Stock images (bigger than a slot, the reference's fw1-only path) are
  refused with FWU_EFIT; going back to stock still needs maskrom.
- exFAT / FAT16 / fragmented images unsupported.

## VERDICT #25 (2026-10-11): POWER-OFF + CLEAN IMAGE DELETE (m4e)

- Power off = GPIO2.6 (power hold) driven low (reference 0x03051c1c),
  board_power_off() in panel_io_rknanoc.c. Long press ~2 s in the main
  loop (700 passes; 2000 passes measured ~6 s on hardware). "Update done"
  screen: press power to turn off. With USB plugged in VBUS keeps it on.
- Bug fixed: retire-by-rename to *.OLD created duplicate HIFIEC90.OLD
  entries on the second update. fw_update_retire() now deletes like stock:
  clusters freed in every FAT, entry + same-sector LFN entries = 0xE5.
  Host check asserts fsck clean with 1 cluster in use. Hardware: m4d -> m4e
  showed "cleanup 0".

Next session:
1. Pick the real product work (audio path / file browser over FAT32 -
   fw_update.c already has a minimal FAT32 reader to grow from).
2. Warm reboot, suspend exit (m3d "USB diag"), high-speed USB: polish.
