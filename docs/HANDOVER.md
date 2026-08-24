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
