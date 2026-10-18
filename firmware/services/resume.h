/*
 * resume.h — playback/browse resume state (BRNOMT-87).
 *
 * The device keeps its runtime state in RECHORD_STATE ("/.rechord/state"), a
 * plain-text key=value file created at first boot. This module owns that file:
 * the format, the parser and the in-place rewrite.
 *
 * It is deliberately SEPARATE from rechord.cfg: config is what the user chose,
 * state is where the player happened to be. The write path is the same one
 * proven for the config (fs_target_file_rewrite on an already-allocated file:
 * data sectors + the dirent size, no allocation, no FAT writes), so a power cut
 * mid-save loses that one save and nothing else.
 *
 * Keys: track=<device path>, pos_ms=<n>, folder=<browser path>, row=<n>.
 */
#ifndef RECHORD_SERVICES_RESUME_H
#define RECHORD_SERVICES_RESUME_H

/* Device paths, leading '/', same form the browser and the player use. */
#define RESUME_TRACK_MAX  208
#define RESUME_FOLDER_MAX 208

typedef struct
{
    char          track[RESUME_TRACK_MAX];
    unsigned long pos_ms;
    char          folder[RESUME_FOLDER_MAX];
    int           row;
} resume_state_t;

/* Zero the struct (empty track/folder, position 0, row 0). */
void resume_default(resume_state_t *s);

/* Read RECHORD_STATE into `s`. Unknown/rotten lines are ignored, so a torn
 * write only costs the lost values. Returns 0, or < 0 when the file is
 * missing/unreadable (the struct is left at its defaults). */
int resume_load(resume_state_t *s);

/* Serialize and rewrite RECHORD_STATE in place. Returns 0, or < 0 when the
 * write failed (missing file / read-only volume / exFAT); the previous
 * contents are left intact in that case. */
int resume_save(const resume_state_t *s);

#endif /* RECHORD_SERVICES_RESUME_H */
