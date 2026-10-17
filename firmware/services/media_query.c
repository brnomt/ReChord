/*
 * media_query.c — category queries over the media index (see media_query.h).
 *
 * Both passes stream the index once (media_db_get) and accumulate into the
 * shared scratch region:
 *   names  -> fixed 24-byte cells, deduplicated, sorted, "Undefined" first
 *   filter -> uint32_t record indices
 * The two modes are mutually exclusive, which is why they can share the region.
 */
#include <stdio.h>
#include <string.h>

#include "media_db.h"
#include "fs.h"
#include "media_query.h"
#include "scratch.h"

#define MQ_CELL 24u
#define MQ_UNDEFINED "Undefined"

/* Scratch views (the region is reinterpreted, never copied). */
static char    *s_cells;                 /* names mode: count * MQ_CELL */
static uint32_t *s_idx;                  /* filter mode: record indices */
static unsigned s_cap;                   /* capacity in the current mode */
static unsigned s_count;
static int      s_truncated;
static void   (*s_pump)(void);           /* optional player_process() pump */

#define MQ_PUMP_EVERY 32u

void mq_set_pump(void (*fn)(void))
{
    s_pump = fn;
}

const char *mq_category_name(int cat)
{
    switch (cat) {
    case MQ_ALL:          return "All Songs";
    case MQ_ALBUM:        return "Album";
    case MQ_ARTIST:       return "Artist";
    case MQ_GENRE:        return "Genre";
    case MQ_YEAR:         return "Year";
    case MQ_LABEL:        return "Label";
    case MQ_ALBUM_ARTIST: return "Album Artist";
    case MQ_COMPOSER:     return "Composer";
    default:              return "?";
    }
}

/* The value of `rec` in `cat`, as text. A year of 0 is "Undefined"; the scan
 * already stored "Undefined" for unusable text fields. */
static const char *rec_value(const media_rec_t *rec, int cat, char *buf,
                             size_t cap)
{
    switch (cat) {
    case MQ_ALBUM:        return rec->album;
    case MQ_ARTIST:       return rec->artist;
    case MQ_GENRE:        return rec->genre;
    case MQ_LABEL:        return rec->label;
    case MQ_ALBUM_ARTIST: return rec->album_artist;
    case MQ_COMPOSER:     return rec->composer;
    case MQ_YEAR:
        if (rec->year == 0u)
            return MQ_UNDEFINED;
        snprintf(buf, cap, "%u", (unsigned)rec->year);
        return buf;
    default:
        return MQ_UNDEFINED;
    }
}

/* "Undefined" first, then case-sensitive strcmp: deterministic and cheap. */
static int name_cmp(const char *a, const char *b)
{
    int ua = strcmp(a, MQ_UNDEFINED) == 0;
    int ub = strcmp(b, MQ_UNDEFINED) == 0;

    if (ua != ub)
        return ua ? -1 : 1;
    return strcmp(a, b);
}

static void sort_names(void)
{
    /* Insertion sort: the list is bounded by the scratch (~1.3k cells) and
     * this runs once per screen enter, not per frame. */
    unsigned i, j;

    for (i = 1; i < s_count; i++) {
        char tmp[MQ_CELL];

        memcpy(tmp, s_cells + i * MQ_CELL, MQ_CELL);
        j = i;
        while (j > 0 && name_cmp(s_cells + (j - 1) * MQ_CELL, tmp) > 0) {
            memcpy(s_cells + j * MQ_CELL, s_cells + (j - 1) * MQ_CELL, MQ_CELL);
            j--;
        }
        memcpy(s_cells + j * MQ_CELL, tmp, MQ_CELL);
    }
}

int mq_names_init(int cat)
{
    unsigned total, i;

    mq_done();
    s_cells = scratch_claim(SCRATCH_OWNER_INDEX);
    if (s_cells == 0)
        return FS_ERR_IO;
    /* The module owns the index lifetime: without this the screens would have
     * to remember media_db_open(), and forgetting it reads as "empty index". */
    if (media_db_open() != FS_OK) {
        mq_done();
        return FS_ERR_OPEN;
    }
    s_cap = SCRATCH_BYTES / MQ_CELL;
    s_count = 0;
    s_truncated = 0;
    total = media_db_count();
    for (i = 0; i < total; i++) {
        media_rec_t rec;
        char ybuf[8];
        const char *v;
        unsigned k;

        if (media_db_get(i, &rec) != FS_OK)
            break;
        if (s_pump != 0 && (i & (MQ_PUMP_EVERY - 1u)) == 0u)
            s_pump();                  /* keep the PCM ring fed during scan */
        v = rec_value(&rec, cat, ybuf, sizeof ybuf);
        for (k = 0; k < s_count; k++)
            if (strcmp(s_cells + k * MQ_CELL, v) == 0)
                break;
        if (k < s_count)
            continue;                  /* already have it */
        if (s_count >= s_cap) {
            s_truncated = 1;
            break;
        }
        memset(s_cells + s_count * MQ_CELL, 0, MQ_CELL);
        {
            size_t n = strlen(v);

            if (n > MQ_CELL - 1u)
                n = MQ_CELL - 1u;
            memcpy(s_cells + s_count * MQ_CELL, v, n);
        }
        s_count++;
    }
    sort_names();
    return 0;
}

unsigned mq_names_count(void)
{
    return s_count;
}

const char *mq_names_get(unsigned i)
{
    if (s_cells == 0 || i >= s_count)
        return 0;
    return s_cells + i * MQ_CELL;
}

int mq_filter_init(int cat, const char *value)
{
    unsigned total, i;

    mq_done();
    s_idx = scratch_claim(SCRATCH_OWNER_INDEX);
    if (s_idx == 0)
        return FS_ERR_IO;
    if (media_db_open() != FS_OK) {
        mq_done();
        return FS_ERR_OPEN;
    }
    s_cap = SCRATCH_BYTES / sizeof s_idx[0];
    s_count = 0;
    s_truncated = 0;
    total = media_db_count();
    for (i = 0; i < total; i++) {
        media_rec_t rec;
        char ybuf[8];
        int match;

        if (media_db_get(i, &rec) != FS_OK)
            break;
        if (s_pump != 0 && (i & (MQ_PUMP_EVERY - 1u)) == 0u)
            s_pump();                  /* keep the PCM ring fed during scan */
        if (cat == MQ_ALL || value == 0) {
            match = 1;
        } else {
            const char *v = rec_value(&rec, cat, ybuf, sizeof ybuf);

            match = strcmp(v, value) == 0;
        }
        if (!match)
            continue;
        if (s_count >= s_cap) {
            s_truncated = 1;
            break;
        }
        s_idx[s_count++] = i;
    }
    return 0;
}

unsigned mq_filter_count(void)
{
    return s_count;
}

int mq_filter_get(unsigned i, media_rec_t *out)
{
    if (s_idx == 0 || i >= s_count)
        return FS_ERR_ARG;
    return media_db_get(s_idx[i], out);
}

void mq_done(void)
{
    media_db_close();
    if (s_cells != 0 || s_idx != 0) {
        scratch_release(SCRATCH_OWNER_INDEX);
        s_cells = 0;
        s_idx = 0;
    }
    s_count = 0;
    s_cap = 0;
    s_truncated = 0;
}
