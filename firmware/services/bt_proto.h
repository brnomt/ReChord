/*
 * bt_proto.h — Bluetooth module protocol (ASCII frames over UART1).
 *
 * Pure logic: no registers, no filesystem, no clock source. The caller feeds
 * received bytes, time in milliseconds, and sends the frames this module
 * returns. Host-tested against scripted module streams.
 *
 * Grammar (docs/rewrite/bluetooth.md): frames start with 'C' and a letter;
 * scan results and the connected status carry a two-digit payload length.
 */
#ifndef RECHORD_SERVICES_BT_PROTO_H
#define RECHORD_SERVICES_BT_PROTO_H

#include <stdint.h>

#define BT_DEV_MAX       6
#define BT_ADDR_LEN      6
#define BT_NAME_MAX      32
#define BT_RX_MAX        48     /* max frame: 4 + 6 + 32 (scan result)      */
#define BT_FRAME_MAX     10     /* max frame we send (CL06 + address)       */

typedef struct {
    uint8_t addr[BT_ADDR_LEN];
    char    name[BT_NAME_MAX];
} bt_device_t;

typedef enum {
    BT_STATE_OFF = 0,
    BT_STATE_STARTING,
    BT_STATE_NO_REPLY,
    BT_STATE_READY,
    BT_STATE_SCANNING,
    BT_STATE_CONNECTING,
    BT_STATE_CONNECTED
} bt_state_t;

typedef enum {
    BT_PAIR_NONE = 0,
    BT_PAIR_OK,
    BT_PAIR_FAILED
} bt_pairing_t;

typedef struct {
    bt_state_t    state;
    char          version;
    char          link_status;
    bt_pairing_t  pairing;
    uint8_t       streaming;
    uint8_t       play_state;      /* 0 started, 1 paused, 2 playing      */

    uint8_t       count;
    uint8_t       found_mask;      /* seen in the current scan             */
    uint8_t       saved_mask;      /* user-chosen: persisted + auto-connect */
    bt_device_t   devices[BT_DEV_MAX];
    int8_t        connected;       /* index into devices, -1 = none       */
    int8_t        target;          /* index being connected, -1 = none    */
    uint8_t       target_addr[BT_ADDR_LEN];

    uint8_t       remote_plays;
    uint8_t       remote_pauses;
    uint8_t       remote_nexts;
    uint8_t       remote_prevs;

    /* receive parser */
    uint8_t       rx[BT_RX_MAX];
    uint8_t       rx_len;
    unsigned long last_byte_ms;

    /* state timers */
    unsigned long state_ms;
    unsigned long last_query_ms;
    unsigned long query_paused_ms;
    unsigned long stream_ms;
    uint8_t       answered;
    uint8_t       repeated;
    uint8_t       query_paused;
    uint8_t       resend_stream;
    uint8_t       resend_on;
    uint8_t       stream_pending;
} bt_proto_t;

/* One frame to send. */
typedef struct {
    uint8_t bytes[BT_FRAME_MAX];
    uint8_t len;
} bt_frame_t;

void bt_proto_init(bt_proto_t *bt);
void bt_proto_power_on(bt_proto_t *bt, unsigned long now);
void bt_proto_power_off(bt_proto_t *bt);

/* Feed one received byte. */
void bt_proto_receive(bt_proto_t *bt, uint8_t byte, unsigned long now);

/* Service timers/parser. Returns 1 and fills `out` when a frame must be sent. */
int  bt_proto_poll(bt_proto_t *bt, unsigned long now, bt_frame_t *out);

int  bt_proto_scan(bt_proto_t *bt, unsigned long now, bt_frame_t *out);
int  bt_proto_stop_scan(bt_proto_t *bt, unsigned long now, bt_frame_t *out);
int  bt_proto_connect(bt_proto_t *bt, int device, unsigned long now,
                      bt_frame_t *out);
int  bt_proto_disconnect(bt_proto_t *bt, unsigned long now, bt_frame_t *out);
int  bt_proto_set_stream(bt_proto_t *bt, int on, unsigned long now,
                         bt_frame_t *out);

/* True when the module asked to be woken (a frame is pending). */
int  bt_proto_device_count(const bt_proto_t *bt);
const bt_device_t *bt_proto_device(const bt_proto_t *bt, int index);
const bt_device_t *bt_proto_connected(const bt_proto_t *bt);

/* Seed the remembered devices (from /.rechord/BT) before the module is on.
 * Found-this-scan flags are cleared. */
void bt_proto_seed_devices(bt_proto_t *bt, const bt_device_t *devices,
                           int count);

/* Remove one device (forget) or move it to the front (most recent). */
void bt_proto_remove_device(bt_proto_t *bt, int index);
void bt_proto_promote_device(bt_proto_t *bt, int index);

/* The user chose this device (connect/pair): persist it and let the policy
 * auto-connect to it later. Scan results are not saved until chosen. */
void bt_proto_mark_saved(bt_proto_t *bt, int index);
int  bt_proto_is_saved(const bt_proto_t *bt, int index);

#endif /* RECHORD_SERVICES_BT_PROTO_H */
