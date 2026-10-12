/*
 * test_config.c — host tests for the ReChord persistent config module.
 *
 * Covers: defaults, tolerant parsing (comments/blank lines/spaces/unknown
 * keys/invalid values/CRLF), serialize->parse roundtrip (incl. the EQ band
 * list), CRC trailer roundtrip + tamper rejection, atomic save (.tmp cleaned
 * up, previous file replaced), and the generic keyed get/set API.
 */
#include "rechord_config.h"
#include "fs_host.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

static void files_reset(void)
{
    (void)remove(".work/RC_CONFIG.CFG");
    (void)remove(".work/RC_CONFIG.CFG.tmp");
}

static int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static void write_text(const char *path, const char *text)
{
    FILE *fp = fopen(path, "wb");

    CHECK(fp != NULL);
    if (fp == NULL) {
        return;
    }
    CHECK(fwrite(text, 1, strlen(text), fp) == strlen(text));
    (void)fclose(fp);
}

static void test_defaults(void)
{
    rechord_config_t cfg;

    config_set_defaults(&cfg);
    CHECK(cfg.volume == -23);
    CHECK(cfg.balance == 0);
    CHECK(cfg.replaygain_mode == RECHORD_RG_OFF);
    CHECK(cfg.replaygain_preamp == 0);
    CHECK(cfg.autoeq_enabled == 0);
    CHECK(cfg.autoeq_profile_path[0] == '\0');
    CHECK(cfg.eq_bands_gain[0] == 0 && cfg.eq_bands_gain[9] == 0);
    CHECK(strcmp(cfg.theme_name, "default") == 0);
    CHECK(cfg.brightness == 8);
    CHECK(cfg.screen_timeout_sec == 1800);
    CHECK(strcmp(cfg.language, "en") == 0);
    CHECK(cfg.shuffle == 0);
    CHECK(cfg.repeat_mode == RECHORD_REPEAT_OFF);
    CHECK(cfg.resume_track_path[0] == '\0');
    CHECK(cfg.resume_offset_ms == 0);
}

/* A hostile-but-valid file: comments, blanks, spaces, CRLF, unknown keys,
 * invalid values, out-of-range values. */
static const char k_text[] =
    "# comment line\n"
    "\n"
    "   \n"
    "  volume = -18 \r\n"
    "balance=+50\n"
    "replaygain_mode=album\n"
    "replaygain_preamp=bad\n"
    "autoeq_enabled=on\n"
    "autoeq_profile=rockbass.eq\n"
    "eq_bands=1, -2, 3, -4, 5, -6, 7, -8, 9, -10\n"
    "theme=Dark Theme\n"
    "brightness=9999\n"
    "screen_timeout_sec=60\n"
    "language=es\n"
    "shuffle=false\n"
    "repeat_mode=one\n"
    "resume_track=/MUSIC/a.flac\n"
    "resume_offset_ms=123456\n"
    "unknown_key=42\n"
    "no_equals_sign\n"
    "volume=abc\n"
    "=novalue\n";

static void test_parse_tolerance(void)
{
    rechord_config_t cfg;

    CHECK(config_parse_text(&cfg, k_text) == CONFIG_OK);
    CHECK(cfg.volume == -18);            /* invalid "volume=abc" ignored */
    CHECK(cfg.balance == 50);
    CHECK(cfg.replaygain_mode == RECHORD_RG_ALBUM);
    CHECK(cfg.replaygain_preamp == 0);   /* invalid int -> default */
    CHECK(cfg.autoeq_enabled == 1);
    CHECK(strcmp(cfg.autoeq_profile_path, "rockbass.eq") == 0);
    CHECK(cfg.eq_bands_gain[0] == 1 && cfg.eq_bands_gain[1] == -2);
    CHECK(cfg.eq_bands_gain[8] == 9 && cfg.eq_bands_gain[9] == -10);
    CHECK(strcmp(cfg.theme_name, "Dark Theme") == 0);
    CHECK(cfg.brightness == 255);        /* clamped to uint8 */
    CHECK(cfg.screen_timeout_sec == 60);
    CHECK(strcmp(cfg.language, "es") == 0);
    CHECK(cfg.shuffle == 0);
    CHECK(cfg.repeat_mode == RECHORD_REPEAT_ONE);
    CHECK(strcmp(cfg.resume_track_path, "/MUSIC/a.flac") == 0);
    CHECK(cfg.resume_offset_ms == 123456);

    /* Short EQ list: remaining bands zeroed. */
    CHECK(config_parse_text(&cfg, "eq_bands=3,4\n") == CONFIG_OK);
    CHECK(cfg.eq_bands_gain[0] == 3 && cfg.eq_bands_gain[1] == 4);
    CHECK(cfg.eq_bands_gain[2] == 0 && cfg.eq_bands_gain[9] == 0);
}

static void test_roundtrip(void)
{
    rechord_config_t cfg, cfg2;
    char text[2048];
    int len;

    config_set_defaults(&cfg);
    cfg.volume = -31;
    cfg.balance = -40;
    cfg.replaygain_mode = RECHORD_RG_TRACK;
    cfg.replaygain_preamp = -3;
    cfg.autoeq_enabled = 1;
    memcpy(cfg.autoeq_profile_path, "jazz.eq", 8);
    cfg.eq_bands_gain[0] = 24;
    cfg.eq_bands_gain[9] = -24;
    memcpy(cfg.theme_name, "amber", 6);
    cfg.brightness = 17;
    cfg.screen_timeout_sec = 3600;
    memcpy(cfg.language, "de", 3);
    cfg.shuffle = 1;
    cfg.repeat_mode = RECHORD_REPEAT_ALL;
    memcpy(cfg.resume_track_path, "/MUSIC/x/long track name.flac", 29);
    cfg.resume_offset_ms = 4000000000u;  /* > INT_MAX: u32 round-trip */

    len = config_serialize_text(&cfg, text, sizeof(text));
    CHECK(len > 0);
    CHECK(strstr(text, "crc32=") != NULL);

    CHECK(config_parse_text(&cfg2, text) == CONFIG_OK);
    CHECK(memcmp(&cfg, &cfg2, sizeof(cfg)) == 0);
}

static void test_crc_rejects_tamper(void)
{
    rechord_config_t cfg, cfg2, dflt;
    char text[2048];
    char *vol;
    int len;

    config_set_defaults(&cfg);
    len = config_serialize_text(&cfg, text, sizeof(text));
    CHECK(len > 0);

    /* Torn write simulation: value changed, crc32 line stale. */
    vol = strstr(text, "volume=-23");
    CHECK(vol != NULL);
    if (vol != NULL) {
        vol[9] = '2';                    /* -23 -> -22, same length */
    }
    CHECK(config_parse_text(&cfg2, text) == CONFIG_ERR_CRC);
    config_set_defaults(&dflt);
    CHECK(memcmp(&cfg2, &dflt, sizeof(cfg2)) == 0);   /* defaults restored */

    /* No crc32 line at all (hand-written file): accepted. */
    CHECK(config_parse_text(&cfg2, "volume=-10\n") == CONFIG_OK);
    CHECK(cfg2.volume == -10);
}

static void test_atomic_save_load(void)
{
    rechord_config_t cfg, cfg2, dflt;

    files_reset();
    config_set_defaults(&cfg);
    cfg.volume = -25;
    CHECK(config_save(&cfg, NULL) == CONFIG_OK);
    CHECK(file_exists(".work/RC_CONFIG.CFG"));
    CHECK(!file_exists(".work/RC_CONFIG.CFG.tmp"));   /* renamed, not left */

    CHECK(config_load(&cfg2, NULL) == CONFIG_OK);
    CHECK(memcmp(&cfg, &cfg2, sizeof(cfg)) == 0);

    /* Missing file: error code + usable defaults. */
    files_reset();
    config_set_defaults(&dflt);
    CHECK(config_load(&cfg2, NULL) == CONFIG_ERR_OPEN);
    CHECK(memcmp(&cfg2, &dflt, sizeof(cfg2)) == 0);

    /* Garbage file WITHOUT a crc32 line: tolerated, all defaults (a stale
     * crc line is covered by the tamper test above). */
    write_text(".work/RC_CONFIG.CFG", "volume=zz\nzzzz\n");
    CHECK(config_load(&cfg2, NULL) == CONFIG_OK);
    CHECK(memcmp(&cfg2, &dflt, sizeof(cfg2)) == 0);
}

static void test_keyed_api(void)
{
    rechord_config_t cfg2;
    char buf[64];

    files_reset();
    config_set_defaults(config_current());
    config_set_int("volume", -30);
    CHECK(config_get_int("volume", 0) == -30);
    config_set_int("brightness", 9999);             /* clamped */
    CHECK(config_get_int("brightness", 0) == 255);
    CHECK(config_set_int("repeat_mode", 2) == 0);
    CHECK(config_get_int("repeat_mode", -1) == RECHORD_REPEAT_ALL);
    CHECK(config_set_int("repeat_mode", 7) == -1);  /* out of range */
    CHECK(config_set_bool("shuffle", 1) == 0);
    CHECK(config_get_bool("shuffle", 0) == 1);
    CHECK(config_set_string("theme", "amber") == 5);
    CHECK(config_get_string("theme", buf, sizeof(buf), "") == 5);
    CHECK(strcmp(buf, "amber") == 0);

    /* Unknown keys: get -> default, set -> error. */
    CHECK(config_get_int("nope", 77) == 77);
    CHECK(config_set_int("nope", 1) == -1);
    CHECK(config_get_bool("nope", 1) == 1);
    CHECK(config_set_string("nope", "x") == -1);
    CHECK(config_get_string("nope", buf, sizeof(buf), "dft") == 3);
    CHECK(strcmp(buf, "dft") == 0);

    /* Persist the singleton and read it back through config_init(). */
    CHECK(config_save(config_current(), NULL) == CONFIG_OK);
    CHECK(config_init() == CONFIG_OK);
    CHECK(config_get_int("volume", 0) == -30);
    CHECK(config_get_string("theme", buf, sizeof(buf), "") == 5);
    CHECK(strcmp(buf, "amber") == 0);
    CHECK(config_load(&cfg2, NULL) == CONFIG_OK);
    CHECK(cfg2.volume == -30 && cfg2.shuffle == 1);
}

int main(void)
{
    (void)mkdir(".work", 0777);   /* fine if it already exists */
    fs_host_set_root(".work");

    test_defaults();
    test_parse_tolerance();
    test_roundtrip();
    test_crc_rejects_tamper();
    test_atomic_save_load();
    test_keyed_api();

    printf("test_config: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
