/*
 * media_scan.h — one full scan that builds the media index (BRNOMT-74).
 *
 * Browsing must not walk the FAT on every navigation, so this runs once (from
 * the "Rebuild index" menu entry) and writes the records through media_db:
 * every audio file on the mounted volumes, tags from the container when they
 * parse, filename as the title fallback.
 */
#ifndef RECHORD_SERVICES_MEDIA_SCAN_H
#define RECHORD_SERVICES_MEDIA_SCAN_H

#include <stdint.h>

/* Called every few files with the count so far and the name being handled.
 * Return nonzero to abort the scan. */
typedef int (*media_scan_progress_t)(uint32_t files, const char *name);

/* Returned when the progress callback asked to stop. */
#define MEDIA_SCAN_ABORTED (-100)

/* Scan and commit the index. Returns 0 when the new index is live,
 * MEDIA_SCAN_ABORTED when stopped (the previous index stays current), or a
 * negative FS_ERR_* code. */
int media_scan_build(media_scan_progress_t progress);

#endif /* RECHORD_SERVICES_MEDIA_SCAN_H */
