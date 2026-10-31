/*
 * bt_store.c — Bluetooth settings file (on/off + up to six devices).
 *
 * Parsing is strict per line: an unknown or malformed line is skipped, so a
 * half-written file never loses the whole choice. Saving rewrites the file in
 * place through fs_target_file_rewrite() (same class of write as the config;
 * never an allocation).
 */
#include <stdio.h>
#include <string.h>

#include "bt_store.h"
#include "fs.h"

#define BT_TEXT_MAX 384

void bt_store_default(bt_store_t *st)
{
    if (st == 0)
        return;
    memset(st, 0, sizeof *st);
}

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static void parse_text(const char *text, bt_store_t *st)
{
    const char *p = text;

    while (p != 0 && *p != '\0') {
        const char *eol = p;

        while (*eol != '\0' && *eol != '\n' && *eol != '\r')
            eol++;
        if (eol - p == 4 && strncmp(p, "on=", 3) == 0 && st != 0) {
            st->on = (p[3] == '1') ? 1 : 0;
        } else if (eol - p >= 15 && p[0] == 'd' && p[1] == '=' &&
                   st != 0 && st->count < BT_DEV_MAX) {
            int i, ok = 1;

            for (i = 0; i < BT_ADDR_LEN; i++) {
                int hi = hex_nibble(p[2 + i * 2]);
                int lo = hex_nibble(p[3 + i * 2]);

                if (hi < 0 || lo < 0) {
                    ok = 0;
                    break;
                }
                st->devices[st->count].addr[i] = (uint8_t)((hi << 4) | lo);
            }
            if (ok && p[14] == ',') {
                const char *name = p + 15;
                unsigned n = 0;

                while (name + n < eol && n + 1u < BT_NAME_MAX) {
                    st->devices[st->count].name[n] = name[n];
                    n++;
                }
                st->devices[st->count].name[n] = '\0';
                st->count++;
            }
        }
        p = (*eol != '\0') ? eol + 1 : eol;
    }
}

int bt_store_load(bt_store_t *st)
{
    char buf[BT_TEXT_MAX];
    fs_file_t *f;
    long got;

    if (st == 0)
        return -1;
    bt_store_default(st);

    f = fs_open_read(BT_STORE_PATH);
    if (f == 0)
        return -1;
    got = fs_read(f, buf, (unsigned long)sizeof buf - 1u);
    fs_close(f);
    if (got < 0) {
        bt_store_default(st);
        return -1;
    }
    buf[got] = '\0';
    parse_text(buf, st);
    return 0;
}

int bt_store_save(const bt_store_t *st)
{
    char text[BT_TEXT_MAX];
    int n = 0, i, j;

    if (st == 0)
        return -1;
    n = snprintf(text, sizeof text, "# ReChord Bluetooth\non=%d\n",
                 st->on ? 1 : 0);
    if (n < 0 || (unsigned)n >= sizeof text)
        return -1;
    for (i = 0; i < st->count && i < BT_DEV_MAX; i++) {
        int left = (int)sizeof text - n;

        if (left < 18)
            return -1;
        n += snprintf(text + n, (size_t)left, "d=");
        for (j = 0; j < BT_ADDR_LEN; j++) {
            left = (int)sizeof text - n;
            n += snprintf(text + n, (size_t)left, "%02x",
                          st->devices[i].addr[j]);
        }
        left = (int)sizeof text - n;
        n += snprintf(text + n, (size_t)left, ",%.*s\n", BT_NAME_MAX - 1,
                      st->devices[i].name);
        if (n < 0 || (unsigned)n >= sizeof text)
            return -1;
    }
    if (fs_truncate(BT_STORE_PATH) != FS_OK)
        return -1;
    if (fs_append(BT_STORE_PATH, text, (uint32_t)n) != FS_OK)
        return -1;
    return 0;
}
