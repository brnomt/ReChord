/*
 * bt_proto.c — Bluetooth module protocol state machine.
 *
 * The module owns the stack; this file speaks its ASCII command language and
 * keeps the small amount of state the player needs (link, devices, stream,
 * headphone button presses). Behaviour follows the hardware notes in
 * docs/rewrite/bluetooth.md (derived from EchoPod's field work): poll periods,
 * connect/scan retries, the play/pause notices, and answering a new link with
 * "stream off" before "stream on".
 *
 * No I/O and no clock: the caller feeds bytes + milliseconds and sends the
 * frames returned. Buffers are fixed-size; no allocation.
 */
#include <string.h>

#include "bt_proto.h"

enum {
    BT_SCAN_HEADER = 4,
    BT_SCAN_PAYLOAD_LIMIT = 38,
    BT_CONNECTED_HEADER = 5,
    BT_SHORT_FRAME = 2,
    BT_STATUS_FRAME = 3,
    BT_WORD_FRAME = 4,
    BT_COMMAND_SIZE = 5,
    BT_ADDRESS_COMMAND_SIZE = 10,

    BT_STREAM_ON_LEVEL = 0x1E,
    BT_PLAY_NOTICE = 0x86,
    BT_PAUSE_NOTICE = 0x87,

    BT_PLAY_STARTED = 0,
    BT_PLAY_PAUSED = 1,
    BT_PLAY_PLAYING = 2,

    BT_SILENCE_MS = 2,
    BT_READY_TIMEOUT_MS = 5000,
    BT_SCAN_RETRY_MS = 4000,
    BT_SCAN_LIMIT_MS = 30000,
    BT_QUERY_PERIOD_MS = 3000,
    BT_LINK_QUERY_MS = 500,
    BT_CONNECT_QUERY_MS = 1000,
    BT_CONNECT_RETRY_MS = 4000,
    BT_CONNECT_LIMIT_MS = 30000,
    BT_PLAY_NOTICE_DELAY_MS = 100,
    BT_QUERY_PAUSE_MS = 50000
};

static int is_digit(uint8_t b)
{
    return b >= '0' && b <= '9';
}

static int word_is(const uint8_t *b, char a, char c)
{
    return b[0] == (uint8_t)a && b[1] == (uint8_t)c;
}

static uint32_t digits2(const uint8_t *b)
{
    return 10u * (uint32_t)(b[0] - '0') + (uint32_t)(b[1] - '0');
}

/* ---- device table ------------------------------------------------------- */

static int device_index(const bt_proto_t *bt, const uint8_t *addr)
{
    int i;

    for (i = 0; i < bt->count; i++) {
        int k = 0;

        while (k < BT_ADDR_LEN && bt->devices[i].addr[k] == addr[k])
            k++;
        if (k == BT_ADDR_LEN)
            return i;
    }
    return -1;
}

static void copy_device(bt_device_t *out, const uint8_t *payload, uint32_t len)
{
    uint32_t i, n;

    memset(out, 0, sizeof *out);
    if (len < BT_ADDR_LEN)
        return;
    for (i = 0; i < BT_ADDR_LEN; i++)
        out->addr[i] = payload[i];
    n = len - BT_ADDR_LEN;
    if (n > BT_NAME_MAX - 1u)
        n = BT_NAME_MAX - 1u;
    for (i = 0; i < n; i++)
        out->name[i] = (char)payload[BT_ADDR_LEN + i];
    out->name[n] = '\0';
}

/* Find or add a device; returns its index or -1. */
static int remember_device(bt_proto_t *bt, const uint8_t *payload, uint32_t len)
{
    int idx;

    if (len <= BT_ADDR_LEN)
        return -1;
    idx = device_index(bt, payload);
    if (idx >= 0) {
        bt->found_mask = (uint8_t)(bt->found_mask | (1u << idx));
        return idx;
    }
    if (bt->count == BT_DEV_MAX) {
        int i;

        for (i = 1; i < BT_DEV_MAX; i++)
            bt->devices[i - 1] = bt->devices[i];
        bt->count--;
        bt->found_mask = (uint8_t)(bt->found_mask >> 1);
        bt->saved_mask = (uint8_t)(bt->saved_mask >> 1);
        if (bt->connected > 0)
            bt->connected--;
        else if (bt->connected == 0)
            bt->connected = -1;
        if (bt->target > 0)
            bt->target--;
        else if (bt->target == 0)
            bt->target = -1;
    }
    copy_device(&bt->devices[bt->count], payload, len);
    bt->found_mask = (uint8_t)(bt->found_mask | (1u << bt->count));
    return bt->count++;
}

/* ---- state transitions -------------------------------------------------- */

static void set_state(bt_proto_t *bt, bt_state_t state, unsigned long now)
{
    if (state == BT_STATE_CONNECTED && bt->state != BT_STATE_CONNECTED) {
        /* A new link is always answered with stream-off before stream-on. */
        bt->resend_stream = 1;
        bt->resend_on = 0;
    }
    if (state != BT_STATE_CONNECTED) {
        bt->streaming = 0;
        bt->stream_pending = 0;
        bt->play_state = BT_PLAY_STARTED;
        bt->resend_stream = 0;
    }
    bt->state = state;
    bt->state_ms = now;
    bt->last_query_ms = now;
}

static void mark_connected(bt_proto_t *bt, unsigned long now)
{
    bt->pairing = BT_PAIR_OK;
    bt->connected = bt->target;
    set_state(bt, BT_STATE_CONNECTED, now);
}

static void finish_connect(bt_proto_t *bt, bt_pairing_t pairing,
                           unsigned long now)
{
    bt->pairing = pairing;
    if (bt->state == BT_STATE_CONNECTING)
        set_state(bt, BT_STATE_READY, now);
}

/* ---- frames received ---------------------------------------------------- */

static uint32_t scan_frame_size(const uint8_t *b, uint32_t available)
{
    if (available < BT_SCAN_HEADER || !is_digit(b[2]) || !is_digit(b[3]))
        return BT_SCAN_HEADER;
    {
        uint32_t len = digits2(b + 2);

        return len >= BT_SCAN_PAYLOAD_LIMIT ? BT_SCAN_HEADER
                                            : BT_SCAN_HEADER + len;
    }
}

static uint32_t status_frame_size(const uint8_t *b, uint32_t available)
{
    if (available < BT_STATUS_FRAME || b[2] != 'O')
        return BT_STATUS_FRAME;
    if (available < BT_CONNECTED_HEADER || !is_digit(b[3]) || !is_digit(b[4]))
        return BT_CONNECTED_HEADER;
    return BT_CONNECTED_HEADER + digits2(b + 3);
}

static uint32_t result_frame_size(const uint8_t *b, uint32_t available)
{
    if (available < BT_WORD_FRAME)
        return BT_WORD_FRAME;
    return (word_is(b + 2, 'O', 'K') || word_is(b + 2, 'E', 'R'))
               ? BT_WORD_FRAME
               : BT_SHORT_FRAME;
}

static uint32_t frame_size(const uint8_t *b, uint32_t available)
{
    switch (b[1]) {
    case 'S':
        return scan_frame_size(b, available);
    case 'Q':
        return status_frame_size(b, available);
    case 'T':
    case 'H':
        return result_frame_size(b, available);
    case 'V':
    case 'K':
    case 'L':
        return BT_WORD_FRAME;
    case 'A':
    case 'B':
    case 'P':
    case 'C':
        return BT_SHORT_FRAME;
    default:
        return 1;
    }
}

static void handle_scan(bt_proto_t *bt, const uint8_t *b, uint32_t size,
                        unsigned long now)
{
    if (word_is(b + 2, 'E', 'N') || size == BT_SCAN_HEADER) {
        if (!word_is(b + 2, 'E', 'R') && bt->state == BT_STATE_SCANNING)
            set_state(bt, BT_STATE_READY, now);
        return;
    }
    (void)remember_device(bt, b + BT_SCAN_HEADER, size - BT_SCAN_HEADER);
}

static void handle_ready(bt_proto_t *bt, uint8_t version, unsigned long now)
{
    bt->version = (char)version;
    if (bt->state == BT_STATE_STARTING)
        set_state(bt, BT_STATE_READY, now);
}

static void handle_status(bt_proto_t *bt, const uint8_t *b, uint32_t size,
                          unsigned long now)
{
    bt->link_status = (char)b[2];
    if (b[2] == 'O' && size > BT_CONNECTED_HEADER) {
        int idx = remember_device(bt, b + BT_CONNECTED_HEADER,
                                  size - BT_CONNECTED_HEADER);

        bt->pairing = BT_PAIR_OK;
        bt->connected = (int8_t)idx;
        set_state(bt, BT_STATE_CONNECTED, now);
    } else if (bt->state == BT_STATE_CONNECTED) {
        bt->pairing = BT_PAIR_NONE;
        bt->connected = -1;
        set_state(bt, BT_STATE_READY, now);
    }
}

static void handle_link(bt_proto_t *bt, const uint8_t *b, uint32_t size,
                        unsigned long now)
{
    if (size == BT_WORD_FRAME && word_is(b + 2, 'E', 'R')) {
        finish_connect(bt, BT_PAIR_FAILED, now);
    } else if (b[1] == 'H') {
        mark_connected(bt, now);
    } else if (word_is(b + 2, 'O', 'K')) {
        finish_connect(bt, BT_PAIR_OK, now);
    }
}

static void handle_disconnected(bt_proto_t *bt, unsigned long now)
{
    bt_state_t state = bt->state;

    bt->pairing = BT_PAIR_NONE;
    bt->connected = -1;
    if (state == BT_STATE_CONNECTED || state == BT_STATE_CONNECTING)
        set_state(bt, BT_STATE_READY, now);
}

static void handle_play_report(bt_proto_t *bt, int playing, unsigned long now)
{
    if (bt->state != BT_STATE_CONNECTED && bt->pairing == BT_PAIR_OK)
        mark_connected(bt, now);
    bt->play_state = playing ? BT_PLAY_PLAYING : BT_PLAY_PAUSED;
    bt->query_paused = bt->state == BT_STATE_CONNECTED;
    bt->query_paused_ms = now;
    if (playing)
        bt->remote_plays++;
    else
        bt->remote_pauses++;
}

static void handle_frame(bt_proto_t *bt, const uint8_t *b, uint32_t size,
                         unsigned long now)
{
    if (b[1] != 'Q')
        bt->answered = 1;
    switch (b[1]) {
    case 'V':
        /* The module's ready frame has been seen as both "CV<ver>" and
         * "CV<status><ver>"; the version is the last byte of either. */
        handle_ready(bt, size > 3 ? b[3] : (size > 2 ? b[2] : '0'), now);
        break;
    case 'S':
        handle_scan(bt, b, size, now);
        break;
    case 'Q':
        handle_status(bt, b, size, now);
        break;
    case 'L':
    case 'H':
        handle_link(bt, b, size, now);
        break;
    case 'T':
        handle_disconnected(bt, now);
        break;
    case 'P':
    case 'C':
        handle_play_report(bt, b[1] == 'C', now);
        break;
    case 'A':
        bt->remote_prevs++;
        break;
    case 'B':
        bt->remote_nexts++;
        break;
    default:
        break;
    }
}

/* Consume as many complete frames as possible. `silent` lets a short frame
 * (CV1) be accepted once the line has been quiet for 2 ms. */
static void parse(bt_proto_t *bt, int silent, unsigned long now)
{
    uint32_t start = 0;

    while (start < bt->rx_len) {
        const uint8_t *b = bt->rx + start;
        uint32_t available = bt->rx_len - start;
        uint32_t consumed;

        if (b[0] != 'C') {
            start++;
            continue;
        }
        if (available < BT_SHORT_FRAME) {
            if (!silent)
                break;
            consumed = available;
        } else {
            consumed = frame_size(b, available);
            if (consumed > available) {
                if (!silent)
                    break;
                consumed = available;
            }
            if (consumed > 1)
                handle_frame(bt, b, consumed, now);
        }
        start += consumed;
    }
    if (start > 0) {
        uint32_t kept = bt->rx_len - start;
        uint32_t i;

        for (i = 0; i < kept; i++)
            bt->rx[i] = bt->rx[start + i];
        bt->rx_len = (uint8_t)kept;
    }
}

/* ---- commands ----------------------------------------------------------- */

static int command(bt_proto_t *bt, char letter, bt_frame_t *out)
{
    (void)bt;
    out->bytes[0] = 'C';
    out->bytes[1] = (uint8_t)letter;
    out->bytes[2] = '0';
    out->bytes[3] = '0';
    out->bytes[4] = 0;
    out->len = BT_COMMAND_SIZE;
    return 1;
}

static int address_command(bt_proto_t *bt, char letter, bt_frame_t *out)
{
    int i;

    out->bytes[0] = 'C';
    out->bytes[1] = (uint8_t)letter;
    out->bytes[2] = '0';
    out->bytes[3] = '6';
    for (i = 0; i < BT_ADDR_LEN; i++)
        out->bytes[4 + i] = bt->target_addr[i];
    out->len = BT_ADDRESS_COMMAND_SIZE;
    return 1;
}

static int stream_frame(bt_proto_t *bt, int on, bt_frame_t *out)
{
    (void)bt;
    out->bytes[0] = 'C';
    out->bytes[1] = 'Y';
    out->bytes[2] = '0';
    out->bytes[3] = on ? BT_STREAM_ON_LEVEL : 0;
    out->bytes[4] = 0;
    out->len = BT_COMMAND_SIZE;
    return 1;
}

static int notice_frame(int notice, bt_frame_t *out)
{
    out->bytes[0] = 'C';
    out->bytes[1] = (uint8_t)notice;
    out->bytes[2] = '0';
    out->bytes[3] = '0';
    out->bytes[4] = 0;
    out->len = BT_COMMAND_SIZE;
    return 1;
}

int bt_proto_scan(bt_proto_t *bt, unsigned long now, bt_frame_t *out)
{
    if (bt->state != BT_STATE_READY)
        return 0;
    set_state(bt, BT_STATE_SCANNING, now);
    bt->found_mask = 0;
    bt->answered = 0;                  /* this scan has not answered yet */
    bt->repeated = 0;
    return command(bt, 'S', out);
}

int bt_proto_stop_scan(bt_proto_t *bt, unsigned long now, bt_frame_t *out)
{
    if (bt->state != BT_STATE_SCANNING)
        return 0;
    set_state(bt, BT_STATE_READY, now);
    return command(bt, 'P', out);
}

int bt_proto_connect(bt_proto_t *bt, int device, unsigned long now,
                     bt_frame_t *out)
{
    if (bt->state != BT_STATE_READY && bt->state != BT_STATE_SCANNING)
        return 0;
    if (device < 0 || device >= bt->count)
        return 0;
    bt->target = (int8_t)device;
    memcpy(bt->target_addr, bt->devices[device].addr, BT_ADDR_LEN);
    bt->pairing = BT_PAIR_NONE;
    set_state(bt, BT_STATE_CONNECTING, now);
    bt->answered = 0;
    bt->repeated = 0;
    return address_command(bt, 'L', out);
}

int bt_proto_disconnect(bt_proto_t *bt, unsigned long now, bt_frame_t *out)
{
    int paired = bt->state == BT_STATE_READY && bt->pairing == BT_PAIR_OK;

    if (bt->state == BT_STATE_CONNECTING) {
        set_state(bt, BT_STATE_READY, now);
    } else if (bt->state != BT_STATE_CONNECTED && !paired) {
        return 0;
    }
    bt->pairing = BT_PAIR_NONE;
    bt->connected = -1;
    return command(bt, 'T', out);
}

int bt_proto_set_stream(bt_proto_t *bt, int on, unsigned long now,
                        bt_frame_t *out)
{
    uint8_t notice = 0;

    if (bt->state != BT_STATE_CONNECTED)
        return 0;
    bt->streaming = on ? 1 : 0;
    bt->stream_ms = now;
    bt->stream_pending = 0;
    if (on && bt->play_state == BT_PLAY_PAUSED) {
        notice = BT_PLAY_NOTICE;
        bt->play_state = BT_PLAY_PLAYING;
    } else if (!on && bt->play_state == BT_PLAY_PLAYING) {
        notice = BT_PAUSE_NOTICE;
        bt->play_state = BT_PLAY_PAUSED;
    }
    if (notice == 0)
        return stream_frame(bt, on, out);
    /* The module wants the notice first; the stream frame follows after
     * 100 ms (BT_PLAY_NOTICE_DELAY_MS). */
    bt->stream_pending = 1;
    return notice_frame(notice, out);
}

/* ---- timers ------------------------------------------------------------- */

static int query_status(bt_proto_t *bt, unsigned long now, bt_frame_t *out)
{
    unsigned long period;

    if (bt->query_paused && now - bt->query_paused_ms < BT_QUERY_PAUSE_MS)
        return 0;
    bt->query_paused = 0;
    if (bt->state == BT_STATE_CONNECTING)
        period = BT_CONNECT_QUERY_MS;
    else if (bt->state == BT_STATE_READY && bt->pairing == BT_PAIR_OK)
        period = BT_LINK_QUERY_MS;
    else
        period = BT_QUERY_PERIOD_MS;
    if (now - bt->last_query_ms < period)
        return 0;
    bt->last_query_ms = now;
    return command(bt, 'Q', out);
}

static int send_pending_stream(bt_proto_t *bt, unsigned long now,
                               bt_frame_t *out)
{
    if (!bt->stream_pending ||
        now - bt->stream_ms < BT_PLAY_NOTICE_DELAY_MS)
        return 0;
    bt->stream_pending = 0;
    return stream_frame(bt, bt->streaming, out);
}

static int answer_new_link(bt_proto_t *bt, bt_frame_t *out)
{
    if (!bt->resend_stream)
        return 0;
    bt->resend_stream = 0;
    return stream_frame(bt, bt->resend_on, out);
}

int bt_proto_poll(bt_proto_t *bt, unsigned long now, bt_frame_t *out)
{
    parse(bt, now - bt->last_byte_ms >= BT_SILENCE_MS, now);
    switch (bt->state) {
    case BT_STATE_STARTING:
        if (now - bt->state_ms >= BT_READY_TIMEOUT_MS)
            set_state(bt, BT_STATE_NO_REPLY, now);
        return 0;
    case BT_STATE_SCANNING: {
        unsigned long elapsed = now - bt->state_ms;

        if (elapsed >= BT_SCAN_LIMIT_MS) {
            set_state(bt, BT_STATE_READY, now);
            return command(bt, 'P', out);
        }
        if (!bt->answered && !bt->repeated && elapsed >= BT_SCAN_RETRY_MS) {
            bt->repeated = 1;
            return command(bt, 'S', out);
        }
        return query_status(bt, now, out);
    }
    case BT_STATE_CONNECTING: {
        unsigned long elapsed = now - bt->state_ms;

        if (elapsed >= BT_CONNECT_LIMIT_MS) {
            bt->pairing = BT_PAIR_FAILED;
            set_state(bt, BT_STATE_READY, now);
            return command(bt, 'T', out);
        }
        if (!bt->answered && !bt->repeated &&
            elapsed >= BT_CONNECT_RETRY_MS) {
            bt->repeated = 1;
            return address_command(bt, 'H', out);
        }
        return query_status(bt, now, out);
    }
    case BT_STATE_CONNECTED:
        return answer_new_link(bt, out) ||
               send_pending_stream(bt, now, out) ||
               query_status(bt, now, out);
    case BT_STATE_READY:
        return query_status(bt, now, out);
    default:
        return 0;
    }
}

/* ---- lifecycle ---------------------------------------------------------- */

void bt_proto_init(bt_proto_t *bt)
{
    memset(bt, 0, sizeof *bt);
    bt->connected = -1;
    bt->target = -1;
    bt->state = BT_STATE_OFF;
}

void bt_proto_power_on(bt_proto_t *bt, unsigned long now)
{
    /* Keep the remembered devices across an off/on cycle (the module does not
     * report them again until it answers). */
    uint8_t count = bt->count;
    uint8_t saved = bt->saved_mask;
    bt_device_t devices[BT_DEV_MAX];

    memcpy(devices, bt->devices, sizeof devices);
    memset(bt, 0, sizeof *bt);
    memcpy(bt->devices, devices, sizeof devices);
    bt->count = count;
    bt->saved_mask = saved;
    bt->connected = -1;
    bt->target = -1;
    bt->state = BT_STATE_STARTING;
    bt->state_ms = now;
    bt->last_query_ms = now;
}

void bt_proto_power_off(bt_proto_t *bt)
{
    bt_state_t state = BT_STATE_OFF;

    bt->state = state;
    bt->streaming = 0;
    bt->connected = -1;
    bt->target = -1;
    bt->pairing = BT_PAIR_NONE;
    bt->rx_len = 0;
    bt->resend_stream = 0;
    bt->stream_pending = 0;
}

void bt_proto_receive(bt_proto_t *bt, uint8_t byte, unsigned long now)
{
    bt->last_byte_ms = now;
    if (bt->rx_len == BT_RX_MAX)
        parse(bt, 1, now);
    if (bt->rx_len == BT_RX_MAX)
        return;
    bt->rx[bt->rx_len++] = byte;
}

int bt_proto_device_count(const bt_proto_t *bt)
{
    return bt->count;
}

const bt_device_t *bt_proto_device(const bt_proto_t *bt, int index)
{
    if (index < 0 || index >= bt->count)
        return 0;
    return &bt->devices[index];
}

const bt_device_t *bt_proto_connected(const bt_proto_t *bt)
{
    if (bt->connected < 0 || bt->connected >= bt->count)
        return 0;
    return &bt->devices[bt->connected];
}

void bt_proto_seed_devices(bt_proto_t *bt, const bt_device_t *devices,
                           int count)
{
    int i;

    if (bt == 0 || devices == 0 || count <= 0)
        return;
    if (count > BT_DEV_MAX)
        count = BT_DEV_MAX;
    for (i = 0; i < count; i++)
        bt->devices[i] = devices[i];
    bt->count = (uint8_t)count;
    bt->found_mask = 0;
    bt->saved_mask = (uint8_t)((1u << count) - 1u);
    bt->connected = -1;
    bt->target = -1;
}

void bt_proto_remove_device(bt_proto_t *bt, int index)
{
    int i;

    if (bt == 0 || index < 0 || index >= bt->count)
        return;
    for (i = index; i + 1 < bt->count; i++)
        bt->devices[i] = bt->devices[i + 1];
    bt->count--;
    {
        uint8_t low = (uint8_t)(bt->found_mask & ((1u << index) - 1u));
        uint8_t high = (uint8_t)((bt->found_mask >> (index + 1)) << index);

        bt->found_mask = (uint8_t)(low | high);
        low = (uint8_t)(bt->saved_mask & ((1u << index) - 1u));
        high = (uint8_t)((bt->saved_mask >> (index + 1)) << index);
        bt->saved_mask = (uint8_t)(low | high);
    }
    if (bt->connected == index)
        bt->connected = -1;
    else if (bt->connected > index)
        bt->connected--;
    if (bt->target == index)
        bt->target = -1;
    else if (bt->target > index)
        bt->target--;
}

void bt_proto_mark_saved(bt_proto_t *bt, int index)
{
    if (bt != 0 && index >= 0 && index < bt->count)
        bt->saved_mask = (uint8_t)(bt->saved_mask | (1u << index));
}

int bt_proto_is_saved(const bt_proto_t *bt, int index)
{
    if (bt == 0 || index < 0 || index >= bt->count)
        return 0;
    return (bt->saved_mask >> index) & 1u;
}

void bt_proto_promote_device(bt_proto_t *bt, int index)
{
    bt_device_t saved;
    int i;

    if (bt == 0 || index <= 0 || index >= bt->count)
        return;
    saved = bt->devices[index];
    for (i = index; i > 0; i--)
        bt->devices[i] = bt->devices[i - 1];
    bt->devices[0] = saved;
    {
        /* [index][0..index-1][index+1..] — move the chosen bit to 0, shift
         * the lower ones up, leave the rest where they are. */
        uint8_t moved = (uint8_t)((bt->saved_mask >> index) & 1u);
        uint8_t low = (uint8_t)(bt->saved_mask & ((1u << index) - 1u));
        uint8_t high = (uint8_t)(bt->saved_mask &
                                 (uint8_t)~((1u << (index + 1)) - 1u));

        bt->saved_mask = (uint8_t)(high | (low << 1) | moved);
    }

    if (bt->connected == index)
        bt->connected = 0;
    else if (bt->connected >= 0 && bt->connected < index)
        bt->connected++;
    if (bt->target == index)
        bt->target = 0;
    else if (bt->target >= 0 && bt->target < index)
        bt->target++;
}
