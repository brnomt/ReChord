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

/* Release the scratch region and drop the current query. */
void mq_done(void);

#endif /* RECHORD_SERVICES_MEDIA_QUERY_H */
