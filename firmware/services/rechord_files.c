/*
 * rechord_files.c — the device's own files on the internal volume.
 *
 * Until now every write the device did was an in-place rewrite of a file that
 * already existed (the settings path): creating anything needed the FAT
 * allocation path, which stayed off while the project bricked twice. That
 * turned out to be the .bss ceiling at 0x0304A000, not this code (DFIU
 * fb41cb15), so the first-boot initializer lives here:
 *
 *   /.rechord/rechord.cfg   key=value settings, 64 KiB allocated
 *   /.rechord/state         runtime state for resume (track + position)
 *
 * Both are idempotent through fs_target's create primitives: an existing
 * directory or file is left alone, so running this on every boot never
 * touches user data. The device still only ever *rewrites* what is already
 * there after this point -- nothing else allocates.
 */
#include <string.h>

#include "fs.h"
#include "fs_target.h"
#include "settings.h"

#include "media_db.h"
#include "rechord_files.h"

/* The cfg is created with room to grow (the in-place rewrite keeps the file's
 * clusters and only patches the size), so the user editing it later cannot
 * run the device out of space for its own settings. state and LOG are small
 * on purpose: both are rewritten by the device. */
#define CFG_BYTES   (64u * 1024u)
#define STATE_BYTES (4u * 1024u)
#define LOG_BYTES   (32u * 1024u)

/* Written into a freshly created file so it reads as text instead of a block
 * of zeros. The device replaces the content on its next write. */
static const char k_state_template[] =
    "# ReChord runtime state -- written by the device, not by hand\n"
    "# track=<full path of the last played file>\n"
    "# pos_ms=<position inside it>\n"
    "# folder=<browser folder>\n"
    "# row=<browser row>\n";

static const char k_log_template[] =
    "# ReChord log -- rewritten on every flush (USB storage / power off),\n"
    "# so it holds the lines of the latest one. Timestamps are seconds\n"
    "# since boot.\n";

static int ensure_file(const char *path, uint32_t bytes, const char *template_text)
{
    fs_file_t *f = fs_open_read(path);

    if (f != 0) {
        fs_close(f);
        return FS_OK;                    /* exists: never touched again */
    }
    if (fs_target_file_create(path, bytes) != FS_OK)
        return FS_ERR_IO;
    if (template_text != 0) {
        unsigned len = (unsigned)strlen(template_text);

        if (len > 0u) {
            int r = fs_target_file_rewrite(path, template_text, len);

            if (r != FS_OK)
                return r;
        }
    }
    return FS_OK;
}

int rechord_files_init(void)
{
    settings_t defaults;
    char text[512];
    int r, len;

    /* The volume itself may be absent (no writer, not mounted): then there is
     * nothing to do and nothing to report as a failure. */
    r = fs_target_mkdir(RECHORD_DIR);
    if (r == FS_ERR_OPEN)
        return FS_OK;
    if (r != FS_OK)
        return r;

    len = 0;
    r = ensure_file(RECHORD_CFG, CFG_BYTES, 0);
    if (r != FS_OK)
        return r;

    /* A fresh cfg gets the documented defaults written into it, so the file
     * is valid and readable before the first save. An existing file keeps
     * whatever the user has. */
    settings_defaults(&defaults);
    len = settings_serialize_text(&defaults, text, sizeof text);
    if (len > 0) {
        fs_file_t *f = fs_open_read(RECHORD_CFG);
        long size = f ? fs_file_size(f) : -1;

        if (f != 0)
            fs_close(f);
        if (size == (long)CFG_BYTES)     /* freshly created (all zeros) */
            (void)fs_target_file_rewrite(RECHORD_CFG, text, (uint32_t)len);
    }

    if (ensure_file(RECHORD_STATE, STATE_BYTES, k_state_template) != FS_OK)
        return FS_ERR_IO;
    if (ensure_file(RECHORD_LOG, LOG_BYTES, k_log_template) != FS_OK)
        return FS_ERR_IO;
    /* The media index slots: empty until the first scan (they are binary, so
     * no text header), created here so a build always has a file to target. */
    if (ensure_file(MEDIA_DB_A, 0u, 0) != FS_OK)
        return FS_ERR_IO;
    return ensure_file(MEDIA_DB_B, 0u, 0);
}
