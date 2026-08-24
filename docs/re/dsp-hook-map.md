# DSP hook map (Ghidra, stock section_3_0x00081A14.bin)

Status: **Phase 1 finding — the stock EQ is NOT the SDK's Cortex-M3 biquad
path.** The "patch RockEQProcess with rechord_dsp.c" assumption was wrong.

## Verified (decompiled from the raw binary)

| Symbol | Address | What it actually does |
|---|---|---|
| `DSP_GOODEF_Init` | `0x0300f7dc` | Opens DSP firmware blobs by name via SPI: `"GOODEFGHMP3"` (config) and `"WOOOOONXBIN"` (firmware). Reads a 0x24-byte descriptor (version/type/config_size). Modes: 0/1=load config, 2=abort, 6=quick firmware load. |
| `DSP_GOODEF_Process` | `0x0300fb0e` | Per-frame effect dispatch. Calls `ed25519_test` (a MIS-NAMED function — the repo docs warn Ghidra auto-names are garbage) with (dsp_ctx, pcm_buf, 0). |
| `ed25519_test` @ `0x0300f8ba` | `0x0300f8ba` | The real DSP core: binary-searches a module table, streams PCM through `FUN_02fefa82`/`FUN_02fef58a`/`FUN_02ff2ffc`, with an `XOR 0xf` decrypt pass over 1024-byte chunks. |

The DSP firmware is loaded **over SPI from named files** (`FUN_02fef2aa` =
SPI open, `FUN_02fef470` = SPI read byte, per the decompiled spec in
`docs/re/decomp/firmware/dsp/dsp_goedef.c`). The string `"GOODEFGHMP3"` /
`"WOOOOONXBIN"` and the SPI accessor pattern strongly indicate an **external
GOODE-family DSP chip**, not Cortex-M3 biquads.

## Implication

The SDK's `Effect.c` path (`EffectProcess → RockEQProcess(pBuffer, len)` from
the `RkNano_EQ_24BIT.lib`) is the **generic** RKnanoD SDK EQ — it is NOT what
FiiO shipped in the Echo Mini. FiiO routed EQ/effects to an external DSP chip
loaded over SPI. Therefore:

- `rechord_dsp.c` (Cortex-M3 biquads) does **not** cleanly replace the stock
  EQ hook — there is no simple `BL RockEQProcess` to redirect.
- "Make the DSP ours" now means one of:
  1. **Bypass the external DSP** and do EQ on the M3 — but the external chip
     likely *is* the audio output/DAC, so it can't simply be bypassed.
  2. **Reverse the GOODE chip's SPI protocol** and send it our own
     coefficients instead of FiiO's blobs — a different, external-hardware
     reverse-engineering task.

## Where this leaves the fork strategy

**Key finding: the GOODE chip is an EFFECTS coprocessor, not the DAC.**
The DAC is **integrated in the SoC** (`docs/HARDWARE.md`: "Audio DAC:
Integrated — 90dB SNR"), and the ROM HAL exposes a full audio-output path
independent of the GOODE chip: `rom_dac_mute/unmute`, `rom_i2s_master_config`,
`rom_audio_path_route`, `rom_sample_rate_set`. So the audio chain is:

```
codec decode (M3) -> [optional GOODE DSP over SPI] -> integrated DAC (I2S)
```

This means **M3 pass-through EQ IS viable**: skip `DSP_GOODEF_Process` and do
EQ on the M3 with `rechord_dsp.c`, then feed the integrated DAC directly. The
GOODE chip is optional in the audio path, not mandatory for output.

`eq_key_handler` @ `0x03020810` decompiles to `halt_baddata` (bad-instruction
data) — consistent with an overlay module that isn't statically loaded at that
address, reinforcing that the EQ/DSP code is overlay-based, not a simple
in-place function.
