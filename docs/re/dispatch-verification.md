# Dispatch verification (Ghidra, stock binaries)

Status: **Model 1 CONFIRMED** — `fw1` = AP/UI, `section_3` = BB/audio.

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

- `fw1` (AP) = the UI we are **not** replacing. It boots, draws the cassette,
  and sends mailbox commands (decode/file) to the BB.
- `section_3` (BB) = the audio/DSP half we replace. It receives A2B commands
  and replies B2A. `firmware_entry` is its reset vector, and it must return to
  the ROM (not run a standalone main).
- Mailbox base `0x40110000` confirmed in both halves.

## Still unresolved (mask ROM, invisible — no bytes/xrefs)

The ROM's post-`firmware_entry` behavior on core1 (the BB) is not directly
observable. It is why the exact dispatch entry for the BB's service loop must
be confirmed on-wire (UART) rather than assumed.
