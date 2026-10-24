# ReChord — Audio workstream

> Scope: getting audio playing from files on the device, with two
> quality-of-life requirements this project cares about: **gapless**
> transitions between tracks, and **ReplayGain** tag compatibility
> (we read the same tags the reference CFW reads, and apply them).
> Owned tree: `firmware/player/**` (decode + tags + gapless + gain) and
> `firmware/dsp/` (where gain is modelled as a volume module). Constitution:
> `docs/rewrite/architecture.md` (build-from-source rule §1, codec strategy
> §4, verification standard §5). Protocol facts: `docs/re/refcw-analysis.md`.
> HARD RULE: no third-party CFW code — only open-source codecs compiled from
> `.c` in this tree, and our own clean-room tag/fs code.

## 1. Architecture recap

```
  files on device (eMMC "Internal", microSD "SD")
        |  fs.h  (fs_target.c target backend: FAT16/FAT32/exFAT, read-only)
        v
  codec_io.h  (cio_open/read/seek/tell/size/close)
        |
        v
  codec_t  (self-describing vtable: codecs/{wav,mp3,flac}.c + codec_registry)
        |  interleaved int32 PCM frames (left-justified to 32 bits)
        v
  player.c state machine  --PCM ring--> Core B (our from-source decoder)
        |                                  (B produces the ring; the AP's I2S
  player/rg_math.c (user volume            TX ISR feeds it to the DAC)
                    + ReplayGain)
```

**AP metadata-only build (default).** The AP's codecs exist for the
pre-open/metadata path (format, duration, tags, prescan geometry); the PCM
always comes from Core B. `AP_LOCAL_DECODE=0` (default) compiles the codec
adapters with `RECHORD_AP_METADATA_ONLY`, which drops the vendored decoder
TUs from the AP link (minimp3 and dr_flac: the AP code span fell from 226,428
to 168,816 B) and leaves `decode()` returning `CODEC_ERR_UNSUPPORTED` there.
MP3's first-frame probe becomes a header-table parse; FLAC reads
rate/depth/total from STREAMINFO in the prescan instead of opening dr_flac.
`AP_LOCAL_DECODE=1` restores the old fallback image (both decoders linked)
for diagnostics; it does not fit the loader gate with the rest of the UI.

Two cores, strict boundary:
- **A core (AP, our fw1):** UI, filesystem, decode fallback, tags, gapless,
  gain, the DAC/I2S bring-up, and the I2S TX ISR that drains the PCM ring. It
  also serves Core B's file reads over the A→B file rings (`bfs`).
- **B core (ours since phase 3c, `firmware/bb/`):** decodes the track and
  produces the PCM ring. Note: the target reports `clock_div = 1` unconditionally
  (`ipc_b_target.c`) — the "/1 → /4 clock negotiation" seen in older notes is
  **host-mock only**; there is no negotiation on hardware.

**EQ (BRNOMT-76).** The 5–10 band equalizer is a Core B post-processing stage:
`firmware/dsp/eq_bank.c` runs on the decoded int32 block after ReplayGain and
before `ring_write()`. The AP only parses the active profile and publishes the
curve over the shared block (`firmware/player/eq_ipc.h`, seqlock). Profiles and
the UI contract are in `docs/rewrite/eq.md`. The AP local-decode fallback does
not apply the EQ.

The reference's `ECHOLOG.TXT` is the contract we match:
`B: boundary to track N: FLAC 44100 Hz, ~5840 frames queued (gapless)` and
`end of stream, 5843 frames queued`. Gapless is a decode+ring property of
the AP; the B core must not be restarted between gapless-compatible tracks.

Status today: the player module is **host-complete and host-tested** — WAV,
**MP3 (minimp3)** and **FLAC (dr_flac)** decode for real (the last two
vendored + adapted under `player/codecs/`, licenses pinned), plus gapless /
ReplayGain / codec_io_fs — all covered by host tests in
`firmware/player/tests/run_tests.sh` (11 binaries, ~340 assertions, green).
It is **linked into the AP image** (`AP_PLAYER_SRCS` in `firmware/ap.mk`;
`make cfw-ap` links `build/cfw/rechord_ap.elf`; `make standalone-ap5m` (the
default `make`) → `build/ReChord_AP5M.IMG` behind the packer's scatter gate,
`RESULT: PASS`). **Audio is live on hardware**: the
DAC renderer landed (I2S1 TX-IRQ feeder over the shared PCM ring), and since
phase 3c **Core B owns the decode** (B image in `firmware/bb/`, started by the
AP); the AP serves the file over the A→B file ring and feeds the DAC. See
`docs/rewrite/core-b-phase3.md`.

## 2. Files

```
firmware/player/
├── player.h            public API (states, events, calls, track_info_t)
├── player.c            state machine + codec registry + ring pump
├── codec.h             codec_t interface
├── codec_io.h          byte-source seam (read/seek/tell/size)
├── codec_io_stdio.c    stdio backend (host + newlib target)  <-- host only
├── codec_io.c          shared cio_* dispatch (open lives in the backend)
├── codec_io_fs.c       fs.h backend (target: SD/eMMC)  <-- DONE
├── ipc_b.h             A<->B audio IPC API (the shared-block LAYOUT lives in
│                       ipc_shared.h: PCM ring 64 KiB, file ring 32 KiB +
│                       pre-open ring 16 KiB, IPC_C_*/IPC_SH_* words)
├── ipc_shared.h        authoritative shared-block + file-ring map
├── bfs.h/.c            A-side file service (rings, pre-open, promote)
├── playlist.h/.c       playback list (indices, shuffle, repeat)
├── ipc_b_mock.c        host simulated B core
├── ipc_b_target.c      target shared block + supervisor
├── ringbuf.h/.c        SPSC lock-free ring (PCM streaming, 64 KiB)
├── rg_math.c/.h        ReplayGain dB->linear + PCM scaler (Q1.15)  <-- DONE
├── tags.c/.h           ReplayGain + ID3v2 TXXX / Vorbis parse  <-- DONE
├── player.mk           build fragment
├── codecs/             one file per codec, self-describing descriptors
│   ├── codec_registry.h/.c  extension -> codec_t dispatch
│   ├── wav.c            8/16/24/32-bit PCM + WAVE_FORMAT_EXTENSIBLE (ours)
│   ├── mp3.c            minimp3 adapter (Xing/LAME gapless + RG)  <-- DONE
│   ├── flac.c           dr_flac adapter (pool + pre-scan + RG)     <-- DONE
│   ├── minimp3/         vendored minimp3.h (CC0-1.0, pinned) + LICENSE
│   └── dr_flac/         vendored dr_flac.h (PD/MIT-0, pinned) + LICENSE
└── tests/
    ├── run_tests.sh
    ├── mock_codec.c/.h scripted codec (gapless + RG-aware)
    ├── test_codec_wav.c        WAV golden (sine, byte-exact)
    ├── test_codec_mp3.c        crafted Xing/LAME/ID3v2 fixture      <-- DONE
    ├── test_codec_flac.c       synthesized VERBATIM lossless golden <-- DONE
    ├── test_codec_io_fs.c      FS backend decode = stdio baseline  <-- DONE
    ├── test_player_gapless.c   gapless trim + seamless advance  <-- DONE
    ├── test_player_state.c
    ├── test_player_replaygain.c RH gain apply (scaled, peak-limited) <-- DONE
    └── test_rg_math.c          dB->linear + peak-clamp + scaling <-- DONE
```

## 3. Plan (in order — each host-testable before any hardware)

> Steps 1–6 are **implemented and host-tested** (~340 checks across
> `test_player_gapless`, `test_rg_math`, `test_player_replaygain`,
> `test_codec_io_fs`, `test_tags`, `test_codec_mp3`, `test_codec_flac`).
> Step 7 (DAC renderer) is DONE and audio is live (Core B decodes since phase
> 3c); step 8 (AP flash) is gated on hardware + DFIU preflight.

1. **fs_seek (portability).** DONE — `fs_seek(fs_file_t*, long offset)` added to
   `fs.h`, `fs_target.c` (set `f->pos`; the read loop re-derives the cluster)
   and `fs_host.c` (fseek/ftell). Host-testable via the FS backend (see below).

2. **codec_io_fs.c (decode from SD).** DONE — target backend mapping `cio_*`
   onto `fs_open_read/fs_read/fs_file_size/fs_close` (+ `fs_seek`). The shared
   `cio_*` dispatchers live in `codec_io.c`; `player.mk` selects the backend via
   `PLAYER_CODEC_IO_IMPL` (`codec_io_stdio.c` for host/tests, `codec_io_fs.c` for
   the AP). Host test: `test_codec_io_fs` decodes a WAV through the FS backend
   (backed by `fs_host.c`) and asserts byte-exact equality vs the stdio golden
   decode — proof that "files on device → PCM" works without flashing.

3. **Gapless contract.** DONE — `codec_t` has an optional `gapless` query
   (`codec_gapless_t{delay_samples,padding_samples}`); `track_info_t` carries
   delay/padding/total. `codec_format_t` now carries the codec's exact
   `total_samples` (WAV from its data size; mock from its frame config) so the
   trailing pad is trimmed to the true end, not an ms-truncated estimate. In
   `player.c`: on auto-advance across a same-sample-rate / same-channel
   boundary, `advance_queue` opens the next codec **without** `ipc_b_stop` and
   signals the B core via the new `ipc_b_track_change(duration_ms)` (adds to the
   stream length, keeps the ring and frame counter live) — the B core is **not**
   restarted between gapless-compatible tracks; on a format change it falls back
   to a hard stop/start. Leading `delay_samples` are dropped from the first
   decoded batch; trailing `padding_samples` stop emission at
   `total - delay - padding`. Source of delay/padding per codec: Xing/LAME for
   MP3; STREAMINFO + native frame count for FLAC (naturally gapless); none for
   WAV (hard boundary). Host test: `test_player_gapless` (trim + seamless
   boundary, asserts the B core is not restarted).

   **Phase 3c update (BRNOMT-73): gapless with Core B decoding.** With B owning
   the decoder the AP-side swap above no longer applies. At B's end-of-stream
   the player now takes `advance_to()`: it opens the next codec first (so the two
   formats can be compared), and on a same-format boundary calls
   `ipc_b_next()` → `IPC_C_NEXT`. B reopens its codec on the same file ring
   (`bfs_open` + OPEN-style) **without** touching the PCM ring cursors or the
   DAC/serializer, so the boundary is sample-accurate. The hand-off fires at B's
   decoder EOF — while the 64 KiB PCM ring still holds ~85 ms of the previous
   tail — instead of at ring-empty (which was the ~2–3 s hard boundary). A format
   change is a hard boundary; when B decodes it is deferred (`restart_pending`)
   until the ring drains so the tail is not truncated. The command channel is now
   **sequence-numbered** (`IPC_C_SEQ`/`IPC_C_ACK`): the old `cmd == ack` test
   dropped any repeated command (two `NEXT`s, or two `SEEK`s to the same ms).
   Host test: `test_player_gapless` gains the Core-B hand-off (no restart) and
   the deferred format-change restart.

4. **ReplayGain parse.** DONE — `tags.c`/`tags.h` expose
   `tags_replaygain_id3v2(io, &rg)` (ID3v2 v2.3/v2.4 `TXXX` keys) and
   `tags_replaygain_vorbis(io, offset, length, &rg)` (Vorbis comments), both
   returning `codec_replaygain_t{track_gain_tenth_db, album_gain_tenth_db,
   track_peak_q15, album_peak_q15}`. `track_info_t` carries the fields; the
   codec_t `tags` callback populates them (NULL = no tags). Gain math lives in
   `rg_math.c` (`rg_gain_q16`, `rg_effective_gain_q16` peak-limited,
   `rg_scale_pcm`), applied in the PCM pump **before** the ring push (`>>15`
   Q1.15 scaling + saturation on the int32 left-justified PCM) — this is the
   AP-side pre-attenuation that composes
   with the B-core user volume; `PLAYER_RG_TRACK`/`_ALBUM`/`_OFF` mode is set via
   `player_set_replaygain_mode` (defaults OFF). Design note: routed through
   `rg_math.c` instead of the `dsp/mod_volume.c` chain so the path is host-testable
   end-to-end without wiring the DSP chain into the player; a future milestone
   can fold the gain into the dsp chain (§5.2). Host tests: `test_rg_math`
   (dB→linear, peak clamp, scaling/saturation) and `test_player_replaygain`
   (exact PCM scaling vs `rg_scale_pcm` on the mock).

5. **AP linkage (hardware-gated). DONE (milestone 2a, 2026-10-12).**
   `firmware/ap.mk`: `AP_PLAYER_SRCS` (player.c, codec_io.c, codec_io_fs.c,
   tags.c, rg_math.c, `codecs/{registry,wav,mp3,flac}.c`, ipc_b_target.c,
   ringbuf.c) joined `AP_SRCS`; `-Ifirmware/player` added to
   `AP_INCLUDE_DIRS`. `codec_io_fs.c` rewritten to a 4-slot static pool (the
   freestanding `-nostdlib` link has no malloc/free). Verified:
   `make cfw-ap` → `build/cfw/rechord_ap.elf` (text 99,600 / data 180 /
   bss 283,836; `_bss_end 0x03049570` inside the linker's FW_RAM window to
   0x0304C000) with player symbols present; all player TUs compile
   `-mcpu=cortex-m3 -mthumb -Wall`; host tests stay green.
   `make standalone-ap5m` → AP5M uploaded to the device as HIFIEC90.IMG and installed on hardware
   (ECHOLOG `update: result 0`, fw_end 0x44C000); boot verdict pending a
   power-cycle (DFIU `verify_resolution` after the user confirms).

6. **Codec vendoring (MP3 + FLAC). DONE (m6c, 2026-10-12).**
   minimp3 (CC0-1.0, pin `ea99364f…`) and dr_flac v0.13.4 (PD/MIT-0, pin
   `dfe83776…`) vendored unmodified under `codecs/{minimp3,dr_flac}/` with
   LICENSE files; adapters `codecs/mp3.c` / `codecs/flac.c` are the single
   implementation TUs and self-describing `codec_t`s (registry extracted to
   `codecs/codec_registry.c`). MP3: Xing/Info + LAME gapless (+528+1),
   CBR fallback, TOC seek, ID3v2 RG, ID3v1 tail salvage. FLAC: metadata
   pre-scan (RG + SEEKTABLE budget), decode pool = the shared **workbuf**
   (`WORKBUF_BYTES` 80,896 B) for DRFLAC_MALLOC/REALLOC/FREE, deterministic
   UNSUPPORTED over budget, frame-accurate seek. Host tests synthesize both
   formats byte-exactly (crafted MP3 frames; VERBATIM-subframe FLAC lossless
   golden). RAM: PCM ring 64 KiB, `PLAYER_CODEC_STATE_MAX` 7296 (codec.h).
   Packer (`is_header_shadow`) + Makefile (`build/cfw/*.d` deps) fixes landed
   in the same pass — see HANDOVER m6c.

7. **DAC renderer — DONE.** The PCM ring feeds I2S1 → CS43131 through the
   TX-empty IRQ feeder (`firmware/drivers/rknano_i2s.c`), and since phase 3c the
   ring is produced by **Core B** (the AP keeps the DAC bring-up and the
   feeder). Full plan + evidence in **`docs/rewrite/dac.md`**; the reference
   sequence is documented in `docs/re/dac-output-path.md`. Superseded note:
   the earlier "player decodes but is silent" state no longer applies.

8. **AP flash (hardware-gated).** DFIU `preflight_check` → scatter
   byte-revalidation → flash AP5M (debrick ready: docs/DEBRICK-LINUX.md) →
   `verify_resolution(reproduction)` on the user's confirmed outcome.

## 4. Verification (docs/rewrite/architecture.md §5)

- Host: `cc -Wall -Werror -O1` for every new TU + the host tests in
   `firmware/player/tests/run_tests.sh` (extended with the FS/gapless/RG/tags
   tests; ~270 assertions, all green). Run before any commit.
- Target: `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -Wall -Werror -c`
   for codec_io_fs.c, tags.c (freestanding, no libc) and the existing player
   TUs. `rg_math.c` uses `<math.h>` on host; on a libm-less target it must be
   swapped for a fixed-point LUT (the player depends on it only once linked
   into the AP — milestone 2a).
- Hardware: gated by the pack_fw1.py scatter block rules
  (HANDOVER.md §AUDIO ROADMAP §5 DFIU state). No fw1 flash without a
  `preflight_check` ChangeSummaryV1 + `verify_resolution(reproduction)`.

## 5. Open items

1. ~~minimp3 / dr_flac vendoring~~ DONE (step 6) — MP3/FLAC decode, Xing/Info
   and Vorbis comments work; CPU budget on the B core still unmeasured.
2. ~~**DAC renderer**~~ DONE — the ring consumer landed as the I2S1 TX-IRQ
   feeder (route A) and FLAC plays on hardware (DFIU 75271e5f); the DMA
   ping-pong backend (route B) remains optional (§6).
3. B-core gain/DSP register mapping — where ReplayGain's final multiply is
   applied (AP pre-attenuation vs B-core DSP volume).
4. SD write + settings on target (the RG mode preference: track vs album)
   — needs the volume path convention from the storage workstream.
5. Warm reboot / high-speed USB — polish, not blocking audio.

## 6. Modular pipeline refactor (2026-10-16)

The audio path is now strictly layered (plan: `~/.commandcode/plans/
audio-pipeline-modular-refactor.md`):

```
player.c (state machine) -> codec_t (player/codec.h) -> codecs/{wav,mp3,flac}
   PURE transforms: codec_io only, zero MMIO; output = interleaved int32
   PCM, left-justified to 32 bits (the I2S slot word is the sample itself)
        |
        v
ipc_b_target.c          -- IPC contract glue + producer half of the ring
        |
        v
audio/audio_output.c    -- audio_output_t controller: format coordination
   |        (rate table in audio/ao_rate_table.c, pure/host-tested)
   +-> drivers/rknano_i2s.c  -- CRU clock tree, ACODEC PLL mux, I2S1, TX
   |                            feeder (polled + TX-empty IRQ) + BSP
   +-> drivers/cs43131.c     -- I2C0 master, MAP access, chip sequences
```

- **audio_output_* API** (`audio/audio_output.h`): `init`, `configure(fmt)`
  (the verified bring-up order: power GPIOs -> gates -> I2C probe -> rate
  table -> PLL mux -> I2S1 -> CS43131 sequence -> unmute), `start(ring)`,
  `stop/detach/hold/kick`, `frames_played`, `feed_blocking`, `power_off`.
- **Feeder seam** (`audio/audio_hal.h`): `ao_feeder_ops` binds
  `ao_feeder_ring_irq` (TX-empty IRQ draining the SPSC ring) today; a DMA
  ping-pong backend (stock descriptor ring at 0x03090000 -> TXDR) can
  implement the same ops later without touching codecs/player/ipc_b.
- **Slot packing** (`audio_hal.h ao_slot_word`): one 32-bit FIFO word per
  channel sample, `sample << 16`, frame = L word then R word (stock
  Bit_Convertor_* contract). Host test `tests/test_ao_slots.c` pins it and
  the 13 rate-table constants (the 2x-speed bug class, DFIU 75271e5f).
- **Volume path** (2026-10-16): `cs43131 dac_volume_set/dac_volume_ramp`
  program `PCM_VOLUME_B/A` (0x090001/0x090002, 0.5 dB/step attenuation,
  0x00 = 0 dB; chip resets to -60 dB — DFIU 19b83f5e). Bring-up fades
  0xFF -> 0x00 with the stock's soft ramp. Hardware attenuation is
  available for coarse user volume; ReplayGain stays DSP-scaled.
- **Play/pause fade** (BRNOMT-67): `dac_volume_ramp_ms(from, to, ms)` keeps
  the stock 6-code step but paces it over the configured duration
  (off/150/300/500 ms, key `fade`, default 300). `audio_output_fade_in/out`
  use it; `ipc_b_stop` uses the fixed `audio_output_quick_mute()` so skipping
  a track does not wait for the user fade. When a fade is active the unmute
  starts at 0xFF (silent) and `fade_in` ramps to the user level; with fade
  off the direct-volume first-Play path is unchanged. The fade blocks the
  caller, so `ipc_b_start`/`ipc_b_pause` keep `bfs_poll()` pumped during it
  to keep Core B's file ring fed. Pause also holds the AP TX feeder
  (`rknano_i2s_hold(1)`) once the fade completes, and resume releases it
  after the fade-in; otherwise the ISR drains the PCM ring while paused,
  masks its source on the dry ring and the resumed stream can come back
  silent with the player already reporting PLAYING.
- **ReplayGain** (player/rg_math.c): pure fixed-point Q1.15 (LUT-based
  dB->linear — no floats, no libm, no malloc), peak-limited clipping
  prevention (`rg_effective_gain_q16`), applied pre-ring by player.c.
  Tags: ID3v2 TXXX (MP3) + Vorbis comments (FLAC; Opus when ported);
  plain WAV has no RG source (NULL tags = neutral).
- **Hardware-verified code moved 1:1** from the old monolithic
  `drivers/dac_cs43131.c` (commit c5d571f): all MMIO defines live in
  `drivers/ao_regs.h` so every register/mask/shift line stayed textually
  identical (machine-checked against c5d571f during the move). Never
  "clean up" these sequences casually — that is exactly how the 2x-speed
  framing bug happened.
