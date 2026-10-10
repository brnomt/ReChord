/*
 * test_settings.c — host tests for the \RECHORD.CFG settings service.
 *
 * Covers: defaults, exact on-device sample values (drop-in schema),
 * round-trip parse/save, unknown-key/comment/CRLF tolerance, missing file,
 * malformed values, and NULL-argument handling.
 */
#include "settings.h"
#include "fs.h"
#include "fs_host.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
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

/* The documented on-device sample configuration: exact values, exact 12-key
 * schema, comment lines. This fixture is the drop-in compatibility target. */
static const char k_sample_cfg[] =
    "# ReChord settings: key=value, one per line. Unknown keys are ignored.\n"
    "play=repeat_track\n"
    "brightness=8\n"
    "screen_off=30\n"
    "auto_off=15\n"
    "usb=ask\n"
    "resume=folder\n"
    "strip=off\n"
    "tags=on\n"
    "cpu=auto\n"
    "volume=-23\n"
    "# where the browser and the player were (first clusters; sd: = the card)\n"
    "browse=\n"
    "cursor=0\n";

/* Expected sample values, spelled out for independent comparison. */
static void check_sample_values(const settings_t *s)
{
    CHECK(strcmp(s->play, "repeat_track") == 0);
    CHECK(s->brightness == 8);
    CHECK(s->screen_off_min == 30);
    CHECK(s->auto_off_min == 15);
    CHECK(strcmp(s->usb, "ask") == 0);
    CHECK(strcmp(s->resume, "folder") == 0);
    CHECK(strcmp(s->strip, "off") == 0);
    CHECK(strcmp(s->tags, "on") == 0);
    CHECK(strcmp(s->cpu, "auto") == 0);
    CHECK(s->volume == -23);
    CHECK(strcmp(s->browse, "") == 0);
    CHECK(s->cursor == 0);
}

static void check_same(const settings_t *a, const settings_t *b)
{
    CHECK(strcmp(a->play, b->play) == 0);
    CHECK(a->brightness == b->brightness);
    CHECK(a->screen_off_min == b->screen_off_min);
    CHECK(a->auto_off_min == b->auto_off_min);
    CHECK(strcmp(a->usb, b->usb) == 0);
    CHECK(strcmp(a->resume, b->resume) == 0);
    CHECK(strcmp(a->strip, b->strip) == 0);
    CHECK(strcmp(a->tags, b->tags) == 0);
    CHECK(strcmp(a->cpu, b->cpu) == 0);
    CHECK(a->volume == b->volume);
    CHECK(strcmp(a->browse, b->browse) == 0);
    CHECK(a->cursor == b->cursor);
}

static void write_cfg(const char *content)
{
    fs_file_t *f = fs_open_write(SETTINGS_PATH);
    unsigned long n = (unsigned long)strlen(content);

    CHECK(f != NULL);
    if (f == NULL) {
        return;
    }
    CHECK(fs_write(f, content, n) == (long)n);
    fs_close(f);
}

static void read_cfg(char *buf, unsigned long cap)
{
    fs_file_t *f = fs_open_read(SETTINGS_PATH);
    long n;

    CHECK(f != NULL);
    if (f == NULL) {
        buf[0] = '\0';
        return;
    }
    n = fs_read(f, buf, cap - 1);
    CHECK(n >= 0);
    buf[(n > 0) ? n : 0] = '\0';
    fs_close(f);
}

static void test_defaults(void)
{
    settings_t s;

    memset(&s, 0xAA, sizeof(s));
    settings_defaults(&s);
    check_sample_values(&s);

    settings_defaults(NULL);            /* must not crash */
    CHECK(1);
}

static void test_sample_round_trip(void)
{
    settings_t a, b, c;
    char text[768];
    int rc;

    /* 1. parse the exact on-device sample */
    settings_defaults(&a);
    rc = settings_parse_text(k_sample_cfg, &a);
    CHECK(rc == SETTINGS_OK);
    check_sample_values(&a);

    /* 2. save it back out */
    write_cfg("garbage to be truncated");
    rc = settings_save(&a);
    CHECK(rc == SETTINGS_OK);

    /* 3. load again and compare field by field */
    settings_defaults(&b);
    CHECK(settings_load(&b) == SETTINGS_OK);
    check_sample_values(&b);
    check_same(&a, &b);

    /* 4. serialize again: pure parse/serialize round trip */
    rc = settings_serialize_text(&b, text, (unsigned long)sizeof(text));
    CHECK(rc > 0);
    settings_defaults(&c);
    CHECK(settings_parse_text(text, &c) == SETTINGS_OK);
    check_same(&b, &c);

    /* 5. the saved file is key=value one per line, 12 keys in schema order */
    read_cfg(text, (unsigned long)sizeof(text));
    CHECK(strstr(text, "play=repeat_track\n") != NULL);
    CHECK(strstr(text, "brightness=8\n") != NULL);
    CHECK(strstr(text, "screen_off=30\n") != NULL);
    CHECK(strstr(text, "auto_off=15\n") != NULL);
    CHECK(strstr(text, "usb=ask\n") != NULL);
    CHECK(strstr(text, "resume=folder\n") != NULL);
    CHECK(strstr(text, "strip=off\n") != NULL);
    CHECK(strstr(text, "tags=on\n") != NULL);
    CHECK(strstr(text, "cpu=auto\n") != NULL);
    CHECK(strstr(text, "volume=-23\n") != NULL);
    CHECK(strstr(text, "browse=\n") != NULL);
    CHECK(strstr(text, "cursor=0\n") != NULL);
    CHECK(strstr(text, "\nplay=") < strstr(text, "\nbrightness="));
    CHECK(strstr(text, "\nbrightness=") < strstr(text, "\nscreen_off="));
    CHECK(strstr(text, "\nscreen_off=") < strstr(text, "\nauto_off="));
    CHECK(strstr(text, "\nauto_off=") < strstr(text, "\nusb="));
    CHECK(strstr(text, "\nusb=") < strstr(text, "\nresume="));
    CHECK(strstr(text, "\nresume=") < strstr(text, "\nstrip="));
    CHECK(strstr(text, "\nstrip=") < strstr(text, "\ntags="));
    CHECK(strstr(text, "\ntags=") < strstr(text, "\ncpu="));
    CHECK(strstr(text, "\ncpu=") < strstr(text, "\nvolume="));
    CHECK(strstr(text, "\nvolume=") < strstr(text, "\nbrowse="));
    CHECK(strstr(text, "\nbrowse=") < strstr(text, "\ncursor="));
}

static void test_tolerance(void)
{
    settings_t s;

    settings_defaults(&s);
    CHECK(settings_parse_text(
        "# comment line\n"
        "\n"
        "   # indented comment\n"
        "unknown_key=whatever\n"
        "play=repeat_track\r\n"          /* CRLF */
        "brightness=notanumber\n"        /* invalid int: value kept */
        "brightness=15\n"                /* duplicate: last one wins */
        "no_equals_sign_here\n"
        "  volume = -7  \n"              /* whitespace around key and value */
        "browse=/MUSIC/Album\n"
        "this_line_is_far_too_long_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "brightness=99\n",               /* tail of the over-long line */
        &s) == SETTINGS_OK);
    CHECK(strcmp(s.play, "repeat_track") == 0);
    CHECK(s.brightness == 15);           /* not 99: over-long line ignored */
    CHECK(s.volume == -7);
    CHECK(strcmp(s.browse, "/MUSIC/Album") == 0);
    /* untouched keys keep defaults */
    CHECK(s.screen_off_min == 30);
    CHECK(s.auto_off_min == 15);
    CHECK(strcmp(s.cpu, "auto") == 0);

    /* value truncation: play[16] holds 15 chars + NUL */
    settings_defaults(&s);
    CHECK(settings_parse_text("play=0123456789ABCDEFGHIJKLMNOP\n", &s)
          == SETTINGS_OK);
    CHECK(strcmp(s.play, "0123456789ABCDE") == 0);
}

static void test_missing_file_and_args(void)
{
    settings_t s;

    (void)remove(".work/RECHORD.CFG");
    settings_defaults(&s);
    CHECK(settings_load(&s) == SETTINGS_ERR_OPEN);
    check_sample_values(&s);             /* defaults still applied */

    CHECK(settings_load(NULL) == SETTINGS_ERR_ARG);
    CHECK(settings_save(NULL) == SETTINGS_ERR_ARG);
    CHECK(settings_parse_text(NULL, &s) == SETTINGS_ERR_ARG);
    CHECK(settings_parse_text("a=b", NULL) == SETTINGS_ERR_ARG);
    CHECK(settings_serialize_text(&s, NULL, 10) == SETTINGS_ERR_ARG);
    CHECK(settings_serialize_text(NULL, NULL, 10) == SETTINGS_ERR_ARG);
}

int main(void)
{
    settings_t s;
    char small[8];

    if (mkdir(".work", 0777) != 0 && errno != EEXIST) {
        printf("FAIL cannot create .work\n");
        return 1;
    }
    fs_host_set_root(".work");

    test_defaults();
    test_sample_round_trip();
    test_tolerance();
    test_missing_file_and_args();

    /* buffer-too-small serialization */
    settings_defaults(&s);
    CHECK(settings_serialize_text(&s, small, sizeof(small)) == SETTINGS_ERR_FULL);

    printf("test_settings: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
