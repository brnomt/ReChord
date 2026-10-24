/*
 * settings.h — ReChord settings service: the key=value "\RECHORD.CFG" schema.
 *
 * Format (drop-in compatible with the documented on-device config format):
 *   - plain text, one "key=value" per line,
 *   - lines starting with '#' are comments, blank lines are ignored,
 *   - unknown keys are ignored on load (forward compatibility),
 *   - CRLF and LF line endings both accepted.
 *
 * The keys (fixed schema, stable order on save):
 *   play, brightness, screen_off, auto_off, usb, resume,
 *   strip, tags, cpu, volume, browse, cursor, shuffle, repeat_mode,
 *   fade, theme, replaygain
 *
 * API contract (other modules code against exactly this):
 *   settings_load   — read and parse \RECHORD.CFG into a settings_t
 *   settings_save   — serialize a settings_t back to \RECHORD.CFG
 *   settings_defaults — fill sane default values
 */
#ifndef RECHORD_SERVICES_SETTINGS_H
#define RECHORD_SERVICES_SETTINGS_H

typedef struct { char play[16]; int brightness; int screen_off_min; int auto_off_min; char usb[8]; char resume[8]; char strip[8]; char tags[8]; char cpu[8]; int volume; char browse[64]; int cursor; char shuffle[8]; char repeat_mode[8]; char fade[8]; char theme[32]; char replaygain[8]; } settings_t;

/* Config file location: the internal eMMC volume, in the user-visible
 * .rechord folder (BRNOMT-71). Plain text, so it can be edited/copied from a
 * PC; on the target it is rewritten IN PLACE (no FAT-table writes) by
 * settings_store.c. Override at compile time if a build needs another name. */
#ifndef SETTINGS_PATH
#define SETTINGS_PATH "/Internal/.rechord/rechord.cfg"
#endif

/* Return codes. */
#define SETTINGS_OK          0
#define SETTINGS_ERR_ARG   (-1)  /* NULL pointer argument */
#define SETTINGS_ERR_OPEN  (-2)  /* config file missing/unopenable; defaults applied */
#define SETTINGS_ERR_IO    (-3)  /* read or write failure */
#define SETTINGS_ERR_FULL  (-4)  /* serialization buffer too small */

/* Read SETTINGS_PATH and fill *s. Unknown keys and malformed lines are
 * ignored. On any failure *s still holds defaults, so callers can always use
 * the struct. Returns SETTINGS_OK or an error code. */
int settings_load(settings_t *s);

/* Write *s to SETTINGS_PATH (create/truncate), all keys in schema order with a
 * short comment header. Note: keys not in the schema are not preserved (the
 * struct cannot hold them). Returns SETTINGS_OK or an error code. */
int settings_save(const settings_t *s);

/* Fill *s with default values (the documented sample configuration). */
void settings_defaults(settings_t *s);

/* Helpers (pure, no I/O) — used by the UI and by host tests. */
int settings_parse_text(const char *text, settings_t *s);      /* 0 or ERR */
int settings_serialize_text(const settings_t *s, char *buf,
                            unsigned long cap);                /* length or ERR */

#endif /* RECHORD_SERVICES_SETTINGS_H */
