# Battery reading (BRNOMT-83) — plan

> Goal: an **approximation** is enough (user's call). The exact stock UI behavior
> does not need to be reproduced.

## The key finding: the stock battery code is already in the tree

No REA is needed for the approximation. The SDK sources shipped with the project
already define the whole measurement:

- `firmware/rockchip/system/sysservice/battery.c` / `battery.h`
  - `RealBattValue(adc)` = `((adc << 1) * 2500) >> 10` → millivolts, with the
    board's 1/2 divider folded in (`VCC_VALUE 2500` = ADC reference).
  - 64-sample running average (`BatteryBuffer[64]`), then a **5-step ladder**
    (`SysConfig.h: BATTERY_LEVEL 5`): empty 3350, L0 3500, L1 3600, L2 3800,
    full 4000 mV (USB-full 4100).
  - `Battery_GetLevel()` returns 0..4; `Battery_GetChargeState()` uses
    `CheckVbus()` for charging/full.
- `firmware/rockchip/driver/ADC/Adc.h`: **`ADC_CHANEL_BATTERY = 0`**
  (the key ladder is channel 1 — the same SARADC block our `keys_adc()` already
  drives).
- The SDK implementation is coupled to the BB/OS side (`SysTickCounter`,
  `SendMsg`, `Adc->` registers). We do **not** link it on the AP; we take its
  constants and formula.

## AP plan (no REA)

1. `firmware/drivers/battery.{h,c}`:
   ```c
   void battery_init(void);
   int  battery_level(void);      /* 0..4, -1 while unavailable   */
   int  battery_percent(void);    /* level/(steps-1)*100, -1 if n/a */
   unsigned battery_mv(void);     /* last averaged millivolts      */
   ```
   - Read SARADC **channel 0** with the same TRM sequence already used by
     `keys_adc()` (`panel_io_rknanoc.c: adc_read_fresh()` — parameterise the
     channel), average 8 samples (the SDK averages 64; 8 is plenty at our
     cadence), convert with `((adc << 1) * 2500) >> 10`.
   - Ladder → level: `< 3500 → 0`, `< 3600 → 1`, `< 3800 → 2`, `< 4000 → 3`,
     else `4`; percent = `level * 25`.
   - Cache the result and refresh at most every ~15 s from the app loop (never
     per frame, never from the audio path).
   - `usb_cable()` (already available) marks "charging" for the indicator.
2. UI: the main-menu status row (already carrying the volume) appends
   `BATT 75%` (or `BATT --` when unavailable, `BATT CHG` while charging).
   The Now Playing screen can show it once the status row has room.
3. Tests: pure host test for `battery_pct_from_mv(mv)` over the ladder
   boundaries (3349/3500/3599/3600/3799/3800/3999/4000/4100) plus `-1` handling;
   the ADC read itself is verified on device.

## If we do want REA later (optional, not required)

The stock UI's *presentation* (icon shape, hysteresis, the low-battery shutdown
threshold and its message) is only in the stock firmware. The flow we already
used for the DAC applies:

- Decompile the stock AP image with Ghidra 12.x via REA (`open_binary` on the
  ELF wrapper, then `search_procedures`/`xrefs`); start from the strings
  `BATT`/`battery`/`low power` and from `GetAdcData`'s xrefs.
- Look for: the icon/percent mapping table, the hysteresis counters
  (`BatteryCounter`/`BatteryCounter1` in the SDK), `BATT_POWEROFF_CNT=20`, and
  the shutdown call. Record addresses in `docs/re/`.
- Only worth it if we want to match the stock warning/shutdown exactly; the
  measurement above is otherwise the same hardware path.

## Budgets

- RAM: ≤ 64 B static (last mV, level, counter, timestamp) — well inside the
  ~16 KiB `.bss` headroom measured for BRNOMT-69.
- Time: one SARADC conversion sequence (~tens of µs) every ~15 s.
