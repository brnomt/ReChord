# ReChord — Echo Mini Hardware Map

## SoC: Rockchip RKnanoC (chip) / RKnanoD (familia SDK)
> La comunidad (blog RSE, `docs/community.md`) lo llama "RKnanoD" porque
> el **SDK** es la familia RKnanoD; el **chip específico** del Echo Mini es
> el **RKnanoC** (Cortex-M3). El TRM del RKnanoD fue localizado y archiveado
> por la comunidad (Internet Archive).
- **Package:** LQFP64 (likely — matches Echo Mini form factor)
- **CPU:** ARM Cortex-M3 (from Ghidra's `ARM:LE:32:v8-m` language detection)
- **Clock:** 24 MHz crystal + internal PLL
- **Audio DAC:** Integrated — 90dB SNR, 95dB digital volume (-0.375dB steps)
- **RAM:** Internal SRAM (exact size TBD from datasheet)
- **Flash:** SPI NAND (shared bus with LCD via FLASH_DATA pins)
- **USB:** USB 2.0 OTG (driverless DAC mode confirmed)
- **Peripherals:** SPI, I2C, UART, SDMMC, SAR-ADC (3 ch), PWM, I2S
- **PMU:** Integrated battery charger, DC-DC buck, 3 LDOs

## Pin Assignments (inferred from firmware + datasheet)
| Function | RKnanoC Pin | Firmware Evidence |
|----------|-------------|-------------------|
| LCD display | FLASH_DATA[7:0] + PA2/LCD_RS + PB0/LCD_WR + PA7/LCD_CSN | LVGL/parallel 8080 interface |
| Audio DAC | Internal (PMU_HPL/PMU_HPR) | DAC filter strings |
| Keypad | LADC_AIN[0:2] (SAR ADC key matrix) | 3 button modes (A/B/C) |
| USB | USB_DP / USB_DM | USB DAC + data mode |
| Flash | SPI via PA5/PA6 + FLASH_DATA | U:\ drive, Flash Memory string |
| SD Card | SDMMC via PC1-PC6 | Card Memory string |
| I2C | PD0/SDA + PC7/SCL | DAC IC control? |
| UART | PB5/TXD + PB6/RXD | debug_printf output |
| Bluetooth | SDMMC or UART interface | BT Music string |
| Line-in | PMU_INL / PMU_INR | recording service |

## Fonts (pixel fonts) — de la comunidad (FlameOcean / RSE)

Las fuentes del UI son **mapas de bits 1 bit/píxel** (0=espacio, 1=píxel):
- **Stride por glifo:** `SMALL = 32`, `LARGE = 33` columnas.
- El stride **33** viene de una optimización del compilador: `×33` →
  `(x << 5) + x` (shift+suma, no multiplicación).
- **Firmas de footer de glifo** (byte al final de cada glifo):
  `0x90, 0x8f, 0x89, 0x8b, 0x8d, 0x8e, 0x8c`.
- En Ghidra: `Font12_CompiType @ 03010f98` (render de la fuente).

Útil para renderizar texto en nuestra UI from-source (M1b+).

## Address Space

```
0x00000000 - 0x02FDFFFF   Boot ROM (RKnano internal ROM)
0x02FE0000 - 0x02FFFFFF   ROM API region (I2S, DMA, GPIO, etc.)
                           Ghidra: stub memory block ROM_API (r-x, empty bytes) + named stubs
                           - rom_i2s_master_config @ 0x02ff5752
                           - rom_audio_clock_setup @ 0x02ff5c30
                           - func_0x02ff55c0/55ba: DAC IC init/deinit (unnamed stub TBD)
                           - rom_dma_config @ 0x02ff6814
                           - rom_i2s_dma_start @ 0x02ff68f0
                           - rom_sample_rate_set @ 0x02ffa410 (48000 observed)
                           - rom_audio_path_route / rom_audio_path_disable @ 0x02ffa6f0 / a72a
                           - func_0x02ffa224: audio buffer status
                           - rom_dac_mute / rom_dac_unmute @ 0x02ff44ce / 4580
                           - rom_memcpy @ 0x02ff952e (+ 0x02ff957c memset?)
                           - rom_usb_connect @ 0x02ffb2e0 (+ 0x02ffb3e6 disconnect?)
                           - func_0x02fee544: SPI command
                           - func_0x02feda18: LCD update
                           - rom_alloc / rom_hw_init @ 0x02feeedc / eebe
                           - rom_early_init @ 0x02fe860e
                           - func_0x02fef2b2: unknown (SPI/I2C?)
0x03000000 - 0x04F4FFFF   Main firmware (loaded from IMG section 3)
    0x03000000 - 0x0300000F   RKnanoFW header (magic + load addr + flags)
    0x03000010                firmware_entry (reset vector)
    0x03000010 - 0x03010000   Core kernel + DSP + event system
    0x03010000 - 0x03020000   Media library + format list
    0x03020000 - 0x03030000   Audio services (MusicInit, MusicService)
    0x03040000 - 0x03070000   Codec dispatch (DICTDECODER)
    0x03060000 - 0x030E0000   Codec implementations
    0x030E0000 - 0x030F5000   APE + tag parsers
    0x030F5000 - 0x03110000   DSD/DSF decoder
    0x04F00000 - 0x04F50000   Resource section (ROCK26IMAGERES + bitmaps + fonts)
0x04F50000 - 0x04F427E3   Padding / unused
```

## Segment Table (from section_1, entries at IMG offset 0x200)

Each segment descriptor in the RKnanoFW container:

| IMG Offset | Load Addr | Size | Content (inferred) |
|-----------|-----------|---------|------|
| `0x0200` | `0x03050000` | 91 | ??? |
| `0x0220` | `0x03005AFC` | 101,456 | Audio buffer (96KB+) |
| `0x0240` | `0x0301E7D0` | 0 | Zero-init region |
| `0x0260` | `0x0301E750` | 0 | DSP input buffer? |
| `0x0280` | `0x0301E74C` | 0 | DSP output buffer? |
| `0x02A0` | `0x0301E74C` | 2 | ??? |
| `0x02C0` | `0x0301E76C` | 12 | Codec work buffer |
| `0x02E0` | `0x0301E770` | 54 | ID3 parse buffer |
| `0x0300` | `0x0301E778` | 65,536 | FAT cache (64KB) |
| `0x0320` | `0x0301E794` | 16,384 | Main stack (16KB) |
| `0x0340` | `0x03041550` | 0 | FLAC work buffer? |
| `0x0360` | `0x0301E7D4` | 578 | OGG decode buffer |
| `0x0380` | `0x0301EB30` | 75,792 | MP3 frame buffer (74KB) |
| `0x03A0` | `0x03031454` | 65,748 | WMA work buffer (64KB) |
| `0x03C0` | `0x0301E76C` | 53 | APE work buffer |
| `0x03E0` | `0x0301E7D0` | 20 | DSD buffer |
| `0x0400` | `0x0301EE5C` | 27,871 | AAC decoder buffer (27KB) |
| `0x0420` | `0x03025F90` | 37,812 | WAV decode buffer (37KB) |
| `0x0440` | `0x0301E770` | 409 | Dict decoder buffer |
| `0x0460` | `0x030234B8` | 4,845 | Media library index (~200 entries) |
| `0x0480` | `0x03024868` | 64,288 | UI framebuffer (320x240x2?) |
| `0x04A0` | `0x030247BC` | 33,860 | BT stream buffer (33KB) |
| `0x04C0` | `0x0301E78C` | 8 | USB DMA control |
| `0x04E0` | `0x0301EABC` | 6 | Record buffer |
| `0x0500` | `0x0301E784` | 4,566 | JPEG decode buffer (4.5KB) |
