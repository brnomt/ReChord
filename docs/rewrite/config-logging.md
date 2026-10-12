# rechord_config + rechord_log — persistent config and circular logging

> Workstream: SERVICES (v2). Scope: `firmware/config/**`, `firmware/util/**`
> — clean-room C, per `docs/rewrite/architecture.md`. Status: **host-tested,
> linked into the clean-room AP (fw1); target persistence activates when the
> FAT write backend lands** (fs_target is read-only today). Date: 2026-10.

## 1. Files

```
firmware/config/
├── rechord_config.h / .c   Rockbox-style config: schema, tolerant parser,
│                           atomic save (.tmp + rename + crc32), keyed API
├── config.mk               build fragment (included by the root Makefile)
└── tests/                  host tests (cc -Wall -Werror), run_tests.sh

firmware/util/
├── rechord_log.h / .c      Circular logger: leveled, ring-buffered,
│                           watermark flush, file rotation
├── util.mk                 build fragment
└── tests/                  host tests, run_tests.sh
```

Both modules talk to storage ONLY through `firmware/services/fs.h`
(`fs_open_read/write/append`, `fs_read/write`, `fs_close`, and the new
`fs_rename`/`fs_remove` added for atomicity and rotation). They run unchanged
on host tests (fs_host.c) and on target (fs_target.c).

## 2. rechord_config

### 2.1 On-disk format (`\RC_CONFIG.CFG`, compile-time overridable)

Plain-text key=value, Rockbox style. Deliberately NOT `\RECHORD.CFG` — that
path belongs to the stock-format 12-key service (`services/settings.c`), and
both files can coexist on one volume.

```
# rechord config v1 - key=value, '#' comments, unknown keys
# are ignored on load. If you edit by hand, DELETE the
# trailing crc32 line (it is a whole-file integrity check).
volume=-23
balance=0
replaygain_mode=off
replaygain_preamp=0
autoeq_enabled=off
autoeq_profile=
eq_bands=0,0,0,0,0,0,0,0,0,0
theme=default
brightness=8
screen_timeout_sec=1800
language=en
shuffle=off
repeat_mode=off
resume_track=
resume_offset_ms=0
crc32=1a2b3c4d
```

Parser tolerance: blank lines, `#` comments, surrounding spaces, CRLF, lines
without `=`, unknown keys, invalid integers (keep default), over-long values
(truncated). Enums accept names (`off/track/album`, `off/one/all`) or raw
indices; bools accept `0/1`, `on/off`, `true/false`.

### 2.2 Power-loss robustness

`config_save()` is a two-phase replace:

1. serialize the whole schema (+ `crc32=` trailer) and write `\RC_CONFIG.CFG.tmp`
   in full, then close (close flushes in both fs backends);
2. `fs_rename()` the tmp over the real file — until this lands, the previous
   config is untouched.

A power cut in phase 1 loses only the new values; a torn file that still
reaches the parser is caught by the CRC check (`CONFIG_ERR_CRC`, struct reset
to defaults). The CRC is optional by design: hand-edited files simply delete
the `crc32=` line and are accepted as-is.

RAM cost: one shared 2 KiB static text buffer + the ~300 B singleton (the AP
RAM window `FW_RAM` is tight — see §5).

### 2.3 API

```c
rechord_config_t *config_current(void);         /* module singleton */
int  config_init(void);                         /* defaults + load CONFIG_PATH */
void config_set_defaults(rechord_config_t *cfg);
int  config_load (rechord_config_t *cfg, const char *path);  /* NULL = default */
int  config_save (const rechord_config_t *cfg, const char *path);

/* Rockbox cfg_int/cfg_string style, over the singleton + schema table */
int  config_get_int (const char *key, int default_val);
int  config_set_int (const char *key, int value);       /* clamped to field */
int  config_get_bool(const char *key, int default_val);
int  config_set_bool(const char *key, int value);
int  config_get_string(const char *key, char *out, unsigned long cap,
                       const char *default_val);
int  config_set_string(const char *key, const char *value);

/* pure text helpers (host tests, tools) */
int config_parse_text(rechord_config_t *cfg, const char *text);
int config_serialize_text(const rechord_config_t *cfg, char *buf,
                          unsigned long cap);
```

Return codes: `CONFIG_OK 0`, `ERR_ARG -1`, `ERR_OPEN -2` (missing file —
defaults still applied), `ERR_IO -3`, `ERR_CRC -4` (defaults restored),
`ERR_FULL -5`.

Context rules: `config_load`/`config_save` are NOT reentrant (shared static
buffer) — call from the main loop, never from an ISR.

## 3. rechord_log

### 3.1 Line format

```
[<ms since boot>][<LEVEL>][<subsystem>] <message>
[123456][INFO][player] open /MUSIC/a.flac
[12345][WARN][player] underrun, 2304 samples late
```

### 3.2 Context contract (bare-metal)

- `rlog_printf()` formats to a 160 B stack scratch and memcpy()s the finished
  line into the ring (single-producer, commit-after-copy). No I/O, no locks —
  safe from the audio ISR as long as it is the ONLY producer. Lines that do
  not fit are dropped as whole units (no torn lines) and counted; the count
  is reported at the next successful flush.
- `rlog_flush()` does storage I/O — main-loop context only. Poll
  `rlog_should_flush()` (>= 75 % of the 4 KiB ring) from the idle loop, or
  flush on a timer / at known quiet points (track change, power-off).

### 3.3 Rotation

After a flush, files over `rlog_set_file_limit()` bytes (256 KiB default) are
renamed `\RECHORD.OLD` (previous .old deleted first), starting a fresh
`\RECHORD.LOG`. The size counter is stat-ed from the file on the first flush
after boot, so the limit holds across reboots. A failed rotation keeps
logging into the oversized file and retries at the next flush — logs are
never dropped to enforce rotation.

## 4. Integration examples

### 4.1 Boot + main loop (`firmware/startup/ap_main.c`, `ap_ui.c`)

```c
#include "config/rechord_config.h"
#include "util/rechord_log.h"

/* Board tick source (SysTick timer once the clock tree is up; the existing
 * panel_io_delay_ms busy-wait era can use a loop counter). */
static unsigned long app_tick_ms(void) { return board_ticks_ms(); }

uint32_t Main(void)
{
    rlog_init(app_tick_ms);

    /* Bring-up breadcrumbs: visible even if the config load fails. */
    rlog_printf(RLOG_INFO, "boot", "ReChord %s starting", RECHORD_VERSION);

    int rc = config_init();                 /* defaults + \RC_CONFIG.CFG */
    rlog_printf(rc == CONFIG_OK ? RLOG_INFO : RLOG_WARN,
                "boot", "config rc=%d vol=%d", rc,
                config_get_int("volume", -23));

    /* Config-driven log verbosity (key added by the app, ignored if absent). */
    rlog_set_level(config_get_int("log_level", RLOG_INFO));
    rlog_set_file_limit((unsigned long)config_get_int("log_max_bytes", 0)
                        ? (unsigned long)config_get_int("log_max_bytes", 0)
                        : RLOG_FILE_MAX_BYTES);

    display_init();
    ...
    ap_ui_loop();
    return 0;
}
```

In the idle loop (`ap_ui_loop`), non-blocking flush at the watermark:

```c
    for (;;) {
        ...input scan, ui_handle_event, player_process()...

        /* Idle housekeeping: flush logs when the ring is >= 75% full.
         * Never in an ISR; the work is bounded by one ring drain. */
        if (rlog_should_flush()) {
            int lrc = rlog_flush();
            if (lrc != RLOG_OK)
                rlog_printf(RLOG_WARN, "log", "flush rc=%d, retrying", lrc);
        }

        ui_render();
        panel_io_delay_ms(1);
    }
```

At shutdown (`power_off()`), drain what is left so the last seconds before a
power cut survive on disk:

```c
static void power_off(void)
{
    /* Persist the resume point + current settings before losing power. */
    rechord_config_t *cfg = config_current();
    track_info_t ti;

    if (player_get_state() == PLAYER_PLAYING && player_get_track(&ti) == 0) {
        copy_trunc(cfg->resume_track_path, ti.path, sizeof cfg->resume_track_path);
        cfg->resume_offset_ms = (uint32_t)player_get_position_ms();
    }
    rlog_printf(RLOG_INFO, "app", "power off, saving config");
    int rc = config_save(cfg, NULL);
    rlog_printf(rc == CONFIG_OK ? RLOG_INFO : RLOG_ERROR,
                "app", "config_save rc=%d", rc);
    (void)rlog_flush();                     /* best effort, then die */

    display_clear(0x0000);
    board_power_off();
}
```

### 4.2 Audio renderer / player glue

Apply the config to the audio path once, and on every settings change:

```c
/* Map the persisted config onto the player + DSP. Call at boot (after
 * player_init) and whenever a settings screen commits a change. */
static void apply_audio_config(const rechord_config_t *cfg)
{
    player_set_volume(cfg->volume);                    /* dB, matches API */
    player_set_replaygain_mode(cfg->replaygain_mode);  /* 0/1/2 == off/track/album */

    /* Preamp rides along with ReplayGain in the DSP stage. */
    rch_dsp_config_t dsp = { 0 };
    dsp.mode = cfg->autoeq_enabled ? RCH_DSP_PARAM_EQ : RCH_DSP_BYPASS;
    dsp.sample_rate = 44100;
    dsp.channels = 2;

    /* The schema stores 10 Rockbox-style bands; the current DSP core is
     * 5 biquads — map the middle five 1:1 until the core grows. */
    for (int i = 0; i < RCH_DSP_BANDS && i < RECHORD_EQ_BANDS; i++)
        dsp.db_gain[i] = cfg->eq_bands_gain[i] / 2.0;   /* 0.5 dB steps */

    if (rch_dsp_configure(&dsp) != 0)
        rlog_printf(RLOG_ERROR, "dsp", "configure failed, bypassing");
}
```

Resume playback at boot:

```c
    const rechord_config_t *cfg = config_current();

    if (cfg->resume_track_path[0] != '\0' &&
        player_open(cfg->resume_track_path) == PLAYER_OK) {
        if (cfg->resume_offset_ms > 0)
            (void)player_seek((long)cfg->resume_offset_ms);
        rlog_printf(RLOG_INFO, "player", "resume %s @%lu ms",
                    cfg->resume_track_path, (unsigned long)cfg->resume_offset_ms);
    }
```

Log player events from the existing callback (ISR-side safe: rlog_printf only
touches the ring):

```c
static void player_events(int ev, void *user)
{
    (void)user;
    switch (ev) {
    case PLAYER_EV_TRACK: {
        track_info_t ti;
        if (player_get_track(&ti) == 0)
            rlog_printf(RLOG_INFO, "player", "track %d: %s", ti.track_no, ti.title);
        break;
    }
    case PLAYER_EV_ERROR:
        rlog_printf(RLOG_ERROR, "player", "decode/B-core error, state=%d",
                    player_get_state());
        break;
    default:
        break;
    }
}
```

## 5. Build wiring and constraints

- `Makefile` includes `firmware/config/config.mk` and `firmware/util/util.mk`;
  `firmware/ap.mk` links both into the clean-room AP (`AP_CONFIG_SRCS`,
  `AP_UTIL_SRCS`). Verified with `make cfw-ap`.
- RAM budget: the AP linker region `FW_RAM` (`.data`+`.bss` below
  `0x0304C000`) is a hard constraint — this is the same memory map that
  bricked a device when violated (2026-08-25, see `docs/fw1-packing.md`).
  Keep module statics lean; the 4 KiB log ring + 2 KiB shared config buffer
  were sized to fit. Builds with more room can raise the ring with
  `-DRLOG_BUF_SIZE=8192`.
- Target persistence caveat: `fs_target.c` is read-only today, so on device
  `config_save()`/`rlog_flush()` return `ERR_IO`/`ERR_OPEN` (config load
  still works from factory images written over USB MSC). The full save path
  — including `fs_rename`/`fs_remove` — is exercised by the host tests and
  will light up with the FAT write work.
- Host tests:

```
firmware/config/tests/run_tests.sh   # 79 checks
firmware/util/tests/run_tests.sh     # 49 checks
firmware/services/tests/run_tests.sh # regression (fs rename/remove)
```
