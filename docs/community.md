# Community.md — Lo que la comunidad ha encontrado y dicho (Snowsky Echo Mini)

> Recopilación de recursos externos de la comunidad sobre el **FiiO/Snowsky
> Echo Mini** (RKnano), descargados y escaneados el 2026-08-12. Fuentes:
> el blog RSE de Takobin, la herramienta **FlameOcean**, y los **SDK
> leakeados** de Rockchip. Al final, cómo encaja con nuestros hallazgos en
> ReChord y qué actualizamos en los docs.
>
> Material crudo descargado en `community/` (fuera de git por tamaño):
> `community/echo-mini-rse.html`, `community/flame-ocean-website/`,
> `community/sdks/*`.

---

## 1. Fuentes

| Recurso | Tipo | Contenido |
|---------|------|-----------|
| `https://im.not.ci/echo-mini-rse/` | Blog RSE (2026-02-17) | Relato completo del RE del Echo Mini por **Takobin** |
| `https://github.com/losses/flame-ocean-website` | Repo | **FlameOcean**: herramienta web de customización de firmware (SvelteKit) |
| `https://gitlab.com/brian0218/RKNanoD_Wireless_Audio_SDK_V1.5/` | SDK leak | RKnanoD Wireless Audio SDK v1.5 |
| `https://github.com/xinghuaman/RKNanoD_MP3_V1.3_20161102` | SDK leak | RKnanoD MP3 SDK v1.3 (2016-11-02) |
| `https://github.com/linuxhan/rk3399-table-RKNanoC` | SDK leak | RK3399 VR SDK con **RKnanoC** (sensor/audio MCU) |
| `https://livetrack.club/debrick` | Guia | **Debrick Echo Mini** via maskrom (guardada en `community/debrick-livetrack.md`) |
| `https://github.com/rockchip-linux/rkdeveloptool` | Herramienta | Rockchip USB flash/debrick tool (clonado en `community/rkdeveloptool/`) |
| `https://github.com/rockchip-linux/rkbin` | Repo | Rockchip binary blobs: bootloaders/miniloader (clonado en `community/rkbin/`) |

---

## 2. El blog RSE (Takobin) — hallazgos clave

Autor: **Takobin**, psicólogo/neurocientífico + diseñador UI/frontend, **sin
experiencia en reverse engineering**. Usó un pipeline de varios LLMs
(GLM 4.7 como ejecutor + NotebookLM como base de conocimiento + Claude como
organizador), con un "árbol de problemas" y documentos de tarea.

### 2.1 Confirmaciones de hardware/software
- **Chip:** Rockchip **RKnanoD** (familia). Encontró el **TRM** (Technical
  Reference Manual) siguiendo una URL muerta migrada (de wiki→main site) y
  lo respaldó en Internet Archive. → Ver §5 para la aclaración RKnanoC/D.
- **Firmware:** el IMG **NO tiene verificación de firma**. Flasheó un
  firmware modificado y "se lo comió sin verificación alguna".
  - Existe una función **CRC** que devuelve solo **8 bits** (Rockchip
    **RKCRC**), y al final del firmware hay un bloque tipo hash, pero **no
    encontró código que lo lea/compare**. Verificación típicamente en la
    herramienta de flasheo, no en el HW.
- **Método de flasheo:** copiar el `.IMG` a la raíz del almacenamiento y
  reiniciar (el dispositivo lo detecta y actualiza solo). Sin herramienta.

### 2.2 Formato de fuentes (pixel fonts)
- Las fuentes son **mapas de bits 1 bit/pixel** (0=espacio, 1=píxel).
- **Stride del glifo:** `SMALL = 32`, `LARGE = 33` columnas.
- El stride 33 viene de una **optimización del compilador**: `×33` se
  compila como `(x << 5) + x` (shift + suma), no como multiplicación real.
  Este fue el "misterio" que costó 2 días (el LLM veía `32` donde era `33`).
- **Firmas de footer de glifo** (bytes al final de cada glifo):
  `0x90, 0x8f, 0x89, 0x8b, 0x8d, 0x8e, 0x8c`.
- El render tiene una función que localizaron subiendo desde los datos del
  glifo; el pipeline completo de renderizado quedó mapeado.

### 2.3 Bitmaps / recursos
- Los bitmaps UI están **incrustados como BMP (RGB565)** en el bloque
  `ROCK26IMAGERES`.
- "Corrección" de un script inicial corrompió la interpretación (por eso el
  tearing de caracteres) — lección: validar las herramientas de extracción.

### 2.4 Metodología (lecciones transferibles)
- **Multi-modelo:** un solo modelo cae en puntos ciegos; hacer que dos
  modelos se critiquen mutuamente mejora el análisis.
- **Árbol de problemas:** registrar lo que NO se sabe (y relaciones), no lo
  hecho.
- **Documentos de tarea ↔ informes** entre modelos para no contaminar el
  contexto.
- El estado emocional del usuario es una "variable de ingeniería" (jurar
  contamina el contexto del LLM).
- El patrón "cuando el LLM dice tonterías = falta información" (pidió el TRM).

---

## 3. FlameOcean (herramienta de la comunidad)

Web app (Svelte 5 + SvelteKit + Web Workers). Funcionalidades:
- **Análisis de firmware**, **extracción de recursos** (glifos SMALL/LARGE
  organizados por plano Unicode, imágenes RGB565), **visualización**,
  **reemplazo de imágenes** (drag&drop/pegar), **reemplazo de fuentes**
  (TTF/OTF/WOFF), **operaciones por lote**, **export** (ZIP o firmware).
- Probado específicamente para **Snowsky Echo Mini**.

### 3.1 Detalles técnicos del formato (de `src/lib/rse/`)
- **Tabla de particiones en `0x80`**: `part_2_firmware_b` = offset + size
  (= nuestra section_3).
- `SMALL_BASE` se detecta desde config en `0x78`/`0x7a`.
- Signature `ROCK26IMAGERES` para el bloque de recursos.
- Fuentes: `LARGE_STRIDE = 33`, `SMALL_STRIDE = 32`, footer signatures
  `{0x90,0x8f,0x89,0x8b,0x8d,0x8e,0x8c}`; detección por ventana + stride.

### 3.2 Theme patcher (código)
- Identifica **escrituras de color** (`STRH`) y sus `MOVW` de carga.
- Parchea re-ensamblando instrucciones Thumb (`encodeBl`, `encodeMovw`,
  `encodeMovt`) — decodificador/encodificador Thumb completo
  (`theme/thumb/decoder.ts`, `encoders.ts`, `instructions.ts`).
- **NOP slide:** encuentra un NOP slide funcional en `0x588A8–0x79B70` para
  inyectar código de parche.
- **Switch-case patcher** y **discovery** (localiza la función FLAC y la de
  menú por firma).
- Bytes originales que parchea:
  - Función **FLAC** (tema): `CMP R1, #4` + `ITE EQ` = `04 29 0c bf`.
  - Función **menú**: `MOV.W R12, #0` = `4f f0 00 0c`.
- **Metadatos de parche** (`ECHO` magic + versión + timestamp + 5 colores
  FLAC + 15 colores de menú + CRC16).

### 3.3 Mods de la comunidad
- Boot animations (gif→frames), y **skins completas**: cassette auténtico,
  **EVA**, **Fallout**, **macOS** (énfasis en "pixel perfect").
- Apareció un sitio para **recolectar firmwares modificados** + tutoriales.
- Publicado en Reddit; un día después llegó un PR de boot animation.

---

## 4. SDKs leakeados

### 4.1 RKnanoD_Wireless_Audio_SDK_V1.5 (gitlab)
Estructura: `App/`, `BBSystem/`, `Codecs/`, `Cpu/NanoD/`, `Driver/`
(AD, Audio, Bcore, Display, DMA, EMMC, FIFO, File, FM, I2C, I2S, Key, ...).

### 4.2 RKnanoD_MP3_V1.3_20161102 (github) — el más cercano a nuestro SDK
- `Common/`: BBSystem, **BootLoader** (`Start.s`, `Cortex-m3.S`), Codec,
  Display, Driver, FileSys, **FwUpdate**, Include, Plug, SortFileInfo, System.
- `SDK_160_128/`: Build, Development, Firmware, **Resource**, **Scatter**, UI.
- **Scatter files** (`BuildAll.sct`) confirman el mapa de memoria:
  ```
  SYS_DATA_BASE  0x03000000  (0x60000 → 0x03060000)
  SYS_CODE_BASE  0x03060000  (0x40000 → 0x030A0000)
  LOADER_DATA_BASE 0x0304A000   LOADER_CODE_BASE 0x03088000
  HRAM_CODE  0x01000000   HRAM_DATA 0x01020000
  HAUDIO_*_DATA_BASE 0x01010000/0x01020000 (buffers codecs)
  ```
  → **`SYS_DATA` en `0x03000000` coincide con nuestra section_3** cargada en
  `0x03000000`. Confirma que el layout del SDK RKnanoD es el del Echo Mini.

### 4.3 rk3399-table-RKNanoC (github) — chip NanoC
- `NanoC_VR_Release/`: `Common/` (BootLoader, Codec, Driver [ADC, AD_KEY,
  **DAC**, DMA, GPIO, I2C, IIS, IMDCT36_SYNTH, MemDev, NVIC, PLL, PMU, PWM,
  RTC, SCU, SDMMC, SPI, SYSTICK, TIMER, UART, USB...], FileSys, System) +
  `Document/` (PDFs en chino: compilación, flasheo, display, key/tp, sensor)
  + `SDK/`.
- Es un **headset VR RK3399** que usa el **RKnanoC** como MCU de audio/sensor,
  pero los drivers NanoC (DAC/ADC/I2S/DMA) son útiles.

---

## 5. Cómo encaja con ReChord (y qué corregimos)

| Tema | Comunidad | ReChord (nuestro) | Acción |
|------|-----------|-------------------|--------|
| Chip | "RKnanoD" | "RKnanoC" | Ambos: **RKnanoC es el chip**; **RKnanoD es la familia/SDK**. Aclarar en HARDWARE.md |
| Verificación de firma | **NO hay** (RKCRC 8-bit sin enforcement) | Sospechábamos `fw2 compare error` | Confirmar en FLASHING.md y bootloader-analysis.md: el checksum existe pero **no se verifica en el HW** |
| Flasheo | copiar IMG + reboot, sin tool | Idéntico (documentado) | ✓ ya coincidía |
| Fuentes | stride 32/33, footer sigs, `×33` opt | Teníamos `Font12_CompiType @ 03010f98` | Añadir formato de fuente a HARDWARE.md (para nuestra UI from-source) |
| Recursos | BMP RGB565 en `ROCK26IMAGERES` | Ya sabíamos el bloque de recursos | ✓ |
| Tabla de particiones | en `0x80` (part_2_firmware_b) | Nuestra section_3 en IMG `0x81A14` | ✓ consistente |
| Memoria SDK | `SYS_DATA 0x03000000`, `SYS_CODE 0x03060000` | section_3 → `0x03000000` | ✓ confirma |
| Patching de UI | theme patcher: STRH/MOVW + NOP slide `0x588A8-0x79B70`, bytes FLAC/menú | Nuestro camino B (from-source) | Referencia útil si algún día queremos patch selectivo |

### Oportunidades directas para ReChord
1. **`theme_patcher.py` / `extract_font_universal.py` / `extract_resource_smart.py`**
   (en `flame-ocean-website/references/`) son **Python** — reutilizables para
   extraer/parchear recursos y fuentes sin Node/Svelte.
2. El **formato de fuente** (stride 32/33 + footer sigs) es justo lo que
   necesitamos para **renderizar texto en nuestra UI from-source** (M1b+).
3. El **TRM del RKnanoD** (archiveado por la comunidad) puede resolver las
   direcciones de registros que en Ghidra son "stubs".

---

## 6. Pendientes / límites
- La comunidad solo parchea **recursos y colores de tema**; no reconstruye la
  UI desde fuente (nuestro objetivo M1b es más ambicioso).
- El blog no publica el TRM ni los datos exactos de las fuentes (solo la
  metodología + los strides/firmas).
- Los SDK leakeados son RKnanoD genéricos (MP3/Wireless/VR); el SDK exacto
  del Echo Mini (con la capa FiiO) **no está** en ellos.

---

## 7. Recovery / debrick (maskrom) — aportado por el usuario

Nuevos recursos de recovery aportados en sesión (clonados/guardados en
`community/`):

- **`rkdeveloptool`** (`community/rkdeveloptool/`) — herramienta oficial de
  Rockchip para flashear por USB (maskrom). Compilable en Mac/Linux.
- **`rkbin`** (`community/rkbin/`) — blobs binarios de Rockchip
  (bootloaders/miniloader/DDR). Referencia para la cadena de arranque.
- **`debrick-livetrack.md`** (`community/debrick-livetrack.md`) — guía de
  debrick del Echo Mini (EchoThemes / livetrack.club).

### 7.1 Recovery por hardware (maskrom) — importante

El Echo Mini **sí entra en modo maskrom** (bootloader USB de Rockchip,
`idVendor=2207`), lo que permite recuperar un brick real sin depender del
método "copiar IMG + reboot" (que exige que el dispositivo arranque):

1. Apagar con el agujero **RST** (clip), no con el botón power.
2. Mantener **todos los botones menos power**.
3. Conectar USB manteniendo los botones (pantalla negra = normal).
4. Confirmar maskrom en `dmesg`: `idVendor=2207, idProduct=262d`.
5. Flashear un IMG bueno con `rkdeveloptool`.

Esto refuerza la regla de "recovery simple": incluso si un flash nuestro deja
el dispositivo sin arrancar, existe un camino de recuperación por hardware.
