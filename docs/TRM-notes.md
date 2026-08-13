# RKnanoD Datasheet & TRM — notes

> Notes distilled from the two confidential Rockchip PDFs the user added under
> `community/`:
> - `community/RKNanoD.pdf` — **RKNanoD Datasheet Rev 0.1 (Jan 2015)**.
> - `community/Rockchip RKNanoD TRM V1.0.pdf` — **RKNanoD TRM** (427 pages).
>
> These are Rockchip Confidential / copyrighted; they are gitignored and are
> reference-only (do not redistribute).

## 1. Architecture — DUAL Cortex-M3 (confirmed)

The RKnanoD is a **dual ARM Cortex-M3** SoC (Datasheet §1.2.3, TRM §2):

| Core | Rockchip name | Role | Memory |
|---|---|---|---|
| Core 0 | **system core** (`SYS_M3`, fclk_sys_core) | UI / control → **fw1 (AP)** | 320 KB IRAM + 256 KB DRAM |
| Core 1 | **calculation core** (`CAL_M3`, fclk_cal_core) | audio / codec → **section_3 (BB)** | 128 KB IRAM + 256 KB DRAM |

This is the hard confirmation of the two-firmware model: the AP and BB run
**simultaneously on separate M3 cores** and talk through the mailbox.

## 2. Mailbox

Datasheet §1.2.3 / TRM §2:
- Four mailbox elements; each = **one command word + one data word + one flag
  bit** (the flag drives the interrupt).
- **Four interrupts to the system core, four to the calculation core**
  (INT9–INT12 = mailbox0_int…mailbox3_int).
- APB interface, 64 KB window.

This matches `driver/mailbox/mailbox.c`'s `MAIL_BOX` struct (A2B and B2A
cmd/data/status/int-en banks).

## 3. Boot & memory

- 16 KB **boot ROM** at `0x00000000` (remap via `GRF_SOC_CON0[8]`).
- Boot sources: SDMMC card, eMMC, SPI Nand/Nor, USB.
- Memory (address map Fig 2-1): `SYSRAM0` (320 KB) @ `0x03000000` (Core 0),
  `SYSRAM1` (256 KB) @ `0x03090000` (Core 1), `HIGHRAM0` (128 KB) @
  `0x01000000`, 64 KB PMU SRAM, `HIFIACC` @ `0x01060000`.
- Peripherals at `0x4000_0000`+ (I2S/I2C/SPI/UART/…), `0x5001_0000` GRF,
  `0x6000_0000`+ DMA/LCDC/USB/SDMMC.

## 4. IMPORTANT — address-map discrepancy (do not trust the TRM for peripherals)

The TRM's Fig 2-1 peripheral layout **does not match the Echo Mini's stock
firmware**. The TRM puts `MAILBOX @ 0x400A_0000` and `CRU @ 0x4019_0000`, but
the `RKNanoD_MP3` SDK `hw_memap.h` (which the stock binary actually uses) puts
`MAILBOX @ 0x4011_0000` and `CRU @ 0x4018_0000` — and the stock section_3
contains `0x4011_0000` 366× and `0x4018_0000` 14×, confirming the SDK map.

**Conclusion:** the SDK's `hw_memap.h` is the authoritative peripheral map for
the Echo Mini; the TRM (V1.0) describes an earlier/different revision. Do not
"correct" the SDK addresses to match the TRM.

## 5. Display (VOP) — a DIRECT path that avoids the ROM API

**Key finding:** the TRM documents the **VOP** (Video Output Processor) — the
i8080 MCU display controller — in Chapter 15, with a full register map. This
means the LCD can be driven **directly by register writes**, no boot-ROM API
required (which is exactly what the handshake verification needed).

VOP registers (offset from VOP base):

| Register | Offset | Purpose |
|---|---:|---|
| `VOP_MCU_CON` | 0x00 | mode control: input format (RGB565/YUV420), data width (8/16-bit), write phase |
| `VOP_MCU_TIMING` | 0x08 | CSN/RDN/WRN strobe timing |
| `VOP_MCU_LCD_SIZE` | 0x0C | panel width/height (max 400×400) |
| `VOP_MCU_CMD` | 0x28 | command/index write entry |
| `VOP_MCU_DATA` | 0x2C | data write entry |
| `VOP_MCU_START` | 0x30 | start VOP |

Base address (authoritative = SDK `hw_memap.h`, confirmed by stock fw1):
- **`VOP @ 0x6007_0000`** (fw1 references it 4×).
- `LCD @ 0x6009_0000` with `LCD_DAT @ 0x6009_000C` (fw1 references it).
- The TRM's `LCDC @ 0x6003_0000` is a **different revision** — 0x6003_0000 is
  EMMC in the SDK map. Do not use it.

Pins: `LCD_WRN`, `LCD_D0-7`, `LCD_RS`, `LCD_CSN`; built-in i8080 MCU interface,
**RGB565** output (matches the 16-bit UI framebuffer).

## 6. What this does NOT provide

The TRM does **not** list the boot-ROM display functions (`rom_lcd_refresh` /
`FUN_02feda18`) — those are in the (invisible) ROM API. But the **VOP register
map (§5) is the hardware-level substitute**: writing `VOP_MCU_CMD`/`VOP_MCU_DATA`
at `0x6007_0028`/`0x6007_002C` can drive the panel without the ROM, which is the
next candidate for an on-screen "BB alive" indicator.
