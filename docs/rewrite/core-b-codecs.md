# Core B codec budget (AAC / Opus / Vorbis)

This note records why item 9 (AAC/M4A + OGG/Vorbis/Opus) is **not** in the
current Core-B image, with measured numbers. It is the decision record for
2026-10-31: keep FLAC/MP3/WAV as the on-device decoders; revisit only with a
codec-overlay design.

## Measured Core-B budget

`build/cfw/bbcore/bb_core.elf` (current image):

```
text 74,612   data 112   bss 97,768
```

- Code region `BCODE` (HIGHRAM0): `0x01000000..0x01020000` = **128 KiB**.
  Free text after the current image: **~53 KiB**.
- Data region `BB_RAM`: `0x01040000..0x01060000` = **128 KiB**; `.data` +
  `.bss` = 97,880 B, and the stack grows down from `0x01060000`, leaving
  **~33 KiB** of stack. `.bss` already contains the 80,896 B shared decode
  pool (`WORKBUF_BYTES`) and the 8 KiB PCM chunk.

EchoPod's core1 programmes, built from `community/echopod` with the same
Cortex-M3 toolchain (`cmake -DECHO_BUILD_DEVICE=ON`, CMake Release,
arm-none-eabi 14.2.1) and measured with `arm-none-eabi-size`:

| Image | text | bss | extra | stack requirement |
|---|---:|---:|---:|---|
| `echo_core1_aac` | 62,948 | 1,736 | — | **48 KiB** (`core1_aac.ld` assert) |
| `echo_core1_opus` | 91,632 | 16,096 | `core1_big` buffers below `0x01020000` | 48 KiB (assert) |

These images are standalone: they include their own stream/demux/main and
are loaded at `0x01000000` on demand. They are not linked with FLAC/MP3/WAV.

## What does not fit

- **AAC** would add 62,948 B of text to 74,612 B = 137,560 B, i.e. **~10 KiB
  over** the 128 KiB code region. Its 48 KiB stack is also **~15 KiB over**
  the ~33 KiB currently available (the 80 KiB pool would have to shrink).
- **Opus** would add 91,632 B: **~39 KiB over** the code region before its
  `core1_big` buffers, and it also wants a 48 KiB stack.
- **Vorbis** was not ported/measured, but even a compact fixed-point decoder
  plus an OGG demux leaves no room for AAC and Opus in the same image.

The 80 KiB shared pool is already the limit for FLAC 192 kHz/16k blocks
(`WORKBUF_BYTES = 80,896`); it cannot absorb a 48 KiB decoder stack without
regressing FLAC.

## Why EchoPod can do it

EchoPod keeps **one decoder per core1 image** (`core1_aac.ld`,
`core1_opus.ld`, `core1_mp3.ld`) and loads the selected image at
`0x01000000` when a track of that type opens. Its core1 code region is the
same 128 KiB; the stack lives below `0x0102E000` and is reserved by the linker
assert. The device-side loader swaps the programme, so the decoders never
coexist.

ReChord currently loads a single `bb_core_blob` at boot. Adopting the overlay
model needs, at minimum:

1. Per-format B images (AAC, Opus, Vorbis) built at fixed `0x01000000`.
2. A storage/update path for those images: a section in `HIFIECnn.IMG`
   written to `/.rechord/codecs/*.bin` by the updater, or an equivalent
   asset mechanism (the AP image's 24 KiB of free code cannot carry them).
3. An AP loader that copies the selected image into `0x01000000`, resets
   Core B, and reports the codec to the player/IPC.
4. A per-format data/stack map (AAC/Opus reserve 48 KiB below `0x0102E000`;
   the shared decode pool and PCM ring must not overlap it).

That is an architecture change, not a decoder port. The current decision is to
document the limit and keep FLAC/MP3/WAV.

## Alternatives considered

- **AAC only in the main image**: free ~10 KiB of B code (e.g. move MP3 to the
  AP) and shrink the 80 KiB pool to ~65 KiB for FAAD2's 48 KiB stack. This
  trades FLAC headroom and gapless MP3-on-B for one format; not chosen.
- **Decode on the AP**: the AP has ~24 KiB of free code and no spare .bss;
  FAAD2 (63 KiB with demux) does not fit; the 80 KiB workbuf is not a code
  region.
- **Compress the codec into the AP image**: the AP code segment is 24 KiB
  from its limit (`0x03081970` vs `0x03088000`), and a codec image must still
  be copied into B's 128 KiB code region; no net win.

## Reproduce

```
cd community/echopod
cmake -S . -B /tmp/echobuild -DECHO_BUILD_DEVICE=ON -DECHO_BUILD_SIM=OFF \
      -DECHO_BUILD_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
      -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build /tmp/echobuild --target echo_core1_aac echo_core1_opus -j4
arm-none-eabi-size /tmp/echobuild/platform/device/echo_core1_aac.elf \
                   /tmp/echobuild/platform/device/echo_core1_opus.elf
arm-none-eabi-size build/cfw/bbcore/bb_core.elf
```
