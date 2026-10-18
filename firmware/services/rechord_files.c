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

/* ---- standard theme packages (BRNOMT-69) -------------------------------- */
/* Materialized on first boot through the SAME create/mkdir path that makes
 * /.rechord and the cfg, so no pre-existing files are needed. Identical to the
 * authoring copies shipped in themes/<id>/theme.cfg. Created once; a later boot
 * never overwrites a file the user may have edited. */
#define THEME_LAYOUT_TAIL \
    "pad=4\ntitle_h=16\nitem_h=12\nnp_title_y=24\nnp_artist_y=36\n" \
    "np_state_y=48\nnp_info_y=60\nnp_bar_y=74\nnp_bar_h=2\nnp_time_y=84\n" \
    "np_mode_y=148\nnp_vol_y=162\nnp_vol_h=2\n"

static const char k_theme_mono[] =
    "# ReChord Mono (house look)\nname=Mono\n"
    "background=0x0000\nsurface=0x0000\ntext=0xFFFF\ntext_dim=0x8410\n"
    "accent=0xFFFF\ndanger=0xC618\nsuccess=0xFFFF\nspacing=8\nradius=0\n"
    THEME_LAYOUT_TAIL;

static const char k_theme_classic[] =
    "# ReChord Classic (dark slate)\nname=Classic\n"
    "background=0x1083\nsurface=0x2146\ntext=0xEF7E\ntext_dim=0x94B4\n"
    "accent=0x44FF\ndanger=0xFA8A\nsuccess=0x56AF\nspacing=8\nradius=6\n"
    THEME_LAYOUT_TAIL;

static const char k_theme_paper[] =
    "# ReChord Paper (warm light)\nname=Paper\n"
    "background=0xF79C\nsurface=0xFFFE\ntext=0x3144\ntext_dim=0x7B8C\n"
    "accent=0x9AC2\ndanger=0xB145\nsuccess=0x2BE8\nspacing=10\nradius=2\n"
    THEME_LAYOUT_TAIL;

static const char k_theme_night[] =
    "# ReChord Night (dim)\nname=Night\n"
    "background=0x0841\nsurface=0x10A3\ntext=0xAD56\ntext_dim=0x630D\n"
    "accent=0x23B6\ndanger=0xB2AA\nsuccess=0x44AC\nspacing=8\nradius=6\n"
    THEME_LAYOUT_TAIL;

static const struct
{
    const char *dir;
    const char *path;
    const char *text;
} k_theme_seeds[] = {
    { RECHORD_THEMES_DIR "/mono",    RECHORD_THEMES_DIR "/mono/theme.cfg",
      k_theme_mono },
    { RECHORD_THEMES_DIR "/classic", RECHORD_THEMES_DIR "/classic/theme.cfg",
      k_theme_classic },
    { RECHORD_THEMES_DIR "/paper",   RECHORD_THEMES_DIR "/paper/theme.cfg",
      k_theme_paper },
    { RECHORD_THEMES_DIR "/night",   RECHORD_THEMES_DIR "/night/theme.cfg",
      k_theme_night },
};

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
    if (ensure_file(MEDIA_DB_B, 0u, 0) != FS_OK)
        return FS_ERR_IO;

    /* Standard theme packages + the active manifest (BRNOMT-69). The FS create
     * and mkdir paths are the same ones that make /.rechord and the cfg, so no
     * pre-existing file is involved. Best effort: a theme that cannot be
     * created (full volume, exFAT refusal) must never fail the boot. */
    if (fs_target_mkdir(RECHORD_THEMES_DIR) == FS_OK) {
        unsigned i;

        for (i = 0; i < (unsigned)(sizeof k_theme_seeds /
                                   sizeof k_theme_seeds[0]); i++) {
            if (fs_target_mkdir(k_theme_seeds[i].dir) == FS_OK)
                (void)ensure_file(k_theme_seeds[i].path, 1024u,
                                  k_theme_seeds[i].text);
        }
    }
    /* Active manifest starts as Mono (theme_boot_load reads THEME_FILE_PATH,
     * the same path). Never overwritten once the user edits it. */
    (void)ensure_file("/Internal/.rechord/theme.cfg", 1024u, k_theme_mono);
    return FS_OK;
}
