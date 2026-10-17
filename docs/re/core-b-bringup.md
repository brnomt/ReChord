# Core B bring-up — Fase 0 (REA de la referencia)

> BRNOMT-68, Fase 0. Fuente: REA del fw1 de la referencia
> (`community/refcfw/ghidra_input/ref_fw1_ram0304F490.bin`, base `0x0304F490`)
> + SDK (`driver/BB/BBSystem.c`, `app_main.c`, `Hw_cru.h`, `PMU.c`) + guía
> `docs/re/AP-MAILBOX-GUIDE.md`. Sin hardware.

## 1. Secuencia real de arranque (ref `0x0305B93C`)

Llamada `(r0 = ptr a la imagen B, r1 = longitud, r2 = timeout)`:

1. `CRU+0xCC = 0x00040004` → **assert `CAL_CORE_SRST`** (id 50 = SOFTRST[3] bit 2,
   write-enable bit 18).
2. `DelayMs(5)`.
3. `memcpy(0x01000000, imagen, len)` → **la imagen B se copia a `0x01000000`
   (HIGHRAM0)**.
4. `memset(0x01020000, 0, 0x1000)` → limpia el bloque compartido (4 KiB).
5. `shared[8] = frecuencia/clock`, `shared[16] = len`; `DSB`.
6. `MAILBOX+0x2C = 0xF` y `MAILBOX+0x04 = 0xF` (W1C: limpia B2A/A2B status).
7. `CRU+0xCC = 0x00040000` → **release `CAL_CORE_SRST`**.
8. `DelayMs(5)`, y sondea:
   - `B2A_STATUS` bit0 (canal 0) → ack (escribir bit0) → `B2A_CMD(0)`;
   - éxito si `B2A_CMD(0) == 1` (`SYSTEM_START_OK`);
   - luego exige `shared[0] == 0x00EFBEEF` (magic "B listo"), si no → `-2`;
   - timeout (r2, 200 en el boot) → `-1`.

**Respuesta a las preguntas de Fase 0:**
- El Core B **no arranca desde section_3**: la referencia lo arranca por CRU y
  ejecuta una imagen plana copiada a **HIGHRAM0 `0x01000000`**. El vector table
  de Core B debe estar en `0x01000000` (el release de `CAL_CORE_SRST` hace que
  Core 1 vectorice desde ahí).
- El bloque compartido es **`0x01020000`** (magic en `[0]`, clock en `[8]`).
- El "HIGHRAM0 0x0307A000" del log es la **ventana del AP** para la imagen
  (carga/validación), no la dirección de ejecución de Core B.

## 2. Init previo (SDK `BBSystemInit`, una vez)

Power-up y clocks antes del arranque (IDs de `Hw_cru.h`):

- `PmuPdLogicPowerDown(0)` (sube el dominio "high"; `PMU_PWRDN_CN` bit 1).
- Gates ON (`ScuClockGateCtr(id,1)`, activo-bajo en `CRU+0x80+4*(id/16)`):
  `HCLK_CAL_CORE_GATE=2`, `HDRAM0..7=48..55`, `HDRAM_INTERFACE=56`,
  `HIRAM0..3=64..67`, `HIRAM_INTERFACE=72`, `HCLK_HIGH=73`,
  `HCLK_HIGH_MATRIX=74`, `HCLK_ASYNC_BRG=76`, `PCLK_MAILBOX=146`.
- SoftReset de-assert (`CRU+0xC0+4*(id/16)`, bit+16 WE):
  `HIGH_MATRIX_SRST=32`, `HIGHRAM1_SRST=33`, `HIGHRAM0_SRST=34`,
  `MAILBOX_SRST=30`.
- Mailbox B2A int0/int3 enable + ISR (solo si el AP habla por mailbox).

## 3. Mailbox (hechos, ya documentados)

- Base `0x40110000`, struct `0x50` B; ch0=system, ch1=decode, ch2=file,
  ch3=debug; IRQ 9..12 (slots 25..28); status W1C.
- Handshake: B escribe `B2A_CMD(0)=1` (`SYSTEM_START_OK`); el AP sondea
  `B2A_STATUS` bit0 y lee `B2A_CMD(0)`.

## 4. Diseño de Fase 1 (heartbeat, sin tocar audio)

Imagen B mínima y propia:
- Linker script propio a **`0x01000000`**, vector table al inicio
  (`SP`, `Reset → bb_main`).
- `bb_main`: escribe `shared[0] = 'BRDY'` (o `0x00EFBEEF` para compatibilidad),
  `shared[1]` = contador incremental, y `WFI` en bucle.
- Sin RTOS, sin libc, sin mailbox (Fase 1 solo prueba que el core corre).

AP (`ap_core_b_start()`), no bloqueante:
1. (VDD 1.2 V ya la deja `ap_clock_set_200mhz()`).
2. `PmuPdLogicPowerDown(0)` + gates ON de HIRAM/HDRAM/`HCLK_CAL_CORE`.
3. `HIGHRAM0_SRST` release.
4. `CAL_CORE_SRST` assert → `memcpy(0x01000000, blob, len)` → `DSB` →
   `DelayMs(5)` → release.
5. Sondea `shared[0]` con timeout (~200 ms); si no aparece, **loguea y sigue**
   (el audio queda como hoy).

Empaquetado: compilar `firmware/bb/` a binario crudo y embeberlo como rodata en
el AP (evita depender del formato de overlay del SDK). Tamaño objetivo < 4 KiB.

## 5. Incógnitas / riesgos abiertos

- **PMU base**: `PmuPdLogicPowerDown` usa `PMU_PWRDN_CN` bit1 (`hw_pmu.h`); falta
  fijar la base exacta para nuestro código (mecánico, leer `hw_pmu.h`).
- **Clock del Core B**: la referencia arranca B a 400 MHz y negocia `/4`. Para
  el heartbeat no hace falta; para Fase 2/3 sí (CRU de `CAL_CORE`/PLL y
  `chip_freq`).
- **Vector table de Core B**: asumimos que el release vectoriza desde
  `0x01000000`. Es la hipótesis a validar con el heartbeat (si no arranca, el
  AP sigue funcionando: timeout y log).
- **Brick**: la imagen no cambia el layout del AP; `_bss_end` intacto. Aun así,
  imagen known-good a mano y debrick por maskrom.
