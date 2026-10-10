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

Data path: codec decodes **interleaved int16 PCM** (the one stream format
of the whole pipeline) -> `ipc_b_ring_fill()` -> SPSC ring -> B core
consumes and renders. `player_process()` is the pump: it is called
periodically (UI loop on AP; an audio task on target) and refills the ring
from the codec in bounded rounds.

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

## 2. Codec interface (`codec.h`)

```c
typedef struct codec_s {
    const char *name;                                /* "wav", "mp3", ... */
    int  (*open)(void *st, const char *path);        /* 0 / CODEC_ERR_*   */
    int  (*decode)(void *st, int16_t *pcm, int max_frames);
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
  pool (`PLAYER_CODEC_STATE_MAX` = 4096, no malloc on target) and zeroes
  it before `open`. Codecs own nothing else.
- **Output format**: interleaved int16, 1 or 2 channels — matches the
  ring/DAC path and every vendored candidate's native s16 output.
- **Sample-rate reporting** (WAV deliverable): the `format()` extension
  reports `sample_rate/channels/bits`; the player forwards the rate to
  `ipc_b_start()` and uses it for position math. May be NULL (player then
  assumes 44100/2).
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
| Fault dump `B FAULT kind %u pc %08x lr %08x cfsr %08x` | §5d | `ipc_b_shared_t.fault_kind/pc/lr/cfsr` words; `IPC_B_FAULT` state -> player `PLAYER_ERROR` |
| Shared IPC block around `0x01020xxx` | §5c/§6b | `ipc_b_shared_t` mapped at `0x01020000` in `ipc_b_target.c` (our own field set — exact offsets TODO on hardware) |
| Mailbox-style doorbell between cores (SDK mailbox at `0x40110000`, channels 0-3) | firmware/ipc.h (our mailbox contract) | `target_doorbell()` candidate: channel 1 (MB_CH_DECODE); TODO confirm IRQ wiring |

OUR design choice (the reference does not tell us this, and we do not
look): **the AP decodes with our from-source codecs and streams PCM; the
B core renders PCM to the DAC/I2S path** (and may apply DSP, e.g. the
freestanding `rechord_dsp_core` for volume/EQ). This keeps all codec code
in our portable, host-testable tree and confines the B image to a
renderer — the cleanest split for the build-from-source rule.

## 4. Codec vendoring plan (architecture.md §4: prefer open-source C)

Stubs live now; the vendored trees will live under
**`firmware/player/codecs/`** (upstream files unmodified, version-pinned
here, licenses verified at vendoring time):

| Format | Implementation | License (verify at vendoring) | Location | Backed by | Notes |
|---|---|---|---|---|---|
| WAV | ours (`codec_wav.c`) | ours | in-tree | complete | reference codec |
| MP3 | **minimp3** (single header) | CC0-1.0 | `firmware/player/codecs/minimp3/minimp3.h` | `codec_mp3_stub.c` -> `codec_mp3.c` | s16 output natively; duration/seek via Xing/Info TOC, CBR fallback estimate (document accuracy class) |
| FLAC | **dr_flac** (single header) | MIT-0 / Unlicense (dual) | `firmware/player/codecs/dr_flac/dr_flac.h` | `codec_flac_stub.c` -> `codec_flac.c` | `drflac_read_pcm_frames_s16` matches codec_t exactly; frame-accurate seek; route `DRFLAC_MALLOC` to the static pool |
| Vorbis | stb_vorbis | MIT | `firmware/player/codecs/stb_vorbis/` | new `codec_ogg.c` (later) | CPU budget on B clock needs proving |
| Opus | opus reference (silk+celt) | BSD-3-Clause | `firmware/player/codecs/opus/` | new `codec_opus.c` (later) | biggest integration cost; last |

Rules for the vendored drop:

1. Upstream files are copied **unmodified**; all glue lives in our adapter
   `.c` files. Pin the exact upstream commit/version in this table when
   vendoring.
2. Nothing from third-party CFW: only upstream open-source projects,
   compiled from their `.c/.h` in this tree (build-from-source rule).
3. The decoder output must be interleaved int16 to fit `codec_t` (minimp3
   and dr_flac already do; no DSP glue needed).
4. Before enabling: host-measure frames-per-poll budget on a mock B clock
   (the `busy 10%, out min 5576` telemetry gives the reference workload
   shape), then size `PLAYER_CODEC_STATE_MAX` to the largest adapter state.

## 5. Files

```
firmware/player/
├── player.h            public API: track_info_t, states, events, calls
├── player.c            state machine + codec registry + ring pump
├── codec.h             codec_t interface (build-from-source codecs)
├── codec_io.h          byte-source seam (read/seek/tell/size)
├── codec_io_stdio.c    stdio backend (host + newlib target)
├── codec_wav.c         FULL WAV/RIFF PCM decoder (ours, reference codec)
├── codec_mp3_stub.c    minimp3 adapter scaffold (TODO vendoring)
├── codec_flac_stub.c   dr_flac adapter scaffold (TODO vendoring)
├── ipc_b.h             A<->B audio IPC contract + shared-block layout
├── ipc_b_mock.c        host: simulated B core (ring consumer + sink hook)
├── ipc_b_mock.h        host-only test knobs (tick rate, sink, fault)
├── ipc_b_target.c      target: shared block @0x01020000 + supervisor
├── ringbuf.h/.c        lock-free SPSC ring (PCM streaming)
├── player.mk           build fragment (root Makefile includes it)
└── tests/
    ├── run_tests.sh    cc -Wall -Werror build + run of all tests
    ├── test_util.h     PASS/FAIL harness (dsp-tests style)
    ├── mock_codec.c/.h scripted codec (sine-like generator + failures)
    ├── test_ringbuf.c  wrap / overflow / underflow / counter wrap
    ├── test_codec_wav.c WAV golden test (synthesized sine, exact match)
    ├── test_player_state.c state machine transitions + queue + errors
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
