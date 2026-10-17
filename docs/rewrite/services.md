# ReChord service layer — API contract, format compatibility, verification

> Workstream: SERVICES. Scope: `firmware/services/**` (config, logging,
> input queue, storage abstraction) — clean-room C, per
> `docs/rewrite/architecture.md`. Status: **host-complete, target stubs
> marked TODO**. Date: 2026-10.

## 1. Clean-room rule

All code here is written from scratch. The only things reused from the
reference CFW are **documented FORMAT FACTS** (config schema, log line
format, file names) — no third-party code, decompiled or paraphrased. The
reference analysis (`docs/re/refcfw-analysis.md`) is treated strictly as a
data-format description.

## 2. Files

```
firmware/services/
├── settings.h / settings.c   12-key \RECHORD.CFG parser/serializer
├── log.h / log.c             ring-buffered timestamped logger
├── input.h / input.c         key event queue + ADC threshold mapping
├── fs.h                      minimal storage abstraction (one backend per image)
├── fs_host.c / fs_host.h     HOST backend (POSIX real filesystem; tests/tools)
├── fs_target.c               TARGET backend — TODO stub for the SDK FAT stack
├── scratch.h / scratch.c     shared working region (USB MSC / media index)
├── workbuf.h / workbuf.c     shared region for the mutually-exclusive heavy
│                             users that must NOT share scratch (FLAC decode
│                             pool / mp3 buffers / fw_update staging)
├── services.mk               build fragment for the root Makefile (not yet included)
└── tests/                    host tests (cc -Wall -Werror), run_tests.sh runner
```

## 3. API contract (other modules code against exactly these shapes)

### 3.1 settings.h

```c
typedef struct { char play[16]; int brightness; int screen_off_min;
    int auto_off_min; char usb[8]; char resume[8]; char strip[8];
    char tags[8]; char cpu[8]; int volume; char browse[64]; int cursor; }
    settings_t;

int  settings_load(settings_t *s);       /* parse the config file  */
int  settings_save(const settings_t *s); /* serialize it back      */
void settings_defaults(settings_t *s);   /* sane defaults          */
```

Helpers (pure, no I/O): `settings_parse_text(text, s)`,
`settings_serialize_text(s, buf, cap)` (returns written length or a negative
code). The parser/serializer live in `settings.c`; the I/O halves are
backend files, one per image (same pattern as the fs backend):

| Backend | Image | Save semantics |
|---|---|---|
| `settings_fs.c` | host tools/tests | whole-file `fs_open_write`/`fs_write` (create/truncate) |
| `settings_store.c` | target (AP) | `fs_target_file_rewrite()`: rewrite the existing file's **data sectors** in place + patch its dirent size — no FAT-table writes, no allocation. Missing file ⇒ load returns `SETTINGS_ERR_OPEN` (defaults stay), save returns IO error. |

`SETTINGS_PATH` is `/Internal/.rechord/rechord.cfg` — the user-visible
`.rechord` folder on the internal eMMC volume (BRNOMT-71), plain text so it
can be edited/copied from a PC. The file must pre-exist with allocated
clusters (created once from a PC; a first-boot init is future work). Note:
a file whose size is smaller than its cluster chain is FAT *slack* — fsck
offers to truncate it, so do not run a filesystem **repair** on that volume
unless the config file's allocation may be sacrificed.

Return codes: `SETTINGS_OK 0`, `SETTINGS_ERR_ARG -1`, `SETTINGS_ERR_OPEN -2`
(file missing — defaults are still applied), `SETTINGS_ERR_IO -3`,
`SETTINGS_ERR_FULL -4` (serialize buffer too small).

Config path: `SETTINGS_PATH` = `"\\RECHORD.CFG"` (compile-time overridable).

### 3.2 log.h

```c
void log_printf(const char *fmt, ...);   /* buffered; drops+counts when full */
int  log_flush(const char *tag);         /* append all to \ECHOLOG.TXT       */
```

Also: `log_init(log_clock_fn)` (millisecond clock injection),
`log_dropped()`, `log_pending()`. Ring: `LOG_BUFFER_LINES` 32 lines of
`LOG_LINE_MAX` 160 chars (compile-time tunable). Return codes:
`LOG_OK 0`, `LOG_ERR_ARG -1`, `LOG_ERR_OPEN -2`, `LOG_ERR_IO -3` (on error the
buffer is kept and retried at the next flush).

### 3.3 input.h

```c
typedef struct { int type; int adc_value; int kind; int long_press; } input_event_t;
enum { KEY_NONE, KEY_UP, KEY_DOWN, KEY_SELECT, KEY_BACK, KEY_POWER, KEY_MENU };
enum { INPUT_PRESS, INPUT_LONG, INPUT_REPEAT, INPUT_RELEASE };

int input_poll(input_event_t *ev);        /* 1 = event, 0 = empty, -1 = arg */
int input_scan(int adc_value, int power); /* one key sample; 1 = event pushed */
```

Producers/consumers: `input_push(type, adc_value)` (ISR/driver/host-mock
entry), `input_map_adc(adc_value)` (raw reading → key type), `input_scan()`
(the app loop's once-per-pass scanner: debounced on
`INPUT_DEBOUNCE_SCANS` = 3 consecutive agreeing samples — 8 for the vol+
pair — and the pressed-bottom ADC minimum, matching the stock scan
0x03057eb0), `input_mock_clear()`, `input_pending()`. Queue:
`INPUT_QUEUE_LEN` 16 events, FIFO; overflow drops with `INPUT_ERR_FULL`.

Press model (2026-10-17): a short press is exactly one `PRESS` +
`RELEASE{long_press=0}`. A held press is `PRESS`, one `LONG` at
`INPUT_LONG_MS` (~1 s), then `REPEAT`s while held (only the keys in
`INPUT_REPEATS` — vol+/vol-, which double as UI up/down), then
`RELEASE{long_press=1}`. The repeat accelerates: one per
`INPUT_REPEAT_MS` (~0.25 s), and after `INPUT_FAST_HOLD_MS` more of
holding one per `INPUT_FAST_REPEAT_MS` (~0.05 s, i.e. one per UI pass), so
long lists and volume sweeps can be crossed without spam-clicking
(BRNOMT-80; the ladder table that defines the tiers is in `input.c`).
Consumers act on `PRESS`/`REPEAT` only — the acceleration is a shorter
interval, not a new event kind.

The hold ladder counts **milliseconds**, not samples. The first cut counted
`input_scan()` calls, which tied the feel to the UI pass rate — and that
rate is repaint-bound: a cursor move in a full-screen list repaints and
flushes the whole panel (~50-100 ms per pass), while the player screen's
band redraws run much faster. Time comes from `input_set_clock()`: the
target installs ap_ui's DWT cycle counter (24,000 cycles/ms at the ~24 MHz
AP core clock); with no clock, or one that stops advancing, the service
falls back to a synthetic per-scan clock (`INPUT_FALLBACK_MS_PER_SCAN`) so
repeats keep working instead of silently dying.

### 3.4 fs.h

```c
int        fs_list_dir(const char *path, fs_dirent_cb cb);
fs_file_t *fs_open_read(const char *path);
long       fs_read(fs_file_t *f, void *buf, unsigned long cap);
long       fs_file_size(fs_file_t *f);
void       fs_close(fs_file_t *f);
fs_file_t *fs_open_write(const char *path);    /* create/truncate */
fs_file_t *fs_open_append(const char *path);
long       fs_write(fs_file_t *f, const void *buf, unsigned long len);
```

Exactly one backend is linked per image: `fs_host.c` (host) or
`fs_target.c` (device). Paths are device-style volume-rooted
(`\RECHORD.CFG`); the host backend maps them under a scratch root
(`fs_host_set_root()` in `fs_host.h`, host-only header). Errors: negative
`FS_ERR_*` codes; open calls return NULL.

### 3.5 store.h (2026-10-17)

```c
typedef struct {
    int (*read)(void *user, uint32_t lba, uint32_t count, void *buf);
    int (*write)(void *user, uint32_t lba, uint32_t count, const void *buf);
    void *user;
} store_io_t;

typedef struct { const store_io_t *io; uint32_t lba; uint16_t slot_sectors; } store_t;

void     store_init(store_t *s, const store_io_t *io, uint32_t lba,
                    uint16_t slot_sectors);
uint32_t store_capacity(const store_t *s);   /* slot_sectors*512 - 32 */
int      store_load(store_t *s, void *buf, uint32_t cap, uint32_t *out_len);
int      store_save(store_t *s, const void *data, uint32_t len);
```

Crash-safe record store over **raw sectors, no filesystem**: two slots of
`slot_sectors` sectors, each with a 32-byte header (magic `RCHS`, version,
sequence, length, payload CRC32, header CRC32), payload-first /
header-last writes and a verified read-back. `store_load()` returns the
valid slot with the newest sequence number, so a cut at any point leaves
the previous record authoritative; both slots invalid reads as *empty*
(fresh region), never as a failed boot. CRC = the image-trailer poly
(`0x04C10DB7`, bitwise here so it costs no 1 KiB table in .bss).

Why not a file: our FAT **write** path is the one that bricked devices
(DFIU `fa3968cd` / `15e1c9f5`); block writes above the eMMC user-volume
floor are hardware-proven (USB MSC LUN0, `fw_update_retire`'s FAT/dirent
sector writes). The store only ever touches the region the caller hands it
(`lba .. lba+2*slot_sectors`) — placement is a caller contract, see
BRNOMT-71 (settings) / BRNOMT-74 (media index). Costs no static RAM.

## 4. Format compatibility notes (drop-in facts)

### 4.1 `\RECHORD.CFG` — 12-key schema

- Plain text, one `key=value` per line; `#` starts a comment; blank lines
  ignored; LF and CRLF both accepted.
- Unknown keys are ignored on load (forward compatibility). Keys without
  `=` and lines longer than 127 chars are ignored.
- The 12 keys, fixed schema order on save:
  `play, brightness, screen_off, auto_off, usb, resume, strip, tags, cpu,
  volume, browse, cursor`
  (key `screen_off` → field `screen_off_min`, `auto_off` → `auto_off_min`).
- Verified round-trip of the exact on-device sample values:
  `play=repeat_track brightness=8 screen_off=30 auto_off=15 usb=ask
  resume=folder strip=off tags=on cpu=auto volume=-23 browse= cursor=0`.
- Defaults = those sample values (`settings_defaults`), so a missing config
  file boots with the documented sample behavior.
- Whitespace around keys/values is trimmed; values longer than their field
  are truncated to fit; invalid integers keep the previous (default) value;
  duplicate keys: last one wins.
- Difference from the reference file: keys outside the 12-key schema are
  **not preserved on save** (the struct has no room for them) — a user-added
  key survives until the next `settings_save()`.

### 4.2 `\ECHOLOG.TXT` — log format

- Line format is the documented device log format:
  `%lu.%03lu %s` = `seconds.milliseconds message`, e.g.
  `1.034 pll: switching to 200 MHz`. Timestamps are ms-since-boot captured
  when the line is logged.
- Flush convention: `log_flush(tag)` first appends a `log flush (tag)` line
  (same format), then appends the whole ring to the file and resets it —
  matching the observed device pattern (`1.034 log flush (boot)`).
- Overflow: when the ring is full, new lines are dropped and counted; the
  next successful flush writes one extra line:
  `<ts> log: N lines dropped (buffer full)`, then resets the counter.
- File is opened in append mode: successive flushes accumulate, and a failed
  flush loses nothing (buffer kept).
- File name `ECHOLOG.TXT` and the log line format are the documented device
  format facts (tools that pull the device log keep working); the drop-line
  wording is ReChord's own.

## 5. Verification results (2026-10)

Host tests (`firmware/services/tests/run_tests.sh`, `cc -Wall -Werror -O1`):

```
test_settings: 123 checks, 0 failures
test_log: 35 checks, 0 failures
test_input: 85 checks, 0 failures
test_fs: 20 checks, 0 failures
ALL SERVICES TESTS PASSED
```

Coverage: settings defaults / exact-sample round-trip (parse → save → load →
serialize) / schema order in the saved file / unknown-key + comment + CRLF +
whitespace tolerance / over-long-line skip / value truncation / missing-file
and NULL-arg behavior; log line format incl. exact expected file content /
flush tag marker / append semantics / ring overflow with drop counter /
failed flush keeps the buffer; input FIFO order / overflow drop / invalid
types / ADC band boundaries incl. the documented 1023 idle reading; fs path
mapping / read / write / append / size / list_dir / error paths.

Target compile check (`arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c`,
also re-run with `-Wall -Werror`) — all five non-test sources compile clean:

| object | text | data | bss |
|---|---|---|---|
| settings.o | 1967 | 0 | 2048 (read buffer) |
| log.o | 504 | 0 | 5136 (32 × 160 ring + state) |
| input.o | 256 | 0 | 136 (16-event queue) |
| fs_target.o | 38 | 0 | 0 |
| fs_host.o | 483 | 512 | 0 (host-only; never linked on target) |

Note: `fs_host.c` guards its `dirent` use (`FS_HOST_HAVE_DIRENT`) because the
bare-metal newlib has no `<dirent.h>`; on such toolchains `fs_list_dir()`
returns `FS_ERR_UNSUPPORTED`. The file is linked only into host binaries.

## 6. Open items

1. **SDK FAT backend** (`fs_target.c`): implement on the RKnanoD file API
   (user volume rooted at the IDB-reported `user_base`), keep the fs.h
   contract (esp. `fs_open_append` for `ECHOLOG.TXT`). Until then the target
   stub returns `FS_ERR_UNSUPPORTED`/NULL on every call.
2. **AD_KEY hookup** (`input.c`): wire the SDK ADC key scan (channels seen in
   the bring-up log: `keys: adc ch 1..3`) and the power GPIO
   (`keys: power pin`) to `input_push()`. Replace the **provisional** ADC band
   table with calibrated values (idle reading 1023 is confirmed by the
   documented log; band boundaries are ours and unmeasured).
3. **Clock source**: the board layer must call `log_init()` with a real
   ms-since-boot tick before logging; timestamps are 0.000 until then.
4. **Makefile wiring**: `services.mk` is ready but the root Makefile does not
   include it yet (out of this workstream's ownership). Suggested:
   `SERVICES_TARGET_SRCS` into the AP/BB object list, `fs_target.c` only.
5. **Config load return code**: the reference log shows `cfg: load -4` on a
   config that parses fine; its meaning is unknown. ReChord defines its own
   codes (§3.1) and returns `SETTINGS_OK` in that situation.
6. **Unknown-key preservation on save** (see §4.1) — possible follow-up: a
   pass-through copy of unrecognized lines.
7. **Reentrancy**: services use static buffers (no malloc, ISR-friendly
   producers); `log_printf`/`settings_*` are not reentrant — call from one
   task, or add a lock at the app layer if services are shared between cores.
8. **Log sizing**: `LOG_BUFFER_LINES` 32 / `LOG_LINE_MAX` 160 chosen for the
   small RAM windows; revisit once services live in high RAM.
