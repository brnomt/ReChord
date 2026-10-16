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

## 6. Recovered facts — reference fw1 binary (Ghidra MCP, 2026-10-12)

Read from `community/refcfw/ghidra_input/ref_fw1_ram0304F490.bin` (project
ReChord, base 0x0304F490). The reference fw1 contains a **complete raw
CS43131 driver** (not ROM calls) — verified by decompiling its functions:

- **Control bus: I2C0 @ 0x40040000**, programmed master at register level
  (TXDATA @+0x100, RXDATA @+0x200 per SDK `driver/I2C/i2c.h` constants —
  the RX read uses `[base+0x200]` directly). Literal pool of the DAC
  function contains GRF 0x50010000, clock global 0x0300410C, I2C0
  0x40040000.
- **Chip address table @ RAM 0x03060B78**: `{0x60, 0x66, 0x70, 0x68}`.
  chan 0 = **0x60 (7-bit 0x30, ADR=0)**, chan 1 = **0x66 (7-bit 0x33,
  ADR=3)** — the two CS43131s. (String "aplayer" follows.)
- **Write primitive** (`FUN_03055f08` = ref's dac_write): **broadcast** —
  writes `[chipaddr+W][MAP_hi][MAP_mid][MAP_lo][ctl=1 (INCR)][data]` to
  chan 0 AND chan 1 (`FUN_03055e78(chan, map, val)` per chip). Exactly the
  datasheet §4.13 format. **MAP byte order is MSB-first** — byte-exact from
  FUN_03055e78 (`strb map>>16`, `map>>8`, `map&0xFF`) and datasheet Fig 4-32
  ("MAP BYTE High (23:16), Mid (15:8), Low (7:0)"). The "[MAP_lo] first"
  wording previously in this bullet was a transcription error (see §8b).
- **Read primitive** (`FUN_03055fe8`): per-channel (`chan, map24, out`) =
  the datasheet Fig 4-34 "aborted write" preamble `[addr+W][MAP_hi][MAP_mid]
  [MAP_lo][ctl=1]` + **STOP**, then a TRX-mode master-receive:
  `MRXADDR = (1<<24)|(addr|R)`, `MRXRADDR = 0`, `CON = EN|MOD(TRX)|START`
  (0x0B), wait STARTIPD + clear START, `CON |= LASTACK`, `MRXCNT = n`, wait
  MBRFIPD (0x08), read RXFIFO (`+0x200`), then the STOP tail.
  (`FUN_03055da8` is the STOP tail helper — `CON = EN|STOP` 0x11, wait
  STOPIPD 0x20, `CON = 0` — NOT a read, see §8b.)
- **Mono-mode table @ RAM 0x03060AB8**: 24 `(MAP u32, value u32)` pairs —
  **byte-identical to the datasheet §5.12 "Enable Mono Mode" sequence**:
  ```
  0x180009=0x2F  0x18000A=0xB4  0x18000B=0x25  0x18000C=0x80
  0x18000F=0x55  0x180010=0xFA  0x180019=0x2F  0x18001A=0xB4
  0x18001B=0x25  0x18001C=0x80  0x18001F=0x4E  0x180020=0xFD
  0x180029=0x2F  0x18002A=0xB4  0x18002B=0x25  0x18002C=0x80
  0x18002F=0x7C  0x180030=0xFD  0x180039=0x2F  0x18003A=0xB4
  0x18003B=0x25  0x18003C=0x80  0x18003F=0x52  0x180040=0xFE
  ```
  **=> the reference runs the two CS43131s in MONO (differential) mode:
  one chip per channel (L=0x60, R=0x66). The 2-chip L/R topology is
  confirmed from bytes, not guesswork.**
- **Bring-up sequence** (ref `FUN_03056550(rate)` ≈ audiocodec_open):
  audio-path routing ops `(2,0,1)(2,0xe,1)(2,2,1)` + delays; volume dance
  (`0x1A` 1→delay 100 ms→0); per-rate config table (13 entries, stride 16);
  chip register probe: read regs `0x10000..end` on chan 0 then chan 1
  (fail marker 0xEE); MAP writes (both chips): `0x0B0000=0x1F`,
  `0x080000=0x10`, `0x0D0000=0x04`, `0x180000=0x45` with `0x17` toggling
  0x99/0x00 and `0x19` toggling 0x20/0x00 (delays between), plus regs
  0x04/0x0E/0x10/0x12/0x13/0x7D/0x82/0x83/0x95/0x96; **HP-detect read:
  MAP 0x0F0000 from both chips** — bring-up aborts (-2) only if BOTH
  chips error; then the 24-pair mono table (above); then I2S1 + DMA.
- **I2S = I2S1 @ 0x40030000** (literals at 0x030564A4/0x0305654C/0x03056CFC):
  FIFO-reset poll (`[7]=0; [8]=1; wait [8]==0`), `reg[0]=0x401F`,
  `reg[1]=0x401F` (16-bit stereo), `reg[2] = ((rate/(mclk<<6))-1)<<16 |
  0x3F3F` (LRCK/SCLK divider vs system clock), then an 8-entry DMA
  descriptor ring (block 0x600) via `FUN_03055b0c`.
- **CRU clock-gate helpers**: `gate(bit, enable)` with write-1-enable /
  write-1-disable semantics (Rockchip CRU convention), e.g. clocks gated
  before the I2C/I2S setup (`FUN_030513e0(0x83,1)`, `(0x95,1)`, `(0x96,1)`,
  `(0xE,1)`, `(0x10/12/13,1)` …).

**Port plan (clean-room, next session):** `firmware/drivers/dac_cs43131.c`
— I2C0 master polled driver (TX +0x100 / RX +0x200), `dac_write(map24,
val)` broadcast to 0x60/0x66, `dac_read(chan, map24)`, CRU gates, the
§6 register order (datasheet-anchored + ref immediates), HP-detect verify,
then `i2s_rknanoc.c`-style I2S1 setup + polled-tone test. The register
values that live in the ref's codec-handle struct (per-rate) will be
recovered from its runtime table or from the datasheet equivalents.

## 7. Experiment ladder (in order, cheapest first)

> **UPDATE after §6:** the reference fw1 drives the DAC with its OWN raw
> I2S1+I2C0 code from the AP core (no ROM APIs) — so the primary path is
> now "port the ref's raw register recipe" (§6 port plan) and the ROM-API
> route below is the background alternative if the raw port misbehaves.

1. **Exp 1 — DAC sanity (no PCM):** new menu item `DAC test`:
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

## 8. Hardware failure #1 + fix (2026-10-12, DFIU 3a986c9b)

First real-hardware run of `DAC test` failed: `rc -4 step 2 / scan A00 B00
C00 / hp 00 00` — `DAC_ERR_NOCHIP` at the I2C scan. Diagnosis (ref fw1 via
Ghidra MCP + the vendor SDK + the CS43131 datasheet) found **six** porting
bugs in `firmware/drivers/dac_cs43131.c`:

1. **GRF mux register offset**: `grf_iomux()` wrote at `GRF + 0x200 +
   port*0x20 + (pin>>3)*4`; the real address (ref `FUN_03051458`, SDK
   `GRFREG_t`, and our hardware-verified `panel_io_rknanoc.c`) is `GRF +
   port*0x20 + (pin>>3)*4`. The pins never moved to I2C function.
2. **`GRF_IOMUX_CON` (I2C0 A/B/C select)**: was at `GRF+0x04` (inside
   `GPIO_IO0MUX[1]`!), actually `GRF+0xBC`. Ref writes `0x30000` there
   (= WE bits 16/17, value 0 = I2C0_SEL_A) — the board uses candidate A.
3. **CRU gate ids**: driver used 156/157/177 (reserved bits, from a
   miscounted enum); real ids per `Hw_cru.h` + ref calls: PCLK_I2S1=130
   (0x82), PCLK_I2C0=131 (0x83), PCLK_ACODEC=150 (0x96), plus
   PCLK_GPIO1=149 (0x95). Without PCLK_I2C0 the controller had no bus
   clock → every probe timed out (shown as 0 in the scan).
4. **Missing I2C0 soft reset** (`SOFTRST[1]` bit 10, ref
   `FUN_03051430(0x1a,…)`) and the **GPIO2 power/reset dance** that the
   ref runs before any clock gate (pins 0/14/2 high, reset pulse on pin
   1: low 1 ms, high 8 ms, low 2 ms). Chips in HW power-down ACK nothing.
5. **I2C MAP byte order**: the driver sent the 24-bit MAP LSB-first;
   datasheet §4.13/Fig 4-32 (scanned PDF, rendered) and the ref frame
   `FUN_03055e78` both send it **MSB-first** (`[addr+W][23:16][15:8][7:0]
   [ctl INCR=1][data]`).
6. **`acodec_mux()`**: m4/m5 were swapped vs the ref table order
   (`entry[4]→+0x204, entry[5]→+0x208`), the `0x200020` write belongs to
   `CRU_MODE_CON` (`CRU+0x10`), and the `+0x214` load dance + the final
   `+0x200 |= 4` were missing.

Also ported while there: I2S1A pin mux (GPIO1A0..A3 func 1 +
`GRF_IOMUX_CON` bit 13), the four `CRU_CLKSEL_CON[2]` literals, I2S1
soft-reset release, `RXCR=0x401F`, and the ref's CLKDIV formula
(`ceil(pclk_kHz/800)-2` split lo/hi; 100 kHz @ 24 MHz ⇒ 14/14, the old
hard-coded 59 gave ~25 kHz).

**Still open after the fix**: the `dac_map_write` register list is still
the §7 abbreviated one, not the full ref order (harmless once the chips
ACK — verify Exp 2 by ear first). **TXDR offset settled (2026-10-13): it
is +0x24, not +0x400** — SDK `Hw_i2s.h` `I2SReg_t` places TXDR right after
`I2S_CLR` (the `(0x400-0x24)` padding is commented out) and the stock DMA
destination literal is `0x40030024` (ref fw1 pool @ file 0x78a8, beside
the `0x03090000` DMA-block base); no `0x40030400` exists in the ref. The
+0x400 hypothesis ate every tone sample — that was the "no audio with a
fully configured DAC" bug.

## 8b. Hardware failure #2 + fix (2026-10-13, DFIU 5d8b7b3c): reads 0xFF

`dac_map_read` returned 0xFF for every register while writes ACKed. Two
investigations settled it (ref fw1 disassembled with
`arm-none-eabi-objdump -Mforce-thumb`, base 0x0304F490; datasheet §4.13
text layer):

1. **MAP byte order = MSB-first, settled.** FUN_03055e78 packs
   `frame[0]=map>>16, frame[1]=map>>8, frame[2]=map&0xFF, frame[3]=1,
   frame[4]=val`; datasheet Fig 4-32/4-33/4-34 label the wire order
   "MAP BYTE High (23:16) → Mid (15:8) → Low (7:0)". §6's old
   "[MAP_lo] first" note was a transcription error. (An early driver
   revision shipping LSB-first was wrong; MSB-first is correct.)
2. **The read phase never performed a receive.** The port had copied
   `FUN_03055da8`'s `CON=0x11` + wait `IPD 0x20` as "master-receive, MRX
   done" — but FUN_03055da8 is the **STOP tail helper** (0x11 = `EN|STOP`,
   0x20 = STOPIPD; the SDK's `I2C_send_stop_bit` comment says "0x11 b
   10001"). The code emitted only the preamble's STOP and never armed
   MRXCNT, so RXFIFO (empty reads back 0xFFFFFFFF) yielded 0xFF.

Fix (`firmware/drivers/dac_cs43131.c`): `i2c_write()` now ends with the
FUN_03055da8 STOP tail (`CON=EN|STOP` → wait STOPIPD → `CON=0`) as the ref
does, and `i2c_read_map()` follows FUN_03055fe8 byte-for-byte: Fig 4-34
aborted-write preamble + STOP, then `MRXADDR=vld|(addr|R)`, `MRXRADDR=0`,
`CON=EN|MOD(TRX)|START` (0x0B), STARTIPD → clear START, `CON|=LASTACK`,
`MRXCNT=n`, wait **MBRFIPD (0x08)**, read RXFIFO words, STOP tail. Note
the datasheet explicitly blesses the STOP-separated preamble (Fig 4-34) —
no repeated-START is required; the CS43131 keeps its MAP across STOP.

## 9. Open questions / next reads

- Exact buffer contract: `rom_buffer_calculate` semantics + where PCM must
  be written (RE `audio_stream_process.c`, 290 lines).
- Which I2C channel/pins the DAC is on and which `rom_dac_*` mapping the
  stock jumptable (codec-ID selector at `DAT_0302ca9c`) picks for this
  board (`audio_codec_dac_set.c` dispatch). **Partly answered: I2C0
  candidate A (GPIO2B2/B3) per the ref `GRF_IOMUX_CON=0x30000`; the scan
  now also proves it on hardware once the chips ACK.**
- Do both chips ACK on the bus (I²C scan in Exp 1 also settles the 2-chip
  topology question: distinct addresses ⇒ two independently-configured DACs).
- ~~MCLK actually delivered by RKnanoD I2S (TRM) — 22.5792/24.576 or PLL.~~
  **RESOLVED (2026-10-13).** MCLK = SoC ACODEC PLL / 5 via `CRU CLKSEL_CON[3]`
  (divider=4 -> /5, source-select = `CLK_ACODEC_PLL`); the CS43131's 22.5792/
  24.576 internal-MCLK spec only applies in the chip's own XTAL/PLL-standalone
  mode, which this board does NOT use — the SoC is the I2S master and feeds the
  CS43131 MCLK pin at 256xFS. Per-group, the `acodec_mux[]` bytes are
  byte-exact with `ACodecpllTable_*` in `driver/DAC/rockcodec/rockcodec.c`:
  44.1k family PLL = 56.448 MHz -> MCLK = 11.2896 MHz = 256*44100; 48k family
  61.44 MHz -> 12.288 = 256*48000; 32k family 40.96 -> 8.192 = 256*32000. The
  four `CRU_BASE+0x20` writes equal `CLKSEL_CON[3]` = `SetI2SFreq(ACODEC_PLL)`
  + `SetI2SClkOutFreq(I2S1_CLK)` (SDK `driver/IIS/I2s.c`). **Re-resolved
  2026-10-16 (REA byte read of ref RAM 0x030609E8):** the "slightly-under
  256x" `mclk` constants were transcription errors — the real table words
  are EXACT multiples (44100/88200 -> 11289600, 176400 -> 22579200). The
  actual speed/pitch bug was the I2S CKR split: the stock writes
  `((mclk/(rate<<6))-1)<<16 | 0x3F3F` — the whole ratio in MDIV with
  **TSD=RSD=64** (64 BCLK/LRCK = the CS43131's 2x32-bit frame). Our
  home-grown TSD=256 put 256 BCLK in each LRCK, so the DAC's 64-BCLK frame
  counter ticked at SCLK/64 = 4x rate: "stuttering at a high speed and a
  high pitch". `i2s1_setup` now writes the stock values verbatim
  (TXCR/RXCR=0x401F, CKR formula above). **Wire format settled the same
  day from the stock DMA payload (Service.c `Bit_Convertor_*`): one TXDR
  word = one channel sample, `sample << 16`, frame = L word then R word
  (2x32-BCLK slots) — NOT a packed L|R word.** The packed-word mistake is
  what made each CS43131 hear the other channel at 2x rate (2x speed +
  2x pitch) with the progress bar at exactly 2x.

## 10. RAM/code budget notes

The player + codecs currently end `.bss` at `0x03049570` (window to
`0x0304C000`) — ~10 KB headroom; the ROM path needs almost no buffers of
its own (ROM owns them). Keep Exp 1/2 buffer allocations static and small
(a 4 KB tone scratch fits).
