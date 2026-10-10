/* BlueToothWin.h - minimal reconstruction.
 *
 * The synthesized tree lost this BT-window header (the SDK's UI layer
 * declared the BT window globals here). uartif.c includes it; the runtime
 * globals live in firmware/stubs.c. Add declarations here as needed.
 */
#ifndef BLUETOOTHWIN_H
#define BLUETOOTHWIN_H

#include "typedef.h"

extern uint8 BtWinStatus;

#endif /* BLUETOOTHWIN_H */
