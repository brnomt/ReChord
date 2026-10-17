/*
 * rechord_files.h — the device's own files on the internal volume.
 *
 * See rechord_files.c. Called once at boot, after the volume is mounted and
 * its writer is installed; safe to call on every boot.
 */
#ifndef RECHORD_SERVICES_RECHORD_FILES_H
#define RECHORD_SERVICES_RECHORD_FILES_H

#define RECHORD_DIR     "/Internal/.rechord"
#define RECHORD_CFG     "/Internal/.rechord/rechord.cfg"
#define RECHORD_STATE   "/Internal/.rechord/state"

/* First-boot layout: create /.rechord, rechord.cfg (with the default
 * configuration written) and state when they are missing; existing files are
 * left exactly as they are. Returns 0, or an FS_ERR_* code when the volume is
 * missing or the creation failed. */
int rechord_files_init(void);

#endif /* RECHORD_SERVICES_RECHORD_FILES_H */
