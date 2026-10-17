/*
 * bfs.h — A-side file service for Core B (BRNOMT-68 phase 3).
 *
 * The AP owns the filesystem; B's codec_io reads through a shared SPSC ring
 * (firmware/player/ipc_shared.h). A opens a path, then bfs_poll() keeps the
 * ring filled AHEAD of B's read cursor (read-ahead), so a normal read does not
 * round-trip. A seek request from B repositions the file and resets the ring.
 *
 * bfs_poll() must be called regularly from the AP loop while a file is open
 * (it is a no-op otherwise). Only one file is served at a time.
 */
#ifndef RECHORD_BFS_H
#define RECHORD_BFS_H

/* Open `path` for B and publish it. 0 on success, < 0 on I/O error. */
int  bfs_open(const char *path);

/* Service seeks and refill the ring ahead of B. Call every AP loop pass. */
void bfs_poll(void);

/* Stop serving and close the file (idempotent). */
void bfs_close(void);

/* 1 while a file is open. */
int  bfs_is_open(void);

#endif /* RECHORD_BFS_H */
