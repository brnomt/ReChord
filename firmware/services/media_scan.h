/*
 * media_scan.h — build/update the media index (BRNOMT-74).
 *
 * Browsing must not walk the FAT on every navigation, so this runs when the
 * user asks (System menu) and writes the records through media_db: every audio
 * file on the mounted volumes, tags from the container when they parse,
 * filename as the title fallback.
 *
 * media_scan_update() is the fast path: it compares each directory entry's
 * (path, size, mtime) with the current index and copies unchanged records
 * forward without opening the file. Only new/changed files are tag-read.
 * media_scan_build() always re-reads every file (the repair path).
 */
#ifndef RECHORD_SERVICES_MEDIA_SCAN_H
#define RECHORD_SERVICES_MEDIA_SCAN_H

#include <stdint.h>

/* Called every few files with the count so far and the name being handled.
 * Return nonzero to abort the scan. */
typedef int (*media_scan_progress_t)(uint32_t files, const char *name);

/* Returned when the progress callback asked to stop. */
#define MEDIA_SCAN_ABORTED (-100)

/* Full scan: read every audio file's tags and commit a new index. Returns 0
 * when the new index is live, MEDIA_SCAN_ABORTED when stopped (the previous
 * index stays current), or a negative FS_ERR_* code. */
int media_scan_build(media_scan_progress_t progress);

/* Incremental scan: carry unchanged records forward and tag-read only new or
 * changed files. Falls back to a full scan when no index exists or when the
 * library is too large for the scratch hash table. Same return codes. */
int media_scan_update(media_scan_progress_t progress);

#endif /* RECHORD_SERVICES_MEDIA_SCAN_H */
