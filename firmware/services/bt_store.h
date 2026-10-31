/*
 * bt_store.h — Bluetooth on/off choice + remembered devices.
 *
 * Text file (same dialect as rechord.cfg/state) at /.rechord/BT:
 *
 *   # ReChord Bluetooth
 *   on=1
 *   d=<12 hex chars>,<name>
 *
 * Up to six devices, most recent first. Buffers are stack-only (0 B .bss):
 * the AP's RAM headroom is the tight budget.
 */
#ifndef RECHORD_SERVICES_BT_STORE_H
#define RECHORD_SERVICES_BT_STORE_H

#include "bt_proto.h"

#define BT_STORE_PATH "/Internal/.rechord/BT"

typedef struct {
    uint8_t     on;
    uint8_t     count;
    bt_device_t devices[BT_DEV_MAX];
} bt_store_t;

void bt_store_default(bt_store_t *st);

/* 0 on success; on failure `st` is left at defaults (a missing file is not an
 * error worth failing the boot). */
int  bt_store_load(bt_store_t *st);

/* 0 on success, -1 when the file could not be rewritten. */
int  bt_store_save(const bt_store_t *st);

#endif /* RECHORD_SERVICES_BT_STORE_H */
