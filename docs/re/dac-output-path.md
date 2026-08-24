# M3 → DAC output path (resolved)

Status: **RESOLVED.** The EQ patch point is in SDK source we already compile —
no binary patching needed.

## The audio path (verified in source, not Ghidra)

The decode→DAC loop is the SDK's own `AudioDecoding()` in
`firmware/rockchip/system/sysservice/Service.c:247`, driven by the DMA ISR:

```
DMA IRQ (DAC needs data)
  -> AudioDmaIsrHandler()                    Service.c:76
    -> AudioDecoding()                       Service.c:247
      -> codec decode -> AudioPtr            (24-bit PCM)
      -> Bit_Convertor_DEC()                 Service.c:326  shift samples to bit 0
      -> EffectProcess((EQ_TYPE*)AudioPtr, AudioLen)   Service.c:390  <== OUR M3 EQ
      -> Bit_Convertor_shift()               Service.c:453  shift back
      -> (buffer handed to DMA -> I2S -> integrated DAC)
```

`EffectProcess` is OUR function (`rechord_dsp.c` → `rechord_dsp_core`), reached
every frame under `#ifdef _RK_EQ_`, which `make bb` defines via `-D_RK_EQ_`.

## The GOODE chip is a FiiO binary addition, absent from our source

`DSP_GOODEF_Init/Process/Reload` (documented in `fiio_map.h` at
`0x0300f7dc`/`0x0300fb0e`/`0x0301022c`) are FiiO's external GOODE DSP driver,
driven from `MainUI_KeyHandler` on EQ-change events. They are NOT in the
Rockchip SDK source we compile, so **our BB build does not drive the GOODE
chip at all.** The M3 SDK EQ path (`EffectProcess`) is the one in our build.

## Conclusion

- M3 pass-through EQ needs **no binary patch**: `Service.c:390` already calls
  our `EffectProcess`, which is wired to the host-tested `rechord_dsp_core`.
- The GOODE chip is neutralized by omission (not in our source).
- `rechord_main` calls `Main2()` (the SDK audio service loop), which reaches
  `AudioDmaIsrHandler` → `AudioDecoding` → `EffectProcess`.

## Remaining (Phase 3 — hardware only)

1. Flash `make release` → `build/ReChord_BB.IMG`.
2. Play a track, toggle EQ presets + bass.
3. Verify audio plays and EQ is audible; no crash, no UI regression.

This is a hardware-verification gate, not a code or RE gate — the code path is
in source and already wired end-to-end.
