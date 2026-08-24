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

The "DSP fork first" plan assumed a Cortex-M3 EQ hook. The verified stock
DSP is an external chip over SPI. This is a significant scope change and
needs a decision before any more code is written:

- Option 1: reverse the GOODE DSP SPI protocol (external-chip RE).
- Option 2: revert to the SDK's Cortex-M3 `EffectProcess` EQ by making the
  decode loop call `rechord_dsp.c` *instead of* `DSP_GOODEF_Process` — but
  this changes the audio path and may require the external chip to be
  configured for pass-through (unknown).

Neither is a one-line patch. Both need the GOODE chip's role clarified first.
