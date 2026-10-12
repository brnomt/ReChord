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
  codec_t  (codec_wav fully ours; codec_mp3/codec_flac scaffolds for minimp3/dr_flac)
        |  interleaved int16 PCM frames
        v
  player.c state machine  --ipc_b ring->  B core (RKnanoD SDK audio) -> DAC/I2S
        |                                  B started @400 MHz -> /4 (100 MHz)
  dsp/mod_volume.c (AP-side gain: user volume + ReplayGain pre-attenuation)
```

Two cores, strict boundary:
- **A core (AP, our fw1):** UI + decode + tags + gapless + gain. This workstream.
- **B core (SDK):** PCM ring consumer -> DAC. Hardware-close; changed last.

The reference's `ECHOLOG.TXT` is the contract we match:
`B: boundary to track N: FLAC 44100 Hz, ~5840 frames queued (gapless)` and
`end of stream, 5843 frames queued`. Gapless is a decode+ring property of
the AP; the B core must not be restarted between gapless-compatible tracks.

Status today: the player module is **host-complete and host-tested** — WAV codec
byte-exact, and the gapless / ReplayGain / codec_io_fs paths are implemented and
covered by host tests in `firmware/player/tests/run_tests.sh`. It is still
**not linked into the AP image**: `firmware/ap.mk` excludes it ("NOT linked in
milestone 1a"; see §3.5). The target `codec_io_fs.c` backend over `fs_target`
(the "decode from SD" path) is implemented; the FS backend is exercised in host
tests through `fs_host.c`.

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
├── codec_wav.c         FULL WAV/RIFF PCM decoder (ours)
├── codec_mp3_stub.c    minimp3 adapter scaffold
├── codec_flac_stub.c   dr_flac adapter scaffold
├── ipc_b.h             A<->B audio IPC contract + shared block @0x01020000
├── ipc_b_mock.c        host simulated B core
├── ipc_b_target.c      target shared block + supervisor
├── ringbuf.h/.c        SPSC lock-free ring (PCM streaming)
├── rg_math.c/.h        ReplayGain dB->linear + PCM scaler (Q1.15)  <-- DONE
├── tags.c/.h           ReplayGain + ID3v2 TXXX / Vorbis parse  <-- DONE
├── player.mk           build fragment
└── tests/
    ├── run_tests.sh
    ├── mock_codec.c/.h scripted codec (gapless + RG-aware)
    ├── test_codec_wav.c        WAV golden (sine, byte-exact)
    ├── test_codec_io_fs.c      FS backend decode = stdio baseline  <-- DONE
    ├── test_player_gapless.c   gapless trim + seamless advance  <-- DONE
    ├── test_player_state.c
    ├── test_player_replaygain.c RH gain apply (scaled, peak-limited) <-- DONE
    └── test_rg_math.c          dB->linear + peak-clamp + scaling <-- DONE
```

## 3. Plan (in order — each host-testable before any hardware)

> Steps 1–4 are **implemented and host-tested** (55 checks across
> `test_player_gapless`, `test_rg_math`, `test_player_replaygain`,
> `test_codec_io_fs`, `test_tags`). Step 5 (AP linkage) is gated on hardware
> + DFIU preflight.

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

4. **ReplayGain parse.** DONE — `tags.c`/`tags.h` expose
   `tags_replaygain_id3v2(io, &rg)` (ID3v2 v2.3/v2.4 `TXXX` keys) and
   `tags_replaygain_vorbis(io, offset, length, &rg)` (Vorbis comments), both
   returning `codec_replaygain_t{track_gain_tenth_db, album_gain_tenth_db,
   track_peak_q15, album_peak_q15}`. `track_info_t` carries the fields; the
   codec_t `tags` callback populates them (NULL = no tags). Gain math lives in
   `rg_math.c` (`rg_gain_q16`, `rg_effective_gain_q16` peak-limited,
   `rg_scale_pcm`), applied in the PCM pump **before** the ring push (`>>15`
   Q1.15 scaling + saturation) — this is the AP-side pre-attenuation that composes
   with the B-core user volume; `PLAYER_RG_TRACK`/`_ALBUM`/`_OFF` mode is set via
   `player_set_replaygain_mode` (defaults OFF). Design note: routed through
   `rg_math.c` instead of the `dsp/mod_volume.c` chain so the path is host-testable
   end-to-end without wiring the DSP chain into the player; a future milestone
   can fold the gain into the dsp chain (§5.2). Host tests: `test_rg_math`
   (dB→linear, peak clamp, scaling/saturation) and `test_player_replaygain`
   (exact PCM scaling vs `rg_scale_pcm` on the mock).

5. **AP linkage (hardware-gated).** Only after host tests are green and a
   DFIU `preflight_check` passes: link `firmware/player` into `ap.mk`
   (milestone 2a), guarded by the `make_fw1.py`/`pack_fw1.py` scatter gate
   (no row below 0x03000000..0x03004000) and a reference-image diff, then a
   `verify_resolution(reproduction)` hardware verdict.

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

1. minimp3 / dr_flac vendoring — unblocks MP3/FLAC decode, Xing/Info and
   Vorbis comments. CPU budget on the B core still unmeasured.
2. B-core gain/DSP register mapping — where ReplayGain's final multiply is
   applied (AP pre-attenuation vs B-core DSP volume).
3. SD write + settings on target (the RG mode preference: track vs album)
   — needs the volume path convention from the storage workstream.
4. Warm reboot / high-speed USB — polish, not blocking audio.
