# Core B phase 3 — decode on B (BRNOMT-68)

> Status: **design** (not implemented). Phase 2 is hardware-verified: 96k/88.2k
> play without crackles and the 64 KiB shared ring fixed the stutter. Phase 3
> is the architectural completion: B owns decode + render, A keeps UI/input/FS.

## 1. Why

- 96k/88.2k already work with the 64 KiB ring (phase 2). What remains:
  - The UI is slightly heavier at 88.2k because **the AP still decodes**.
  - The 192k/24 file (max_block 16384) needs a ~143 KB dr_flac window and is
    rejected by the 80 896 B workbuf region.
- Moving decode to B frees the AP entirely and gives the audio path its own
  memory budget.

## 2. Prerequisite — CAL core clock (hardware risk)

**We never programmed Core B's clock explicitly, but the AP clock switch already
did it for us:** `ap_clock_set_200mhz()` writes `cal_core_div = 2` in
`CLKSEL_CON[0]` (value `0xFFFF1001`), so the CAL core runs at APLL/2 =
**200 MHz**, the same clock class as the AP. The reference additionally starts
B at 400 MHz and negotiates `/4` (100 MHz) via `chip_freq` + CRU; we do not
need that for phase 3 if 200 MHz decodes in real time.

- Confirm with a **benchmark on B** (decode N frames from HIGRAM, report ms)
  before moving codecs.
- If 200 MHz is short, REA the reference's CAL clock sequence and validate
  stepwise (the AP 200 MHz bug was a CRU offset error, DFIU `505c0c1f`).
- Gate: real-time 96k/24 throughput on B.

## 3. Memory map

Confirmed by the TRM (`docs/TRM-notes.md:17-18,39-41`): the SoC has **two**
cores with **separate** SRAM.

| region | address | owner | use |
|---|---|---|---|
| SYSRAM0 (320 KiB) | `0x03000000` | Core 0 (AP) | AP stack + .data/.bss |
| DRAM (256 KiB) | `0x03050000` | Core 0 (AP) | AP code image (FW_CODE) |
| **SYSRAM1 (256 KiB)** | **`0x03090000`** | **Core 1 (B)** | **B decode pool + PCM ring** |
| HIGRAM0 IRAM (128 KiB) | `0x01000000` | Core 1 (B) | B image (code) + file ring |
| PMU SRAM (64 KiB) | — | shared | control block (`0x01020000`) |

- HIGRAM alone (128 KiB) could **not** hold image + FLAC pool (~56-70 KiB) +
  the 64 KiB PCM ring — that was the gate for "decode on B". SYSRAM1 is
  Core 1's own 256 KiB, so B's pool and PCM ring live there.
- The AP can reach HIGRAM (it already copies the B image to `0x01000000`) but
  not necessarily SYSRAM1 — so the **A→B file ring stays in HIGRAM**; the
  **PCM ring + decode pool move to SYSRAM1** (B-only traffic).
- The 192k/24 file needs a ~143 KB window: SYSRAM1 (256 KiB) can hold it,
  which makes PIXELATED KISSES potentially reachable in 3d.

### CORRECTED map (2026-10-17): use the SDK's B_Core.sct, not SYSRAM1

The vendor B-core scatter file (`community/sdks/.../Scatter/B_Core.sct`) is
authoritative and contradicts the SYSRAM1 guess:

```
HRAM_CODE_BASE 0x00000000   HRAM_DATA_BASE 0x01020000
HRAM_CODE_SIZE 0x00020000   HRAM_DATA_SIZE 0x00040000   (to HIFIACC 0x01060000)
```

So Core 1 runs code from HIGHRAM0 and keeps its **data + stack in
`0x01020000`..`0x01060000` (256 KiB)**, NOT in SYSRAM1. And Core B already
proves it reaches that region — it reads the control block at `0x01020000`
every poll in phase 2.

**Layout now used** (bb.ld + ipc_shared.h):

| range | use |
|---|---|
| `0x01000000..0x01010000` | B code (64 KiB) + .data LMA |
| `0x01010000..0x01020000` | PCM ring (B) |
| `0x01020000..0x01021000` | control block (shared) |
| `0x01021000..0x01029000` | A→B file ring (shared) |
| `0x01031000..0x01060000` | B .data + .bss (pool) + stack |

Negative result kept for the record: `822761d` put B's SP at `0x03090000`
and broke playback (stuck at 0:00) — SYSRAM1 is not Core-1-reachable as
assumed; reverted in `cb95438`.

## 4. IPC and the file-service boundary (FS stays on A)

- **A→B commands** (control block + mailbox/doorbell or polling):
  `OPEN(path, format)`, `PLAY`, `PAUSE`, `STOP`, `SEEK(ms)`, `VOLUME(db)`,
  `TRACK(length)`.
- **B→A file requests**: `OPEN`, `READ(off,len)`, `SEEK`, `TELL`, `SIZE`,
  `CLOSE` — A owns the FS and the file handle.
- **Read-ahead**: A prefetches into a shared file ring ahead of B's cursor so
  a read does not round-trip per chunk. B's `codec_io` copies from that ring;
  it only stalls when the ring is empty (A must keep it ahead).
- Telemetry B→A: state, position_ms, duration_ms, sample_rate, underruns.
- Ordering: single-producer/single-consumer indices with DMB (M3 has no data
  cache), same discipline as the PCM ring.

## 5. B loop

One core does decode + render; they must interleave:
- Top the I2S FIFO from the PCM ring (as phase 2 does) — the hard real-time
  part.
- Decode the next slice into the PCM ring between feeds.
- dr_flac decodes a whole block atomically, so the PCM ring must hold one
  block (85.3 ms @96k) — it does (64 KiB). A file read must never stall longer
  than the ring depth; read-ahead is the mitigation.

## 6. Sub-phases (each buildable / testable)

1. **3a — CAL clock** + B throughput benchmark. No audio changes.
2. **3b — file service**: A-side server + shared file ring + B-side
   `codec_io`; B reads a real track and verifies bytes against A's read.
3. **3c — move codecs + player state to B**; A's `player_*` becomes an IPC
   client (UI unchanged).
4. **3d — verification matrix**; then revisit 192k/16384 (B memory or a
   streaming decoder).

## 7. Risks and rules

- **Clock bring-up** is the same class as the AP 200 MHz freeze (DFIU
  `505c0c1f`): REA first, stepwise, readback, known-good image at hand.
- **File-read latency** stalling the render → read-ahead + ring; instrument
  underruns on B.
- **No mixing**: one sub-phase per flash.
- `_bss_end` < `0x0304A000`; debrick via maskrom (`docs/DEBRICK-LINUX.md`).
- Clean-room: protocol facts only; all code compiled from this tree.
