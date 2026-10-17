# Next session — FLAC 24/32-bit + sample rates up to 384 kHz (BRNOMT-89)

This document is self-contained: a fresh session should be able to start from
here without any prior conversation. It says what to build, what to read, what
the current state is, and how to verify each step on the device.

---

## 1. The goal

Play **FLAC at 24 and 32 bits**, at **44.1 / 48 / 88.2 / 192 / 384 kHz**, and
**tell the DAC the real format** so it reproduces that depth (not just decode
and truncate). The user's words: *"no es solo reproducirlo, sino comunicárselo
al DAC y que reproduzca como corresponda."*

Linear: **BRNOMT-89** (Todo, priority 2). The datasheet is in the repo:
`community/CS43131_DS1155F2.pdf`.

## 2. Current state (measured, not assumed)

The pipeline today is **16-bit end to end**:

- `firmware/player/codecs/flac.c` decodes with `drflac_read_pcm_frames_s16`
  (line ~412). 24/32-bit files *play*, but truncated to 16 bits.
- `firmware/audio/audio_output.h` defines the seam:
  ```c
  typedef struct { uint32_t sample_rate; uint8_t bits; uint8_t channels; } audio_format_t;
  ```
  `bits` is already plumbed and documented as "16 supported now; 24/32
  reserved"; `audio_output_configure(fmt)` **accepts bits==16 only**.
- `firmware/drivers/rknano_i2s.h`: `i2s1_setup(rate, mclk)`; the TX feeder
  pulls **one packed word per frame** `L=[15:0], R=[31:16]` and the driver
  expands each to the wire as `L<<16, R<<16` — i.e. **the wire slot is already
  32-bit**, carrying 16-bit samples in the top half. Widening to 24/32 bits is
  mostly "put the full sample in the slot".
- `firmware/audio/ao_rate_table.c`: 13 rates, **8 kHz … 192 kHz**, each entry
  `{rate, mclk, {4 config bytes}, index}`. 44.1/48/88.2/96/176.4/192 already
  work. **384 kHz has no entry.**
- `firmware/drivers/cs43131.c` + `firmware/audio/audio_output.c`: the DAC
  bring-up is the *verbatim* stock `dac_init()` sequence; register writes go
  through `dac_map_write(map24, val)` (`cs43131.h`). Facts/comments:
  `docs/rewrite/dac.md` §6.
- Decoders other than FLAC (MP3/WAV) produce s16; the DSP chain
  (`firmware/dsp/`, Q2.30 over s16) sits between decode and output.

## 3. What the datasheet already answers

Extracted from `CS43131_DS1155F2.pdf` (do not re-derive; verify on hardware):

- **Sample bit size is a register field**: `ASP_SPSIZE` / `XSP_SPSIZE`, with
  `00 = 32 bits`, `01 = 24 bits`; `ASP_RX_CHn_RES = 11` means 32 bits per
  channel. This is the "tell the DAC" part.
- **Clock generation, Tables 4-6/4-7 (page 51)**: columns are
  `LRCK rate / ×64 / xSP_LCPR / xSP_N[15:0] / xSP_M[15:0]`.
  With **MCLK 24.576 MHz** there are entries for 352.8k and **384k**:
  `384.000 → 64 → 63 → N=1 → M=1`. The text states that for 384 kHz in Master
  Mode, MCLK_INT must be **24.576 MHz** (the 48k family; 44.1k family uses
  22.5792 MHz).
- The chip is 32-bit / 384 kHz by specification: the current ceiling is our
  configuration, not the DAC.

## 4. Increments (each one buildable, flashable and testable on its own)

Do them in this order; do not mix two in one A/B flash.

### 4.1 Internal s32 path
- `codecs/flac.c`: `drflac_read_pcm_frames_s32` (samples are left-aligned per
  source bps). Decide the internal convention: **int32, left-aligned** (the
  same shape dr_flac returns), then every consumer shifts as needed.
- `codecs/mp3.c`, `codecs/wav.c`: widen s16 → s32 (shift left 16).
- `player/ringbuf`, `audio_output`, `rknano_i2s`: hold 32-bit frames; the TX
  pull already expands to 32-bit wire words, so the change is the *contents*.
  The 32 KiB PCM ring keeps its size (half the frames per fill; the feeder
  refills).
- DSP: **bypass when the source is >16 bits** (the Q2.30 s16 chain would
  truncate). Porting the modules to s32 is a separate, later task.
- Tests: host bit-exactness against a reference WAV for a 24-bit and a 32-bit
  fixture (generate with `flac`/`ffmpeg`, see §6).

### 4.2 DAC format per file
- `audio_output_configure()`: accept `bits` 16/24/32; set `ASP_SPSIZE`
  (32 for 24- and 32-bit sources is the simplest correct choice: the sample is
  left-aligned and the DAC ignores the unused low bits; verify), through
  `dac_map_write`.
- Callers (`player.c` open path) must pass the *source* bit depth from the
  codec descriptor / FLAC STREAMINFO.

### 4.3 384 kHz
- Add the entry to `ao_rate_table.c`, derived from the datasheet's N/M (1/1)
  with MCLK 24.576 MHz, using the existing 192k entry as the encoding template
  for the 4 config bytes.
- Confirm the I2S clock path can produce the required bit clock
  (`rate × 64 = 24.576 MHz`) and that `i2s1_setup(384000, 24576000)` is valid
  on this SoC (RKNanoD TRM is in `community/`).
- If the derived bytes do not lock, reverse the stock path
  (`docs/re/refcfw-analysis.md`, `community/refcfw/`) instead of guessing.

### 4.4 Verification matrix
For each of 44.1 / 48 / 88.2 / 192 / 384 kHz × 16 / 24 / 32-bit: opens, plays,
no artifacts, and the DAC reports/locks the rate (the existing diag rows and
the LOG are the tools).

## 5. Build, flash, test

From `/home/brnomt/Desktop/ReChord` (branch `master`; remote
`git@github.com:brnomt/ReChord.git`, push over SSH — the HTTPS remote has no
credentials):

```sh
make cfw-img                     # AP image; prints "RESULT: PASS" + AP5M CRC
arm-none-eabi-nm -n build/cfw/rechord_ap.elf | grep _bss_end   # must stay < 0x0304A000
sha256sum build/ReChord_AP5M.IMG
```

Flash (device mounted as `ECHO MINI`; unmount by *path*, never by device
letter — the letter changes between sessions):

```sh
cp build/ReChord_AP5M.IMG "/run/media/brnomt/ECHO MINI/HIFIEC90.IMG" \
  && sync && umount "/run/media/brnomt/ECHO MINI" \
  && sha256sum build/ReChord_AP5M.IMG
```

Known-good fallback image kept in `build/`: `ReChord_AP5M_known-good_4D1ED427.IMG`
(boots, AP5M CRC 0x4D1ED427). Debrick: `docs/DEBRICK-LINUX.md` (maskrom).

Host test suites (all must pass):

```sh
(cd firmware/services/tests && sh run_tests.sh)
(cd firmware/ui/tests       && sh run_tests.sh)
(cd firmware/player/tests   && sh run_tests.sh)
cc -Wall -Werror -O2 -o /tmp/t_theme firmware/theme/tests/test_theme.c \
   firmware/theme/theme.c firmware/theme/themes_builtin.c firmware/theme/theme_file.c && /tmp/t_theme
python3 tools/test_fs_target.py        # FAT harness: slow (loop-mounted writes), run with a long timeout
```

`firmware/libc/tests` does **not** build on this host gcc (a pre-existing
`strncasecmp` builtin conflict); it is not part of the gate.

## 6. Fixtures

Generate with the system tools, not by hand:

```sh
ffmpeg -f lavfi -i "sine=frequency=997:sample_rate=384000" -t 2 \
       -sample_fmt s32 -c:a flac /tmp/t384_32.flac
# variants: -sample_fmt s16/s32, -ar 44100/48000/88200/192000/384000
ffmpeg -i /tmp/t384_32.flac /tmp/ref384.wav     # reference for bit-exactness
```

Keep fixtures out of git (they are large); generate them in the test script or
check in only tiny ones (< 64 KB) if a test needs a fixed blob.

## 7. Rules this codebase enforces (read before changing anything)

- **`.bss` ceiling 0x0304A000**: the AP's usable RAM ends there; the linker
  script refuses builds above it (DFIU `fb41cb15` / rule `R-8cd5fb0eb24a`).
  Current `_bss_end` is `0x0304949c` (~2.9 KB free). Big work buffers go in
  **`services/scratch.h`** (one 32 KiB region, single owner, claim/release), not
  in statics.
- **`long` is 32-bit on target, 64-bit on host**: every ms ⇄ samples conversion
  needs an `int64_t` intermediate (DFIU `R-51cefba856b2`). Anything past
  ~48.7 s at 44.1 kHz wraps otherwise.
- **Weak stubs silently disable features**: before believing a feature works,
  check `arm-none-eabi-nm <elf> | grep ' W '` (rule `R-7cd6518a7e7b`).
- **A fallback that hides a broken dependency amplifies the bug**: log the
  first time it engages (rule `R-e68bf2428377`; this is how the dead DWT clock
  hid for weeks).
- **Modules own the lifetime of shared state they read** (rule
  `R-3f66ea5d12c8`): the query module opens the index; a test must enter
  through the same door.
- **Never add a limit that discards data without a symptom** (the scan depth cap
  silently dropped 110 tracks; rule recorded in DFIU).
- **UI geometry**: every list on the 320×170 panel sizes rows from the
  footer-safe area, page == window, no right-hand column that cannot fit
  (rule `R-1ddae0f0c00b`).
- **`make` does not track flag changes**: after editing CFLAGS, `rm -rf
  build/cfw/objs` before rebuilding, or a stale object links (it overflowed the
  gate once).
- Host tests build with `-Wall -Werror`: no `snprintf` into a smaller buffer
  (use bounded copies), no unused functions.
- **DFIU**: `.dfiu/artifacts/active_rules.md` lists every active rule; use the
  DFIU tools to record failures/rules rather than notes in code.

## 8. What to read, in this order

1. `docs/rewrite/player.md` — codec contract, the int64 rule, ReplayGain.
2. `docs/rewrite/dac.md` §6 — the DAC bring-up facts and the register naming.
3. `docs/rewrite/services.md` — the services layout, the scratch region, the
   index.
4. `docs/HANDOVER.md` — the session log. The most relevant sections:
   §O (the int64 ms bug), §R/§T (the `.bss` ceiling), §U (the SysTick clock),
   §Y–§AG (the index: append/patch primitives, scanner, query module,
   categories) — this is the code the index work left behind.
5. `firmware/audio/ao_rate_table.c`, `firmware/audio/audio_output.c`,
   `firmware/drivers/rknano_i2s.c`, `firmware/drivers/cs43131.c`.
6. `community/CS43131_DS1155F2.pdf` — Tables 4-6/4-7 (page 51) for the clock
   dividers; the `ASP_SPSIZE` / `ASP_RX_CHn_RES` fields for the word length.
7. `community/RKNanoD.pdf` + the TRM for the I2S clock tree (384 kHz bit
   clock = 24.576 MHz).

## 9. State at hand-off

- Branch `master`, everything committed and pushed. Last commit: the DFIU
  artifacts for `R-3f66ea5d12c8`.
- Last image flashed and confirmed by the user: `0x240B05BE` (sha256
  `4012fe36…`) — it boots, the media index works (709 tracks), the Library
  categories work, the SysTick clock works (real LOG timestamps).
- The hold-to-repeat acceleration was still unconfirmed by the user on the
  build with the fixed stuck-clock threshold (`0xC49E04C3`); it is cosmetic and
  deprioritised. If it is retested and works, verify DFIU incident
  `26eed982`'s follow-up and close BRNOMT-80.
- Open Linear items: **BRNOMT-89** (this work), BRNOMT-78 In Review
  (categories verified; an open observation: more artists than expected fall
  into "Undefined" — check one file's tags before assuming our parser is at
  fault), BRNOMT-74 Done, BRNOMT-71 In Review, BRNOMT-87 Backlog (resume,
  `/.rechord/state`).
- Unrelated untracked files in the repo root: `.kiro/`, `interrupt2.d`,
  `kb.sqlite3`, `store_dump.bin` — leave them alone.

## 10. Open questions for the user / hardware

1. For 16-bit sources, should the DAC stay in 16-bit mode (bit-exact) or always
   run in 32-bit slots? (Simplest and most uniform: 32-bit slots everywhere;
   confirm it sounds identical.)
2. 384 kHz needs MCLK 24.576 MHz — confirm the board's MCLK source can be
   switched per rate (the rate table's `mclk` column suggests yes).
3. Should the DSP apply to hi-res at all, or is bypass-only acceptable for v1?
4. Gapless/crossfade paths (`player.c` transitions) currently assume 16-bit
   frames; the s32 change must keep them sample-accurate.
