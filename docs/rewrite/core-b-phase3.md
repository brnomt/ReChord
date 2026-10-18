# Core B phase 3 — decode on B (BRNOMT-68)

> Status: **implemented** (phase 3c — see §8, and §9 for the gapless hand-off).
> Phase 2 is hardware-verified (96k/88.2k play without crackles; the 64 KiB
> shared ring fixed the stutter). Phase 3 was the architectural completion: B
> owns decode, A keeps UI/input/FS, the DAC bring-up and the I2S render. §2–§7
> are the original design notes, kept for provenance — where they disagree with
> the code, `firmware/player/ipc_shared.h` and §8/§9 win.

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

**Layout now used** (`ipc_shared.h` is authoritative; `bb.ld` mirrors it):

| range | use |
|---|---|
| `0x01000000..0x01020000` | B code (HIGHRAM0, 128 KiB) + .data LMA |
| `0x01020000..0x01021000` | control block (shared) |
| `0x01021000..0x01029000` | A→B file ring, "current" (32 KiB) |
| `0x01029000..0x01030000` | A→B file ring, "pre-opened next" (16 KiB) |
| `0x01030000..0x01040000` | PCM ring (64 KiB, produced by B) |
| `0x01040000..0x01060000` | B .data + .bss (decode pool) + stack |

Negative result kept for the record: `822761d` put B's SP at `0x03090000`
and broke playback (stuck at 0:00) — SYSRAM1 is not Core-1-reachable as
assumed; reverted in `cb95438`.

## 4. IPC and the file-service boundary (FS stays on A)

- **A→B commands** (as shipped — `ipc_shared.h`, all polling, no doorbell):
  `IPC_C_OPEN` (ARG = codec id), `IPC_C_NEXT` (gapless reopen over the live
  ring), `IPC_C_SEEK`, `IPC_C_CLOSE`, each gated by the `IPC_C_SEQ`/`IPC_C_ACK`
  handshake; run/pause/idle come from `IPC_SH_STATE`. There is no
  `PLAY/PAUSE/STOP/VOLUME/TRACK` opcode — gain and trim travel as published
  words (`IPC_C_RG`, `IPC_C_GAP_*`).
- **B→A file "requests":** B does **not** round-trip per operation. A owns the
  handle and fills a shared ring; B consumes bytes from it and only *requests a
  seek* (`IPC_F_SEEK_TO` + `IPC_F_SEEK_SEQ` → A copies `IPC_F_SEEK_ACK`). EOF and
  failures are published as flags (`IPC_F_EOF`, `IPC_F_OPEN`); a read that can
  never be refilled is bounded and counted (`IPC_C_BSTALL`).
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

## 8. Phase 3c implemented (2026-10-17)

- **Core B decodes.** `firmware/bb/bb_main.c` opens the codec by id (the AP
  passes it in `IPC_C_ARG`), reads the track through `bcio.c` over the A→B file
  ring, and is the PCM-ring PRODUCER (writes `IPC_SH_WR`). After each push it
  re-arms the AP's TX-empty IRQ (`I2S_INTCR` bit0) because the AP ISR masks the
  source when it finds the ring dry.
- **The AP keeps the DAC/I2S bring-up and the feeder.** `ipc_b_target.c`'s
  `shared_pull` drains the ring in the TX ISR (consumer); the AP serves the
  track with `bfs_open`/`bfs_poll` and no longer decodes while B is up
  (`player.c`: `ipc_b_b_decodes()`). So library/UI work can no longer starve
  the audio — the Library→All Songs cut is gone at the root.
- **Fallback.** If B does not ACK the OPEN handshake, the AP decodes exactly as
  phase 2 (`ipc_b_ring_fill`); audio never breaks because of the handoff.
- **Memory.** B code in HIGHRAM0 `0x01000000..0x01020000` (blob ~65 KiB,
  embedded in fw1 — FW_CODE enlarged to `0x3C000`); control block `0x01020000`;
  file ring `0x01021000` (32 KiB) + **pre-open ring** `0x01029000` (16 KiB); PCM
  ring `0x01030000` (64 KiB); B data/pool/stack `0x01040000..0x01060000`.
- **PCM post-processing on B:** ReplayGain (Q16 gain in `IPC_C_RG`, re-synced
  on mode changes) and the gapless trim (`IPC_C_GAP_DELAY/PAD/TOTAL`) are
  applied in B's decode loop, mirroring the previous AP logic; B links
  `rg_math.c`. Seek forwards to B via `ipc_b_seek()` and resets B's trim
  cursors. (v1 shipped without these; added right after.)
- AP5M CRC `0xE27C15B8` (RG+gapless build), `_bss_end 0x030418c8`; host player
  tests pass.

### Hardware verified (2026-10-17)

User flashed `0x88C41346`: 16/44.1 and 96k/24 (Hyperdrama) play with **no
crackles**, UI fluid; a **slight lag remains** (minor, accepted). The
Library → All Songs audio cut is **gone at the root** (B no longer depends on
the AP pumping). BRNOMT-68 can be closed as far as this phase goes.

DFIU: incident `6d0ae36b` (Library → All Songs cut) **verified_resolved**
(effective pass, reproduction, 2026-10-17). Logged-out item: a slight lag
remains (accepted, not blocking).

## 9. Gapless hand-off (BRNOMT-73) — how the next track reaches B

Verified on device for **same-format** tracks (same rate / channels / bit
depth); a format change stays a hard boundary because the DAC must re-lock.

- **Second file ring.** Besides the 32 KiB "current" ring, A keeps a 16 KiB
  **pre-open** ring (`IPC_FILE2_BASE`). ~3 s before the end of a track it opens
  the next file into it (`bfs_preopen`) and streams its head there.
- **Pending open (A side).** In the same window the AP opens the next track's
  codec (`preopen_next_codec`, `player.c`) and keeps its metadata in `pending_*`
  while `g.track`/`g.fmt` still describe the playing track (Now Playing must not
  jump ~3 s early). The index→path resolution is cached too (`peek_*`), so the
  boundary performs no media-DB lookup either.
- **Prescan hand-off.** A publishes the FLAC geometry it discovered
  (`IPC_C_PRE_*`, via `codec_t.prescan_info` → `ipc_b_set_prescan`). B installs
  a reader (`b_prescan_read`) and `flac_open()` skips its own `flac_prescan()`
  when the values are present and plausible — that walk is a seek round-trip per
  metadata block and used to sit inside the boundary.
- **The boundary.** At B's decoder EOF the AP promotes the pre-open ring
  (`bfs_promote`, which also clears the retired ring's `OPEN`) and sends
  `IPC_C_NEXT` with `IPC_C_RING`; B reopens its codec reading the ring already in
  RAM. Nothing is stopped — the PCM ring (64 KiB ≈ 162 ms at 44.1k) covers B's
  ~90 ms reopen. The boundary pass issues the command *before* the ring refill
  and the app event, so neither delays the reopen.
- **Failure is safe.** If the pre-open does not match the resolved path (the
  list changed under us), `ipc_b_next()` returns -1 and the caller takes the hard
  path instead of playing the wrong file.
- **96 kHz.** Every budget above halves at 96k (PCM cover ≈ 74 ms, TX-FIFO
  refill margin ≈ 0.15 ms), so the boundary and the feeder are tight there —
  tracked separately in HANDOVER/STATUS.
