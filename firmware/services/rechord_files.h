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
#define RECHORD_LOG     "/Internal/.rechord/LOG"
/* Installable theme packages (BRNOMT-69): materialized on first boot. */
#define RECHORD_THEMES_DIR "/Internal/.rechord/themes"
/* Active package id (directory name under RECHORD_THEMES_DIR). Rewritten in
 * place when the user picks a theme in Settings. */
#define RECHORD_ACTIVE_THEME "/Internal/.rechord/active_theme"

/* First-boot layout: create /.rechord, rechord.cfg (with the default
 * configuration written), state and LOG (each with a short text header, so
 * they read as text rather than as blocks of zeros) when they are missing;
 * existing files are left exactly as they are. Returns 0, or an FS_ERR_* code
 * when the volume is missing or the creation failed. */
int rechord_files_init(void);

#endif /* RECHORD_SERVICES_RECHORD_FILES_H */
