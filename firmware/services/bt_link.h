/*
 * bt_link.h — Bluetooth policy: the user choice, auto-connect, streaming and
 * the headphone buttons.
 *
 * Sits between the protocol (services/bt_proto.c), the hardware
 * (drivers/bt_hw.c) and the player. The app calls bt_link_service() from its
 * main loop; the Bluetooth screen calls the action functions.
 */
#ifndef RECHORD_SERVICES_BT_LINK_H
#define RECHORD_SERVICES_BT_LINK_H

#include "bt_proto.h"

#define BT_IDLE_OFF_MS 300000UL   /* 5 minutes off after a disconnect */
#define BT_RETRY_MS     5000UL    /* reconnect attempt spacing        */
#define BT_SWITCH_MS    2000UL    /* gap when switching devices       */

void bt_link_init(unsigned long now);
void bt_link_service(unsigned long now);

int  bt_link_enabled(void);
void bt_link_set_enabled(int on, unsigned long now);

const bt_proto_t *bt_link_state(void);

/* Screen reachability: opening it wakes the module / keeps it from idling. */
void bt_link_screen_opened(unsigned long now);
void bt_link_screen_closed(unsigned long now);

void bt_link_scan(unsigned long now);
void bt_link_connect(int device, unsigned long now);
void bt_link_disconnect(unsigned long now);
void bt_link_forget(int device, unsigned long now);

/* Before power_off(): a powered module keeps the player from switching off. */
void bt_link_power_off(void);

/* Right-aligned Settings value: Off / Starting… / Searching… / Connecting… /
 * <name> / Not connected / On. */
const char *bt_link_status_text(void);

#endif /* RECHORD_SERVICES_BT_LINK_H */
