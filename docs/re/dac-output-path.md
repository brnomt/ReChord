# M3 → DAC output path (Ghidra trace)

Status: **Phase 1 — substantially mapped, one gap remains.**

## What is now verified (stock `section_3_0x00081A14.bin`)

### Playback init path — `AudioPlayback_Start` @ `0x0302a398`
After codec open, the integrated DAC is driven **directly over I2S, 24-bit,
stereo**, with NO GOODE DSP step inline:

```
rom_dma_config(4,1)
rom_i2s_master_config(0, 0x17, 0, 1)          ; 0x17 = 24-bit
rom_playback_start(1, 2, 1, rate, 0, 0x17, 0)  ; mode 1, 2ch, 24-bit
rom_i2s_dma_start(1, 4, 0)
rom_audio_path_route(1, 4)
rom_audio_path_disable(1, 4)
rom_dac_mute(1, 4)
dac_gain_curve_apply(0x50)
```

### The GOODE DSP is out-of-band, not inline
`DSP_GOODEF_Init/Process` are called from **`MainUI_KeyHandler`** on EQ-apply
events (`0x20000040`), not from the playback loop. They re-run the effect
after an EQ change; they do not sit between codec and DAC on every frame.

### Dead ends (Ghidra auto-names are unreliable)
- `AudioDecoding` @ `0x03086e2c` = **WMA codec**, not output path.
- `MusicService` @ `0x03029afc` / `0x0302b646` = message/event dispatcher.
- `FUN_03004608` = text/string parser (not the DAC-feed call its caller site
  suggested).
- `eq_key_handler` @ `0x03020810` = `halt_baddata` (overlay, not static).

## The one remaining gap

The **per-frame codec → DAC loop** (where decoded PCM is written into the
buffer that `rom_playback_start`/DMA consumes) has not been pinned to a single
function. It lives in a codec/audio task whose entry is overlay-dispatched,
and the names are garbage. Candidate to trace next: the caller of
`rom_playback_start`, and the `rom_buffer_ready(5)` poll loop in the service.

## Conclusion for the DSP fork

- **M3 pass-through EQ is viable and simpler than feared**: the DAC path is a
  clean ROM-HAL sequence (24-bit/32), and the GOODE chip is optional/out-of-band.
- `rechord_dsp.c` (`EQ_TYPE = long` = 24-bit-in-32) matches the buffer width.
- The only missing piece is the exact PCM handoff function — a narrow,
  localized Ghidra trace (find who writes the DMA buffer), not a redesign.

## Next concrete step
Trace the write path into the DMA/I2S buffer: xrefs on the buffer pointer
passed to `rom_playback_start` / `rom_i2s_dma_start`, or the codec task that
polls `rom_buffer_ready(5)` and fills the frame. Once that function is named,
Phase 2 (insert `rechord_dsp.c`) has its patch point.
