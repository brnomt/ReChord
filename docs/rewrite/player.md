# ReChord — Player workstream

> Scope: the audio player service — playback state machine, codec
> interface with FROM-SOURCE C codecs, and the dual-core (A<->B) IPC
> layer. Owned tree: `firmware/player/**` + this document.
> Constitution: `docs/rewrite/architecture.md` (build-from-source rule §1,
> codec strategy §4, verification standard §5). Protocol facts:
> `docs/re/refcfw-analysis.md`. HARD RULE: no third-party CFW code — the
> reference firmware's CODE is off-limits, its PROTOCOL FACTS are ours to
> implement cleanly.

## 1. Architecture

```
                 UI workstream (ui/player_screen, event loop)
                        |  player.h  (API + events)
                        v
   +--------------------------------------------------------------+
   | player.c — playback state machine                            |
   |   open -> play -> pause -> seek -> stop, next-track queue     |
   |   player_process(): pump codec -> ring, EOF/auto-advance      |
   |   event hook: STATE / TRACK / END_OF_TRACK / QUEUE_EMPTY /    |
   |               ERROR                                           |
   +-----------------------+----------------------+---------------+
                           | codec.h              | ipc_b.h
                           v                      v
   +--------------------+        +---------------------------------+
   | codec registry     |        | A<->B audio IPC                 |
   |  codec_wav.c  (us, |        |  ipc_b_mock.c   host: simulated |
   |    complete)       |        |    B core consuming the ring    |
   |  codec_mp3_stub.c  |        |  ipc_b_target.c target: shared  |
   |  codec_flac_stub.c |        |    block @0x01020000 + clock    |
   +---------+----------+        |    negotiation + supervisor     |
             | codec_io.h        +---------------+-----------------+
             v                                   | ringbuf.h/.c
   +--------------------+                        | (SPSC lock-free,
   | codec_io_stdio.c   |                        |  "ring rd/wr")
   |  (stdio backend;   |                        v
   |   target TODO:     |        +---------------------------------+
   |   services/fs)     |        | B core (audio renderer)         |
   +---------+----------+        |  PCM -> DSP -> I2S/DAC          |
             |                   |  started at 400 MHz, runs at    |
             v                   |  negotiated 100 MHz steady      |
        storage / files          +---------------------------------+
```

Data path: codec decodes **interleaved int32 PCM, left-justified to 32 bits**
(the one stream format of the whole pipeline; a 16-bit source lands at
`sample << 16`, 24-bit at `sample << 8`, 32-bit as-is — codec.h) ->
`ipc_b_ring_fill()` -> SPSC ring -> B core consumes and renders.
`player_process()` is the pump: it is called periodically (UI loop on AP; an
audio task on target) and refills the ring from the codec in bounded rounds.

### State machine (player.c)

```
  [none] --open--> STOPPED --play--> PLAYING --pause--> PAUSED
                     ^                 |  ^               |
                     |                 |  +-----play------+
                     +------stop-------+        |
                     ^                         v
                  ERROR <----(decode error / B FAULT)---+
                  ERROR --stop()/open()--> STOPPED
```

Semantics (crisp, so tests and UI agree):

| call | legal in | effect |
|---|---|---|
| `player_open(path)` | any | closes previous track (implicit stop), opens codec, emits `TRACK`; -> STOPPED |
| `player_play()` | STOPPED, PAUSED | STOPPED: `ipc_b_start()` (B core up at 400 MHz, clock negotiation follows); PAUSED: `ipc_b_resume`; -> PLAYING. Idempotent in PLAYING |
| `player_pause()` | PLAYING | ring content kept, B consumption paused; -> PAUSED |
| `player_stop()` | any | B core off + ring flush, **rewind to 0** (track stays queryable/open); -> STOPPED |
| `player_seek(ms)` | STOPPED, PLAYING, PAUSED | clamps to `[0, duration]`, codec reposition + ring flush (buffered PCM belongs to the old position) |
| `player_next()` | any | pops the next-track queue -> open + play; empty queue: stop + `QUEUE_EMPTY` + `PLAYER_ERR_EMPTY` |
| `player_process()` | PLAYING | poll supervisor, refill ring, EOF -> auto-advance queue or stop + `END_OF_TRACK` |

Events for the UI (`player_set_callback`): `PLAYER_EV_STATE`,
`PLAYER_EV_TRACK`, `PLAYER_EV_END_OF_TRACK`, `PLAYER_EV_QUEUE_EMPTY`,
`PLAYER_EV_ERROR`. Delivered synchronously from API calls.

Position: `player_get_position_ms()` = seek base + frames the B core
consumed since (`frames_consumed * 1000 / sample_rate`). It advances only
while the sink consumes — on the host that is per `player_process()` tick
(mock B core), which keeps the model deterministic.

### Playback list (BRNOMT-90)

`player_play_list(idx, n, start)` plays a list of **media_db record indices**
(indices, not paths: 706 paths would be ~90 KB; `PLAYLIST_MAX 1024` = 2 KB).
End-of-stream auto-advances within the list, `player_next()` follows it, and
`player_open()` clears it. `player_set_resolver()` maps an index to a path (the
app installs a `media_db_get`-backed resolver; host tests install stubs).
Rockbox-style shuffle (reorder; current track kept first) and repeat
off/all/one live in `playlist.c`; `track_info_t.playlist_pos/count` feed the
UI's "i/N".

**Gapless on Core B (BRNOMT-73).** With the decoder on Core B, the phase-2
"swap the AP codec on the live ring" trick does not apply — B owns the decoder.
Instead, at B's end-of-stream the AP opens the next file (`bfs_open`) and sends
B a **gapless track change** (`ipc_b_next` → `IPC_C_NEXT`): B reopens its codec
on the same file ring while the PCM ring cursors and the DAC/serializer stay
live, so same-format adjacent tracks join sample-accurately. The hand-off fires
at B's decoder EOF, while the 64 KiB PCM ring still holds the previous tail
(~85 ms), not at ring-empty. A format change is a hard boundary; when Core B
holds the decoder that stop/start is deferred (`restart_pending`) until the ring
drains so the previous tail is not truncated. The command channel is
**sequence-numbered** (`IPC_C_SEQ`/`IPC_C_ACK`): the old "cmd == ack" test
silently dropped a repeated command — two gapless `NEXT`s in a row, or two
`SEEK`s to the same ms. `advance_to()` opens the next codec first so the two
formats can be compared before committing to gapless vs hard.

**Pre-open (BRNOMT-73 option B).** The boundary still paid the SD open inside
`bfs_open`, which costs more than the 64 KiB PCM ring covers (~85 ms @96k; the
ring cannot grow — 0x01030000..0x01040000 is exactly the window below Core B's
data). A **second file ring** (`IPC_F2_*` / `IPC_FILE2_BASE` = 0x01029000,
16 KiB, the free gap) lets the AP **pre-open the next file ~3 s before the
end** (`ipc_b_preopen` → `bfs_preopen`, driven from `player_process()` with
`playlist_peek_next()`; once per track). At the boundary the ring is already
full: `bfs_promote()` swaps it in, `IPC_C_RING` tells B which ring to read, and
B reopens its codec **from RAM** (no SD latency). Two codecs are never open at
once — FLAC/MP3 claim the single `workbuf` at `open()` — so B pre-opens nothing;
it only reopens. A pre-open that does not match the boundary track is rejected
(hard path), and `b_cmd_wait` now allows 1500 ms so a slow reopen does not trip
the old 200 ms fallback into a DAC relock.

## 2. Codec interface (`codec.h`)

```c
typedef struct codec_s {
    const char *name;                                /* "wav", "mp3", ... */
    int  (*open)(void *st, const char *path);        /* 0 / CODEC_ERR_*   */
    int  (*decode)(void *st, int32_t *pcm, int max_frames);
                                                     /* frames, 0=EOF, <0 */
    int  (*seek)(void *st, long ms);                 /* clamp to duration */
    long (*duration_ms)(void *st);                   /* <0 if unknown     */
    void (*close)(void *st);                         /* idempotent        */
    int   state_size;                                /* bytes, static pool*/
    int  (*format)(void *st, codec_format_t *out);   /* optional ext.     */
} codec_t;
```

Rules and rationale:

- **State block**: the player allocates `state_size` bytes from a static
  pool (`PLAYER_CODEC_STATE_MAX` = 7296, no malloc on target) and zeroes
  it before `open`. Codecs own nothing else.
- **Output format**: interleaved int32, left-justified to 32 bits, 1 or 2
  channels — matches the ring/DAC path (the wire slot word is the sample
  itself); the source bit depth is reported through `format().bits`. s16
  codecs (minimp3, our WAV) widen with `<< 16`, dr_flac returns s32 directly.
- **Sample-rate reporting** (WAV deliverable): the `format()` extension
  reports `sample_rate/channels/bits`; the player forwards the rate and the
  source depth (`bits`) to `ipc_b_start()` and uses the rate for position
  math. May be NULL (player then assumes 44100/2).
- **Errors**: `CODEC_ERR_IO / FORMAT / UNSUPPORTED / STATE` (< 0).
  Unsupported-but-valid files must return `UNSUPPORTED`, malformed ones
  `FORMAT` (tested strictly for WAV).
- **codec_io seam**: codecs never touch stdio/FS directly; they use
  `codec_io.h` (`cio_open/read/seek/tell/size/close`). Host backend:
  `codec_io_stdio.c` (also valid with newlib on target). Swap point for a
  `services/fs` backend when storage lands.
- **Registry**: `player_register_codec(ext, codec)` dispatches by file
  extension (case-insensitive). Built-ins registered by `player_init()`:
  wav, mp3, flac. Host tests inject a scripted mock codec the same way.

### WAV reference codec (`codec_wav.c`, ours, complete)

Full RIFF/WAVE subset implementation — the codec that proves the pipeline
end-to-end from `.c`:

- WAVE_FORMAT_PCM (1) only, 16-bit, mono/stereo, any rate up to 192 kHz
  (reported via `format()`).
- Chunk walking with correct even-size padding (JUNK/LIST safe), fmt
  allowed after data, streamed `data` sizes (`0xFFFFFFFF`) clamped to the
  actual file length, declared-vs-actual size clamping.
- Strict rejection: 8-bit/24-bit/float/RIFX/other containers return
  `CODEC_ERR_UNSUPPORTED`/`CODEC_ERR_FORMAT` (golden-tested).
- Frame-accurate `seek(ms)` (int64 math — `ms * rate` overflows 32-bit
  `long` on the M3), stable EOF.

## 3. IPC protocol mapping — facts -> our clean-room implementation

Source of facts: `docs/re/refcfw-analysis.md` (binary/string analysis and
live device log of the reference firmware). Its CODE is off-limits; the
table records how OUR `ipc_b.h` maps the observable protocol behavior.

| Protocol FACT (observed) | Where documented | OUR implementation (clean-room) |
|---|---|---|
| Dual Cortex-M3: AP (UI/system) + B (audio) | refcfw-analysis §5d/§6c | player runs on AP; `ipc_b.h` is the only AP->B surface |
| B core runs the audio path, started at 400 MHz — `player: B start 0 at 400 MHz` (m78 loaded into HIGHRAM0, second core started) | §6c | `ipc_b_start(track)`: START command + `clock_div = 1` (400 MHz) in the shared block; state `IPC_B_STARTING` |
| PCM streams through a ring buffer — `ring rd %u wr %u` | §5d | `ringbuf.h/.c`: lock-free SPSC byte ring (pow2, free-running counters = the logged rd/wr pair, DMB on M3), filled via `ipc_b_ring_fill()` |
| Clock negotiation `B /1 -> /4 (100 MHz; busy 10%, out min 5576)` | §6c | `ipc_b_status_t.clock_div` (1 -> 4) + `busy_pct`; mock simulates the step, target TODO: CRU sequence |
| Supervisor state machine, 30 s timeouts, states "B drained", "B is gone", "B off" | §5c/§6b (`FUN_03057694`: states 0-4, 30 s timeout) | `ipc_b_state_t` OFF/STARTING/RUNNING/DRAINED/FAULT; `ipc_b_poll_state()` runs the supervisor (tick-budget guard; TODO convert to 30 s once poll cadence is fixed) |
| Fault reporting by B | §5d | `IPC_C_BERR` (negative codec error) + `IPC_C_BSTATE = ERROR`; a failed B command maps to player `PLAYER_ERROR`. The `ipc_b_shared_t` fault_kind/pc/lr/cfsr words in `ipc_b.h` are a legacy, **unreferenced** struct — do not code against them. |
| Shared IPC block at `0x01020000` | §5c/§6b | **`firmware/player/ipc_shared.h` is the contract**: fixed word indexes `IPC_SH_*` (state, PCM rd/wr, frames, rate/ch/bits) and `IPC_C_*` (command, arg, ack, seq, ring, RG/gap, prescan geometry, stall counter), plus the two file-ring flag sets `IPC_F_*` / `IPC_F2_*`. `ipc_b_shared_t` in `ipc_b.h` is legacy and unreferenced. |
| Mailbox-style doorbell between cores (SDK mailbox at `0x40110000`, channels 0-3) | firmware/ipc.h (our mailbox contract) | **Not used.** The shipped channel is the polling control block with the `IPC_C_SEQ`/`IPC_C_ACK` handshake; the PCM ring is drained by the AP's I2S TX IRQ. |

Design note — **superseded by phase 3c**: B **decodes** with our from-source
codecs and is the PCM-ring producer; the **AP renders** it (DAC/I2S bring-up +
the TX-empty IRQ feeder) and serves B's file reads over the A→B file rings.
Codec code thus stays in our portable, host-testable tree, while the B image
carries the decoders + `rg_math.c` + the shared `workbuf`.

## 4. Codec vendoring plan (architecture.md §4: prefer open-source C)

Stubs live now; the vendored trees will live under
**`firmware/player/codecs/`** (upstream files unmodified, version-pinned
here, licenses verified at vendoring time):

| Format | Implementation | License (verify at vendoring) | Location | Backed by | Notes |
|---|---|---|---|---|---|
| WAV | ours (`codec_wav.c`) | ours | in-tree | complete | reference codec |
| MP3 | **minimp3** (single header) | CC0-1.0 | `firmware/player/codecs/minimp3/minimp3.h` | `codec_mp3_stub.c` -> `codec_mp3.c` | s16 output natively; duration/seek via Xing/Info TOC, CBR fallback estimate (document accuracy class) |
| FLAC | **dr_flac** (single header) | MIT-0 / Unlicense (dual) | `firmware/player/codecs/dr_flac/dr_flac.h` | `codec_flac_stub.c` -> `codec_flac.c` | `drflac_read_pcm_frames_s32` is already int32 left-justified = codec_t's format; frame-accurate seek; `DRFLAC_MALLOC` routes to the shared **workbuf** region (`services/workbuf.h`), sized for the 8192-sample stereo decode window (~69.9 KiB) |
| Vorbis | stb_vorbis | MIT | `firmware/player/codecs/stb_vorbis/` | new `codec_ogg.c` (later) | CPU budget on B clock needs proving |
| Opus | opus reference (silk+celt) | BSD-3-Clause | `firmware/player/codecs/opus/` | new `codec_opus.c` (later) | biggest integration cost; last |

Rules for the vendored drop:

1. Upstream files are copied **unmodified**; all glue lives in our adapter
   `.c` files. Pin the exact upstream commit/version in this table when
   vendoring.
2. Nothing from third-party CFW: only upstream open-source projects,
   compiled from their `.c/.h` in this tree (build-from-source rule).
3. The decoder output must be interleaved int32 left-justified to fit
   `codec_t` (dr_flac's s32 already is; minimp3's s16 is widened with
   `<< 16`).
4. Before enabling: host-measure frames-per-poll budget on a mock B clock
   (the `busy 10%, out min 5576` telemetry gives the reference workload
   shape), then size `PLAYER_CODEC_STATE_MAX` to the largest adapter state.

Codec contract (2026-10-16 refactor, `player/codec.h`): the vtable IS the
unified codec interface — `init()` (optional one-time setup, called from
`player_register_codec()`; the hook a fixed-point Opus port would use),
`open(file)`, `format()` = `get_format(sample_rate, bits, channels)`,
`decode()` = `decode_frame(pcm, max)`, `close()`, plus the optional
seek/duration/gapless/tags extensions. Codecs stay pure software
transforms (codec_io only — no I2C/I2S/clock/DMA knowledge); the
hardware side is the `audio_output` HAL (docs/rewrite/audio.md §6).
**Opus is a reserved drop-in slot**: porting it means adding
`codecs/opus/` + one `codec_opus.c` descriptor — zero core changes.

**Time math is 64-bit (2026-10-17, HANDOVER §O):** on the target `long` is
32 bits, so every ms <-> samples/frames conversion needs an `int64_t`
intermediate (`(long)(((int64_t)samples * 1000) / rate)`). A 32-bit product
already wraps above ~48 s at 44.1 kHz, and the wrapped (often negative)
result is what Now Playing showed as a bogus "0:02"/"0:19" track length
while playback itself stayed correct (BRNOMT-65/66). `codecs/wav.c` always
had the cast; `mp3.c` (Xing duration, CBR total_samples, seek), `flac.c`
(duration, seek frame) and `player.c` (`player_get_position_ms`) now match
it. Cover with a >48 s fixture whenever adding a conversion.

## 5. Files

```
firmware/player/
├── player.h            public API: track_info_t, states, events, calls
├── player.c            state machine + codec registry + ring pump + hand-off
├── playlist.h/.c       playback list (indices, shuffle, repeat, peek/advance)
├── codec.h             codec_t interface (build-from-source codecs)
├── codec_io.h          byte-source seam (read/seek/tell/size)
├── codec_io_stdio.c    stdio backend (host + newlib target)
├── codec_io_fs.c       fs.h backend (target: SD/eMMC)
├── ipc_shared.h        authoritative shared block + file-ring map
├── ipc_b.h             A<->B audio IPC API (target + mock share it)
├── ipc_b_mock.c        host: simulated B core (ring consumer + sink hook)
├── ipc_b_mock.h        host-only test knobs (tick rate, sink, fault)
├── ipc_b_target.c      target: shared block @0x01020000 + supervisor + feeder
├── bfs.h/.c            A-side file service (current + pre-open rings, promote)
├── ringbuf.h/.c        lock-free SPSC ring (PCM streaming)
├── rg_math.h/.c        ReplayGain dB->linear + PCM scaler
├── tags.h/.c           ID3v2 / Vorbis tags + ReplayGain
├── player.mk           build fragment (root Makefile includes it)
├── codecs/             one file per codec + vendored trees
│   ├── codec_registry.h/.c extension -> codec_t dispatch
│   ├── wav.c           FULL WAV/RIFF PCM decoder (ours)
│   ├── mp3.c           minimp3 adapter (Xing/LAME gapless + RG)
│   ├── flac.c          dr_flac adapter (workbuf pool + prescan + RG)
│   ├── minimp3/        vendored (CC0-1.0, pinned) + LICENSE
│   └── dr_flac/        vendored (MIT-0, pinned) + LICENSE
└── tests/
    ├── run_tests.sh    cc -Wall -Werror build + run of all tests
    ├── test_util.h     PASS/FAIL harness (dsp-tests style)
    ├── mock_codec.c/.h scripted codec (sine-like generator + failures)
    ├── test_ringbuf.c  wrap / overflow / underflow / counter wrap
    ├── test_codec_wav.c WAV golden test (synthesized sine, exact match)
    ├── test_playlist.c playlist / shuffle / repeat
    ├── test_player_state.c state machine transitions + queue + errors
    ├── test_player_gapless.c trim, continuity, Core-B hand-off, repeat
    ├── test_codec_flac.c / test_codec_mp3.c / test_tags.c codec goldens
    └── test_ipc_mock.c end-to-end: play -> B consumes -> position advances
```

API summary (other workstreams code against `player.h`):

| call | purpose |
|---|---|
| `player_init()` | registry + IPC init |
| `player_open(path)` / `player_play()` / `player_pause()` / `player_stop()` | lifecycle |
| `player_seek(ms)` / `player_next()` | navigation |
| `player_get_state()` / `player_get_position_ms()` | `PLAYER_STOPPED/PLAYING/PAUSED/ERROR`, ms |
| `player_set_volume(db)` / `player_get_volume()` | volume in dB (clamped -60..+20), forwarded to B |
| `player_get_track(track_info_t*)` | title/artist/path/track_no/duration_ms |
| `player_set_callback(fn, user)` | UI event hook |
| `player_register_codec(ext, codec)` | codec registry (mock injection for tests) |
| `player_enqueue(path)` | fill the next-track queue |
| `player_process()` | pump one streaming step (call periodically) |

IPC summary (`ipc_b.h`): `ipc_b_start(track)`, `ipc_b_stop()`,
`ipc_b_set_volume(db)`, `ipc_b_poll_state(status*)`,
`ipc_b_ring_fill(pcm, frames)` + extensions `ipc_b_init()`,
`ipc_b_pause(on)`, `ipc_b_flush()` (seek).

## 6. Verification (2026-10-09, this change)

- Host tests: `firmware/player/tests/run_tests.sh` —
  `test_ringbuf`, `test_codec_wav`, `test_player_state`, `test_ipc_mock`
  — **ALL PASS** (cc -Wall -Werror -O1; 220 checks, 0 failures).
  Highlights: WAV golden decode is byte-exact vs a synthesized 440 Hz
  sine (plus an independent sine cross-check), ring wrap/overflow/underflow
  and free-running counter wrap are covered, and the e2e test proves
  "play -> B consumes -> position advances" with frame-exact PCM through
  the ring (88200/88200 frames) and the /1 -> /4 clock step.
- Target: every non-test TU compiles with
  `arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -Wall -Werror -c`
  (player.c, ringbuf.c, codec_io_stdio.c, codec_wav.c, codec_mp3_stub.c,
  codec_flac_stub.c, ipc_b_mock.c, ipc_b_target.c).

## 7. Open items

1. **IPC register bring-up** (blocked on hardware): doorbell/IRQ channel
   (candidate MAILBOX1/IRQ 10 at `0x40110000` per `firmware/ipc.h`),
   exact shared-block word offsets at `0x01020000`, CRU sequence for the
   /1 -> /4 clock negotiation, physical home of the PCM ring (B-visible
   DMA window), fault-dump word order (`kind/pc/lr/cfsr`). All marked
   `TODO(bring-up)` in `ipc_b_target.c`.
2. **Supervisor timing**: convert the poll-tick budget to the documented
   30 s timeout once the poll cadence is fixed on target.
3. **codec_io target backend**: wire newlib syscalls onto the storage
   driver, or swap in a `services/fs`-backed codec_io (decision with the
   storage workstream).
4. **Vendor minimp3 + dr_flac** per §4 (paths and licenses above), then
   flip `codec_mp3_stub.c`/`codec_flac_stub.c` to real adapters and pin
   versions. Host-measure decode budget first.
5. **Metadata**: `track_info_t.title/artist` currently come from the
   filename heuristic; parse ID3v2 (mp3) / Vorbis comments (flac) /
   RIFF LIST-INFO (wav) in the adapters when they land.
6. **WAV edge formats** (explicitly out of the current subset):
   WAVE_FORMAT_EXTENSIBLE, 24/32-bit PCM, RIFX. Add when a real need
   shows up; rejection is clean (`UNSUPPORTED`) today.
7. **Volume application point**: `player_set_volume()` is forwarded to
   the B core (`ipc_b_set_volume`); where the gain is applied (B-side
   DSP, e.g. `rechord_dsp_core` volume module, vs DAC registers) is the
   audio-DAC bring-up's call. Note: `docs/rewrite/` cfg evidence shows
   stock volume stored as dB (`volume=-23`), matching our units.
8. **Module packaging**: when the overlay module format lands
   (route-b-minimum §6), package the player as a loadable module
   (`mod: m%02u` container) and re-check `PLAYER_CODEC_STATE_MAX` and
   ring placement against the module's bss window.
9. **B-core firmware itself** (the renderer image for the second core)
   is out of this workstream's scope — the target IPC stub is written so
   the player works as soon as something honors the shared-block contract
   (even a stub B that just advances `frames_consumed`).
