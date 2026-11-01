# ReChord — feature inventory and DAP gap analysis

> Snapshot: 2026-10-31. This is a living inventory: what ReChord has **today**,
> what the stock firmware/firmware class expects (evidence: the official
> changelogs in `docs/re/MULTI-VERSION-PLAN.md` and the SDK menu resource IDs),
> and a prioritized list of what to add. "Verified" = seen working on the
> device; "host-tested" = covered by host tests only; "planned" = not built.

## 1. Current features

### Playback

| Feature | State |
|---|---|
| FLAC 16/24-bit up to 96 kHz, MP3, WAV | ✅ device (96k margins still chased) |
| Core B decodes; AP owns DAC/I2S and the PCM ring | ✅ device |
| Gapless same-format tracks | ✅ device |
| Format-change boundary | hard by design (DAC re-lock) |
| Play/pause, next/prev, seek (hold VOL ±5 s) | ✅ device |
| Volume (dB steps), mute, play/pause fade | ✅ device |
| Shuffle (Rockbox-style reorder), repeat off/all/one | ✅ device |
| ReplayGain off/track/album/auto | ✅ device |
| 5–10 band EQ with runtime profiles | ✅ device |
| Resume last track + position, queue rebuilt by origin | ✅ device |
| Auto-off (standby) when paused/stopped, screen-off timer | ✅ device |
| Sleep timer while **playing** | ⬜ |
| Jack detection: auto-pause on unplug (BRNOMT-70) | ✅ host-tested; device check pending |
| A-B repeat | ⬜ |
| Crossfade, skip-silence, playback speed, balance, mono downmix | ⬜ |
| Volume limit, headphone gain (low/high) | ⬜ |
| Bookmarks (named positions) | ⬜ (only the last position) |

### Library and browsing

| Feature | State |
|---|---|
| Media index (v3, dual slot + CRC), iterative scan | ✅ device |
| Album / Artist / Genre / Year / Label / Album Artist / Composer / All Songs | ✅ device |
| Case-insensitive sort, track-number tiebreak | ✅ device |
| Filesystem browser, play folder, "i/N" | ✅ device |
| Live queue view, jump to entry | ✅ device |
| Album sort with **disc number** (CD# → track# → filename) | ⬜ (no disc field) |
| Search (title / artist / album) | ⬜ |
| Favorites / Recently played / play counts | ✅ (rated stars not planned; "Most played" covers the 32-entry recent window) |
| Named playlists, M3U import/export, "add to queue"/"play next" | ⬜ (`player_enqueue` exists, no UI) |
| Folder skip (next/prev folder) | ⬜ |
| File delete/rename/format from the UI | ⬜ (PC via MSC instead) |
| Letter jump / fast scroll | ⬜ (pagination only) |

### Display and UI

| Feature | State |
|---|---|
| Theme packages (declarative skins, layout, palette, animated) | ✅ device |
| Now Playing: title/artist/album, format, i/N, mode, progress, volume | ✅ device |
| Album art (120×120/48×48 + palette) | ✅ `services/art_pack.c` reads the PC-built `ARTPACK.BIN`; themes use the `art` meter and `art_*` palette |
| Embedded cover art (JPEG/PNG from the file) | ⬜ **intentionally out of scope** (user decision 2026-10-31): tracks without an ARTPACK entry keep the placeholder |
| Lyrics (.lrc) / full-screen lyrics | ⬜ (stock had it; `.lrc` files exist on the user's card) |
| Clock / 12–24 h / screensaver clock | ⬜ |
| Low-battery warning / charging screen | ⬜ (gauge only; VBUS park is dark) |
| Button modes (single/double/long actions), volume swap | 🟡 long-press actions and VOL orientation only |
| Volume/seek OSD outside Now Playing | ⬜ (theme-dependent) |
| Portrait orientation | ⬜ (EchoPod added it; ReChord is 320×170 landscape) |
| Languages / font size | ⬜ (English, theme-defined fonts) |

### System and I/O

| Feature | State |
|---|---|
| USB mass storage (MSC), safe eject, USB modes ask/charge/MSC | ✅ device |
| Self-updater (`HIFIECnn.IMG`), index rebuild | ✅ device |
| Diagnostics (index/keys/eMMC/SD/USB), rotating log | ✅ device |
| Power off / VBUS park, Hold | ✅ device |
| Battery gauge (SARADC, 5-step ladder) | ✅ device |
| Bluetooth headphones (A2DP source, pairing, 6 devices, remote keys, idle-off) | ✅ device (BLIK-A400) |
| Bluetooth receiver mode / codec selection | ⬜ |
| USB DAC mode (UAC1, driver-free 48 kHz/16-bit stereo, host volume) | ✅ device (`drivers/usb_dac_rknanoc.c`, `services/usb_dac_sink.c`) |
| Jack detection: auto-pause on unplug, wired/BT switching | ✅ auto-pause (BRNOMT-70); auto wired↔BT switching still open |
| Line-in / microphone recording | ⬜ (hardware has line-in; stock had a recorder) |
| FM radio | ⬜ (SDK has it; board tuner unconfirmed) |
| Wi-Fi / DLNA / streaming | ⬜ (no radio) |
| Video / photo playback | ⬜ (not a DAP feature; no decode path) |
| DSD, APE, WMA, M4A/AAC, OGG/Vorbis, Opus | ⬜ (stock shipped OGG/M4A/APE/WMA/DSD) |

### Audio formats versus the stock firmware

The stock changelogs confirm the class expectation: **OGG (1.7.0), M4A/AAC
sorting (2.6/3.6.0), APE/WMA ID3 (3.8.0), DSD (3.3.0)** and MP3 cover art
(3.8.0). ReChord decodes FLAC/MP3/WAV only. AAC/ALAC and Opus are the most
realistic additions (EchoPod already runs FAAD2/libopus on its second core);
APE/WMA/DSD are large or constrained by the 80 KB decode pool.

## 2. What a standard DAP has that ReChord does not

Prioritized by value for this device and fit with the current
stability/usability/battery goals.

### P1 — high value, fits the current architecture

1. ✅ **Jack detection + auto-pause** (BRNOMT-70, 2026-10-31) —
   `drivers/jack.c` (GPIO0.13/GPIO2.8, EXT_PORTA), `services/jack_sense.c`
   (7-sample debounce), app pause on unplug, skipped while Bluetooth owns the
   output. Still open: auto-switch wired↔BT on plug/unplug (out of the issue's
   scope).
2. **Lyrics (`.lrc`)** — synced and static, full-screen. The files are already
   on the card, the parser is small, and the stock firmware had it.
3. ✅ **Favorites + Recently played + play counts** (2026-10-31) —
   `services/library_extras.c` stores 32-bit FNV-1a path hashes (48 favorites,
   32 recent with counts) in `/.rechord/favorites` and `/.rechord/recent`;
   Library gains **Favorites**, **Recently played** and **Most played** rows
   (one index pass each, views in `media_query`), long-SELECT toggles a
   favorite in the song lists and on Now Playing, and a "Fav"/"Nx" value
   marks the rows. Stock had Favorites (1.3.0) with resume position (3.5.0).
4. **Named playlists + M3U import/export + "Add to queue"/"Play next".**
   `player_enqueue` exists but nothing calls it.
5. **Sleep timer while playing** (off/15/30/60/90 min), distinct from the
   paused/stopped standby auto-off. Stock had it (3.1.0).
6. **Search** by title/artist/album over the index.
7. **A-B repeat** and **disc-number sorting** (CD# → track# → filename).
8. ✅ **Artwork** — already covered by the ARTPACK reader/theme art path;
   on-device decoding of embedded pictures is explicitly out of scope (kept
   the PC `tools/artpack` step).
9. ✅ **USB DAC mode (UAC1)** (2026-10-31) — `drivers/usb_dac_rknanoc.c`
   enumerates a 48 kHz/16-bit stereo UAC1 function on the existing DWC2
   stack; `services/usb_dac_sink.c` converts EP1 isochronous OUT packets to
   the I2S ring (20 ms cushion, drop/repeat drift correction) and the host
   mute/volume controls map to the CS43131. Entry points: root menu
   **USB DAC**, or `usb=dac` on the cable edge. Stock v2.4.0 had the same
   mode.
10. **AAC/M4A and OGG/Vorbis/Opus** decode on Core B (the stock supported them).

### P2 — standard but larger or lower urgency

- Crossfade, skip-silence, playback speed/pitch, balance, mono downmix,
  volume limit, headphone gain.
- RTC clock (12/24 h, screensaver clock), low-battery warning, charge screen.
- Bluetooth receiver mode and codec/status display.
- Button modes (single/double/long), folder skip, volume OSD, volume-step
  granularity (stock had 120 levels).
- Bookmarks (named), playlist editing, delete/rename files in the UI.
- Languages/i18n, font size, portrait orientation, cover flow.
- CUE-sheet albums, APE/WMA tags, file-format display.
- Line-in/mic recording (hardware present; stock had a recorder).

### P3 — not applicable or not worth it on this hardware

- Wi-Fi/DLNA/streaming apps, touch, video/photo, games.
- DSD/MQA (decode/DAC constraints; not in the current pool).
- FM radio unless the board tuner is confirmed.

## 3. Constraints that shape the order

- **RAM**: AP `.bss` has ~2.6 KB headroom under `0x0304A000`; features must
  keep their buffers on the stack or in `scratch`/`workbuf`.
- **Decode pool**: 80,896 B shared by FLAC/MP3/fw_update; 192 kHz/16k-block
  FLAC does not fit. New codecs compete for the same pool.
- **Core B** is stopped while idle (battery); a new format or feature that
  needs it must start it through the same `ipc_b_*` path.
- **UI**: 320×170, theme-driven. New screens should be theme-skin aware and
  keep the compiled fallback.
- **Battery**: new always-on work (jack poll, play counts, RTC) should be
  sampled, not per-tick.

## 4. Suggested next milestone

Pick the P1 items in this order: **jack detection**, **lyrics**, **favorites /
recently played**, **playlists + add-to-queue**, **sleep timer**, then
**search**; they are small, testable on the host, and directly improve daily
use. USB DAC and extra codecs are the next large projects after that.
