# ReChord — DAC workstream (CS43131 → first audible sound)

> Scope: get audible sound out of the Echo Mini's **CS43131** DACs from our
> from-source fw1. Owned: the I2S+DAC bring-up, the PCM→ROM-buffer feed,
> and (fallback) our own CS43131 I2C driver. Constitution: no third-party
> CFW code; hardware facts / interface contracts (datasheet, stock call
> sequences, ROM API contracts) are fair game.
> Sources: `community/CS43131_DS1155F2.pdf` (local datasheet),
> `docs/re/decomp/apps/audio/*` (stock 3.7.0 section_3 decomp, annotated),
> `docs/re/decomp/stubs/rom_api_stubs.c` (ROM API inventory), SDK
> `driver/IIS/I2s.c` + `driver/I2C/i2c.c`.

## 1. The chip: CS43131 (datasheet facts)

- **One CS43131 is a STEREO DAC** with an integrated ground-centered stereo
  headphone driver (HPOUTA + HPOUTB = L/R of that one chip), 130 dB DR,
  up to 384 kHz, **Mono Mode (differential) support**, HP impedance
  detection. So a single chip already does L+R for one jack.
- **Two chips on the board** therefore means one of:
  a. **One chip per channel in Mono (differential) mode** — §5.12 gives the
     exact 24-byte register sequence (MAP `0x180009`..`0x180040`) to enable/
     disable mono mode. Doubles per-channel drive; a known DAP technique and
     the user's L/R intuition matches this topology.
  b. Two separate outputs (HP path + line-out — the stock ROM routing
     distinguishes `g_audio_routing == 0` (headphone) vs line-out, each with
     its own left `(...,0)` / right `(...,0xe)` path ops).
  *Confirmed how?* I2C scan (§3) reads both chip addresses; wiring topology
  then decides. Not required to start: the ROM's routing calls abstract it.
- **Serial input (ASP)**: I²S / LJ / RJ / TDM / DoP; **master or slave**
  (`xSP_M/S`) — we run the SoC as I2S **master**, chips as **slave**.
  Before enabling the port: set `xSP_SPRATE` (rate) and `ASP_SPSIZE` (bit
  depth). ASP pins = SDIN1/SCLK1/LRCK1 (XSP/SDIN2 = DSD path, unused).
- **Control port (§4.13)**: I²C, up to 1 MHz. **Register access = 3-byte
  MAP address (24-bit) + control byte (bit0 = INCR autoincrement) + data**
  — a write is `[chipaddr+W][MAPhi][MAPmid][MAPlo][ctl][data…]`. Chip
  address bits `[6:3] = 0b0110` → 7-bit range `0x30..0x37`, lower bits set
  by the ADR strap (two chips ⇒ two distinct addresses).
- **Power sequencing (§5.2)**: VP first; all other supplies any order;
  release `RESET` last. Power-down: assert RESET first, VP last.
- **Clocking (§5.1)**: `MCLK_INT` must be 22.5792 or 24.576 MHz; if the
  external MCLK is neither, the PLL must be programmed over I²C (start-up
  uses RCO, switches to PLL in ≤1 ms). Crystal tuning register `0x20052`
  only matters if we use the chip's XTAL mode (board feeds MCLK from the
  SoC, so likely PLL path).

## 2. The shortcut: the mask ROM already owns this (stock sequence)

Stock/ref firmware does **not** bit-bang the CS43131 themselves — the
**mask ROM** (chip HAL, `0x00000000..0x02FFFFFF`) exposes audio APIs that
do I2S + DMA + DAC-I²C. Stock section_3's bring-up (annotated decomp,
`docs/re/decomp/apps/audio/music_service.c`) is:

```c
event_set(0xf7);
rom_peripheral_reset();                  /* reset audio peripherals       */
audiocodec_open(48000);                  /* stock codec-IC open (jumptable)*/
rom_i2s_master_config();                 /* I2S master block config        */
rom_dma_buffer_config();
rom_dma_channel_set(4, 1);               /* audio DMA on channel 4         */
rom_i2s_master_init(0, 0x17, 0, 1);      /* format/clock args as stock     */
rom_dac_gain_set(gain);
rom_dac_gain_set_channel(0, gain);
rom_buffer_calculate(&buf_size, &buf_count);
rom_memzero(buf_size, buf_count << 2);
rom_dma_set_source(buf_size, buf_count, dma_src);
if (rate is 44.1k family) {              /* != 0x2EE00 check in stock     */
    rom_clock_divider_set();
    rom_pll_reconfig();                  /* SoC-side PLL for 44.1k        */
    rom_audio_path_set_441(mode);
}
rom_dac_output_enable(mode);
while (rom_buffer_ready(5)) hifi_busy_delay(...);   /* wait for buffer    */
/* routing (headphone vs line-out), each with LEFT + RIGHT ops: */
rom_audio_path_set(2, 0);      rom_audio_path_enable(2, 0, 1);
rom_audio_path_select(2, 0);   rom_audio_path_set(2, 0xe, 0);
rom_audio_path_enable(2, 0xe, 1); rom_audio_path_select(2, 0xe, 1);
rom_playback_start(1, 2, 1, sample_rate);
rom_audio_path_set(1, 4, 0); rom_audio_path_enable(1, 4); rom_audio_path_select(1, 4);
rom_dma_start(5, buf_size, dma_dest, buf_count);
rom_volume_sync();
rom_dac_unmute(1, 4);                    /* finally: audible              */
```

Volume path (stock `volume_control.c`):
`rom_dac_volume_set(0x1a); rom_dac_output_config(offset); rom_dac_commit(0x1a);`
Mute: `rom_dac_mute(1, 4)` / unmute `rom_dac_unmute(1, 4)`.
PCM refill into the ROM playback buffer: `rom_dma_or_copy(dst, src)`
(`MusicPlay_AudioCallBack.c`), driven by `audio_stream_process.c`.

**Caveats (why this is "experimental"):**
- Arg counts/types come from annotated decomp + `rom_api_stubs.c`
  (STUBn = n args) — good but not authoritative; a wrong call likely hangs
  the core, not the flash (fw1 keeps its updater; worst case maskrom).
- In stock these run **on the BB core** (section_3). We call them from our
  fw1 (AP core). The registers are shared silicon; ROM static state (buffer
  addresses) lives in ROM RAM and survives our .bss zeroing — but our
  startup quiesces NVIC/ROM-installed IRQs, so anything the ROM relies on
  (DMA IRQ completion) must be re-armed or polled.
- `MusicService_Init.c` annotates some calls inconsistently with
  `music_service.c` (e.g. `rom_i2s_master_config(0,0x17,0,1)` vs `()`):
  prefer the `music_service.c` pairing (`master_config()` +
  `master_init(0,0x17,0,1)`) and verify by behavior.

## 3. Experiment ladder (in order, cheapest first)

1. **Exp 1 — DAC sanity via ROM (no PCM):** new menu item `DAC test`:
   run the §2 sequence up to `rom_dac_unmute(1, 4)` with the DMA buffer
   filled with silence (or a static pattern), volume synced. Success =
   no hang + (maybe) faint DC/click on plug-in. Proves: ROM calls are
   safe from our fw1, DAC I²C config executed (chips respond), routing
   configured. Instrument: log each call; on hang the screen breadcrumb
   tells which call died.
2. **Exp 2 — tone:** fill the ROM playback buffer with a 1 kHz sine (from
   our player's `ringbuf` or generated), `rom_playback_start` +
   `rom_dma_start`, unmute. Success = audible tone. Proves I2S master
   clocking + MCLK/PLL + ASP slave config end-to-end.
3. **Exp 3 — player → sound:** wire `player_process()` PCM into the ROM
   refill contract (`rom_dma_or_copy` / `audio_stream_process.c` model —
   RE that file first) → play a WAV from SD. This replaces the "ipc_b B
   core" consumer with the ROM path for the single-core fw1 (the two-core
   `ipc_b` reimplementation remains a later option, not a blocker).
4. **Fallback (if ROM path misbehaves):** our own drivers — SDK
   `driver/IIS/I2s.c` (already in the SDK AP manifest) + `driver/I2C/i2c.c`
   primitives to bit-bang the §4.13 write (`[addr][MAP 3B][INCR][data]`),
   init sequence from datasheet §5 (power/PLL/ASP/PDN), two addresses via
   I²C scan. Heavier, fully understood.

## 4. Open questions / next reads

- Exact buffer contract: `rom_buffer_calculate` semantics + where PCM must
  be written (RE `audio_stream_process.c`, 290 lines).
- Which I2C channel/pins the DAC is on and which `rom_dac_*` mapping the
  stock jumptable (codec-ID selector at `DAT_0302ca9c`) picks for this
  board (`audio_codec_dac_set.c` dispatch).
- Do both chips ACK on the bus (I²C scan in Exp 1 also settles the 2-chip
  topology question: distinct addresses ⇒ two independently-configured DACs).
- MCLK actually delivered by RKnanoD I2S (TRM) — 22.5792/24.576 or PLL.

## 5. RAM/code budget notes

The player + codecs currently end `.bss` at `0x03049570` (window to
`0x0304C000`) — ~10 KB headroom; the ROM path needs almost no buffers of
its own (ROM owns them). Keep Exp 1/2 buffer allocations static and small
(a 4 KB tone scratch fits).
