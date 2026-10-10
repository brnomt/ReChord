/*
 * fs_host.h — host-only extension of the fs abstraction (tests, tools).
 *
 * The target build never includes this; device code uses fs.h only.
 */
#ifndef RECHORD_SERVICES_FS_HOST_H
#define RECHORD_SERVICES_FS_HOST_H

/* Set/get the host root directory that device-style paths ("\\RECHORD.CFG")
 * resolve under. Defaults to ".". A NULL/empty dir resets to ".". */
void fs_host_set_root(const char *dir);
const char *fs_host_get_root(void);

#endif /* RECHORD_SERVICES_FS_HOST_H */
