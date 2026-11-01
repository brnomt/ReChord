/*
 * library_extras.h — favorites, recently played and play counts.
 *
 * Stored as 32-bit FNV-1a hashes of the full device path ("/SD/...") instead
 * of the paths themselves: 32 favorites + 32 recent entries cost ~450 B of
 * RAM instead of ~13 KB, survive an index rebuild, and the files stay under
 * 1 KB so a track start writes only a few hundred bytes.
 *
 * Files (created at first boot, rewritten through fs_store_file):
 *   /.rechord/favorites   one 8-hex hash per line
 *   /.rechord/recent      "<8-hex hash> <plays>" per line, most recent first
 *
 * A hash that no longer matches any indexed record (file deleted, card
 * removed) is simply skipped by the views.
 */
#ifndef RECHORD_SERVICES_LIBRARY_EXTRAS_H
#define RECHORD_SERVICES_LIBRARY_EXTRAS_H

#include <stdint.h>

#define EXTRAS_FAV_MAX    48
#define EXTRAS_RECENT_MAX 32

/* Load both files (idempotent; harmless when they are missing). */
void extras_init(void);
int  extras_loaded(void);

/* Hash a full device path ("/SD/Album/track.flac"). */
uint32_t extras_path_hash(const char *path);

/* Favorites. */
int      extras_is_favorite(const char *path);
int      extras_is_favorite_hash(uint32_t hash);
int      extras_favorite_slot(uint32_t hash);   /* index, or -1 */
int      extras_toggle_favorite(const char *path); /* 1 added, 0 removed, -1 */
unsigned extras_favorite_count(void);
uint32_t extras_favorite_hash(unsigned slot);

/* Recently played / play counts. */
void     extras_note_play(const char *path);    /* MRU + count, saves */
int      extras_recent_slot(uint32_t hash);     /* index, or -1 */
unsigned extras_recent_count(void);
uint32_t extras_recent_hash(unsigned slot);
unsigned extras_recent_plays(unsigned slot);
unsigned extras_plays_for_hash(uint32_t hash);

/* Slots ordered by play count (desc), recency breaking ties. Fills `order`
 * with up to `cap` slot indices and returns the count. */
unsigned extras_most_played_order(unsigned char *order, unsigned cap);

#endif /* RECHORD_SERVICES_LIBRARY_EXTRAS_H */
