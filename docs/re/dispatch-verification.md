# Dispatch verification (Ghidra, stock binaries)

Status: **UNRESOLVED — the evidence contradicts itself.** Do not treat either
model as settled.

- **"BB/audio" model** (README/HANDOVER, dispatch-map CORRECCIÓN): `fw1` =
  AP/UI, `section_3` = BB/audio, two builds of the same SDK over a mailbox.
- **"whole app" model** (bootloader-analysis UPDATE): `section_3` contains the
  full app (UI + audio + USB + IPC).

The UI functions below are physically inside `section_3_0x00081A14.bin`,
which is strong evidence for "whole app" — but both fw1 and section_3 are
builds of the *same* SDK, so shared UI/font code may simply be linked into
section_3 without being the active UI. Static analysis alone cannot settle it
because the ROM's post-`firmware_entry` dispatch is in the invisible mask ROM.

## What was verified

### `section_3_0x00081A14.bin` (the BB we replace) — loads at 0x03000000
- `firmware_entry` @ `0x03000010`: HW-init callback that tail-calls
  `rom_hw_init2(0x18f/0x191)` and **returns to the ROM** (zero internal xrefs).
- `main2_entry` @ `0x03000aba`: **USB Mass-Storage** dispatcher
  (`MscSendCSW`, `WriteData_To_Flash`) — not the audio main.
- `application_start` @ `0x0300710a`: UI status-bar renderer.
- `ipc_post_cmd` / `ipc_post_arg` @ `0x03073c7c` / `0x03073ca8`: mailbox
  writers — `*(base + id*0x50 + 0x30+8*ch)` / `...+0x34+8*ch`.
- `HifiFileRead` @ `0x0306b94c`: posts `0x6b` (FILE_READ) on ch2, then
  busy-waits for a reply flag → **the BB asks the AP for file data**.

### `ap_region.bin` (fw1 / AP) — 530 KB raw slice, rebases to 0x03060000
- Contains the same compiled **MailBox API** as section_3 (literal
  `0x40110000` at `0x47d88`, `0x47e5c`, `0x47f10`; the `id*5 / id*0x50`
  accessor sequence; `str [r0,#0]` = A2BIntEn, `str [r0,#4]` = A2BStatus).
  → the AP enables/clears **A2B** interrupts and drives the BB.

### SDK manifests (corroborating)
- `ap.mk` builds `app_main.c` + `ui/MainMenu` + `ui/MusicWin` +
  `display/*` → **UI/AP**.
- `bb.mk` builds `Main2.c` + `bbsystem/*` + audio codecs → **BB/audio**.

## Conclusion

- `firmware_entry` @ `0x03000010` returns to the ROM — **verified** (matches
  stock disassembly exactly).
- Mailbox base `0x40110000` in both `section_3` and `fw1` (ap_region.bin) —
  **verified**.
- **`section_3` physically contains UI code** (`music_menu_draw`, `MusicService`,
  `MusicInit`, `GUI_TextDisplay`, `Font12_CompiType`) **and** audio code
  (`HifiFile*`, `DSP_GOODEF*`, `ipc_post_cmd`) **and** USB (`main2_entry`).
- Whether section_3 is the *active* UI or the *BB* is **unresolved** — it
  depends on the ROM's post-`firmware_entry` dispatch, which is not statically
  observable.

**Implication for the plan:** Phase 2 (wire the BB service loop) cannot be
completed correctly until this is resolved, because "the correct dispatch
entry" differs by model. The only definitive resolution is an **on-wire UART
trace** (Phase 3) — flash a minimal `firmware_entry` that logs "boot reached"
and returns to the ROM, then observe which dispatch offsets the ROM actually
invokes.
