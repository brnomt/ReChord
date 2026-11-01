# USB DAC mode (UAC1)

ReChord can present itself to a PC as a driver-free USB sound card, like the
stock firmware's v2.4.0 USB DAC mode. The PC enumerates a USB Audio Class 1.0
function at **48 kHz, 16-bit, stereo**; every isochronous packet is written
straight into the I2S ring that feeds the CS43131.

## Entry points

- Root menu: **USB DAC** (`startup/ap_ui.c`), next to *USB storage*.
- `Settings → Interface → USB mode`:
  - `dac` opens the DAC session on the cable (VBUS) edge;
  - `msc` opens mass storage;
  - `charge` only charges;
  - `ask` (default) opens a **chooser** — *USB DAC* / *USB storage* /
    *Charge only* — instead of silently entering storage.
- A short POWER press ends any USB session; holding POWER for ~2 s ends it and
  powers off (the same rule as *USB storage*). The press tracker is reset at
  the start of every session: a long-hold power-off used to leave the tracker
  armed, so the next session's first press was measured against the old
  timestamp and powered off immediately.

The player is paused before the session and its volume is restored afterwards.
The session claims the shared 32 KiB scratch region (`SCRATCH_OWNER_USB`), so
it cannot overlap an index build or an MSC session. On exit the session
detaches the I2S feeder before releasing the scratch ring, and powers the DAC
down; a later resume from PAUSED therefore reports failure from
`ipc_b_pause(0)` when the DAC is not ready, so `player_play()` rebuilds the
stream through `ipc_b_start()` instead of resuming into a powered-off DAC.

## Transport

`drivers/usb_dac_rknanoc.c` is a polled DWC2 device, sharing the MSC
bring-up facts (PHY/GRF, core init, FIFO allocation, EP0 poller). Full speed
only: the AHB clock at boot cannot run the HS PHY.

| Item | Value |
|---|---|
| Configuration | 110 bytes, 2 interfaces (AC + AS) |
| AC | header, USB-streaming input terminal, speaker output terminal, feature unit (mute + volume) |
| AS alt 0 | zero bandwidth |
| AS alt 1 | type-I 48 kHz, 2 ch, 16-bit |
| EP1 OUT | isochronous, `wMaxPacketSize` 192, interval 1 ms |
| Class requests | endpoint sampling-frequency; feature-unit mute/volume (`SET/GET_CUR/MIN/MAX/RES`) |
| Host volume | 1/256 dB units -> `audio_output_set_volume_db()` (clamped) |

`SET_INTERFACE(1,1)` arms EP1 (`EPTSIZ` 192, packet count 1) and
`SET_INTERFACE(1,0)` disarms it. `rx_drain()` reads each packet from GRXSTSP
and re-arms the endpoint; the setup/data/status control path captures the
OUT data stage for `SET_CUR`.

## PCM path and drift

`services/usb_dac_sink.c` (pure, host-tested) converts interleaved
little-endian samples to the left-justified 32-bit I2S words
(`s16 << 16`, `s24 << 8`) and writes frames into a `ringbuf_t` in scratch
(32 KiB = 4096 frames = 85 ms).

- Target cushion: 20 ms; prefill writes 960 silent frames at start.
- Dead band: 4 ms. Above target+guard one frame is dropped; below
  target-guard one frame is repeated. This absorbs the independent USB and
  DAC clocks without audible clicks.
- No feedback endpoint is advertised; the correction is host-independent.

**The producer must kick the feeder.** The I2S TX ISR masks its source when
the ring runs dry and stays masked until the producer calls
`rknano_i2s_kick()` (see `ipc_b_ring_fill`). The USB path re-arms it after the
prefill and after every packet write it accepts; without those kicks the
session goes silent as soon as the first 20 ms cushion drains, even though
the host is streaming (field report: Android enumerates the card, no sound).

**The session must unmute.** `player_pause()` fades the DAC out (PCM volume at
mute) and `audio_output_configure()` starts silent when a fade is configured,
expecting the caller to fade in. The session applies the host-visible level
and calls `audio_output_fade_in()` after `audio_output_start()`; otherwise
every received packet plays into a muted DAC, and host `SET_CUR` volume
changes cannot unmute it either (a faded-out level is cached, not applied).

The USB host's 48 kHz clock and the APLL-derived I2S clock are close enough
that only occasional single-frame corrections are expected (counted in the
session log).

## Audio route

The MSC bring-up mutes the wired output (`GPIO2.0` high) and disables the amp,
which is right for storage and wrong for a DAC. After the USB bring-up the
session calls `bt_hw_route(0)` (drops any Bluetooth route/override) and then
explicitly selects the wired output:

```
GPIO2.14 = 1   wired output
GPIO2.0  = 0   hardware mute off
GPIO1.4  = 1   amp enable
```

This is done even when `audio_output_configure()` short-circuits on an
already-open 48 kHz stream (the paused player), because configure() would
otherwise skip its own unmute.

## Logging

The session writes one line on exit:

```
usb-dac: exit %d alt %u cfg %u pkts %lu frames %lu drop %lu rep %lu
```

`alt`/`cfg` show the last enumeration state; `pkts`/`frames` are the
isochronous traffic; `drop`/`rep` are the drift corrections. `exit` uses the
shared `USB_EXIT_*` codes (`usb_msc.h`).

## Limits

- Full speed, 48 kHz/16-bit stereo only (the stock DAC mode was 48 kHz too).
- No 24-bit altsetting, no 44.1 kHz, no recording, no feedback endpoint.
- USB DAC and MSC are exclusive sessions; the cable edge honors the `usb`
  setting, the root menu always offers both.
- The host volume overrides the device volume while connected; the device
  volume is restored when the session ends.

## Verification

Host:

```
cd firmware/services/tests && sh run_tests.sh   # test_usb_dac_sink
cd firmware/ui/tests && sh run_tests.sh
make -j4                                        # AP image PASS
```

Device checklist:

1. Flash, boot with the cable connected, choose **USB DAC** (or set
   `usb=dac` and plug in).
2. The PC lists **ReChord USB DAC** as a 48 kHz/16-bit output.
3. Play audio; confirm sound on the 3.5 mm/4.4 mm output, not Bluetooth.
4. Pause/play, change the host volume, and unplug/replug.
5. Read `/.rechord/LOG`: `usb-dac: exit ... pkts > 0`, small `drop`/`rep`.

Android hosts: the log's `pkts` separates the two failure classes. `pkts 0`
means the phone never selected the streaming interface — check
**Developer options → Disable USB audio routing** (must be OFF) and that media
audio is routed to USB. `pkts > 0` with no sound was the muted-DAC/feeder bug
described above.
