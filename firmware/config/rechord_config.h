/*
 * rechord_config.h — Rockbox-style persistent configuration for ReChord.
 *
 * WHY a second config module: services/settings.c holds the 12-key schema
 * that is drop-in compatible with the STOCK on-device "\RECHORD.CFG". This
 * module is the ReChord-native schema (audio/DSP, UI, playback) and does NOT
 * touch that file — it lives in its own file ("\RC_CONFIG.CFG" by default)
 * so both can coexist on one volume during the transition.
 *
 * On-disk format (text, Rockbox .cfg style):
 *   - one "key=value" per line, '#' comments and blank lines ignored,
 *   - unknown keys ignored (forward compatibility),
 *   - values longer than their field are truncated (NUL-terminated),
 *   - a trailing "crc32=XXXXXXXX" line covers everything before it; it is
 *     written by config_save() and verified by config_load(). Hand-edited
 *     files may simply DELETE that line (absent = no integrity check).
 *
 * Power-loss robustness (bare-metal requirement):
 *   config_save() writes "<path>.tmp" first and then fs_rename()s it over
 *   the real file, so a power cut mid-write leaves the previous config
 *   intact. A torn/corrupt file that still reaches the parser is rejected by
 *   the CRC check and the caller gets defaults + CONFIG_ERR_CRC.
 *
 * No dynamic memory anywhere: serialization uses a static buffer, so
 * config_save() / config_load() are NOT reentrant (call from one context,
 * typically the main loop — never from an ISR).
 *
 * Generic keyed access (Rockbox cfg_int/cfg_string style) is provided over a
 * module singleton: config_init() fills it, config_get_* / config_set_* look
 * fields up by key name in the schema table.
 */
#ifndef RECHORD_CONFIG_RECHORD_CONFIG_H
#define RECHORD_CONFIG_RECHORD_CONFIG_H

#include <stdint.h>

/* Config file location. Deliberately NOT "\RECHORD.CFG" (that path belongs
 * to the stock-format settings service). Override at compile time if needed. */
#ifndef CONFIG_PATH
#define CONFIG_PATH "\\RC_CONFIG.CFG"
#endif

/* Number of parametric-EQ band gains stored (0.5 dB steps, -24..+24). */
#define RECHORD_EQ_BANDS 10

typedef enum {
    RECHORD_RG_OFF = 0,
    RECHORD_RG_TRACK,
    RECHORD_RG_ALBUM
} rechord_replaygain_mode_t;

typedef enum {
    RECHORD_REPEAT_OFF = 0,
    RECHORD_REPEAT_ONE,
    RECHORD_REPEAT_ALL
} rechord_repeat_mode_t;

typedef struct {
    /* ---- audio / DSP ---- */
    int                       volume;              /* dB, negative = attenuation (stock default -23) */
    int                       balance;             /* -100 (left) .. 100 (right), 0 = center */
    rechord_replaygain_mode_t replaygain_mode;
    int                       replaygain_preamp;   /* dB, extra gain before ReplayGain */
    int                       autoeq_enabled;      /* bool */
    char                      autoeq_profile_path[64];
    int16_t                   eq_bands_gain[RECHORD_EQ_BANDS]; /* 0.5 dB steps */

    /* ---- UI / UX ---- */
    char    theme_name[32];
    uint8_t brightness;                            /* 0..100 */
    uint16_t screen_timeout_sec;                   /* 0 = never */
    char    language[8];

    /* ---- playback ---- */
    int       shuffle;                             /* bool */
    rechord_repeat_mode_t repeat_mode;
    char      resume_track_path[128];
    uint32_t  resume_offset_ms;
} rechord_config_t;

/* Return codes. */
#define CONFIG_OK          0
#define CONFIG_ERR_ARG   (-1)  /* NULL pointer / unusable path */
#define CONFIG_ERR_OPEN  (-2)  /* file missing/unopenable; defaults applied */
#define CONFIG_ERR_IO    (-3)  /* read or write failure */
#define CONFIG_ERR_CRC   (-4)  /* integrity check failed; defaults applied */
#define CONFIG_ERR_FULL  (-5)  /* serialization buffer too small */

/* Fill *cfg with defaults (volume -23 dB, RG off, repeat off, brightness 8,
 * 30-minute screen timeout to match the stock default, English UI). */
void config_set_defaults(rechord_config_t *cfg);

/* Read and parse `path` (NULL = CONFIG_PATH) into *cfg. *cfg is ALWAYS valid
 * on return: defaults first, then whatever keys parse cleanly. Returns
 * CONFIG_OK, CONFIG_ERR_OPEN, CONFIG_ERR_IO or CONFIG_ERR_CRC. */
int config_load(rechord_config_t *cfg, const char *path);

/* Atomically write *cfg to `path` (NULL = CONFIG_PATH): serialize to a
 * static buffer, write "<path>.tmp", fs_rename() over the real file. Returns
 * CONFIG_OK, CONFIG_ERR_ARG, CONFIG_ERR_OPEN, CONFIG_ERR_IO, CONFIG_ERR_FULL.
 * NOT reentrant; call from the main loop, never from an ISR. */
int config_save(const rechord_config_t *cfg, const char *path);

/* Pure text helpers (no I/O; used by the parser tests and by tools).
 * config_parse_text() applies defaults first and enforces the CRC when a
 * "crc32=" line is present; config_serialize_text() appends the crc32 line.
 * Both return CONFIG_OK/ERROR or the serialized length (>= 0). */
int config_parse_text(rechord_config_t *cfg, const char *text);
int config_serialize_text(const rechord_config_t *cfg, char *buf,
                          unsigned long cap);

/* ---- generic keyed access over the module singleton ---- */

/* The singleton the keyed accessors operate on. It starts zeroed; call
 * config_init() (or config_load into it) before reading it. */
rechord_config_t *config_current(void);

/* Defaults + load CONFIG_PATH into the singleton. Same return codes as
 * config_load(); the singleton is always usable afterwards. */
int config_init(void);

/* Key lookup in the schema. Unknown key / wrong type -> default_val.
 * Numeric keys (int/bool/u8/u16/u32/enum-index) go through config_get_int;
 * strings through config_get_string; the EQ list through the struct. */
int  config_get_int(const char *key, int default_val);
int  config_set_int(const char *key, int value);          /* clamped to field */
int  config_get_bool(const char *key, int default_val);
int  config_set_bool(const char *key, int value);
int  config_get_string(const char *key, char *out, unsigned long cap,
                       const char *default_val);          /* length copied */
int  config_set_string(const char *key, const char *value); /* length stored */

#endif /* RECHORD_CONFIG_RECHORD_CONFIG_H */
