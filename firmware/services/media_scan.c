/*
 * media_scan.c — the scan behind the media index (BRNOMT-74).
 *
 * Walks the mounted volumes page by page (fs_list_dir), takes audio files by
 * extension, reads the container tags through the player's tag reader (which
 * handles the container detection and bounds the work to the header region)
 * and feeds one fixed record per file to media_db. The title falls back to the
 * filename, so a library without tags still browses.
 *
 * The scan is deliberately frame-based and **iterative**, not recursive: an
 * explicit traversal stack lives in the shared scratch region (no .bss, no
 * per-level AP stack frame), the depth is capped at 8, paths that would not fit
 * the record are skipped (a truncated path is useless as a key), and the
 * progress callback gets a chance to abort between files. An aborted or failed
 * scan commits nothing: media_db targets the slot that is not current, so the
 * previous index survives.
 */
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "media_db.h"
#include "media_scan.h"
#include "scratch.h"
#include "log.h"

#include "../player/codec.h"
#include "../player/tags.h"

/* Depth cap (levels below a volume root) and the explicit traversal stack. */
#define SCAN_DEPTH_MAX 8
#define SCAN_FRAMES    (SCAN_DEPTH_MAX + 1u)

typedef struct {
    uint32_t index;                    /* next entry to list at this level  */
    char     rel[MEDIA_PATH_MAX];      /* volume-relative dir ("SD/A/B")    */
} scan_frame_t;

/* Carved out of the shared scratch region together with media_db's staging
 * buffer: [stack | staging]. One bounded buffer instead of one AP stack frame
 * per nested directory (BRNOMT-92). */
#define SCAN_STACK_BYTES ((uint32_t)(sizeof(scan_frame_t) * SCAN_FRAMES))

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

/* Iterative, depth-first walk of one volume. `stack` is the caller's frame
 * array (SCAN_FRAMES deep); `root_rel` is the volume name ("Internal", "SD").
 * The device path is "/" + rel, so a frame only stores the relative path.
 * One reused fs_dirent_t, one bounded buffer, no recursion -> the walk costs
 * a constant amount of the AP stack regardless of how deep the tree is. */
static int walk_volume(const char *root_rel, scan_frame_t *stack)
{
    fs_dirent_t e;
    int sp = 1;

    copy_field(stack[0].rel, sizeof stack[0].rel, root_rel);
    stack[0].index = 0;

    while (sp > 0) {
        scan_frame_t *fr = &stack[sp - 1];
        char dev[MEDIA_PATH_MAX + 16u];
        char sub_rel[MEDIA_PATH_MAX];
        uint32_t count = 1, total = 0;

        if (snprintf(dev, sizeof dev, "/%s", fr->rel) >= (int)sizeof dev) {
            sp--;                          /* cannot address it: skip */
            continue;
        }
        if (fs_list_dir(dev, fr->index, &e, 1u, &count, &total) != FS_OK ||
            count == 0) {
            sp--;                          /* directory exhausted */
            continue;
        }
        fr->index++;                       /* consume this entry */

        if (snprintf(sub_rel, sizeof sub_rel, "%s/%s", fr->rel, e.name)
                >= (int)sizeof sub_rel)
            continue;                      /* too long to be a usable key */

        if (e.is_dir) {
            if ((uint32_t)sp >= SCAN_FRAMES)
                continue;                  /* depth cap: ignore deeper dirs */
            copy_field(stack[sp].rel, sizeof stack[sp].rel, sub_rel);
            stack[sp].index = 0;
            sp++;
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
    uint8_t      *region;
    scan_frame_t *stack;
    uint8_t      *dbuf;
    uint32_t      dbuf_bytes;
    int r;

    s_progress = progress;
    s_files = 0;
    s_aborted = 0;

    /* One claim of the shared region: [traversal stack | record staging]. The
     * stack lives here (not in .bss) so the walk costs no static RAM. */
    region = (uint8_t *)scratch_claim(SCRATCH_OWNER_INDEX);
    if (region == 0)
        return FS_ERR_IO;
    stack = (scan_frame_t *)region;
    dbuf = region + SCAN_STACK_BYTES;
    dbuf_bytes = SCRATCH_BYTES - SCAN_STACK_BYTES;

    r = media_db_begin(dbuf, dbuf_bytes);
    log_printf("index: begin rc %d", r);
    if (r != FS_OK) {
        scratch_release(SCRATCH_OWNER_INDEX);
        return r;
    }
    r = walk_volume("Internal", stack);
    if (r == FS_OK)
        r = walk_volume("SD", stack);
    log_printf("index: scan rc %d, %lu files", r, (unsigned long)s_files);
    if (r != FS_OK) {
        media_db_abort();
        scratch_release(SCRATCH_OWNER_INDEX);
        return r;
    }
    r = media_db_end();
    scratch_release(SCRATCH_OWNER_INDEX);
    log_printf("index: end rc %d", r);
    return r;
}
