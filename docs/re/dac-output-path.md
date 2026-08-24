# M3 → DAC output path (Ghidra trace)

Status: **Phase 1 — complete enough to plan the patch.** The DSP model is now
understood.

## Renames applied (persisted in Ghidra project)

| Old (wrong) name | Address | New name | Why |
|---|---|---|---|
| `AudioDecoding` | `0x03086e2c` | `WMA_AudioDecoding` | it is the WMA codec |
| `FUN_03004608` | `0x03004608` | `TextConfigParser` | text/`[]<>` parser |
| `ed25519_test` | `0x0300f8ba` | `GOODE_DSP_Stream` | GOODE SPI PCM streaming |
| `eq_key_handler` | `0x03020810` | `EqApplyOverlayEntry` | overlay EQ-apply entry |
| `Http_Close` | `0x0304d022` | `MusicScanLoop` | music index scan loop |
| `FUN_0302c950` | `0x0302c950` | `CodecFeedDispatch` | codec function-pointer table |

## Verified model

1. **DAC is direct I2S, 24-bit stereo** (`AudioPlayback_Start` → `rom_i2s_master_config(0x17)`
   → `rom_playback_start(mode1,2ch,0x17)` → `rom_dac_mute`).
2. **The GOODE chip does continuous EQ in hardware.** The M3 only re-programs
   it on EQ-change events (`MainUI_KeyHandler` event `0x20000040` →
   `DSP_GOODEF_Init("WOOOOONXBIN")` → `GOODE_DSP_Stream`). It is NOT inline
   in the per-frame decode loop.
3. **The per-frame codec feed** is `CodecFeedDispatch` @ `0x0302c950`: a
   function-pointer table (`DAT_0302caa0 + codec_type*4`), calling the active
   codec with sub-function `0xe`. The dispatch tail lands in the codec body
   (`0x0300cc9a`).

## Implication for "own the DSP"

To run our `rechord_dsp.c` on the M3:
1. **Neutralize the GOODE chip** — stop `MainUI_KeyHandler` from re-programming
   it (or leave it flat), so its hardware EQ is bypassed.
2. **Insert M3 DSP** in `CodecFeedDispatch` (after the codec returns the frame,
   before the buffer goes to `rom_playback_start`/DMA), running
   `rechord_dsp.c`'s biquads (24-bit-in-32 = `EQ_TYPE long`).

The exact buffer handoff within `CodecFeedDispatch`'s codec body is the last
narrow detail; the patch point is this dispatch, not `DSP_GOODEF_Process`.
