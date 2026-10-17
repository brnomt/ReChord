/*
 * settings_store.c — target settings persistence (BRNOMT-71).
 *
 * The config is the plain-text 12-key file SETTINGS_PATH, inside the
 * user-visible /.rechord folder of the internal eMMC volume. Loads use the
 * read-only fs.h path; SAVES go through fs_target_file_rewrite(): the data
 * sectors of the existing file are rewritten and its 4-byte size is patched,
 * so no directory entry is created, nothing is allocated and the FAT table
 * is never touched (the class of write that bricked earlier builds is not on
 * this path at all — see HANDOVER §R / DFIU fb41cb15).
 *
 * The file must therefore pre-exist with allocated clusters (created once
 * from a PC, or later by the first-boot init). When it is missing or
 * read-only, load falls back to defaults and save reports an error — the
 * device never tries to create it here.
 *
 * Buffers are stack-only (≤ 768 B): the target .bss budget is exhausted
 * under the 0x0304A000 linker gate, and this backend must cost 0 B of it.
 */
#include "settings.h"
#include "fs.h"
#include "fs_target.h"

/* Longest config text read back (the 12 keys live in the first lines). */
#define CFG_READ_MAX 512

int settings_load(settings_t *s)
{
    char buf[CFG_READ_MAX];
    fs_file_t *f;
    long got;
    int rc = SETTINGS_OK;

    if (s == 0)
        return SETTINGS_ERR_ARG;
    settings_defaults(s);

    f = fs_open_read(SETTINGS_PATH);
    if (f == 0)
        return SETTINGS_ERR_OPEN;        /* missing: defaults stay           */
    got = fs_read(f, buf, (unsigned long)sizeof(buf) - 1);
    if (got < 0) {
        rc = SETTINGS_ERR_IO;
        got = 0;
    }
    buf[got] = '\0';
    fs_close(f);

    if (rc == SETTINGS_OK)
        rc = settings_parse_text(buf, s);
    return rc;
}

int settings_save(const settings_t *s)
{
    char text[768];
    int len;

    if (s == 0)
        return SETTINGS_ERR_ARG;
    len = settings_serialize_text(s, text, (unsigned long)sizeof(text));
    if (len < 0)
        return len;
    /* In place: data sectors + the dirent size. Any failure (missing file,
     * volume without a writer, exFAT, not enough allocated clusters) leaves
     * the previous file contents intact. */
    if (fs_target_file_rewrite(SETTINGS_PATH, text, (uint32_t)len) != FS_OK)
        return SETTINGS_ERR_IO;
    return SETTINGS_OK;
}
