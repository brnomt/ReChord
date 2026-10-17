/*
 * media_scan.c — the scan behind the media index (BRNOMT-74).
 *
 * Walks the mounted volumes page by page (fs_list_dir), takes audio files by
 * extension, reads the container tags through the player's tag reader (which
 * handles the container detection and bounds the work to the header region)
 * and feeds one fixed record per file to media_db. The title falls back to the
 * filename, so a library without tags still browses.
 *
 * The scan is deliberately frame-based, not recursive-descent-into-everything:
 * depth is capped, paths that would not fit the record are skipped (a truncated
 * path is useless as a key), and the progress callback gets a chance to abort
 * between files. An aborted or failed scan commits nothing: media_db targets
 * the slot that is not current, so the previous index survives.
 */
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "media_db.h"
#include "media_scan.h"
#include "log.h"

#include "../player/codec.h"
#include "../player/tags.h"

#define SCAN_DEPTH_MAX 8

static media_scan_progress_t s_progress;
static uint32_t s_files;
static int      s_aborted;

static int is_audio(const char *name)
{
    static const char *const k_ext[] = { ".mp3", ".flac", ".wav" };
    size_t n = strlen(name), k;

    for (k = 0; k < sizeof k_ext / sizeof k_ext[0]; k++) {
        size_t m = strlen(k_ext[k]), i;
        char a;

        if (n <= m)
            continue;
        for (i = 0; i < m; i++) {
            a = name[n - m + i];
            if (a >= 'A' && a <= 'Z')
                a = (char)(a + 32);
            if (a != k_ext[k][i])
                break;
        }
        if (i == m)
            return 1;
    }
    return 0;
}

/* Bounded copy: truncation is intended here (a browse index has small
 * fields), spelled so the compiler does not mistake it for a bug. */
static void copy_field(char *dst, size_t cap, const char *src)
{
    size_t n = 0;

    while (n + 1u < cap && src[n] != '\0')
        n++;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static void base_name(const char *path, char *out, size_t cap)
{
    const char *p = path, *b = path;

    while (*p != '\0') {
        if (*p == '/')
            b = p + 1;
        p++;
    }
    copy_field(out, cap, b);
}

/* Filename without its extension: the title fallback reads better as
 * "01 Something" than "01 Something.flac". */
static void base_title(const char *name, char *out, size_t cap)
{
    size_t n = strlen(name), i;

    for (i = n; i > 0; i--)
        if (name[i - 1u] == '.') {
            n = i - 1u;
            break;
        }
    if (n >= cap)
        n = cap - 1u;
    memcpy(out, name, n);
    out[n] = '\0';
}

/* Category value normalisation (the user's rule): a field that cannot be shown
 * as-is -- empty, spaces only, or carrying control characters -- becomes
 * "Undefined". The views group on that string, so there is no blank row and no
 * garbage, and the sort can put it first. */
static void norm_field(char *dst, size_t cap, const char *src)
{
    size_t i = 0, n = 0;

    if (src != 0) {
        while (src[i] == ' ' || src[i] == '\t')
            i++;
        while (src[i] != '\0' && n + 1u < cap) {
            unsigned char c = (unsigned char)src[i++];

            if (c < 0x20u)
                break;
            dst[n++] = (char)c;
        }
        while (n > 0 && (dst[n - 1u] == ' ' || dst[n - 1u] == '\t'))
            n--;
    }
    if (n == 0u) {
        copy_field(dst, cap, "Undefined");
        return;
    }
    dst[n] = '\0';
}

static int add_file(const char *rel)
{
    media_rec_t r;
    codec_meta_t meta;
    char full[MEDIA_PATH_MAX + 2];
    char base[40];

    full[0] = '/';
    copy_field(full + 1, sizeof full - 1u, rel);
    memset(&meta, 0, sizeof meta);
    (void)tags_read_file(full, &meta, 0);  /* 0 = no tags: filename fallback */

    memset(&r, 0, sizeof r);
    copy_field(r.path, sizeof r.path, rel);
    base_name(rel, base, sizeof base);
    if (meta.title[0] == '\0')
        base_title(base, base, sizeof base);
    copy_field(r.title, sizeof r.title, meta.title[0] ? meta.title : base);
    norm_field(r.artist, sizeof r.artist, meta.artist);
    norm_field(r.album, sizeof r.album, meta.album);
    norm_field(r.album_artist, sizeof r.album_artist, meta.album_artist);
    norm_field(r.composer, sizeof r.composer, meta.composer);
    norm_field(r.genre, sizeof r.genre, meta.genre);
    norm_field(r.label, sizeof r.label, meta.label);
    r.track_no = (uint16_t)(meta.track > 0 ? meta.track : 0);
    r.year = (uint16_t)(meta.year >= 1000 && meta.year <= 9999 ? meta.year : 0);
    return media_db_add(&r);
}

static int walk(const char *dev_path, const char *rel_path, int depth)
{
    fs_dirent_t e;
    uint32_t i, count = 1, total = 0;

    if (depth > SCAN_DEPTH_MAX)
        return FS_OK;
    for (i = 0; fs_list_dir(dev_path, i, &e, 1u, &count, &total) == FS_OK && count; i++) {
        char sub_dev[MEDIA_PATH_MAX + 16u], sub_rel[MEDIA_PATH_MAX];

        if (snprintf(sub_dev, sizeof sub_dev, "%s/%s", dev_path, e.name)
                >= (int)sizeof sub_dev)
            continue;
        if (snprintf(sub_rel, sizeof sub_rel, "%s/%s", rel_path, e.name)
                >= (int)sizeof sub_rel)
            continue;                      /* too long to be a usable key */
        if (e.is_dir) {
            int r = walk(sub_dev, sub_rel, depth + 1);

            if (r != FS_OK)
                return r;
        } else if (is_audio(e.name)) {
            if (add_file(sub_rel) != FS_OK)
                return FS_ERR_IO;
            s_files++;
            if (s_progress != 0 && (s_files & 7u) == 0u &&
                s_progress(s_files, e.name)) {
                s_aborted = 1;
                return MEDIA_SCAN_ABORTED;
            }
        }
    }
    return FS_OK;
}

int media_scan_build(media_scan_progress_t progress)
{
    int r;

    s_progress = progress;
    s_files = 0;
    s_aborted = 0;
    r = media_db_begin();
    log_printf("index: begin rc %d", r);
    if (r != FS_OK)
        return r;
    r = walk("/Internal", "Internal", 0);
    if (r == FS_OK)
        r = walk("/SD", "SD", 0);
    log_printf("index: scan rc %d, %lu files", r, (unsigned long)s_files);
    if (r != FS_OK) {
        media_db_abort();
        return r;
    }
    r = media_db_end();
    log_printf("index: end rc %d", r);
    return r;
}
