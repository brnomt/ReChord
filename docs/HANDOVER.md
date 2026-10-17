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

## VERDICT #26 (2026-10-11): microSD READ WORKS (m5a)

Root menu "SD" row on hardware: size, SDHC, 4-bit, file system of the first
partition all correct; "no card" without one; re-probes on insert/remove.
Driver: second DW-MMC instance at 0x60020000 sharing emmc_rknanoc.c's
command/clock/FIFO code (select_mmc()). Facts (reference 0x03062d84 init,
0x03062a2c read, 0x03062c34 detect):
- Card detect GPIO0.11 input, GRF+0x14 = 0x80008 pull-up, 0 = card in.
- Gates 0x18 (bus) + 0x6B (card clock, on only around transfers), reset
  0x12; CRU+0x30 low half = 0x9F0080 | (prediv-1) (eMMC uses the high half).
- Pinmux GRF+0x20 = 0xFC005400, GRF+0x24 = 0x3F0015 (GPIO1.5..10).
- CMD0, CMD8 0x1AA, CMD55+ACMD41 (0x40FF8000 / 0x00FF8000 for 1.x),
  CMD2, CMD3 (RCA from card), CMD9 CSD (v1/v2 capacity), 24 MHz, CMD7,
  CMD16 for SDSC, ACMD42 0, ACMD6 2 -> CTYPE 1. SDSC = byte addressing.
- Read-only, like the reference.

## VERDICT #27 (2026-10-11): SD AS USB LUN 1 + FILE BROWSER (m5b)

Hardware (m5a -> m5b via self-update): Files shows "Internal" and "SD" and
browses both; on the PC the SD card shows up as a second, read-only disk.
- USB: usb_msc_run() takes a LUN table {read, write (NULL = write
  protected), base, count (0 = no medium)}. GET MAX LUN = nluns-1, CBW LUN
  checked, sense per LUN. LUN 0 = eMMC user volume (r/w), LUN 1 = SD (r/o:
  MODE SENSE WP bit, writes -> 7/0x27; no card -> 2/0x3A). The card state
  is fixed for the session (insert before plugging in). The main loop now
  probes SD before starting the USB session.
- fs_target.c: read-only FAT16 / FAT32 (LFN with checksum, NT lower-case
  bits) / exFAT (FAT chain + NoFatChain), superfloppy or MBR partition 0,
  FAT12 refused. Paths "/<volume>/...", "/" lists the mounted volumes
  (fs_target_mount(): "Internal" after emmc_probe, "SD" after sd_probe,
  Internal remounted after every USB session). One-sector cache (ponytail),
  4 read handles, writes FS_ERR_UNSUPPORTED. Settings-style paths
  ("\RECHORD.CFG") do not resolve on target yet.
- Host check: `python3 tools/test_fs_target.py` builds FAT16, FAT32 (behind
  an MBR) and exFAT images through a udisks loop mount (no root; long and
  Unicode names, 300-entry dir, fragmented files, empty file/dir) and
  compares every listing + content hash with os.walk.

Next session:
1. Audio path: decode from fs_open_read() (multi-sector reads in
   fs_target.c once throughput matters).
2. Settings on target (needs a volume path convention or fs writes).
3. Warm reboot, suspend exit (m3d "USB diag"), high-speed USB: polish.

---

# AUDIO / SD / GAPELESS / REPLAYGAIN ROADMAP (2026-10-12)

> Firmware is currently booting on our standalone fw1 (verdict #20/#21:
> ROM -> row0 code copy -> our PowerOn_Reset -> our Main; device stable on
> "ReChord AP alive"). Keys, navigation, eMMC read, SD read, USB MSC, file
> browser and the self-update all work on hardware. The clean-road AP layer
> (`firmware/player`, `firmware/dsp`, `firmware/ui`, `firmware/services/fs`)
> is host-compiled + host-tested but **not yet linked into the AP image**
> (ap.mk still lists `firmware/player` under "NOT linked in milestone 1a").
> This section is the path from "boots" to "plays audio from SD with gapless
> transitions and ReplayGain applied."

> ## IMPLEMENTATION STATUS (this session)
>
> Steps 1–7 below are **DONE and host-tested** (`firmware/player/tests/run_tests.sh`, 271 assertions green, `ALL PLAYER TESTS PASSED`). What landed:
> - ✅ `fs.h`: `fs_seek` added in `fs.h`/`fs_target.c`/`fs_host.c` (target M3 build clean).
> - ✅ `firmware/player/codec_io_fs.c`: target backend over `fs_*` + `PLAYER_CODEC_IO_IMPL`
>   switch in `player.mk` (stdlib default). Host WAV-decode byte-exact test passes.
> - ✅ Gapless: `codec_t.gapless` (`codec_gapless_t{delay,padding}`), `player.c`
>   leading-trim + trailing-trim (uses codec-authored `codec_format_t.total_samples`
>   for an exact end, not ms-truncated), and `ipc_b_track_change` for seamless
>   same-format advance (B core NOT restarted; hard boundary on format change).
> - ✅ ReplayGain: `tags.c`/`tags.h` (ID3v2 `TXXX` + Vorbis comments), `rg_math.c`
>   (Q1.15 dB→linear, peak clamp, saturating `rg_scale_pcm`) applied in the PCM
>   pump pre-ring; `PLAYER_RG_TRACK/ALBUM/OFF` mode via `player_set_replaygain_mode`.
> - ✅ Step 8 (milestone 2a): player LINKED into `ap.mk` — see the MILESTONE 2A
>   section below (ELF + IMG built, DFIU revalidated, uploaded to the device).
> - DFIU preflight (`ChangeSummaryV1`) ran; surfaced variant alert
>   `R-49b7fd25aca0` (FAT rename/collision in the same volume — informational:
>   `fs_seek` is read-only). New `ipc_b_track_change` mailbox command must await
> the B-core reply per `R-e9b5d8222673` (documented; target stub is a no-op
>   until milestone 2a).

## MILESTONE 2A (2026-10-12): PLAYER LINKED INTO THE AP + IMG UPLOADED

- ✅ **Linkage DONE.** `firmware/ap.mk`: new `AP_PLAYER_SRCS` (player.c,
  codec_io.c, codec_io_fs.c, codec_wav/mp3/flac, tags.c, rg_math.c,
  ipc_b_target.c, ringbuf.c) added to `AP_SRCS`, `-Ifirmware/player` in the
  include roots, milestone comment updated. The real device build is
  `make cfw-ap` (rule `$(CFW_BUILD_DIR)/objs/%.o: %.c` compiles any clean-road
  `.c` — NOT the legacy `build-ap` SDK pipeline, which only covers
  `firmware/rockchip/%`).
- ✅ **Heap eliminated from the target path.** The freestanding AP link
  (`-nostdlib`, `AP_LIBC_SRCS` = mem/str/stdio_min/math_min) has no
  `malloc/free`, so `codec_io_fs.c` was rewritten to a **4-slot static pool**
  (== `fs_target.c`'s 4 read handles; `cio_close` dispatches, the slot is
  returned to the pool in `s_close`). `rg_math.c`'s `pow` resolves via
  `math_min.c`. First link attempt failed on `malloc/free` — that's the whole
  fix. All 40 clean-road TUs + player compile with `-mcpu=cortex-m3 -mthumb`.
- ✅ **ELF:** `build/cfw/rechord_ap.elf`, text 42,436 / data 100 / bss 248,956.
  `_bss_end = 0x03040CE0` — inside the linker's designed `FW_RAM` window
  (LENGTH to `0x0304C000`, `__heap_end = 0x0304C000`), clear of the stack
  zone and the loaded image at `0x0304FA50`. Player symbols present in the
  ELF (player_init/open/process, cio_open, tags_replaygain_id3v2, rg_scale_pcm,
  ipc_b_start, ipc_b_track_change; wav/mp3/flac_open static).
- ✅ **IMG:** `make cfw-img` → `build/ReChord_AP.IMG` (33 MB, packer
  `RESULT: PASS`, trailer CRC `0xC3948CA9`) + **`build/ReChord_AP5M.IMG`**
  (5 MB ref-shape, 5,242,884 B, CRC `0x217BC3B0`, fw_end `0x44C000`) — the
  flash vehicle (packer NOTE: boots on hardware, verdict #20 lineage).
- ✅ **DFIU:** preflight (alert, no blockers) → stale blocker
  `R-70b34b8f2528` re-verified (m5b hardware-booted with identical scope
  files, verdict #27; byte audit of both new images: loaded-RAM row srcs,
  VT @IMG 0xD68 + entry VT @IMG 0xB32C both SP `0x03004000`/reset
  `0x03050145` Thumb, row2 byte-identical to m5b, packer-CRC match,
  no row in the stack zone) → **re-activated (active/blocker, rev 2)** →
  `compile_artifacts` (5 active rules).
- ✅ **UPLOADED:** `build/ReChord_AP5M.IMG` →
  `/run/media/brnomt/ECHO MINI/HIFIEC90.IMG`, md5
  `d72594b0b0b6023c708737b6d80f7349` (byte-identical to the local build).

### Update detection on hardware (ECHOLOG oracle — read this)

The device was **NOT running our fw1/m5b** at upload time: it runs
**RECHO (ref-based CFW, `RECHO 7e287ea v0.9.2 2026-10-07`)** — installed by a
previous boot's `ECHOMINIIMG` (ECHOLOG: `update: found ECHOMINIIMG … result 0`,
then `boot: build 7e287ea … resources @0 (legacy) are RECHO 7e287ea …`).
Consequences for update UX (the "device isn't detecting" confusion):

- The **RECHO/ref updater is SILENT**: no "Firmware update detected" screen —
  it logs to ECHOLOG.TXT only. Our copy WAS detected and INSTALLED:
  `499.850 update: found HIFIEC90IMG (official v0.9.2 2026-10-11), 5242884
  bytes` → `503.752 update: result 0 cleanup 0 fw_end 44c000 fw_size 500000
  fw1_only 0, 3888 ms` (`fw_end 0x44C000` = our AP5M exactly; `result 0` =
  slots written + verified). Trigger: the user pressed POWER in the USB
  session ("usb: exit key pressed").
- Our own prompt (`update_prompt` in ap_ui.c) only runs after
  `USB_EXIT_EJECT` (LoEj) or `USB_EXIT_UNPLUG` (VBUS gone) — the power-key
  exit (`USB_EXIT_USER`) skips it by design; `udisksctl power-off` (port
  disable, no LoEj = suspend) doesn't end our session either (m3d open item).
- `HIFIEC90.IMG` stays on the volume (ref cleanup doesn't delete it). After
  booting our fw1, an eject would re-offer the same image (harmless reinstall)
  — delete it once the boot is confirmed.
- **NEXT: power-cycle** → boots `ReChord_AP5M` (player linked; expect
  "ReChord AP alive" + RC_BUILD label). Then DFIU `verify_resolution` with
  the user's confirmed outcome — no verdict without hardware.

### m6a (2026-10-12): browser → player wired (press play actually does something)

User feedback: pressing PLAY on a file in the browser did nothing — correct,
`browser_screen.c` only logged (the "player module will own file opening"
placeholder). Wired end-to-end:

- `browser_screen.c` file select → bounded `<dir>/<name>` build →
  `player_open()` → on OK: `player_screen_set_status(NULL)` + `set_track`
  (title/duration from `player_get_track`) + `player_play()`; on error:
  title = filename + status "unsupported format" (PLAYER_ERR_UNSUPPORTED)
  or "cannot open file"; always `ui_push_screen(&player_screen)`.
- `player_screen.c` is now a real consumer: `on_enter` syncs
  title/duration/volume/state from the player module; SELECT =
  play/pause via `player_play/pause`; UP/DOWN = volume via
  `player_set_volume` (player clamps -60..+20 dB); BACK pops (playback
  continues); `player_screen_refresh()` (called every loop pass) updates
  position + play state and auto-sets the status line on PLAYER_ERROR
  (**"no audio renderer"** — honest: no B core/DAC consumer exists in this
  fw1 image, so the ipc_b supervisor trips after its 512-poll budget).
- `ap_ui.c` loop: `(void)player_process(); player_screen_refresh();` before
  `ui_render()`; RC_BUILD bumped m5b → **m6a**.
- Seam: `firmware/ui/internal/shims/player.h` forwards to the canonical
  `firmware/player/player.h` (same pattern as `shims/fs.h`); `hal.h` includes
  it in both branches.
- Host tests: `mock_services.c` gained the player mock (scripted open rc /
  state / position, recorded open path / play / pause / volume) + 12 new
  assertions in `test_browser_pagination.c` (file select → open path
  `/t/f00` → player screen pushed → "Playing"; unsupported → status shown).
  **262 checks, 0 failures**; UI M3 target compile OK; player suite green.
- IMG rebuilt: `build/ReChord_AP5M.IMG` md5 `78ef8d16c8aa29996e66f9a7f0cc40d8`
  (packer RESULT: PASS). STILL SILENT by design: pressing play now opens
  Now Playing and decodes up to the ring, but audible output needs the DAC
  renderer (next milestone: consume the ipc_b ring → I2S/DAC, see
  `docs/re/dac-output-path.md` for the SDK's DMA-ISR decode loop) and mp3/flac
  need the vendored codecs (WAV only today).

### m6b (2026-10-12): power-key USB exit now scans for updates

User feedback: pressing POWER to leave the USB session skipped the update
(the running build only scanned on `USB_EXIT_EJECT || USB_EXIT_UNPLUG`,
even though the screen says "eject on the PC **or press power**" — and the
reference updater scans after "usb: exit key pressed" too, per ECHOLOG).
Fix in `usb_session()`: `update_prompt()` now runs for
`EJECT || UNPLUG || USER`. With no image present `fw_update_find` returns
FWU_NONE and the return is seamless (no visible scan screen). RC_BUILD
m6a → **m6b**; IMG md5 `2cf844ccd39f398dfbe2f7c49f79ca2c`, RESULT: PASS.

### m6c (2026-10-12): real MP3 + FLAC codecs vendored (structure adopted)

Per the codec-structure recommendation (uniform vtable + registry + one file
per codec + Makefile-level conditional compilation) — which matches what
`codec_t` already was — the layout was applied while landing the codecs:

- **Vendored (unmodified, version-pinned, license files included):**
  - `firmware/player/codecs/minimp3/` — minimp3 **CC0-1.0**, pin
    `ea99364f61c14656440e8d77e9c233ccf3124633` (2026-07-27).
  - `firmware/player/codecs/dr_flac/` — dr_flac **v0.13.4** dual
    public-domain/MIT-0, dr_libs pin `dfe8377631000664666519fdb83da193fd8037f4`
    (2026-08-31).
  License rationale: CC0/MIT-0 carry **no** attribution or copyleft
  obligations — safer than any derivative of the leaked RKnano SDK codec
  blobs (still proprietary; "modifying" them does not clean them;
  constitution §1 forbids third-party CFW code).
- **Layout** (`firmware/player/codecs/`): `wav.c`, `mp3.c`, `flac.c`
  (each a self-describing `codec_t` with its `extensions` key; mp3.c/flac.c
  are the single implementation TUs for their headers) + `codec_registry.c`
  (extension→codec dispatch extracted from player.c; `codec_registry.h`).
  Disable a codec by dropping its `.c` from `PLAYER_SRCS`/`AP_PLAYER_SRCS`.
- **mp3.c**: ID3v2 skip + ReplayGain, Xing/Info (frames/bytes/TOC), LAME
  gapless delay/padding (+528+1 adjustment), CBR duration fallback,
  1152-frame residual buffer, EOF frame salvage past ID3v1 junk, TOC seek.
  Non-obvious minimp3 facts learned the hard way: it requires frame+NEXT
  header in-buffer (tail frames must collect data, never be discarded), and
  the MPEG version mask is `0x18` (not `0x08`).
- **flac.c**: metadata pre-scan (RG via tags_replaygain_vorbis, SEEKTABLE
  budget), static 56 KiB pool allocator replacing DRFLAC_MALLOC/REALLOC/FREE,
  deterministic UNSUPPORTED for blocksize/seektable > pool, frame-accurate
  seek, gapless 0/0 (FLAC has no encoder delay in-stream).
- **RAM budget**: IPC ring 64K→**32 KiB** (`ipc_b.h`, ~186 ms — bump back
  only if underruns show), `PLAYER_CODEC_STATE_MAX` 4096→**7168** moved to
  codec.h (mp3dec_t is 6668 B), mp3 feed+residual are file-statics. ELF:
  text 99,600 / data 180 / bss 283,836, `_bss_end 0x03049570` ≤
  `__heap_end 0x0304C000` (linker-enforced, 10.6 KB margin).
- **Packer fix (tools/pack_fw1.py)**: some link layouts map the ELF header
  into the first PT_LOAD (file offset 0); that segment's "content" is the
  ELF headers, not firmware — it poisoned the 0x5B0 prefix calculation
  (assert fired) and would have copied headers over RAM 0x03004000. Now
  filtered via `is_header_shadow()` from payload chunks and the .data-image
  filter (kept in the rw math so row1's data+bss total stays exact).
- **Makefile fix**: `-include` deps now cover `build/cfw/*.d` (was bb+ap
  only) — a header change previously left a STALE 64 KiB ipc_b_target.o
  in a flashable image.
- Tests: `test_codec_mp3.c` (crafted ID3v2+Xing/LAME+silence frames: exact
  counts, gapless 576/623, TOC seek, RG e2e) and `test_codec_flac.c`
  (synthesized VERBATIM-subframe FLAC: LOSSLESS byte-for-byte golden,
  frame-accurate seek @2028, RG from VORBIS_COMMENT). Full suite:
  **ALL PLAYER TESTS PASSED** (11 binaries, ~340 assertions). M3 compile of
  every player TU clean; `make cfw-img` → RESULT: PASS, AP5M md5
  `5f2a55d301d8861711f762def778245b`, fw1 end 0x1932C, CRC 0x593367AE.
  (FLAC fixture pitfalls pinned in the test: subframe VERBATIM type is
  1 → header byte 0x02; rate code 1001=44100; the 16-bit blocksize is
  bitstream-ordered AFTER the coded frame number.)
- **DFIU**: preflight (alert: 2 blockers + 3 stale on the packer change) →
  byte-level revalidation of AP5M on all five (rows loaded-RAM, entry VT
  @IMG 0x19278 SP/reset/Thumb and not-0xD68, stack clean, row parity vs m5b,
  self-copy @0x44C000, CRC 0x500000 = 0x593367AE) → 3 effective passes
  recorded → R-70b34b8f2528 / R-5add7c251c3f / R-9f6e141101cb reactivated →
  `compile_artifacts`: **7 active rules**.
- NOT DONE: DAC renderer (still no sound — ipc_b_target doorbell is a
  placeholder), B-core IPC bring-up, and flashing this build to the device.

## 1. Where audio actually comes from

Two layers, strict separation:

- **B core (rendering):** built from the Rockchip RKnanoD SDK
  (`firmware/rockchip/audio/**`). Owns the DAC/I2S path and the PCM ring
  consumer. Hardware-close; changed last.
- **A core / AP (decode + tags + gapless + gain):** the clean-road player
  module (`firmware/player/**`). Decodes to interleaved int16 PCM via
  `codec_t`, pushes into the SPSC ring (`ipc_b`) consumed by the B core.
  The B core starts at 400 MHz and clock-negotiates to 100 MHz
  (facts in `docs/re/refcfw-analysis.md` and `docs/rewrite/player.md` §3).

The reference CFW's ECHOLOG proves the contract we must match:
  - `B: boundary to track N: FLAC 44100 Hz, ~5840 frames queued (gapless)`
  - `end of stream, 5843 frames queued`
  - `stat A: ... pos 462468 ms ... busy 1-28% ... gap <ms>` (the "gap" stat is
    the inter-frame scheduler slack, NOT an audible gap).

Interpretation for the clean-road player: gapless is a **decode + ring**
property, owned by the AP/player. The B core must not be restarted between
gapless-compatible tracks (a restart flushes the DAC and introduces a
click/silence). ReplayGain is an AP-side gain applied to PCM before it
enters the ring (or as the B-core DSP volume param — see §5).

## 2. The missing piece: codec_io over fs_target

`codec_io_stdio.c` is the only file backend today (host tests + newlib on
target). The target path is explicitly a TODO ("newlib syscalls route fopen
onto the storage driver"). That's the gap between "files are readable"
(SD browse works) and "audio decodes from SD."

Concrete next step: a target `codec_io_fs.c` backend mapping
`cio_open/read/seek/tell/size/close` onto the `fs.h` abstraction
(`fs_open_read/fs_read/fs_file_size/fs_close`). `fs.h` has **no seek**
yet — the first sub-task is to add `fs_seek(fs_file_t*, long offset)` to
`fs.h` + `fs_target.c` (set `f->pos`; the existing read walk already
re-derives the cluster from pos) + `fs_host.c` (fseek/ftell). Then
`codec_io_fs.c` and a `player.mk` switch (`PLAYER_CODEC_IO_IMPL`, default
stdio) so the AP links the FS backend while host tests keep stdio. A host
test decoding a WAV through the FS backend (backed by fs_host) proves the
seam without hardware.

## 3. Gapless playback contract

Design (host-testable on the mock codec path):

- `codec_t` gains an **optional** `gapless` query:
  `int (*gapless)(void *st, codec_gapless_t *out)` where
  `codec_gapless_t{int delay_samples, padding_samples}`. NULL = not gapless-aware
  (treat as 0/0: no trimming; a hard boundary still happens). delay_samples =
  encoder delay before the first audible sample; padding_samples = trailing
  padding to drop at end of stream.
  - MP3: from the Xing/Info header frame count + length (LAME `Info`/`Xing`
    carries total frames and a per-track byte layout; the pad/trim values
    come from the frame count vs the real sample count).
  - FLAC: STREAMINFO has no delay/padding; FLAC is naturally gapless, but
    `totalSamples` minus an embedded padding value (if a reference gapless
    tag exists) gives the trim. NULL when absent.
  - WAV: no gapless concept; NULL query -> 0/0.
- `codec_format_t` carries an exact `total_samples` (WAV: data size / frame;
  mock: cfg.total_frames) so the player's trailing trim ends at
  `total_samples - delay - padding` instead of an ms-truncated estimate.
- `track_info_t` carries `delay_samples`/`padding_samples` + `total_samples`.
- Player gapless advance: when the queue auto-advances and the next track's
  sample rate/channels match the current one, `advance_queue` calls
  `ipc_b_track_change(duration_ms)` and opens the next codec **without**
  `ipc_b_stop()`/`ipc_b_start()` — the ring and the B-core frame counter keep
  flowing (no DAC restart/click). Leading `delay_samples` are dropped from the
  first decoded batch; feeding stops once `total_samples - delay - padding` is
  emitted. If the format differs, fall back to a hard boundary (stop + start),
  which matches the reference's "boundary to track N: … frames queued (gapless)"
  without restarting the DAC. `ipc_b_track_change` extends the B-core's
  expected stream length and resumes RUNNING (no ring flush).
- Host test: `test_player_gapless` — leading/trailing trim on a mock codec
  (delay=100/padding=100 → exactly 43900 of 44100 frames) + a two-track
  same-format auto-advance asserting the B core is **not** restarted
  (`ipc_b_start_count` stays at 1) and no discontinuity in the sink.

## 4. ReplayGain tag compatibility

Compatibility (we read the same tags the reference uses), not a feature to
guess at:

- **MP3:** ID3v2 `TXXX` frames `REPLAYGAIN_TRACK_GAIN`,
  `REPLAYGAIN_TRACK_PEAK`, `REPLAYGAIN_ALBUM_GAIN`, `REPLAYGAIN_ALBUM_PEAK`.
- **FLAC:** Vorbis comments `REPLAYGAIN_TRACK_GAIN`, ..., `*_PEAK`
  (dr_flac surfaces the comment list; the adapter forwards it).
- **Opus/Vorbis (later):** same Vorbis comments.
- **WAV:** RIFF `iXML`/`bext`/`cues` — out of scope until a need shows up
  (no clean RG box; reject to `UNSUPPORTED` for RG rather than guess).

Implementation:
- New module `firmware/player/tags.c` + `tags.h`: the codec-facing parsers are
  `tags_replaygain_id3v2(codec_io_t *io, codec_replaygain_t *out)` (walks ID3v2
  v2.3/v2.4 `TXXX` frames — `REPLAYGAIN_TRACK_GAIN`, `*_PEAK`,
  `REPLAYGAIN_ALBUM_GAIN`, `*_ALBUM_PEAK` — case-insensitive) and
  `tags_replaygain_vorbis(codec_io_t *io, long offset, long length,
  codec_replaygain_t *out)` (Vorbis comments, case-insensitive keys). Both return
  0 if a field was filled, -1 if absent. Clean-room, no third-party code.
  Host-tested on synthetic blobs (ID3v2 `TXXX` + raw Vorbis comment).
- `codec_replaygain_t{track_gain_tenth_db, album_gain_tenth_db,
  track_peak_q15, album_peak_q15}` carried by `track_info_t`; a codec may expose
  them via the optional `codec_t.tags` callback (`codec_replaygain_t *out`),
  NULL if none.
- Gain math in `firmware/player/rg_math.c` (Q1.15, 1.0x = 32768):
  `rg_gain_q16(tenth_db)`, `rg_effective_gain_q16(tenth_db, peak_q15)`
  (peak-limited so `gain <= 32768/peak` when peak > 0), and
  `rg_scale_pcm(pcm, n_samples, gain_q16)` (saturating `>>15`).
- Player: `open_codec_only` calls `c->tags` and stores the fields on the track;
  `recalc_rg_gain` caches the effective Q16 gain; the PCM pump applies
  `rg_scale_pcm` to the decoded batch **before** `ipc_b_ring_fill`
  (AP-side pre-attenuation, composed with the B-core user volume). Mode via
  `player_set_replaygain_mode(PLAYER_RG_TRACK/ALBUM/OFF)` (defaults OFF; set
  before `player_open` to take effect for the track).
- Decision point: gain is applied AP-side (pre-ring) rather than through
  `dsp/mod_volume.c`, so the path is fully host-testable end-to-end without
  wiring the DSP chain into the player. Milestone 3a can fold it into the dsp
  chain; the math is isolated in `rg_math.c` so a no-libm target swap (fixed-point
  LUT) is a one-file change.
- Host tests: `test_rg_math` (dB→linear, peak clamp, scaling/saturation) and
  `test_player_replaygain` (exact PCM scaling vs `rg_scale_pcm` on the mock).

## 5. DFIU state (read this session)

Active rules (from `.dfiu/artifacts/active_rules.md`, 4 live invariants):
- `R-e9b5d8222673` [warning] mailbox reply: any reply (even DEC_OPEN_ERR)
  breaks the AP wait; silence is the only deadlock. — relevant: our player
  must always emit an ipc_b ACK/STOP, never leave the B core hanging.
- `R-e9e18de6c692` [blocker] fw1 scatter src addresses live in the loaded RAM
  image, not XIP; verify byte-for-byte against the reference before flash.
  — gates the eventual AP linkage of the player/codecs.
- `R-9461aeaecbe2` [blocker] pack_fw1.py safety gate; no scatter row below
  0x03000000..0x03004000; reference diff before any fw1 write.
- `R-1f1c5d829375` [blocker] pack_img.py: never zero-fill the stock
  section_3 tail; byte-diff it after every repack.

Per the in-doc DFIU routine (HANDOVER §"DFIU USAGE ROUTINE"): run
`preflight_check` with a ChangeSummaryV1 before every hardware variable
change, and `verify_resolution` with `method=reproduction` after every
hardware outcome — no verdict without hardware. The audio/SD/gapless/
ReplayGain changes below are **host-testable first**; they only become a
hardware verdict once linked into an fw1 image and flashed, at which point
the four rules above are the gate.

## 6. Immediate next steps (in order, cheapest + least brick risk first)

1. ✅ DONE — `fs.h`: `fs_seek` added; implemented in `fs_target.c` + `fs_host.c`.
   `fs_seek` is read-only (set `f->pos`); the existing read walk already
   re-derives the FAT/exFAT cluster from `pos`.
2. ✅ DONE — `firmware/player/codec_io_fs.c`: target backend over `fs_*` +
   `PLAYER_CODEC_IO_IMPL` in `player.mk` (default `codec_io_stdio.c`). Host test
   decodes a WAV through the FS backend (backed by fs_host), byte-exact vs the
   stdio WAV golden test.
3. ✅ DONE — Host test: WAV-through-FS byte-exact (see 2).
4. ⚠️ PARTIAL — `codec.h` has the optional `gapless` query (`codec_gapless_t`);
   `codec_wav.c` leaves it NULL (no gapless concept). `codec_mp3_stub.c`/
   `codec_flac_stub.c` are stubs; the real Xing/Info (MP3) and
   STREAMINFO (FLAC) delay/padding sources are documented in §3 but are filled in
   when the real codec adapters are vendored.
5. ✅ DONE — `player.c` gapless seamless advance on a same-format boundary; hard
   boundary on format change. Host test with the mock codec (`test_player_gapless`).
6. ✅ DONE — `firmware/player/tags.c` + `tags.h`: ReplayGain parse
   (ID3v2 `TXXX` + Vorbis comments), host-tested on synthetic blobs.
7. ✅ DONE — Player gain application: AP-side pre-attenuation in the PCM pump via
   `rg_math.c`, peak-limited; `PLAYER_RG_TRACK/ALBUM/OFF` mode. Host-tested
   (`test_rg_math`, `test_player_replaygain`).
8. ⏳ NEXT (hardware-gated milestone 2a) — link `firmware/player` into `ap.mk`
   (AP<->player<->codec<->IPC), guarded by the pack_fw1.py scatter gate, and
   re-run DFIU `preflight_check` before the flash. Also: fill in the real
   `codec_mp3`/`codec_flac` adapters (minimpp/dr_flac vendored) + the target
   `ipc_b_track_change` reply-wait (mailbox invariant `R-e9b5d8222673`).

# SESSION LOG (2026-10-13): DAC bring-up, a boot brick, and the CDC console

## A. DAC bring-up: fixed the I2C scan + I2S feed (no audio yet)

Brought up the CS43131 dual-DAC on hardware. Iterated via the on-screen
"DAC test" (menu index 0) register dump + bisect, all confirmed on-device:

- **I2C scan no-chip (rc -4, A00 B00 C00) fixed** (`fd62725`): wrong GRF mux
  offset (+0x200 phantom), GRF_IOMUX_CON at +0x04 not +0xBC, CRU gate ids
  pointing at reserved bits (real PCLK_I2C0=131/0x83), missing I2C0 soft-reset
  and the GPIO2 power/reset dance. Verified against ref fw1 (objdump) + SDK.
- **I2C START bit** (`ac8e345`) was the scan blocker: `i2c_write` never raised
  START, so no transfer began (every probe NAK/timeout). Ref FUN_03055dcc does
  CON=EN|START|TX, wait STARTIPD. After this: `rc 0 step 13`, chip 0 (0x60) ACKs.
- **I2S TX FIFO wait was inverted** (`a82d2a0`): waited while the FIFO was
  EMPTY (`==0`) instead of for room (`level<0x1e`, SDK I2s.c). Fix: tone feed
  now writes 132352 frames (was 0 + timeout).
- **CS43131 register sequence ported verbatim** from ref FUN_03056550
  (`b734fb5`): ~30 MAP writes (0x00020052, 0x00040010-19, 0x00050000/1,
  0x00090000/3/4, 0x000D0001, FUN_03055ecc per-chip L=7/R=15, amp GPIO(1,4,1))
  + the exact 0x00090003=0xEC. My earlier abbreviated list was wrong.

**Status 2026-10-13 — three root causes fixed, STILL SILENT on hardware
(unresolved)**: instrumented run
(`fifo 00-1d`, `clk 04`, `pdn f6`, `hp 10 10` = XTAL_READY_INT) proved the
whole chain live: I2S FIFO drains at audio rate, MCLK arrives (chip
reports XTAL ready), RCO-mode tone also silent. Byte-diff vs ref
FUN_03056550 tail found the missing piece: **FUN_03056c68(mask) — the PDN
power-up RMW** (clears `0x020000` bits on both chips). Stock parks the
chip powered DOWN (`0x020000=0xF6`) during config, then calls
`FUN_03056c68(0x40)` (PDN_ASP up) and `FUN_03056c68(0x10)` (PDN_HP up).
The port had misread both calls as `dac_delay_ms(64)`/`dac_delay_ms(16)`
(the `movs r0,#64/#16` masks!). Fixed: `dac_pdn_up()` + the missing
`0x010009=0xFF` pre-mute. `pdn` should now read `a6` after bring-up
(0xF6 & ~0x50). **Hardware test after all three fixes: STILL SILENT
(user report 2026-10-13) — unresolved.** Next session: capture the
post-fix `pdn`/`fifo` readbacks — if `pdn` is still `f6` the RMW isn't
landing; if `a6`, the fault is outside the register-visible path (I2S1A
pin wiring, amp GPIO1.4 polarity, jack/amp stage). Earlier verified
fixes kept: I2C read TRX sequence (5d8b7b3c resolved), TXDR=+0x24,
MSB-first MAP. **Audio WORKS and the DAC bring-up is CLOSED as of
2026-10-13** (DFIU 60609d7f verified_resolved): the missing piece was the
FUN_030563b0 output-enable (GPIO2.0 is a hardware mute, released only in
the jack-plugged branch), then two lifecycle bugs: test exit must run the
stock stop+close (`FUN_03055F80` + `FUN_030564d8` — hw mute + amp off +
**I2S1 soft-reset asserted** + pads Hi-Z) and `i2s1_setup` must cold-reset
the block before programming (2nd-run noise root cause). The RCO diagnostic
tone was removed (RCO = uncalibrated RC standby oscillator, noisy by
design). Hardware verdict: repeated clean 1 kHz tones, silence after every
exit.

## B. A boot brick (fixed by bisect) — NOT the DAC code

Flashing a DAC-fix IMG hard-bricked the device (black, no USB, no power;
stock reflash recovered it). Bisect of the 4 commits isolated it:
`9e97097` (DAC driver) boots, `fcd6b8c` (config + logging) bricks
deterministically (SHA256 0121a3c9ae). The DAC fix (`fd62725`) has byte-identical
.data/.bss to fcd6b8c — not the cause. Workaround: build the DAC fix WITHOUT
fcd6b8c (base 9e97097 + dac_cs43131.c/h) = `ReChord_AP5M_DACFIX*.IMG`, which
boots. fcd6b8c's boot bug (a .data/reloc collision) is still open before
config/logging can be re-added. **DFIU a14e948f / 66654652.**

**UPDATE (2026-10-13): `fcd6b8c` REVERTED from master** (`279a00e`) so HEAD is
bootable — `make cfw-ap && make cfw-img` from HEAD now builds a safe image
(verified: .data back to 180, no config/logging linked, row1 obj ~9e97097).
The config/logging work is preserved on branch **`feat/config-logging`**;
re-add it only after root-causing the boot bug (ap_startup.c never copies
.data, so fcd6b8c's first initialized .data static breaks the boot). **DFIU
cacf65c6.** Until then, `make cfw-img` from a HEAD that includes fcd6b8c
bricks — that is what hit the other agent's build.

Gotcha: when building from a git worktree, `git checkout HEAD -- <files>`
resolves HEAD to the WORKTREE's commit (9e97097), not the repo master — the
image silently built from the wrong (broken) driver. Always use an explicit
SHA in worktree builds. **DFIU c200c71d.**

## C. USB CDC-ACM debug console — PAUSED (device doesn't enumerate in console)

`firmware/drivers/usb_cdc_rknanoc.c`: a polled DWC2 CDC-ACM console (shares
usb_msc's bring-up via `usb_dwc2_init()`), "USB console" menu entry, live
on-screen enumeration diag (resets/setups/desc counts/last_setup/GINTSTS),
+ `r<map>`/`w<map><val>`/`s` commands for I2C. Debugged via the on-screen diag
+ sysfs raw descriptors. Confirmed fixes: EP0 OUT re-arm after every EP0 event
(host lost the GET_DESCRIPTOR config SETUP -> reset loop), SET_CONFIGURATION EP
activation (missing USBAEP/DAINTMSK/DCTL clear-NAK -> no interfaces). Two
wrong turns worth remembering: DTXFSTS(ep) is for dedicated FIFOs, EP0 space
is GNPTXSTS (DTXFSTS(0)=0 -> xhci "setup device command" timeout); and the EP0
FIFO is already big enough (NPTX 128B > 62B config) - a resize was a red herring.

**Current blocker — addressed 2026-10-13 (pending hardware verify)**: with
the device in "USB console", the host sees NOTHING (no 071b, no xhci
activity) - the CDC doesn't connect. Auto-MSC on cable plug hijacked the USB
core and mode switches never produced a host-visible disconnect (the
`DCTL.SftDiscon` pulse in `core_init()` was microseconds long and could be
cleared by the core soft reset itself, shorter than the host's ~100 ms
unplug debounce — the host kept the stale MSC device and never
re-enumerated). Fixed: `usb_dwc2_stop()` (soft-disconnect + 150 ms hold +
FIFO flush + PHY power-down) on every session exit, `core_init()` re-asserts
soft-disconnect and holds it 150 ms before connect, CDC SET_CONFIGURATION
allocates DIEPTXF1/DIEPTXF2 for EP1/EP2 in dedicated-FIFO mode, the VBUS
edge now opens the CDC console by default and MSC moved to a "USB storage"
menu entry (no more `usb_msc_run()` hijack). **DFIU 8720b71d.**

## D. Immediate next steps (this session's leftovers)

1. ~~Fix `dac_map_read` (I2C master-receive)~~ **done 2026-10-13** (ref
   FUN_03055fe8 TRX receive, dac.md §8b): verify on hardware that the dump
   shows real PWR/MUT/VOL values -> then diagnose no-sound from the screen
   (DFIU 5d8b7b3c).
2. Root-cause the fcd6b8c boot bug so config/logging can be re-added
   (DFIU a14e948f).
3. (Deferred) CDC-ACM console: close the console-mode enumeration
   (DFIU 8720b71d).

## E. Session 2026-10-13 late — DAC works, FLAC plays, but speed/pitch wrong

**Wins (all hardware-verified):** the DAC is alive end-to-end. The I2C
master-receive fix (ref FUN_03055fe8) made the register dump return real
values (`PWR 1F, MUT 00, VOL 01, CH0/CH1 07, OUT 01` — CS43131 configured,
both chips ACK `scan A11`, no mute) and audio plays. FLAC decoding is enabled:
`player_init()` was never called in the AP (codecs registered nowhere ->
"unsupported format" for everything); now wired at AP startup. `fs_seek` reset
the cluster pointer to 0 instead of `first` (broke every FLAC with `-2` after
the first metadata seek); fixed. Oversized FLAC SEEKTABLEs (~95KB) blew the
56KB pool (`-3`); now dropped (>16KB) — dr_flac seeks fine without them. Error
codes are surfaced in the UI for future diagnosis.

**Current blocker — playback speed/pitch WRONG + stutter.** Music plays but at
the wrong rate and it stutters ("sample, silence, sample"). Two separate causes
identified:

- **Clock divisor — RESOLVED.** Compared `dac_cs43131.c` against the Rockchip
  SDK (`I2s.c I2sSetSampleRate` + `rockcodec.c ACodec_PLL_Set`). Finding: the
  **actual MCLK is exact**, so the "MCLK differs from the rate-table" worry
  (dac.md §9) is a red herring. `acodec_mux[]` is byte-exact with the SDK's
  `ACodecpllTable_*`, and the four `CRU_BASE+0x20` writes are `CLKSEL_CON[3]` =
  `SetI2SFreq(CLK_ACODEC_PLL)` (sel=ACODEC, /5) + `SetI2SClkOutFreq(I2S1_CLK)`:
  for 44100 the mux `{6,36,25,3}` == `ACodecpllTable_56448[F_SOURCE_24000KHz]`
  (= 56.448 MHz) -> MCLK = 56.448 MHz / 5 = 11.2896 MHz = **exactly 256*44100**.
  The CKR formula was 1.3x-wrong (fixed `0x3F3F`) and was corrected by 741f15c
  to derive TMD/TSD from `mclk/rate`. The *residual* ~0.4-0.8% sharp is NOT an
  MCLK mismatch: the ref's recovered `mclk` for the 44.1 kHz family sits just
  *under* 256x/128x rate (44100/88200 -> 11288960 vs true 11289600; 176400 ->
  22577664 vs 22579200), so **truncating** `mclk/rate` yielded TSD=255/127 instead
  of 256/128 -> Fs ~= 44273 (0.4% fast) / 88896 (0.8% fast), heard as "faster,
  high-pitched." **Fix applied:** round the ratio (`(mclk + rate/2)/rate`) in
  `i2s1_setup` so TSD lands on 256/128 (verified for all 13 table entries).
  Compile-checked (`arm-none-eabi-gcc` AP flags). Still hardware-untested (no
  rig here) — confirm by ear / sample-rate counter.
- **Streaming is LINEAR, not parallel (stutter — NOT yet fixed).** decode ->
  `dac_i2s_feed` (blocks) -> next, on one core; no DMA/IRQ double-buffer. The
  I2S FIFO is tiny (~0.7ms = 32 stereo frames), so the multi-ms decode burst in
  `player_process` (`PLAYER_PUMP_FRAMES=512`) drains it to empty mid-decode ->
  "sample, silence, sample". (The blocking feed also pins the UI thread while
  playing.) Fix = IRQ/DMA-driven TX feeding from the PCM ring so decode and
  playback overlap. **Verified facts for the port (not yet wired):**
  `INT_ID_I2S1 = 32` (vector slot 48; `NVIC_ISER1` bit 0 — see
  `community/sdks/.../I2C/I2C interrupt.c` `IntRegister` usage + the
  `firmware/bb_stub/bb_stub.c` NVIC/vector pattern); I2S `INTCR` bit 0 =
  TX-empty interrupt enable, `INTSR` bit 0 = TX-empty status (auto-clears on a
  `TXDR` write per the SDK `I2sIntIsr`); the 32 KB `g_ring` (`ringbuf.c`,
  SPSC + `DMB`, ISR-safe) + `ipc_b`'s async-consumer contract (mirrored by
  `ipc_b_mock.c`, so `player.c` is already written for it). The AP vector
  table (`ap_vectors[16]`, `firmware/startup/ap_startup.c`) holds only the 16
  Cortex-M core exceptions today — wiring IRQ 32 needs extending it to slot 48
  and enabling `NVIC_ISER1`. Two routes: **(A) lighter** — TX-empty ISR drains
  `g_ring`->TXDR (keeps the polled `dac_i2s_feed` for the `DAC test` tone);
  **(B) ref-true** — port the ref DMA block (`0x03090000`, channel 4, 8-entry
  descriptor ring via `FUN_03055b0c`) per `docs/re/dac-output-path.md`'s
  `AudioDmaIsrHandler -> AudioDecoding -> DMA -> I2S` pipeline. **Defer until
  hardware-verified**: this is a verified-boot-vector + live-audio-ISR change
  (gapless/EOS tail-drain edge case); do not ship blind. **Safe probe (no
  hardware vectoring): shrink `PLAYER_PUMP_FRAMES` (player.c, currently 512) to
  e.g. 64 so each decode burst fits inside the ~0.7ms FIFO life, then read the
  existing `s_fifo_lo`/`s_fifo_hi` diag — if `s_fifo_lo` stops hitting 0 the
  stutter recedes (decode-speed dependent; not a real fix).

DFIU: 158f67d3 (I2C START), 0b6a53e3 (I2S FIFO), 5d8b7b3c (read/no-sound),
cf? (FLAC open), plus this speed/stutter entry.

## F. Session 2026-10-16 — speed/pitch root cause found (CKR frame) + streaming

**Hardware report that started this:** FLAC playback "stuttering @ a high
speed and a high pitch" even after the CKR-rounding fix. REA (Ghidra) on
`community/refcfw/ghidra_input/ref_fw1_ram0304F490.bin` settled it — the
stock's `FUN_03056550` (audiocodec open) writes the I2S regs **verbatim**:

- `TXCR = RXCR = 0x401F` (HWT=1, VDW=32, CSR=ch0 pair). **Wire format
  (settled 2026-10-16 pm, from the stock's DMA payload — Service.c
  `Bit_Convertor_DEC`/`Bit_Convertor_shift` + the CS43131 5050/RES=32
  slots): ONE 32-bit TXDR word = ONE channel sample, left-justified
  (`sample << 16`); a stereo frame is TWO words, L then R, one per
  32-BCLK slot of the 64-BCLK frame.** The interim belief "one word = a
  packed L|R stereo pair" (from the SDK `TXCR_HWT_Bit32` comment) was
  WRONG and produced the last hardware symptom: each DAC chip then
  received every other packed word = the *other* channel's samples at
  2x the content rate -> "2x speed + 2x pitch + stutter", with the
  progress bar counting words at exactly 2x (stock ratio: 2 words/frame).
  The feed and ISR now expand each frame to `L<<16`, `R<<16`.
- `CKR = ((mclk/(rate<<6))-1)<<16 | 0x3F3F` — **TSD=RSD=64 fixed**, the
  whole ratio in MDIV (8/4/2). Frame = 64 BCLK = 2x32-bit slots and
  SCLK = 64*Fs. **This was THE bug**: our TSD=256 split gave 256 BCLK per
  LRCK, so the CS43131's 64-BCLK frame counter ticked at SCLK/64 = **4x
  the file rate** — fast + high pitch — and the FIFO drained 4x faster
  than the player fed it (the stutter). `i2s1_setup` now copies the stock
  formula/values.
- Rate table @ RAM 0x030609E8 byte-read: the "just under 256x" mclk
  constants were **transcription errors**. True words: 44100/88200 ->
  **11289600**, 176400 -> **22579200** (exact multiples; all 13 entries
  match). Table + comments corrected; the "round the ratio" workaround
  story is retired (see dac.md §9).

**Stutter part 2 (decode vs FIFO) — fixed with route A (ISR streaming).**
The linear decode -> blocking-feed pipeline is gone: decoded PCM lands in
the 32 KiB SPSC ring and `dac_i2s_tx_isr` (I2S1 TX-empty/threshold IRQ =
`INT_ID_I2S1` = NVIC IRQn 32 = **vector slot 48**, `NVIC_ISER1` bit 0)
drains it into TXDR while the main loop decodes — the same streaming shape
as the stock's DMA ring. Wired in:
- `ap_startup.c`: `ap_vectors[49]` (196 B, still inside the 256-B VTOR
  window); slot 48 = `dac_i2s_tx_isr`, everything else Default_Handler
  (reset leaves all NVIC lines disabled, so only this IRQ can fire).
- `dac_cs43131.c`: `dac_i2s_attach/detach/kick/hold` + the ISR (pull
  callback keeps the driver ring-agnostic; masks its own IRQ source when
  the ring is dry and re-arms on `kick`). Polled `dac_i2s_feed` kept for
  the DAC tone test.
- `ipc_b_target.c`: the real ring is the sink now — `ipc_b_ring_fill`
  rb_writes + kicks, `ipc_b_poll_state` reports the mock's byte-counter
  contract (`ring_rd/wr` free-running bytes; `frames_consumed` = frames
  the ISR drained). This also un-breaks two things the old stop-gap had
  wrong: `dac_i2s_feed` returns `DAC_OK` (0) so `filled` was always 0 and
  the player's emitted-sample bookkeeping rolled back every batch; and the
  `ring_wr == ring_rd` EOS check could never fire with the +512 "free
  space" hack (tracks never ended). The `IPC_B_PUMP_FRAMES_HINT` hack is
  deleted.

**Verified:** player host tests ALL PASS (mock contract unchanged), AP
cross-build clean (`make cfw-ap`), vector table objdump-checked (slot 48 =
`dac_i2s_tx_isr|1`), `make cfw-img` packs `build/ReChord_AP5M.IMG`
(verification PASS). NOT yet hardware-tested.

**On hardware, check in this order:**
1. Flash `build/ReChord_AP5M.IMG` (the ONLY safe image; a standalone fw1
   has no USB/updater -> maskrom, docs/DEBRICK-LINUX.md).
2. DAC test tone first (isolates the polled path from the ISR path): pitch
   of the 1 kHz sine is the frequency reference the old work lacked.
3. Play a known 44.1 kHz FLAC: tempo and pitch must be natural; the
   progress bar must move at wall-clock speed; the track must END and
   advance. Then 48 kHz + a high-rate (88.2/96k) file — each rate has its
   own CKR/PLL entry now byte-exact.
4. If something hangs at track start, suspect the IRQ wiring first (the
   vector table moved): the safe rollback is to stop calling
   `dac_i2s_attach()` in `ipc_b_start` — playback degrades to the old
   blocking feed but keeps the corrected clock/format.

DFIU: 75271e5f (this entry — VERIFIED RESOLVED 2026-10-16: user flash-tested
commit c5d571f, "FLAC seems to playback correctly"; root cause + resolution
confirmed in the incident record).

## G. Session 2026-10-16 pm — modular audio pipeline refactor (structure only)

Applied the approved plan (`~/.commandcode/plans/audio-pipeline-modular-
refactor.md`) with two binding constraints: vector ISR symbol integrity and
1:1 verbatim moves of all hardware code.

**Layout now** (was one monolithic `drivers/dac_cs43131.c`):

- `drivers/ao_regs.h` — all MMIO defines (verbatim; the mechanism that
  keeps register lines textually identical across the split).
- `drivers/rknano_i2s.{c,h}` — CRU/GPIO/GRF primitives + ACODEC PLL mux +
  I2S1 setup + TX feeder (polled + TX-empty IRQ). ISR symbol is now
  **`rknano_i2s_tx_isr`** (was dac_i2s_tx_isr).
- `drivers/cs43131.{c,h}` — I2C0 master + MAP ops + chip sequences +
  diag state (names kept from c5d571f so moved call sites stay identical).
- `audio/audio_output.{c,h}` — the controller (`audio_format_t`,
  init/configure/start/stop/hold/kick/frames_played/feed_blocking/
  power_off); configure() runs the moved `dac_init` body line-for-line.
- `audio/audio_hal.h` — `ao_slot_word` spec + `ao_feeder_ops` seam
  (bound to the ring-IRQ feeder; DMA ping-pong drops in later).
- `audio/ao_rate_table.{c,h}` — the 13-entry table (pure, host-tested).
- `player/codec.h` — additive `init()` hook + canonical vtable mapping
  docs; Opus = reserved drop-in slot (player.md).

**Verification (all machine-checked this session):**
- Host: ALL PLAYER TESTS PASSED incl. new `test_ao_slots` (slot packing +
  the 13 mclk/CKR constants that caused the 2x bug class).
- `make cfw-ap` clean; `make cfw-img` -> `RESULT: PASS`, AP5M CRC
  0xD1017450.
- **Vector integrity (constraint 1):** nm/objdump proof on the linked ELF:
  `ap_vectors` slot 48 = `0x030552c9` = `rknano_i2s_tx_isr|1`
  (`0x030552c8`), zero `dac_i2s_*` symbols remain.
- **Verbatim move (constraint 2):** machine diff of every
  register/mask/shift/constant statement in c5d571f vs the new modules:
  101/101 present unchanged (only `static` qualifiers dropped for the
  cross-module helpers; one stale forward decl removed).

**Hardware checklist (user, after flashing build/ReChord_AP5M.IMG):**
1. DAC test tone — same pitch as the c5d571f build (no clock drift).
2. FLAC playback still correct (DFIU 75271e5f regression guard).
3. Pause/resume + track end/advance still work.

## H. Session 2026-10-16 pm2 — LOW VOLUME root cause + ReplayGain hardening

**Volume (DFIU 19b83f5e — "extremely quiet" fix).** The port mapped the
stock's volume writer `FUN_03055f28` to MAP `0x010009`. REA + the CS43131
datasheet (`community/CS43131_DS1155F2.pdf`) settled it:

- `FUN_03055f28(v)` actually writes **`0x090001` + `0x090002`** =
  `PCM_VOLUME_B/A` (0.5 dB/step attenuation; datasheet register map p.128).
- Those registers **reset to 0x78 = -60.0 dB** and our firmware never
  programmed them -> whisper-quiet playback. Datasheet-recommended init
  (and the stock's fade target) is 0x00 = **0 dB**.
- `0x01 0007–0x01 000A` is **Reserved** — the old writes went into a void.

Fix: `cs43131.c` gained `dac_volume_set()` (= FUN_03055f28, both regs) and
`dac_volume_ramp()` (= FUN_03055f48, ±6 codes / 2 ms soft ramp). The
bring-up now mutes via `dac_volume_set(0xFF)` and unmutes with
`dac_volume_ramp(0xFF, 0x00)` (0 dB default); `dac_output_off` mutes the
same way. All three sites were the same misattribution.

Checklist results: (a) digital attenuation — was the bug, now 0 dB;
(b) analog/HPA stage — verified stock-parity (`HP Output Control 1` 0x10,
`Class H` HV_EN 0x1F, amp GPIO1.4; datasheet's "0-dB analog gain" config);
(c) PCM scale — verified full-scale: `sample<<16` is byte-identical to the
stock `Bit_Convertor_DEC/shift` output.

**ReplayGain.** Architecture already matched the request (tags.c parsers ->
codec_t.tags -> player fixed Q16 gain applied pre-ring with peak-based
clipping prevention; modes OFF/TRACK/ALBUM, default OFF). Completed:
- `rg_math.c` is now **pure fixed-point** (no floats/libm/malloc): integer
  dB + 0.1 dB Q1.15 LUTs, exact at 0 / ±20 / -60 dB (test-pinned), clamped
  -100..+24.9 dB.
- Per-codec audit: mp3 ✓ (ID3v2 TXXX), flac ✓ (Vorbis comments), wav =
  documented N/A (no RG source in plain WAV), Opus = reserved drop-in slot.
- `cs43131` now exposes `dac_volume_set/ramp` (0.5 dB hardware attenuation)
  for future coarse user-volume mapping; RG itself stays DSP-scaled
  (finer + peak-protected). I2S wire format and ISR feeder untouched.

Verified: all host tests pass (rg/replaygain/tags/ao included), `make
cfw-ap` clean, `make cfw-img` RESULT: PASS (CRC 0x896AEAAB). Hardware
check: music is at normal level; RG-tagged files track-neutral by default
(RG still opt-in); DFIU 19b83f5e to verify after the user listens.

## I. Session 2026-10-16 pm3 — phantom keys / UI instability during playback

**Symptom:** volume-button presses caused phantom navigation (pop to main
menu, jump into "Test DAC") — only while a track played.

**Root cause (all three confirmed against the stock key scanner
`FUN_03057eb0` via REA):**

1. **Stale ADC reads on bursts.** `keys_adc()` (panel_io_rknanoc.c) copied
   the reference's "read previous conversion, start next" pipeline — but
   the reference ONLY works because its scan is rate-gated to 10 ms. Our
   playing loop calls `input_scan()` up to 8x per tick (player_process
   returns instantly once the ring is full), so each read hit ADC[0]
   MID-conversion and restarted the converter: the stale value was stable
   across the burst and mapped into wrong key bands (vol -> Menu/Select/
   Back). This is why only playback mode misbehaved.
2. **1-sample debounce.** `input_scan()` accepted a key after 2 equal
   samples; the stock requires **3 identical consecutive scans (8 for
   vol+-)** — one transient sample was enough to register here.
3. **No IRQ priority policy.** I2S1 TX IRQ ran at default (highest)
   priority.

**Fixes (targeted; I2S wire format + ISR feeder untouched):**

- `keys_adc()` now takes **three fresh, fully-settled conversions
  (~50 us each) and returns the median** — every value is a completed
  conversion, single-shot noise cannot pick a band.
- `input_scan()` debounce: N consecutive identical samples before a press
  registers (`INPUT_DEBOUNCE_SCANS` = 3, `INPUT_DEBOUNCE_SCANS_VOL` = 8,
  input.h) — matches FUN_03057eb0; release also debounced, holds do not
  repeat, band-sweeping bounce can never reach N equal samples.
- `rknano_i2s_attach()` sets IRQ32 NVIC priority to **lowest** (0xE0);
  input is polled main-loop work so it cannot be starved, and the 32 KiB
  ring absorbs any ISR latency (no dropout risk).

**UI stack audit:** `ui.c` bounds were already correct (push refuses
NULL/full, pop reports empty, tests pin it). The phantoms came from bad
EVENTS, not stack handling — those events now die at the input layer.

**Test harness revival:** the UI suite had no runner (it bit-rotted: one
stale text assertion). Added `firmware/ui/tests/run_tests.sh`, link-stub
diag globals in mock_services (types match flac.c), and aligned one stale
assertion (browser shows the diagnostic code "cannot open (-4)", the
deliberate behavior). Now: services 167 checks + player + UI 262 checks
all pass; `make cfw-ap` clean; `make cfw-img` RESULT: PASS (CRC
0x5D5F2DA6). Hardware check: volume keys act as volume under playback;
no phantom menu/Select navigation.

## J. Session 2026-10-16 pm4 — user volume wired to hardware attenuation

**Symptom:** volume up/down on Now Playing moved the bar but never changed
the loudness.

**Root cause:** the UI -> player chain worked (`player_screen` calls
`player_set_volume()` -> `ipc_b_set_volume()`), but the sink only wrote
`volume_db` into the IPC shared block — nothing applied it. (The
player_screen header's "mirrors to the audio service later" comment was
stale; the break was downstream of the player.)

**Fix:** `audio_output_set_volume_db()` maps dB -> `PCM_VOLUME` codes
(0.5 dB/step attenuation, 0x00 = 0 dB max, positive dB clamps at 0 dB)
and applies them with the stock's click-free soft ramp — this is exactly
the stock's volume setter (`FUN_03055f48(0xff, DAT_03004158)` = ramp to
the user level). `ipc_b_set_volume()` (sink side, per the shared-block
contract "volume currently applied by the B side") calls it; the level is
kept in `s_volume_atten` so `configure()` relocks to it (unmute ramps
0xFF -> user level, stock semantics).

Verified: player/services/UI suites pass, `make cfw-ap` clean, `make
cfw-img` RESULT: PASS (CRC 0xB0A8CFD9). Hardware check: volume +/- on Now
Playing changes loudness stepwise without clicks; level survives track
changes.

## K. Session 2026-10-16 pm5 — button round 2: ladder transients + power hold

Hardware feedback after session I: buttons "a bit better" but volume-down
on Now Playing still popped to Main Menu, and power-off was unreliable
while playing.

**Root causes (both behavioral, confirmed against the stock key scan
`FUN_03057eb0` and the RKNanoD TRM SARADC chapter):**

1. **Ladder transients.** Pressing/releasing a resistor-ladder key SWEEPS
   the ADC line through the Menu/Back bands (the RC curve dwells in the
   middle of the range). N-sample debounce alone still accepts a dwelling
   transient — which is exactly what the stock defeats by classifying
   with the **pressed-bottom minimum** (`if (v < min) min = v`, classify
   by min) plus a latched re-arm on release. A single-band phantom on
   release was the "vol- -> Main Menu" (KEY_BACK/KEY_MENU) bug.
2. **ADC reads without end-of-conversion.** The TRM procedure is "start,
   poll SARADC_STAS/CTRL EOC, read SARADC_DATA" (STAS bit0 = conversion
   in progress; CTRL bit6 = EOC, write-0-to-clear). Fixed in
   `keys_adc()` (bounded EOC poll per conversion + median-of-3).
3. **Power-hold counter resets on one bad sample.** `s_power_held = power
   ? n+1 : 0` + exact `== 700` — contact bounce or a single glitched
   GPIO2.5 read restarted the ~2 s count, so power-off never completed
   under load.

**Fixes:**

- `input_scan()`: classify by the run **minimum** (16-count hysteresis so
  noise cannot ratchet it into a lower band); release re-arms the run;
  N-sample stability retained (3 / 8 for vol+-). Press AND release sweeps
  are now unregistrable — tests pin both directions.
- `keys_adc()`: TRM end-of-conversion polling (bounded) per conversion.
- `power_hold_step()` (ap_ui): 3-sample debounced power level, `>= 700`
  threshold — bounce can no longer reset the hold.

Note: `board_power_off()` cuts the PMIC rail (GPIO2.6); **on USB/VBUS the
device stays powered by the host** (documented in the code) — power-off
tests should run on battery.

Verified: all suites pass (services 144 checks incl. sweep/noise cases),
`make cfw-ap` clean, `make cfw-img` RESULT: PASS (CRC 0xAA858C7D).
Hardware check: vol- on Now Playing lowers volume (no menu pop); a held
power key turns the device off within ~2 s during playback.

## L. Session 2026-10-16 pm6 — hardware-volume spec reconciliation

Task spec asked to route `audio_output_set_volume_db()` to "Master Volume
(0x010009/0x01000A)" at "-95.5 dB full scale" with soft ramp enabled.
The volume path already existed (session J) — this pass reconciled the
spec against the hardware evidence. Two spec corrections, both with
datasheet/REF backing (community/CS43131_DS1155F2.pdf):

1. **Registers are `PCM_VOLUME_B/A` = 0x090001/0x090002**, not
   0x010009/0x01000A. The datasheet register map marks **0x01 0007–
   0x01 000A Reserved**; the ref's volume writer (`FUN_03055f28`) writes
   0x90001/0x90002; and writing the reserved MAP was the original
   "extremely quiet" bug (DFIU 19b83f5e: real regs stuck at their 0x78 =
   -60 dB reset default). No change made — the spec addresses are unsafe.
2. **Step is 0.5 dB/step with range 0x00 = 0 dB .. 0xFF = -127.5 dB**
   (datasheet volume decode: "from 0 to -127.5 dB … 0000 0001 = -0.5 dB").
   The "-95.5 dB" figure is the internal RockCodec's 0.375 dB/step spec
   (SDK volume_control.c), not the CS43131. Mapping unchanged:
   `code = -2 * db`, positive dB clamps at 0 (no digital boost).

Confirmed requirements already satisfied:
- **Soft ramp / zero-cross:** `PCM_SZC = 10` ("Enable soft ramp") in the
  stock `0x09 0003 = 0xEC`; "volume changes are dictated by PCM_SZC" —
  the hardware ramps internally, and our `dac_volume_ramp` (FUN_03055f48
  port) additionally sequences the target level. Click-free without
  per-sample CPU work.
- **No software PCM scaling for volume:** `dsp_mod_volume` is linked in
  no playback path (B-core DSP module, referenced only by its own host
  tests). The only PCM scaling in the AP path is ReplayGain
  (`rg_scale_pcm`, opt-in, default OFF) — exactly the requested policy.
- **Integer conversion:** factored to the pure, test-pinned
  `ao_volume_atten_code()` (audio_hal.h); `test_ao_slots` covers 0 dB,
  -1/-20/-60 dB, +20 dB clamp, mute floor.
- **Integration:** player.c/UI -> `player_set_volume` -> `ipc_b_set_volume`
  (sink side) -> `audio_output_set_volume_db` -> `dac_volume_ramp` ->
  I2C; abstractions unchanged.

All suites pass; `make cfw-img` RESULT: PASS (CRC unchanged, no wire-
format touched).

## N. Session 2026-10-16 pm8 — 367066f regression (UI softlock) + follow-ups

**DFIU ae88365a (buttons dead / softlock).** Hardware report after
367066f: vol+- no longer navigates the UI (menus unreachable — softlock),
power-off does not trigger, USB Storage unreachable. Root cause is the
keys_adc end-of-conversion poll added in that commit: the TRM sets
`int_status` (CTRL bit6) at end of conversion **only with `int_en` (bit5)
enabled**, and the code polled bit6 (plus STAS bit0) without ever setting
int_en — so every conversion burned the full 200000-iteration poll
timeout before the settle fallback. At the ~24 MHz boot clock that is
tens of ms per conversion, ~10 s worst case per keys_adc: the 8-sample
vol debounce never completed in usable time and the 700-pass power-hold
needed ~a minute.

**Fix (input task):** enable `int_en` at SOC; EOC poll demoted to a SHORT
bounded fast path (4000 iterations) with an always-there ~200 us settle
floor (>= the <=1 MHz SARADC's conversion time), so a read is <= ~1 ms
regardless of flag semantics — the UI can never crawl on this path again.
`POWER_HOLD_COUNT` re-tuned 700 -> 400 for the faster pass rate (~2 s).

**Follow-up 1 (USB):** the VBUS/cable edge now opens **USB storage**
(MSC, `usb_storage_run`) instead of the CDC console — the pre-CDC
behaviour. The CDC console stays on the "USB console" menu entry.

**BRICK + SPLIT (DFIU fa3968cd, 2026-10-16 late):** the 451be2b-era
build (player-state persistence + FAT write layer) **bricked the device**
— the same pattern as the old `feat/config-logging` branch (fcd6b8c,
DFIU cacf65c6/a14e948f). Per the user's decision the whole
config/persistence feature (player_state.{c,h}, fs_target FAT writes,
sd_write, its tests and docs) is now on branch **`feat/config-persistence`**
and POPPED from main; everything else (audio fixes, both input fixes, the
USB-storage-on-cable fix) stays on main. Main rebuilt clean: all suites
pass, `make cfw-img` RESULT: PASS (CRC 0xB89A8063) — this is the image
to flash back to.

Root cause of the brick is still OPEN (analysis happens on the branch —
do not merge it until the brick is explained). Prime suspects: the FAT
write path on real media (sd_write first use / root dirent or FAT
corruption halting mount loops), or RAM/link pressure from the new
statics (the config-logging brick died on the FW_RAM window).

**Playback polish (DFIU fa3968cd resolved; UI lag + track-change click):**
the popped build boots and the buttons work (A/B-confirmed in DFIU
fa3968cd — the config/persistence feature set caused the brick; it lives
on `feat/config-persistence` and must not return until the mechanism is
explained there). Two follow-up fixes on main:

1. **UI lag while music plays** — the playing loop still ran 8
   `keys_adc()` + `input_scan()` per pass (a leftover from when
   `player_process()` blocked ~90 ms). With the ring+IRQ feeder it is
   non-blocking, so that was 8 SARADC scans + 8 debounce samples per
   pass of pure latency. Keys/power are sampled once per pass (the outer
   loop already did); the pump loop pumps only.
2. **Crackle on track/item change** — stream teardown was raw (no mute
   fade) and every change re-ran the FULL bring-up (I2S reset + ~200 ms
   of init delays) even for same-format tracks. Now the stock's
   transition pattern is in place: `audio_output_fade_out()` /
   `fade_in()` (= FUN_03055f48 / FUN_03056e54 ramp-to-mute before
   stop/pause, back to the user level on resume), and
   `audio_output_configure()` takes the stock FUN_03056550 early-exit
   for same-format re-opens (registers untouched — format changes still
   run the full verified sequence). `audio_output_set_volume_db()` also
   caches pre-DAC volume (UI volume before the first track would i2c-
   wait-spike otherwise).

Both verified host-side (player + services + UI suites pass; `make
cfw-img` RESULT: PASS).

**Hold-to-repeat + short/long press model (input):** holding vol up/down
now continues the action (volume in the player, selection in menus) — no
more spam-clicking. The event model distinguishes a SHORT press from a
LONG one explicitly (`input_event_t.kind` + `long_press`):
PRESS (one per short press) -> LONG (once, at INPUT_LONG_SCANS) ->
REPEAT (every INPUT_REPEAT_SCANS while held; KEY_UP/KEY_DOWN only) ->
 RELEASE{long_press=0|1}. The UI acts on PRESS/REPEAT; LONG/RELEASE are
markers for consumers that need the distinction. Timing is scan-count
based (INPUT_LONG_SCANS=40, INPUT_REPEAT_SCANS=10 ~= 0.4 s / 0.1 s at
the ~10 ms UI pass cadence). test_input grew hold coverage (220 checks).
Note: this supersedes the old "hold: no repeat" contract in the input
tests; playback/UI confirmed crack-free on the previous build
(CRC 0xC0ACE53A).

## O. Session 2026-10-17 — Now Playing timestamp/bar: 32-bit `long` time math

**Symptom** (Linear BRNOMT-65 + BRNOMT-66, both filed from device testing
of the 4d15172-era build): most songs showed a duration of a few seconds on
the Now Playing screen ("2 or 19 seconds"), playback itself fine; the
progress bar misbehaved because of it (it is drawn from position/duration).

**Root cause:** the target's `long` is 32 bits (Cortex-M3 — the host test
suite runs 64-bit `long`, which is why it always passed), and every
millisecond conversion was written with a 32-bit product:

| Site | Expression | Wraps above |
|---|---|---|
| `codecs/mp3.c` Xing duration | `total_samples * 1000 / rate` | ~48.7 s @ 44.1 kHz |
| `codecs/mp3.c` CBR `total_samples` | `rate * duration_ms / 1000` | ~48.7 s |
| `codecs/flac.c` duration | `total_samples * 1000 / rate` | ~48.7 s |
| `codecs/flac.c` seek frame | `ms * rate / 1000` | ~48.7 s |
| `player.c` playhead | `frames_consumed * 1000 / sample_rate` | ~48.7 s |

The wrapped product lands negative (the player clamps the length to 0, so
the screen reads "0:00") or a small positive value — exactly the observed
few-second lengths. Both are display-only: nothing on the audio path uses
`duration_ms` (the sink streams the ring; the gapless trim uses the codec's
`total_samples`, and the player re-derives that from `duration_ms` only when
the codec reports <= 0). The playhead wrap was the bar half of the bug: past
~49 s it went negative -> huge elapsed time + bar at full.

**Diagnostic shortcut** — what the bug displayed for a 44.1 kHz track
(target wrap + the player's negative clamp):

```
3:17 -> 0:02     3:30 -> 0:15     4:00 -> 0:45
3:34 -> 0:19     3:45 -> 0:30     4:55 -> 0:02
```

**Fix:** `int64_t` intermediate at all five sites. `codecs/wav.c` already
did this ("int64 math: ms * rate overflows 32-bit `long` on the target") —
mp3/flac/player.c now match. No behavior change for values that never
wrapped.

**Verification (target toolchain, not the host — the host cannot show it):**

1. Probe with `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os`:
   `(2304000L * 1000) / 44100` warns *"integer overflow in expression of
   type 'long int' results in '-1990967296'"* and the assert on it fails —
   with the int64 form all seven fixed expressions assert clean.
2. New host fixtures past the wrap point: mp3 3000-frame Xing file
   (78367 ms), flac 5,000,000-frame STREAMINFO (113378 ms), player playhead
   on a 5-minute mock track (reaches 50 s sane instead of wrapping).
3. All suites green (player, ui 262, drivers, modules, services 220);
   `make cfw-ap` + `make cfw-img` RESULT: PASS — 5 MB image
   `build/ReChord_AP5M.IMG`, CRC `0x4D1ED427` (previous build:
   `0xC0ACE53A`).
4. Codegen A/B on the shipped objects: `smull` (32x32->64) in
   `player_get_position_ms` 0 -> 1, `mp3_open` 0 -> 2, `flac_open` 0 -> 1 —
   the pre-fix image did the whole conversion in 32-bit registers.

**Rule for the next codec/player/DSP change:** every ms <-> samples
conversion on this target needs the `int64_t` intermediate, and a >48 s
fixture in the test whenever a new one is added (docs/rewrite/player.md §4).

## P. Session 2026-10-17 (cont.) — hold-to-repeat acceleration (BRNOMT-80)

**Request:** the hold-repeat shipped in `4d15172` works, but "if held 1
second more it could navigate even faster through the UI".

**Design:** the repeat is now a *ladder* of (extra-hold, interval) steps in
`input.c`, with the knobs in `input.h`, and the phase tracked by
`s_scan_repeat` instead of a modulo of the hold counter — so a step change
can only make the next REPEAT come earlier, never fire twice at once:

| tier | engages after | REPEAT interval | rate @ ~50 ms/pass |
|---|---|---|---|
| 1 | LONG (`INPUT_LONG_SCANS` 20, ~1 s) | every 6 scans | ~3 steps/s |
| 2 | +`INPUT_FAST_HOLD_SCANS` (20, ~1 s) | every 2 scans | ~10 steps/s |

No consumer change: the fast tier emits the same `INPUT_REPEAT` events, so
menu/browser/settings/player are untouched (they act on PRESS/REPEAT). A
third tier is one more row in `s_repeat_ladder`.

**Verification:** `test_input` grew ladder coverage (199 checks, 0
failures): two tier-1 intervals' worth of scans at tier 1 yield 2 repeats,
the same window at tier 2 yields 6; SELECT held past the tier boundary
still emits PRESS + LONG only (no key but vol+/vol- repeats); a short tap
after a long hold starts from a clean phase (counter reset on
press/release). All suites green; `make cfw-ap` + `make cfw-img` RESULT:
PASS — AP5M CRC `0xE4E4FD01`.

**Tuning note (corrected 2026-10-17 after the hardware test).** The tiers
are counts of *scans*, and a scan is one UI pass; the first cut assumed
~10 ms/pass. The device says otherwise: holding vol+/- in a listing showed
no acceleration at all. A cursor move in a full-screen list calls
`ui_invalidate_all()`, so every step repaints and flushes the whole
320×170 panel — and the flush alone is ~217,600 `bus_write` byte writes
over the GPIO panel bus (~45 ms) before draw time. At ~50 ms/pass the old
knobs put the fast tier ~7 s into the hold: unreachable in practice. The
knobs above are tuned for the ~50 ms/pass case. This is the fragility of
counting passes instead of time: timing in ms via a clock injected into
the input service is the planned follow-up, and the player screen (band
partial redraws) runs at a different cadence than lists do.

## R. Session 2026-10-17 (cont.) — AP RAM ceiling found: 0x0304A000 (explains both bricks)

**Finding.** Cross-build `_bss_end` (`arm-none-eabi-nm`) on every AP build
we have:

| build | `_bss_end` | bss | outcome |
|---|---|---|---|
| `a1c3a9f` duration/bar fix | `0x03049F7C` | 286,400 | **hardware-verified good** |
| `021513b` / `7dab023` (master) | `0x03049F84` | 286,408 | untested, size-neutral |
| `41667f7` tag metadata (BRNOMT-78 pt.1) | `0x0304A694` | 288,216 | **brick** |
| `451be2b` config/persistence (parked) | `0x0304A8AC` | 288,752 | **brick** |

The two bricked builds are exactly the ones that cross **`0x0304A000`**; every
build that boots stays below it. Image geometry, scatter tables and packer
gates were identical/valid in all four cases, and neither bricked feature
runs at boot (tag reads only on track open / browser open; the parked FAT
layer only on a save) — so the shared signature is the **`.bss` end**, not
the feature code. Corollary: the parked `feat/config-persistence` FAT write
layer may be innocent (fa3968cd blamed it with the mechanism unconfirmed).

**Fix.** `firmware_ap.ld`: FW_RAM `LENGTH 0x48000 -> 0x46000` (upper bound
`0x0304A000`) + the SYS_DATA ASSERT text. Verified by building: master links
(`0x03049F84`); `41667f7` (`0x0304A694`) and `451be2b` (`0x0304A8AC`) are now
**refused by the linker** ("SYS_DATA overflow ... below 0x0304A000").

**Consequence — the AP RAM budget is effectively exhausted** (132 B of slack
below the gate). Any further feature must reclaim RAM first. Candidates with
mutually-exclusive lifetimes: the 56 KiB FLAC pool, the 32 KiB USB-MSC SCSI
chunk, the 2×8 KiB `fw_update` staging — a union would free tens of KiB
(playback is stopped before an MSC session / update anyway). Do not raise the
ceiling: the gate is the evidence.

**DFIU:** incident `fb41cb15` + rule `R-8cd5fb0eb24a` (hypothesis until a
device flash confirms builds under the gate); the older, vaguer envelope rule
`R-c423c45206d8` is deprecated.



## S. Session 2026-10-17 (cont.) — settings persistence on the eMMC (BRNOMT-71)

**Where:** `/.rechord/rechord.cfg` on the **internal eMMC volume** (7 GB
FAT32, user-visible from the PC; `lsblk` shows sda1 covering the whole disk,
so there is no unallocated tail to use). Plain text, 12-key schema, 64 KiB
allocation so it can grow. The device only ever rewrites the **data sectors
of that existing file** and patches its dirent size — no FAT-table writes, no
allocation, no directory creation. The write class that bricked earlier
builds stays off this path (§R / DFIU fb41cb15).

**New primitives (`fs_target`):**
- `fs_target_set_writer(idx, wr)` — optional per-volume block writer (NULL =
  read-only). The AP wires it to `emmc_write` right after mounting Internal;
  `emmc_write` itself refuses anything below `s_user` (IDB/firmware area).
- `fs_target_file_rewrite(path, data, len)` — FAT16/FAT32: pre-flight the
  allocation (FAT slack allows growth within the chain; exFAT NoFatChain runs
  follow the size), write the data (zero-padded), patch the 4-byte dirent
  size, drop the sector cache. Refuses exFAT (its size is covered by the
  entry-set checksum).
- **Settings split:** `settings.c` = parser/serializer/defaults;
  `settings_fs.c` = host whole-file I/O (moved); `settings_store.c` = target
  read + in-place rewrite. The 2 KiB read buffer left the target with it.

**Verification (host, real filesystems):** `tools/test_fs_target.py` now
exercises the rewrite against kernel-filled FAT16 / FAT32 (MBR) / exFAT
images and requires `fsck` clean. It caught three real bugs before any device
test:
1. the write path passed **volume-relative** LBAs to the absolute block
   writer (invisible on superfloppy images, wrong behind an MBR — and the
   device's volume starts at `s_user`, so it would have failed on hardware);
2. an oversize rewrite **partially overwrote** the file before discovering
   the allocation was too small -> pre-flight added;
3. exFAT's size patch invalidated the **entry-set checksum** (fsck:
   "checksum of a file is wrong") -> exFAT refused instead of corrupted.
`test_settings_store.c` covers the target glue (defaults/parse/hand-off/error
mapping).

**Budget:** the split *freed* 2 KiB: bss 286,408 -> **284,376 B**,
`_bss_end 0x03049F84 -> 0x03049794` (2,156 B more margin under the
0x0304A000 gate). text +460 B. All suites green; `make cfw-img` RESULT: PASS
— AP5M CRC `0xD9CDCE35`.

**Device check (user):** flash, change a setting (e.g. volume), power-cycle ->
the value persists and `rechord.cfg` on the internal drive shows the new text
at exactly the text length. Do **not** run a filesystem *repair* on that
volume: the preallocated slack is intentional (fsck would truncate it).

**Note:** growth is bounded by the file's cluster chain (64 KiB here). For
more, recreate the file larger; a first-boot init (folder + files created by
a verified FAT allocator) is still future work.

## T. Session 2026-10-17 (cont.) — first hardware test of §P/§S: two dead paths fixed

**Test result on `0xD9CDCE35` (the §S build):** boots; FLAC + bar + volume
keys fine; USB read/write fine; **settings changes do not stick** (change
`play`/`brightness`, BACK, value back to the default); **hold-repeat does
not accelerate** in a listing. Both are fixed here.

**T1 — settings never reached the service (the adapters were stubs).**
`settings_view_load/save` were `__attribute__((weak))` stubs in
`ap_main.c` returning -1: the settings screen asked the "storage" and was
told "none", so it fell back to defaults, and `settings_save()` was never
called — the §S backend was dead code on the device. The real glue now
lives in `firmware/ui/settings_view.c` (its own TU, because the UI host
tests substitute those two symbols in `mock_services.c`; the target picks
it up via `ap.mk`/`ui.mk`). **Lesson: a weak stub links fine and silently
disables a feature — when something "doesn't persist", check the symbol
actually resolves (`arm-none-eabi-nm` shows `W`), don't assume the
backend is at fault.**

**T2 — `screen_off`/`auto_off` could never round-trip.** The schema has
them as ENUM keys (option values 0/10/30/60/120 and 0/5/15/30/60) while the
store keeps them as plain ints, and `to_store()` routed any ENUM key
through the text path — so the two timers were written as the *first*
option's text and the file never matched what the screen showed. The
`from_store()` side also scaled minutes→seconds (`*60`, and `/60` going
back) from a stale header comment; the same number is used on both sides.
Fixed in `ui/settings_schema.c`: both keys copy the int across, and
`from_store` validates the stored number against the option list (an
unknown value keeps the default). `settings_view_defaults()` now zeroes the
whole `browse` buffer (no stale tail), and the header/`integration.md`
units comment is corrected. New host binary
`ui/tests/test_settings_view.c` covers the glue over the real converters
(text↔enum, timers, defaults on load failure, NULL args) — it is what
caught T2. Suite: 23 checks OK.

**T3 — input ladder retuned for the real pass cadence** (see §P's corrected
tuning note): `LONG` 40→20, `REPEAT` 10→6, `FAST_HOLD` 100→20,
`FAST_REPEAT` 4→2 scans, i.e. ~1 s to LONG, ~0.3 s/step, fast tier ~1 s
later, ~0.1 s/step — all at the measured ~50 ms/pass of a listing.

**Build:** `_bss_end 0x03049794`, bss 284,376 B (unchanged: the adapter
uses stack), text 110,816 B. All suites green; `make cfw-img` RESULT: PASS
— AP5M CRC `0x41FE103E`.

**Device check (user), in order:** (1) Settings → change `play` and
`brightness` → BACK (saves on exit) → power-cycle → values persist, and
`/.rechord/rechord.cfg` is ~200 bytes of plain text on the PC; do it twice
to prove the second save works after the shrink. (2) Hold vol+/- in a long
listing: ~1 s to the first step, ~0.3 s/step, and after ~1 s more it should
visibly speed up to ~0.1 s/step. (3) Do **not** run a filesystem repair on
the internal volume (intentional slack).

## U. Session 2026-10-17 (cont.) — settings land on hardware; hold ladder moves to (injected) time

**Device result on `0x41FE103E` (§T):** settings now persist — change →
BACK → power off → power on → the value is still there, and
`/.rechord/rechord.cfg` reads back as clean text (`play=shuffle
brightness=3 ...`, 12 keys, no padding). T1/T2 are settled. The hold ladder
*still* showed no acceleration, so §T3's scan-count retune was the wrong
tool: it only moved the same fragility around.

**T4 — the ladder runs on milliseconds now, not UI passes.** The pass rate
is repaint-bound and unknowable at compile time (a listing pass repaints and
flushes the whole panel: ~217,600 `bus_write` byte writes, ~75-90 ms at
24 MHz, plus draw), so any count-of-scans tuning is a guess per screen. New
shape:

- `input_set_clock(unsigned (*ms)(void))` installs a millisecond source.
  `ap_ui.c` provides it: the Cortex-M3 **DWT cycle counter**
  (DEMCR.TRCENA + DWT_CTRL.CYCCNTENA, `CYCCNT / 24,000` = ms at the ~24 MHz
  AP core clock — the same frequency `panel_io_delay_ms` assumes; no PLL is
  set up in this image). Not SysTick: no interrupt or vector slot needed.
- The ladder is in ms: `INPUT_LONG_MS` 1000, `INPUT_REPEAT_MS` 250,
  `INPUT_FAST_HOLD_MS` 800, `INPUT_FAST_REPEAT_MS` 50 (one step per pass —
  the fastest the UI can move). `s_press_ms` / `s_last_repeat_ms` /
  `s_long_fired` replace the scan counters; the release's `long_press` flag
  is that same `s_long_fired`.
- **Safety net:** a clock that never advances (3 equal readings) is dropped
  and the service falls back to a synthetic per-scan clock
  (`INPUT_FALLBACK_MS_PER_SCAN` 80), so a dead clock degrades the feel
  instead of killing repeats — the failure mode that cost two rounds here.
- Tests: the input suite now drives a fake 50 ms/sample clock (divides every
  constant), plus a dedicated no-clock and frozen-clock fallback test:
  199 checks, 0 failures.

**T5 — the theming half of the same stub bug.** `fopen/fgets/fgetc/feof/
fclose` were the last weak stdio stubs in `ap_main.c`: file themes were
silently inert. Real path: `firmware/libc/stdio_fs.c` (same subset over
`services/fs.h`, read-only, 2-stream static pool; target-only, so not in
`libc.mk`). New `theme_boot_load()` in `theme_file.c` loads
`THEME_FILE_PATH` = `/.rechord/theme.cfg` at boot over the active built-in,
`theme_validate()`s and adopts only if sane; `ap_ui_loop()` calls it after
`ui_init()` and before the first push. Host theme test grew the
missing-file case (must report the miss and leave the theme untouched): all
PASSED. Format + key list in docs/rewrite/customization.md §3.1.

**Build:** `_bss_end 0x030497c4` (284,420 B; +48 B for the stream pool),
text 111,356 B — still ~2.1 KB under the 0x0304A000 gate. All host suites
green; `make cfw-img` RESULT: PASS — AP5M CRC `0xE6B7A5D0`, sha256
`c72631cf27fb0939c0380f749cc60f3142da033b1b920f7f741ef82e720cd8be`.

**Device check (user):** (1) hold vol+/- in a long listing: ~1 s to the
first step, ~0.25 s/step, then after ~1 s more, one step per frame (~10-20/s
— a blur, not a crawl); (2) create `/.rechord/theme.cfg` with e.g.
`accent=0xF800` + `background=0x0000`, power-cycle and check the UI picks
the colors up (a broken file must leave the UI unchanged); (3) the settings
round-trip again, as a regression.

## V. Session 2026-10-17 (cont.) — volume is now runtime state; where the file-creation limit really is

**Device report on `0xE6B7A5D0`:** no acceleration yet (user deprioritised
it: "nvm it's not important right now"), **volume did not persist**, and
neither did the last track / playback position. Plus the key question: why
can't the firmware create its own files (`.cfg`, the DB)?

**V1 — volume persistence.** The volume is owned by the *player module*
while playing (`player_screen` calls `player_set_volume()`), and nothing ever
wrote it into the settings view, so the `.cfg` kept whatever value it had.
The settings screen now owns that runtime state:

- `settings_screen_volume_changed(db, now_ms)` adopts the value into the
  view and **arms** a write (`SETTINGS_FLUSH_QUIET_MS` 800 ms);
  `settings_screen_flush_pending(now_ms)` performs it once quiet, so a
  volume sweep is one `settings_save()` instead of one per step;
  `settings_screen_store()` covers a pending flush (BACK, power-off).
- `ap_ui_loop()` applies the saved volume at boot
  (`player_set_volume(settings_screen_get()->volume_db)`) and watches
  `player_get_volume()` each pass, feeding changes to the screen with
  `ap_ms()`. The power-hold path saves before `power_off()`.
- Test caught a bug in the first cut: `store()` cleared the deadline but not
  the pending flag, so a later flush wrote the same values twice. Now it
  clears both (test_settings_keys +7 checks).

**V2 — why files can't be created (yet), and why that is now a smaller
question than it looks.** The device can only rewrite data sectors of a file
that already exists (`fs_target_file_rewrite`, device-proven by this very
`.cfg`). Creating anything needs the *allocation* path: a free-cluster
search, a FAT chain written to **both** FATs, a dirent (and a new cluster for
a directory). That path was deliberately never enabled because the project
bricked twice while it was being developed — but §R/§T settled that: **the
bricks were the `.bss` ceiling at 0x0304A000, not the FAT code** (that code
never ran at boot; the linker now makes the failure impossible). So:

- risk is down to ordinary FAT correctness, which is *testable on the host*:
  `tools/test_fs_target.py` already validates writes against kernel-made
  FAT16/FAT32/exFAT images with `fsck` — the allocator gets the same
  treatment before it ever runs on the device;
- flashing it is an A/B step of its own (bss gate + a fresh image);
- it unblocks: first-boot `/.rechord/` init, a state file for resume, and the
  BRNOMT-74 index.

**V3 — resume (last track + position) is a feature, not a bug fix.** Nothing
records the current track/position today; `resume` in the schema only says
*what* to resume. Two designs, to be chosen before implementing:
(a) two extra keys in the existing `.cfg` (`track=`, `pos_ms=`) — no new
file, no allocator, but the config stops being the reference's 12-key layout;
(b) a separate `/.rechord/state` written the same in-place way — clean
separation of settings (user) from state (runtime), needs the allocator from
V2 first, and is the same file the index work will want.

**Build:** `_bss_end 0x030497d4` (284,436 B), text 111,572 B — ~2.1 KB under
the gate. All host suites green; `make cfw-img` RESULT: PASS — AP5M CRC
`0x29C435B6`, sha256
`5cea1b9b63d4b7838d14c0c18c11c949c10239ade2759a155871015fd13a52ea`.

## W. Session 2026-10-17 (cont.) — the device creates its own files (the allocation path)

**Request:** "reChord automatically creates the files and folders; if they
already exist it'll just write over, such as settings" — i.e. first-boot
`/.rechord/` + cfg + state, and resume tracked as a Linear feature with the
separate-state-file design (option B).

**The missing piece was allocation**, and §R/§T removed the reason it had
stayed off (the bricks were the `.bss` ceiling, not this code). New
primitives in `fs_target.c` (FAT16/FAT32; exFAT and read-only volumes are
refused, as everywhere else):

- `fs_target_mkdir(path)` and `fs_target_file_create(path, bytes)`, plus the
  internals they need: a sector write-through that invalidates the read
  cache, a FAT entry written to **both** FAT copies (a diverged mirror is
  silent corruption for every other tool), a free-cluster search that starts
  from FAT32's FSInfo hint (Linux keeps it valid) with a wrap-around scan and
  an FSInfo free-count/hint update, a contiguous directory-slot finder that
  extends chained directories by a cluster when full, and a dirent writer
  (plain 8.3 with the NT case flags, or a long-name set with the generated
  `BASE~1` short name).
- **Ordering rule** (same as the device-proven rewrite): data first, the
  directory entry LAST. An interrupted create leaks clusters — which fsck
  recovers — instead of leaving an entry pointing at nothing.

**`rechord_files.c`** is the policy layer: `rechord_files_init()` creates
`/.rechord`, writes the default configuration into a fresh `rechord.cfg`
(64 KiB allocated, sized to the default text) and creates `state` (4 KiB).
Both are idempotent — an existing file is left exactly as it is — so it runs
on every boot; `ap_ui` calls it right after mounting the internal volume
(and after installing the block writer), guarded by `s_user`.

**Harness (`tools/test_fs_target.py`, `test_fs_target.c` `create` mode):**
creates the tree on FRESH kernel-made FAT16 (superfloppy) and FAT32
(behind an MBR) images, then checks the kernel's view (mount + `os.listdir` +
the cfg's text and the state's 4096 zeros), the idempotent second pass, the
refusals (a directory is not a file; non-ASCII names), and 20 extra files
that force the chained-directory extension path. `fsck.fat -n` must report
no structural error — the one expected finding is the cfg's deliberate slack
(allocated 64 KiB, size = the text). Three real bugs it caught in the new
code, all of which would have corrupted data on the device:

1. **`fat_next()` cannot answer "is this cluster free?"** It returns 0 both
   for "end of chain" and for any value outside the cluster range — and in a
   FAT16 volume the EOC marker (0xFFFF) is exactly that. An allocation scan
   built on it reads every allocated cluster as free and hands out clusters
   already in use. New `fat_raw()` reads the entry without the range check;
   free is raw 0, and a read error reports "used" so a failure can never
   allocate something.
2. The generated long-name set was malformed twice over: the LFN checksum
   accumulator was not truncated to 8 bits per step (the reader then rejects
   the whole set), and the entry's first-cluster field carried `0xFFFF` from
   the padding instead of 0 (`fsck`: "Start cluster field in VFAT long
   filename slot is not 0").
3. The lower-case flags were written base→byte 12 / ext→byte 13. Both belong
   in byte 12 (bit 3 base, bit 4 extension); byte 13 is the created-time
   tenths — so `rechord.cfg` displayed as `rechord.CFG`.
4. The new directory's `..` pointed at the **root's cluster** on FAT32 (whose
   root has a real cluster number) instead of the required 0 (`fsck`:
   "Invalid '..' entry in the second slot").
5. FAT32's FSInfo **free count** was decremented per allocation, which drifts
   from reality (`fsck`: "Free cluster summary wrong (128883 vs. really
   129010)"). It is now set to "unknown" (0xFFFFFFFF, always valid) — this
   code cannot know the true count without a full FAT scan — while the
   allocation hint still advances, which is what keeps the next allocation
   fast.

The harness also grew `timeout=` on its `udisksctl` calls: those hang
occasionally in a desktop session (a stuck `loop-delete` wedges the suite).

**Verified on the host:** both filesystems come back clean — FAT16
(superfloppy) and FAT32 (superfloppy + behind an MBR, i.e. through the
volume-base offset the device needs): every create returns 0, the second
pass is idempotent (`0 0 0`), a directory is refused as a file (`-2`),
non-ASCII names are refused (`-4`), the cfg reads back as exactly its 220
text bytes and `state` as 4096 zeros, the kernel sees all 22 entries with
the right names, the FAT is sound (`.` → the new cluster, `..` → 0 at the
root, the directory's second cluster after it fills, empty files allocate
nothing) and `fsck` reports nothing but the deliberate cfg slack.
**Device A/B:** the next flash is the first build that writes FAT metadata;
if it fails, it can only leak clusters (fsck-recoverable) and the previous
image stays in `build/`.

**Follow-up:** `fs_target_file_rewrite()` still refuses to grow a file beyond
its allocation (the pre-flight returns `FS_ERR_UNSUPPORTED`). Now that the
allocator exists, appending clusters there would let the cfg be created at
exactly its text size (no slack) and grow on demand; until then the cfg is
created preallocated at 64 KiB, which is what the device does today.

**Build:** `_bss_end 0x030497e4` (284,452 B), text 114,560 B — the gate has
~2.1 KB of headroom left, so the allocator fits comfortably. AP5M CRC
`0x3A766B50`, sha256
`83afe185ec760ca58d8bbb28271ea497e694f8a6b620ab4ef1d65f1fd78aecbc`.

## X. Session 2026-10-17 (cont.) — the device's own log file

**Request:** the config + logging work was "casi": what was missing is a log
file in `/.rechord` (and `state`/`LOG` read as blocks of zeros, i.e. as
binaries, which is unhelpful for a file the user is meant to read).

- `rechord_files_init()` now creates `/.rechord/LOG` (32 KiB) and writes a
  short text header into every freshly created `state`/`LOG`, so both read as
  text from the PC. The device replaces the content on its next write.
- `log.h` / `log.c`: `LOG_PATH` moved to `/Internal/.rechord/LOG`, and
  `log_flush()` gained an optional whole-file writer (`log_set_writer`). With
  one installed, the target assembles the pending ring and rewrites the file
  in ONE call (`fs_target_file_rewrite` — this backend has no append, so the
  file always holds the latest flush); without one the stdio append path
  stays for the host tests (35 checks, unchanged apart from the path).
- The target's ring is 12 lines (`-DLOG_BUFFER_LINES=12` in the Makefile's
  AP_CFLAGS, which is where the effective flags live — ap.mk's copy is not
  what compiles the image) so the ring plus the assembly buffer fit: that
  **freed 1.2 KiB** of `.bss` (`_bss_end 0x030497e4 → 0x03049324`, ~3.2 KB of
  headroom now). A flag change needs a clean object dir: make does not track
  flags, and the stale `log.o` overflowed the gate on the first try (the
  linker refused the build, exactly as designed).
- Flush points: entering USB storage (the menu entry *and* the cable edge),
  so the log is readable the moment the user plugs in, and before the
  power-hold shutdown. `ap_ui` also feeds the DWT clock to `log_init()`, so
  lines carry real timestamps instead of `0.000`.
- Harness: the create phase covers `LOG` too (four-way idempotency); FAT16
  and FAT32 both stay `fsck`-clean.

**Build:** `_bss_end 0x03049324` (283,236 B), text 115,276 B. All host suites
green; `make cfw-img` RESULT: PASS — AP5M CRC `0x38394815`, sha256
`fc0b7aa6ac09a095064fd4783208edf4fac45895a6e768f753cefee631ffbc99`.

**Device check (user):** boot once (LOG appears with its 3 text lines), then
plug the USB cable in and open `/.rechord/LOG`: it should hold the lines of
the latest flush with real timestamps, not zeros.

**Result on hardware:** confirmed — "LOG funciona" (BRNOMT-88 closed). The
file is created, reads as text and is refreshed at the flush points.

## Y. Session 2026-10-17 (cont.) — BRNOMT-74 step 1: the append primitive

**Why:** the media index is built incrementally and cannot be held in RAM, so
it needs to write past the end of a file — and `fs_target_file_rewrite()`
deliberately refuses to grow (`FS_ERR_UNSUPPORTED`). This is that piece.

- `fs_target_file_append(path, data, len)`: appends at the current size,
  extending the cluster chain (allocating and linking as needed), writing the
  data, and patching the dirent LAST (the commit). The sector the append
  point falls into is read first so the bytes before it survive, and every
  sector the data only partly fills is zero-padded — the result is byte-exact
  for [0, old_size + len). `len == 0` is a successful no-op. FAT16/FAT32;
  exFAT and read-only volumes refused.
- A file with **no clusters yet** (size 0, e.g. a freshly created index) gets
  its first cluster written into the dirent by the same commit — the first
  cut forgot this, and the harness caught it immediately: the chain stayed
  orphaned and fsck reported "Reclaimed 6 unused clusters (3072 bytes)" for a
  file whose chain each append had silently started over.

**Verified (host, both filesystems):** create-empty → append 5 B → append
600 B twice → the file reads back byte-exact (1205 B, FNV checked), the
kernel agrees through a real mount, `fsck.fat -n` reports nothing unexpected
(no reclaimed clusters), and the refusals behave (missing file / a directory
→ FS_ERR_OPEN, zero length → FS_OK).

**Build:** `_bss_end 0x03049324` (unchanged: the append adds no statics),
text 115,432 B; `make cfw-img` RESULT: PASS — AP5M CRC `0xE0394429`, sha256
`b0794bf3e2373ca43e7dfc0d3d769323602edb6402e7e8ff32d3f38b668bfba3`.
All host suites green.

## Z. Session 2026-10-17 (cont.) — the shared scratch region (BRNOMT-74 step 2)

**Finding that changed the plan:** the approved reclaim was "union the FLAC
pool (56 KiB) ∪ the USB-MSC chunk (32 KiB) ∪ the fw_update staging (2×8 KiB)".
Measuring the real statics (`arm-none-eabi-nm -S --size-sort` on the AP ELF)
shows the union does not have to touch the audio path at all: the MSC chunk
(32 KiB, a function-local static in `scsi()`) alone is **enough scratch for
the index**, so the FLAC pool stays exactly where it is and the audio path is
untouched. Zero risk where it matters, and 32 KiB is plenty for a chunked
build plus the unique-name lists (with documented caps).

- New `firmware/services/scratch.{h,c}`: one 8-byte-aligned 32 KiB region and
  a single-owner claim model (`scratch_claim/release/owner/for`). A claim
  lives exactly as long as the operation that took it, a non-owner cannot
  release it, and nothing survives a reboot — so a bug cannot wedge the
  region permanently.
- The USB mass-storage session claims it for the whole session (`scsi()` reads
  the data buffer from it; a NULL there fails the command instead of
  dereferencing) and releases it on its single exit path. A claim failure is
  reported as the new `USB_EXIT_BUSY` (it cannot happen today: the only other
  planned holder, an index build, blocks the UI loop).
- Host test `test_scratch` covers claim/taken/re-claim/non-owner release/
  handover/alignment. Host suites green.

**Budget:** `s_scratch` (32 KiB) replaces the old `buf.0` (32 KiB) — the .bss
is unchanged (`_bss_end 0x03049324 → 0x0304932c`, +8 bytes for the owner
word). The point is not the saving but that the index now has a buffer.
**Build:** AP5M CRC `0xE3950635`.

**Device check (user):** USB storage still mounts and copies (same buffer,
new home) and ejects normally — a regression check, nothing new to see.

## AA. Session 2026-10-17 (cont.) — USB session forensics (host filed a crash report)

**Report:** after the scratch change the device boots and USB mass storage
works (file copied and read back — the regression check passed), but the
user's Ubuntu file manager (Nautilus) **freezes and crashes** when the device
is plugged in. Unknown whether it is the host's fault.

**Review of our side (no smoking gun):** the BOT path chunks READ/WRITE by
`CHUNK_SECTORS` and by the endpoint max packet, reports the residue correctly
in the CSW, answers unknown opcodes with CHECK CONDITION, refuses EVPD INQUIRY,
and answers a zero-length LUN (no SD card) with "medium not present" instead
of a bogus capacity. Two soft spots remain, both only on error paths: a failed
bulk transfer returns without a CSW (the host then waits for its timeout), and
a card removed mid-session would leave LUN 1's advertised size stale.

**What was added:** every session now writes one forensic line to
`/.rechord/LOG` when it ends —
`usb: exit <code> diag <last-opcode/LUN> rd <sectors> wr <sectors>` — and
flushes it (the flush happens after the session, when the volume is free
again). That is the device's half of the story; the host's half is `dmesg -w`
and `journalctl -f` while plugging in. One reproduction with those two
side by side decides whether the freeze is ours or the desktop stack's.

**Note for context:** `udisksctl` hangs repeatedly on this desktop *with loop
devices, unrelated to the device* (several in this session), so udisksd/GVFS
— the same stack Nautilus drives — is a live suspect.

**Build:** `_bss_end 0x0304932c` (unchanged), AP5M CRC `0xD46C21D7`, sha256
`2da04860969c649431b1a1dd9acf8aa6ce21ff243b3a01019b599e006cc53ff6`.

## AB. Session 2026-10-17 (cont.) — BRNOMT-74 step 4: the index core

**Format (see media_db.h):** two slots, one file each (`/.rechord/INDEX.A`,
`INDEX.B`), each holding `[512-byte header][fixed 184-byte records]`. A record
is the volume-relative path (80), title (32), artist (24), album (24), genre
(16), track number and duration — small by design: this is a browse index, not
a tag database. Record `i` sits at a computed offset, so paging needs no
parsing.

**Commit story:** a build targets the slot that is **not current**, truncates
it, appends the records in chunks (through the shared 32 KiB scratch, so RAM is
bounded), and finally patches the header in with `fs_patch()` — one sector, the
commit. An interrupted build therefore leaves the previous index untouched; the
reader picks the valid header with the highest sequence number, and the header's
CRC (over the records, verified on open) catches a damaged index instead of
serving garbage.

**Primitives this needed** (`fs_append`, `fs_patch`, `fs_truncate`, all with
host backends so the index is host-testable). The target harness caught a real
bug in the first cut of the range writer: it zero-padded the rest of every
partially filled sector, right for an append but destructive for a patch.

**Done:** the core builds, commits, reads back and flips slots on rebuild; a
corrupt newest slot falls back to the older one, both corrupt means "no index".
`test_media_db` (25 checks) covers all of it. `rechord_files_init()` now also
creates the two (empty) slots so a build always has a target.

**Not yet (next):** the scanner (walk the volumes, read tags, feed
`media_db_add`) and the "Rebuild index" menu entry with progress — until then
the index exists but nothing fills it.

**Build:** `_bss_end 0x030493ac` (283,372 B), text 119,660 B; AP5M CRC
`0x1D28DDCD`, sha256
`5782a2574a1f5499732c16a0af194fe46c9922d09c0273025ce95d48fa96680f`.

## AC. Session 2026-10-17 (cont.) — BRNOMT-74 step 4 closed: the scanner + the menu

- `media_scan.c`: walks the mounted volumes page by page (`fs_list_dir`), takes
  `.mp3/.flac/.wav` by extension, reads the container tags through the player's
  reader (`tags_read_file`, header region only) and feeds one fixed record per
  file to `media_db`. The title falls back to the filename with the extension
  stripped; paths that would not fit the record are skipped (a truncated path
  is useless as a key); depth is capped; the progress callback runs every 8
  files and can abort.
- `media_db_abort()`: an abandoned build releases the scratch and commits
  nothing, so the previous index stays current (the build always targets the
  slot that is not in use).
- Root menu: **"Rebuild index"** (progress screen: file count + current name,
  power key aborts) and an **"Index"** row showing the track count of the
  current index (read once, cached).
- The host backend's `fs_list_dir()` was not mapping paths like the rest of
  `fs_host.c` (it did a raw `opendir`), which the scanner exposed: fixed.
- `test_media_scan` (12 checks): the walk, the filename fallback, non-audio
  files ignored, and an aborted scan leaving the previous index current.
  `test_media_db` 25 checks. All suites green.

**Build:** `_bss_end 0x030493d4` (283,412 B), text 124,012 B; AP5M CRC
`0x03027491`, sha256
`e0639bcb9f4879eead427d015e0ea27d00afbd817bb536253394beeec21c2f74`.

**Device test:** "Rebuild index" on the root menu (fill Internal/SD with audio
first), then the "Index" row should show the track count; `INDEX.A`/`INDEX.B`
in `/.rechord` are the two slots (one is current, the other is the next
target). The views that read it are BRNOMT-78 / step 5.

## AD. Session 2026-10-17 (cont.) — BRNOMT-78 first view: All Tracks

- `ui/screens/library_screen.c`: a paged view over the media index. One page of
  records (8) is fetched when the cursor moves — never in draw, the same rule
  the browser follows — and each row shows the tag title with the artist on the
  right. SELECT opens the track (`player_open` + `player_play`), BACK pops.
  With no index it says "no index: Rebuild index" and **never** falls back to
  scanning: that is the point of having one.
- Root menu: "Library" (14 rows now).
- The page window is 8 x 184 B = 1.5 KiB of .bss on purpose: 16 records left
  only 156 B of margin under the 0x0304A000 gate (the linker would have
  refused the *next* static), 8 leaves 1.6 KiB.
- UI test mocks for the index (empty state), suites green.

**Build:** `_bss_end 0x030499a4`, text 121,460 B; AP5M CRC `0x379499C7`,
sha256 `85ee2827fddafed3a402710ed9a2f81f8eda9882c620514e90f1b14f3e70c96c`.

**Next:** the Album/Artist/Genre lists (they need the unique-name lists built
during the scan) and paging polish.

### AD.1 The "Index" row said 0 after a rebuild (cache bug)

The index committed fine (the rebuild screen counted the tracks), but the root
menu's "Index" row kept showing 0: it caches the count on first render and that
happens at boot, when no index exists yet. The rebuild now invalidates the cache
and logs the outcome (`index: rebuild ok, N tracks` / `index: rebuild rc N
(previous index kept)`), so the next session's LOG carries the number.
Reported by the user: ~800 files indexed, row said 0. The All Tracks view was
unaffected (it opens the index itself, no cache).

### AD.2 Library layout: rows vs the footer, and the artist column (user report)

On hardware the All Tracks list drew its lower rows blank/overlapped and the
artist column was truncated mid-word. Two causes: the view sized its rows from
`menu_page_size()` — which is a *menu* helper that counts the footer row — while
the fetched window was smaller, and it put a second text column on a 320 px
panel. Now: `LIB_ROWS 10` (rows end at y=136, the footer row starts at 158),
page == window, title only (full tags live in Now Playing), and SELECT on a
track pushes Now Playing instead of only starting playback. DFIU records it as
`1df69d31` / rule candidate `R-1ddae0f0c00b`: *every list on the 320x170 panel
sizes its rows from the footer-safe area, keeps page == window, and adds no
column that cannot fit the width.*
