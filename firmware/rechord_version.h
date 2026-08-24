/*
 * rechord_version.h — canonical ReChord firmware version (single source of
 * truth for the version the device reports).
 *
 * The Echo Mini has NO single "#define VERSION". "3.7.0" is spread across:
 *   1. a UTF-16LE string "Software:3.7.0" in section_3 (IMG 0x96A80,
 *      RAM 0x0301506C) — read by the stock UI at runtime;
 *   2. a second "3.7.0" UTF-16 copy at IMG 0x96AAA;
 *   3. the FIRMWARE_HEADER in flash sector 0 (BCD: MasterVer/SlaveVer/
 *      SmallVer), rendered by the About screen as "Ver: 03.07.0000"
 *      (firmware/rockchip/ui/SetMenu/SystemSet.c -> ProductInfoWinDisplay).
 *
 * The BCD fields cannot carry the "RC " prefix (they are numeric digits);
 * the About screen would need its render code changed to show "RC", which
 * is AP-side work. For the current BB stub, we rebrand the section_3 string
 * (#1) at runtime — a write from our own source, not an IMG byte-patch.
 */
#ifndef RECHORD_VERSION_H
#define RECHORD_VERSION_H

/* Branded version string the device should display. */
#define RECHORD_VERSION_STRING  "RC 3.7.0"

/* BCD fields matching the stock FIRMWARE_HEADER (Ver: MM.SS.ssss).
 * "RC 3.7.0" -> Master=3, Slave=7, Small=0. The "RC" prefix is carried by
 * the string above, not by these numerics. */
#define RECHORD_VERSION_MASTER  0x03u
#define RECHORD_VERSION_SLAVE   0x07u
#define RECHORD_VERSION_SMALL   0x0000u

/* RAM address of the "Software:<version>" UTF-16LE string the stock AP
 * reads (section_3 + 0x1506C == IMG 0x96A80). The BB stub overwrites this
 * at boot so the visible version reads our brand. */
#define RECHORD_SW_VER_RAM      0x0301506Cu

#endif /* RECHORD_VERSION_H */
