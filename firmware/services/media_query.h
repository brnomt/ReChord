/*
 * media_query.h — the category queries the library menu needs (BRNOMT-78).
 *
 * WHY a separate module: the screens must stay thin (draw + keys) and the
 * interesting part -- "which unique album names does this index have" and
 * "which records match this value" -- is testable on the host without a panel.
 *
 * RAM: both operations work out of the SHARED SCRATCH region (services/
 * scratch.h), never out of statics, because the AP has less than a kilobyte of
 * .bss margin. One query is live at a time; mq_done() releases the region.
 */
#ifndef RECHORD_SERVICES_MEDIA_QUERY_H
#define RECHORD_SERVICES_MEDIA_QUERY_H

#include "media_db.h"

/* Category ids, in the order the Library menu shows them. */
enum {
    MQ_ALL = 0,
    MQ_ALBUM,
    MQ_ARTIST,
    MQ_GENRE,
    MQ_YEAR,
    MQ_LABEL,
    MQ_ALBUM_ARTIST,
    MQ_COMPOSER,
    MQ_COUNT
};

/* Menu label for a category ("All Songs", "Album", ...). */
const char *mq_category_name(int cat);

/* Unique, sorted values of a category over the open index. "Undefined" sorts
 * first (the user's rule) and is the only value an unusable field produces.
 * Returns 0, or FS_ERR_* (no index / scratch busy). */
int         mq_names_init(int cat);
unsigned    mq_names_count(void);
const char *mq_names_get(unsigned i);

/* Records matching cat == value; value == 0 means the whole category (All
 * Songs). The matching record indices are collected once, so paging over them
 * never re-reads the index. */
int      mq_filter_init(int cat, const char *value);
unsigned mq_filter_count(void);
int      mq_filter_get(unsigned i, media_rec_t *out);

/* The record indices collected by mq_filter_init(), for a playback list
 * (BRNOMT-90). Returned pointer is valid until mq_done(); NULL when empty. */
const uint16_t *mq_filter_indices(unsigned *out_count);

/* BRNOMT-90: collect every indexed track under `prefix` (a browser folder
 * path such as "/SD/Music/Album"). Results come back through
 * mq_filter_indices(). Returns 0, or FS_ERR_* (no index / scratch busy). */
int mq_folder_init(const char *prefix);

/* Same as mq_folder_init(), but also reports the SORTED filter position of the
 * record whose full device path ("/"+rec.path) equals `target`, in
 * *start_out (-1 when the target is not in the folder or target is NULL).
 * The lookup happens during the same index scan, so the browser no longer
 * re-reads every record just to find the selected file (BRNOMT-98). */
int mq_folder_init_start(const char *prefix, const char *target,
                         int *start_out);

/* Release the scratch region and drop the current query. */
void mq_done(void);

/* Optional pump called every few index records while a query scans the index.
 * The scan is a long blocking loop of media_db_get() calls; without pumping,
 * the player stops refilling the PCM ring and an underrun cuts the audio (at
 * 96 kHz the ring is only ~85 ms). The caller sets this to player_process(). */
void mq_set_pump(void (*fn)(void));

#endif /* RECHORD_SERVICES_MEDIA_QUERY_H */
