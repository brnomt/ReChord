# ReChord — Bluetooth

> Status: **implemented and verified on hardware** — pairing, A2DP streaming
> and the headphone buttons work (first pass with a BLIK-AIR400). The module
> contract below is taken from EchoPod (BasilSkyWalk, GPL-3.0) hardware
> bring-up on the same player (`community/echopod`, branch `bluetooth-audio`);
> the implementation here is ReChord's own and is credited in the README.

## The module

The Echo Mini has an **external Bluetooth module with its own stack**. The SoC
does not run A2DP/SBC/HFP; it powers the module, talks to it over **UART1** with
short ASCII commands, feeds it audio through the normal DAC path, and tells it
when to start/stop the stream.

| Signal | Pin | Notes |
|---|---|---|
| Module power | GPIO2.7 | driven high after the UART pins are parked low |
| UART1 TX/RX | GPIO2.16 / GPIO2.17 | **func 1**, group A (`GRF+0xBC` bit 9 = 0) |
| UART | — | 115200 8N1, FIFO on, polled, no interrupts |
| Wired mute | GPIO2.0 | **high = wired output muted** while routed to BT |
| Wired output | GPIO2.14 | **low** while routed to BT |
| Panel strap | GPIO2.9 | stock drives it low as part of the same housekeeping |

Boot (stock and EchoPod): GPIO2.7/2.16/2.17 low, then UART1 open, then power on.
Power-off must drive those three lows again — a powered module otherwise keeps
the player from switching off.

**Audio routing.** The DAC keeps playing. Routing to the module is only
GPIO2.0 high + GPIO2.14 low (plus the stock housekeeping below), so the existing
I2S1/CS43131 path is reused unchanged. The DAC's own unmute writes must be
overridden while routed (otherwise the next track start sets GPIO2.0 low and
silences the module).

Stock housekeeping applied once, on the first route (from EchoPod, register
dumps matched stock while streaming):
`GRF+0xC4=0x00010000`, `CRU+0x84=0x00920092`, `CRU+0xA0=0x00020002`,
`CRU+0xC0=0x00680068`, `GRF+0x14=0x00200020`, `GRF+0x54=0x00010001`,
GPIO2.9 low.

**Jack.** While routed, the jack-sense pins read wrong (they fake a plugged
headphone); the jack poll must be ignored while routed and for 500 ms after a
route change.

## UART protocol (module → SoC)

Frames start with `C`; the second byte is the kind. ASCII, no checksum:

| Frame | Meaning |
|---|---|
| `CV<digit>` | ready (version digit) |
| `CS<NN><payload>` | scan result: `NN` = payload length (2 digits), payload = 6-byte address + name |
| `CSEN` / `CSER` | scan ended / scan error |
| `CQ<L>` | status letter; `CQO<NN><addr+name>` = connected to that device |
| `CLOK` / `CLER` | pair ok / pair failed |
| `CHOK` / `CHER` | connect ok / connect failed |
| `CT` / `CTOK` | disconnected |
| `CP` / `CC` | paused / playing (headphone buttons, in-ear sensors) |
| `CA` / `CB` | previous / next button |

SoC → module: `CS00` scan, `CP00` stop scan, `CQ00` poll, `CL06`+6-byte address
connect, `CH06`+address connect retry, `CT00` disconnect, `CY0` + `1E`/`00`
stream on/off, `C 86`/`C 87` play/pause notices (answered only after the module
reported the opposite state; the notice goes first, `CY0` 100 ms later).

Rules that mattered on hardware (EchoPod field notes):
- Poll `CQ00` every 3 s while ready/scanning, every 1 s while connecting,
  every 0.5 s while a paired reply waits for the link.
- Pause polling for 50 s after a `CP`/`CC`.
- A connect retries with `CH06` after 4 s without an answer and fails with
  `CT00` after 30 s.
- A scan retries `CS00` after 4 s and ends with `CP00` after 30 s.
- Answer every new link with `CY0 00` before `CY0 1E`.
- A `CP`/`CC` right after `CLOK` also means the link came up.
- Power the module, wait up to 5 s for `CV`; no reply = module fault (off).

## ReChord design

| Piece | File | Role |
|---|---|---|
| UART1 + pins + routing | `firmware/drivers/bt_hw.c` | register-level bring-up, byte I/O, route switch |
| Protocol state machine | `firmware/services/bt_proto.c` | frame parser, commands, state, timers |
| Persistence | `firmware/services/bt_store.c` | `/.rechord/BT`: on/off + up to 6 devices |
| Policy | `firmware/services/bt_link.c` | on/off, auto-connect, retries, route, remote keys |
| UI | `firmware/ui/screens/bluetooth_screen.c` | Settings row + device screen |
| App glue | `firmware/startup/ap_ui.c` | boot, service tick, power-off, jack quirk |

RAM budget: the AP `.bss` has ~440 B free under `0x0304A000`, so the module
keeps only what a draw or a decision needs: state, a 40-byte RX chunk, six
`{address, name}` records (6 × 38 = 228 B) and the policy timers. Diagnostics
use `log_printf`, not RAM.

Rules:
- The module is **off by default**; the user enables it in Settings.
- Never power the module on before the UART pins are parked low.
- Route only while `streaming`; unroute on stop, disconnect, power-off.
- A dropped link pauses playback (and says so in the log).
- With no connection for 5 minutes and the Bluetooth screen closed, power the
  module down until the screen is opened or the player restarts.

## Milestones

1. ✅ `bt_hw` + `bt_proto` + host tests (parser/state machine, no hardware).
2. ✅ `bt_store` + `bt_link` + host tests (round-trip, reconnect policy).
3. ✅ Settings → Interface → Bluetooth screen + live status value; UI host
   tests (the row and the screen contract).
4. ✅ Hardware 2026-10-31, pass 1: the module powers up and answers `CV5`
   over UART1 (`bt: module powered` / `bt: on` / `bt: ready v5`).
   **Pass 2 (same day):** paired and streamed to a **BLIK-AIR400**
   (`bt: found` → `connecting` → `connected`), `bt: audio route module`,
   and the headphone's pause/play reports drove the player
   (`bt: remote pause` / `bt: remote play`). The device list/on-off
   persistence fix (`/.rechord/BT` created at first boot) ships with the
   next image; the first BT image logged `bt: device save failed`.
5. 🟡 Polish: power/idle-off/remote keys/park-wake are implemented. Theme
   skins get `bt` (0 off / 1 on / 2 connected) and `bt_stream`, and the
   bundled cassette Now Playing shows a small BT mark; the jack quirk remains
   (ReChord has no runtime jack detection yet).

## Implemented behaviour

- Off by default; the choice and up to six devices persist in
  `/.rechord/BT` (created at first boot like the other device files, so the
  in-place rewrite always has clusters). Enabling powers the module;
  disabling drives its pins low and gates the UART1 block off.
- Only **user-chosen** devices are saved and auto-connected; devices found by
  a scan are listed but stay ephemeral until the user connects to one.
- Auto-connect to the most recent device at boot and after a drop, one
  attempt every 5 s; a failed pair retries the same way.
- Stream on/off follows the player (PLAYING and a live link), with the
  stock play/pause notices; routing to the module is GPIO2.0 high +
  GPIO2.14 low, with the DAC unmute overridden while routed.
- Headphone pause/play/next/previous drive the player, guarded for 3 s after
  a link/stream/local change; a dropped link pauses playback.
- With no link and the Bluetooth screen closed — or behind a dark display —
  for 5 minutes the module powers down (the choice stays on); opening the
  screen or pressing scan wakes it. A USB park/wake restores it too.
- `power_off()` unroutes and drives the module pins low first (a powered
  module blocks a real switch-off).
