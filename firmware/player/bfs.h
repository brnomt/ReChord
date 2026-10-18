/*
 * bfs.h — A-side file service for Core B (BRNOMT-68 phase 3).
 *
 * The AP owns the filesystem; B's codec_io reads through a shared SPSC ring
 * (firmware/player/ipc_shared.h). A opens a path, then bfs_poll() keeps the
 * ring filled AHEAD of B's read cursor (read-ahead), so a normal read does not
 * round-trip. A seek request from B repositions the file and resets the ring.
 *
 * bfs_poll() must be called regularly from the AP loop while a file is open
 * (it is a no-op otherwise). Two rings exist (BRNOMT-73): the current file and
 * a pre-opened next file, so the gapless boundary does not wait on the SD.
 */
#ifndef RECHORD_BFS_H
#define RECHORD_BFS_H

/* Open `path` for B and publish it (ring 0, discards any pre-open).
 * 0 on success, < 0 on I/O error. */
int  bfs_open(const char *path);

/* Service seeks and refill the rings ahead of B. Call every AP loop pass. */
void bfs_poll(void);

/* Stop serving and close both files (idempotent). */
void bfs_close(void);

/* 1 while a file is open. */
int  bfs_is_open(void);

/* Pre-open the NEXT track into the other ring while the current one plays, so
 * the boundary does not pay the SD open. Idempotent for the same path.
 * Returns 0, or < 0 on I/O error. */
int  bfs_preopen(const char *path);

/* 1 when a pre-open exists AND it is exactly `path` — the caller must not
 * promote a stale pre-open (the list/queue can change mid-track). */
int  bfs_preopen_ready(const char *path);

/* Promote the pre-opened ring to current (the previous file is closed).
 * Returns the ring index B must read (IPC_C_RING), or -1 if nothing is
 * pre-opened. */
int  bfs_promote(void);

#endif /* RECHORD_BFS_H */
