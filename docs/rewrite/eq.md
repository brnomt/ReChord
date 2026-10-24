# ReChord — Equalizer (BRNOMT-76)

Scope: a 5–10 band EQ with user profiles, applied to the decoded PCM on
**Core B** and edited from the AP UI. This document is the contract for the
engine, the IPC curve transport, the profile files and the UI.

## 1. Why Core B

```
file → AP (fs) → A→B file ring → B decode (int32, 24-bit left-justified)
                                        │
                                        ├─ gapless trim
                                        ├─ ReplayGain
                                        ├─ EQ  (firmware/dsp/eq_bank.c)   <-- here
                                        └─ ring_write → PCM ring → AP I2S ISR → DAC
```

The AP's only per-sample consumer is `shared_pull()` inside the I2S TX ISR
(8 bytes at a time, lowest NVIC priority, ~0.15 ms of margin at 96 kHz).
Running biquads there would put ten 64-bit MACs per sample inside the FIFO
refill window. Core B already has the full decode block (up to 1024 frames) in
main-loop context and the buffer is already in the engine's native format.
The AP also has only a few hundred bytes of `.bss` headroom; B's data region is
128 KiB.

The AP-side decode fallback (`AP_LOCAL_DECODE=1`, diagnostic images only) does
**not** apply the EQ: Core B is the product decoder.

## 2. Engine — `firmware/dsp/eq_bank.{h,c}`

* Native int32 stereo processing (the `dsp_module_t` chain in
  `firmware/dsp/` is int16 and stays as the host/experiment layer; using it
  here would truncate the low 8 bits of 24-bit samples).
* Coefficients: the RBJ/Q2.30 biquad math of `rechord_dsp_core.c`, included
  privately with renamed entry points (same trick as `mod_eq.c`) — one source
  of truth, no symbol clash.
* Topology is positional: band 0 = low shelf, band `count-1` = high shelf,
  the middle bands are peaking filters.
* Zero-gain bands are not instantiated: a flat curve is a bit-exact
  pass-through and costs no CPU.
* Bands at or above Nyquist are ignored, so 8 kHz streams work.
* Headroom: when any band boosts, the whole block is pre-scaled by
  `0.9 / max_linear_gain`; a cut-only curve stays at unity. A boost therefore
  changes the shape and leaves the peak level at 0.9 FS (same idea as the
  stock `RockEQReduce9dB`).
* Retuning preserves filter history when the geometry (count/frequency/Q) is
  unchanged, so moving a gain in 0.5 dB steps does not click. A topology change
  or enabling the effect clears the history.
* Reset policy in `firmware/bb/bb_main.c`:
  * `IPC_C_OPEN` (hard start): reset.
  * `IPC_C_SEEK`: reset.
  * `IPC_C_NEXT` (gapless boundary): **keep** the history (same stream).

State: `eq_bank_state_t` is an opaque, 8-byte-aligned 896-byte buffer holding
`Biquad[2][10]` plus the active curve (~800 B used), statically allocated in
Core B. No heap.

Cost: 10 biquads × 2 channels ≈ 7–9 % of Core B at 44.1 kHz, 15–18 % at
96 kHz. If `IPC_SH_UNDERRUNS` starts moving, process the block in 256-frame
chunks (the engine is stateless per call).

## 3. IPC — `firmware/player/eq_ipc.h`

The AP publishes the whole curve through the shared block
(`firmware/player/ipc_shared.h`), B polls it once per decode block:

| Word | Meaning |
|---|---|
| `IPC_C_EQ_EN` (44) | 0 = bypass, 1 = active |
| `IPC_C_EQ_N` (45) | band count, 5..10 |
| `IPC_C_EQ_GEN` (46) | seqlock generation: odd while the AP writes, even when stable |
| `IPC_C_EQ_BASE` (47..56) | 10 packed band words |

Band word: `freq_hz` (15 bits) | `Q*100` (9 bits) << 15 | `gain_tdb` (int8)
<< 24. `eq_ipc_publish()` writes `GEN|1`, the payload, then the next even
generation; `eq_ipc_read()` ignores odd generations, re-checks the generation
after copying, and returns `-1` for a malformed count (marked seen).

`ipc_b_set_eq()` caches the curve and re-publishes it on every start and
gapless handoff, so a curve chosen before playback still reaches B.

## 4. Profile files

Location (Internal volume, writable FAT16/32):

```
/Internal/.rechord/eq.cfg              active curve (enabled + profile + bands)
/Internal/.rechord/eq/customN.cfg      custom slots, N = 1..8
```

`rechord_files_init()` creates the directory, preallocates `eq.cfg` and the
five default slots (1 KiB each; in-place rewrite, no cluster growth). The user
may add `custom6.cfg`..`custom8.cfg` from a PC; the picker shows every slot
file it finds. SD is read-only for the firmware, so files on the card cannot
be saved to — read a profile from the PC into Internal instead.

Format: the same `key=value` dialect as `theme.cfg` / `rechord.cfg`. Comments
(`#`, `;`), blank lines and unknown keys are ignored.

```
# /Internal/.rechord/eq/rock.cfg
format=1
name=Rock
band0=31,70,+40        # freq_hz, Q*100, gain in tenths of dB
band1=62,70,+30
...
band9=16000,70,+30
```

Rules:
* `format=1` is required if present; an unknown value rejects the file.
* 5 to 10 bands, contiguous `band0..band(N-1)`, strictly increasing
  frequencies (20..20000 Hz), Q 0.10..4.00, gain −12.0..+12.0 dB.
* Any violation rejects the whole profile: the active curve is untouched.
* The name is a UI label only. Core B receives just bands, Q and gains.

Built-in presets (flash, not files): Flat, Rock, Jazz, EDM, Vocal, each with
10 ISO bands (31/62/125/250/500/1k/2k/4k/8k/16k Hz, Q 0.70). The curves are
project data (standard graphic-EQ shapes), not copied code.

## 5. UI

Settings → Interface → Equalizer (`firmware/ui/screens/eq_screen.c`):

* `Enabled` — toggles the effect; saved immediately.
* `Profile` — opens the picker: 5 built-ins followed by the custom slots and a
  `Save current...` row (which lists the slots and overwrites the chosen one).
* `Band N` — SELECT enters **edit** mode (`EDIT` in the title row); UP/DOWN
  move the band by 0.5 dB using the normal hold-to-repeat ladder; SELECT or
  BACK leaves edit mode.
* BACK saves the active curve and pops.

The screen has no skin yet: the skin engine exposes label/value strings only,
so per-band bar graphics would need new engine variables. Layout/palette come
from the active theme through the standard `menu_draw` path.

## 6. Budget and limits

* AP `.bss`: the EQ service + screens add ~300 B; `_bss_end` is now
  `0x03049F94` (108 B below the `0x0304A000` ceiling). Keep it in mind before
  adding more AP statics.
* AP code: ~5.5 KB (engine embedded in the Core B blob ~2.6 KB + parser,
  screen and IPC).
* Core B: ~800 B of `.bss` and ~2.6 KB of code.
* Known trade-offs:
  * enabling the effect (or switching to a profile with a different band
    layout) clears the filter history — a short transient, no crossfade;
  * a boost lowers the overall level (headroom rule above);
  * 96 kHz halves the decode cover budget; see
    `docs/rewrite/core-b-phase3.md`;
  * profiles are rewritten in place, so exFAT (writes refused) and SD
    (read-only) cannot hold edits.

## 7. Tests

```bash
sh firmware/dsp/tests/run_tests.sh        # eq_bank math (flat/boost/cut/Nyquist/retune)
sh firmware/services/tests/run_tests.sh   # parser + service + slots
sh firmware/player/tests/run_tests.sh     # eq_ipc packing/seqlock
sh firmware/ui/tests/run_tests.sh         # screen navigation/edit/picker
make test-dsp
make cfw-ap && make standalone-ap5m       # link + loader gate
```
