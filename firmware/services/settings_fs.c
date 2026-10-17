/*
 * settings_fs.c — settings I/O through the whole-file fs.h API (host tools
 * and tests; the target uses settings_store.c).
 *
 * Creates/truncates the file on save, so it is the backend for a POSIX
 * filesystem where files can be allocated freely. The on-disk format is the
 * plain-text 12-key schema parsed/serialized in settings.c.
 */
#include <stddef.h>

#include "settings.h"
#include "fs.h"

/* Read buffer for the whole config file; the format is tiny by design. */
#define SETTINGS_FILE_MAX 2048

int settings_load(settings_t *s)
{
    static char buf[SETTINGS_FILE_MAX];
    fs_file_t *f;
    long got;
    long size;
    int rc = SETTINGS_OK;

    if (s == NULL) {
        return SETTINGS_ERR_ARG;
    }
    settings_defaults(s);

    f = fs_open_read(SETTINGS_PATH);
    if (f == NULL) {
        return SETTINGS_ERR_OPEN;
    }
    size = fs_file_size(f);
    got = fs_read(f, buf, (unsigned long)sizeof(buf) - 1);
    if (got < 0) {
        rc = SETTINGS_ERR_IO;
        got = 0;
    } else if (size > (long)sizeof(buf) - 1) {
        /* Over-long file: parse the part we hold (all 12 keys live in the
         * first lines by format convention). Not an error — tolerant. */
        got = (long)sizeof(buf) - 1;
    }
    buf[got] = '\0';
    fs_close(f);

    if (rc == SETTINGS_OK) {
        rc = settings_parse_text(buf, s);
    }
    return rc;
}

int settings_save(const settings_t *s)
{
    char text[768];
    fs_file_t *f;
    int len;
    long wrote;

    if (s == NULL) {
        return SETTINGS_ERR_ARG;
    }
    len = settings_serialize_text(s, text, (unsigned long)sizeof(text));
    if (len < 0) {
        return len;
    }
    f = fs_open_write(SETTINGS_PATH);
    if (f == NULL) {
        return SETTINGS_ERR_OPEN;
    }
    wrote = fs_write(f, text, (unsigned long)len);
    fs_close(f);
    return (wrote == (long)len) ? SETTINGS_OK : SETTINGS_ERR_IO;
}
