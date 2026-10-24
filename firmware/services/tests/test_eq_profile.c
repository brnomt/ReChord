/*
 * test_eq_profile.c — host tests for the EQ profile service (BRNOMT-76).
 *
 * Pure parser/serializer cases plus the file-backed service through the host
 * fs backend (paths map under .work/).
 */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "eq_profile.h"
#include "fs.h"
#include "fs_host.h"

static int failures;

#define CHECK(cond, name) do {                                             \
    if (cond) {                                                            \
        printf("  PASS: %s\n", name);                                      \
    } else {                                                               \
        printf("  FAIL: %s (%s:%d)\n", name, __FILE__, __LINE__);          \
        failures++;                                                        \
    }                                                                      \
} while (0)

#define TEST(name) printf("== %s ==\n", name)

#define ROOT     ".work"
#define BASE     ROOT "/Internal/.rechord"
#define EQDIR    BASE "/eq"
#define ACTIVE   BASE "/eq.cfg"

static int pub_calls;
static int pub_enabled;
static eq_curve_t pub_curve;

static void recorder(int enabled, const eq_curve_t *curve)
{
    pub_calls++;
    pub_enabled = enabled;
    pub_curve = *curve;
}

static void write_text(const char *path, const char *text)
{
    FILE *f = fopen(path, "wb");

    if (f == 0)
        return;
    fwrite(text, 1, strlen(text), f);
    fclose(f);
}

static void make_dirs(void)
{
    (void)mkdir(ROOT, 0777);
    (void)mkdir(ROOT "/Internal", 0777);
    (void)mkdir(BASE, 0777);
    (void)mkdir(EQDIR, 0777);
}

static const char *good_profile =
    "# test profile\n"
    "format=1\n"
    "name=Rock\n"
    "band0=31,70,40\n"
    "band1=62,70,30\n"
    "band2=125,70,10\n"
    "band3=250,70,-10\n"
    "band4=500,70,-20\n"
    "band5=1000,70,-10\n"
    "band6=2000,70,10\n"
    "band7=4000,70,30\n"
    "band8=8000,70,40\n"
    "band9=16000,70,30\n";

static const char *flat_5 =
    "format=1\nname=Five\n"
    "band0=100,70,0\n"
    "band1=200,70,0\n"
    "band2=400,70,0\n"
    "band3=800,70,0\n"
    "band4=1600,70,0\n";

static void test_parse(void)
{
    eq_curve_t c;
    char name[EQ_PROFILE_NAME_MAX];
    char text[512];
    char small_name[8];
    int n;

    TEST("parse");

    CHECK(eq_profile_parse(good_profile, (int)strlen(good_profile), &c,
                           name, sizeof name) == 0, "valid 10-band profile");
    CHECK(c.count == 10 && c.band[0].gain_tdb == 40 &&
          c.band[9].freq_hz == 16000, "values land in the right bands");
    CHECK(strcmp(name, "Rock") == 0, "name parsed");

    CHECK(eq_profile_parse(flat_5, (int)strlen(flat_5), &c,
                           small_name, sizeof small_name) == 0,
          "5-band profile accepted");
    CHECK(c.count == 5, "count comes from the highest band");
    CHECK(strcmp(small_name, "Five") == 0, "long name truncates safely");

    CHECK(eq_profile_parse("format=2\nband0=1,2,3\n", 20, &c, name,
                           sizeof name) < 0, "unknown format rejected");
    CHECK(eq_profile_parse("format=1\nname=x\nband0=31,70,0\n",
                           40, &c, name, sizeof name) < 0,
          "profile without 5 bands rejected");
    CHECK(eq_profile_parse("format=1\n"
                           "band0=31,70,0\nband1=62,70,0\nband2=125,70,0\n"
                           "band3=250,70,0\nband5=1000,70,0\n",
                           100, &c, name, sizeof name) < 0,
          "gap in the band sequence rejected");
    CHECK(eq_profile_parse("format=1\n"
                           "band0=31,70,0\nband1=62,70,0\nband2=125,70,0\n"
                           "band3=62,70,0\nband4=1000,70,0\n",
                           120, &c, name, sizeof name) < 0,
          "non-increasing frequencies rejected");
    CHECK(eq_profile_parse("format=1\n"
                           "band0=31,70,121\nband1=62,70,0\n"
                           "band2=125,70,0\nband3=250,70,0\n"
                           "band4=500,70,0\n",
                           140, &c, name, sizeof name) < 0,
          "out-of-range gain rejected");
    CHECK(eq_profile_parse("format=1\nbandX=1,2,3\n", 26, &c, name,
                           sizeof name) < 0, "malformed band key rejected");

    n = eq_profile_format(&(eq_curve_t){ 0 }, "Empty", text, sizeof text);
    CHECK(n < 0, "serializing an invalid curve fails");

    {
        eq_curve_t round;
        char name2[EQ_PROFILE_NAME_MAX];

        CHECK(eq_profile_parse(good_profile, (int)strlen(good_profile), &c,
                               name, sizeof name) == 0, "parse for round-trip");
        n = eq_profile_format(&c, name, text, sizeof text);
        CHECK(n > 0, "serialize");
        CHECK(eq_profile_parse(text, n, &round, name2, sizeof name2) == 0,
              "re-parse serialized text");
        CHECK(round.count == c.count && round.band[3].gain_tdb ==
              c.band[3].gain_tdb, "round-trip preserves the curve");
        CHECK(strcmp(name2, name) == 0, "round-trip preserves the name");
    }
}

static void test_format_gain(void)
{
    char buf[12];

    TEST("gain formatting");
    eq_profile_format_gain(35, buf, sizeof buf);
    CHECK(strcmp(buf, "+3.5") == 0, "+3.5");
    eq_profile_format_gain(-120, buf, sizeof buf);
    CHECK(strcmp(buf, "-12.0") == 0, "-12.0");
    eq_profile_format_gain(0, buf, sizeof buf);
    CHECK(strcmp(buf, "0.0") == 0, "0.0");
    eq_profile_format_gain(5, buf, sizeof buf);
    CHECK(strcmp(buf, "+0.5") == 0, "+0.5");
}

static void test_format_freq(void)
{
    char buf[12];
    char small[8];

    TEST("frequency formatting");
    eq_profile_format_freq(31, buf, sizeof buf);
    CHECK(strcmp(buf, "31 Hz") == 0, "31 Hz");
    eq_profile_format_freq(999, buf, sizeof buf);
    CHECK(strcmp(buf, "999 Hz") == 0, "999 Hz");
    eq_profile_format_freq(1000, buf, sizeof buf);
    CHECK(strcmp(buf, "1 kHz") == 0, "1 kHz");
    eq_profile_format_freq(3150, buf, sizeof buf);
    CHECK(strcmp(buf, "3.2 kHz") == 0, "3.2 kHz");
    eq_profile_format_freq(16000, buf, sizeof buf);
    CHECK(strcmp(buf, "16 kHz") == 0, "16 kHz");
    eq_profile_format_freq(0, buf, sizeof buf);
    CHECK(strcmp(buf, "?") == 0, "0 Hz is a placeholder");
    CHECK(eq_profile_format_freq(1000, small, sizeof small) < 0,
          "small buffer is rejected");
}

static void reset_tree(void)
{
    make_dirs();
    (void)remove(ACTIVE);
    write_text(ACTIVE,
               "format=1\nenabled=0\nname=Flat\n"
               "band0=31,70,0\nband1=62,70,0\nband2=125,70,0\n"
               "band3=250,70,0\nband4=500,70,0\nband5=1000,70,0\n"
               "band6=2000,70,0\nband7=4000,70,0\nband8=8000,70,0\n"
               "band9=16000,70,0\n");
    write_text(EQDIR "/custom1.cfg", good_profile);
    write_text(EQDIR "/custom2.cfg", flat_5);
    (void)remove(EQDIR "/custom3.cfg");
    (void)remove(EQDIR "/custom4.cfg");
    (void)remove(EQDIR "/custom5.cfg");
    (void)remove(EQDIR "/custom6.cfg");
}

static void test_service(void)
{
    char text[512];
    const eq_curve_t *c;
    FILE *f;
    size_t n;

    TEST("service");
    fs_host_set_root(ROOT);
    reset_tree();

    pub_calls = 0;
    eq_profile_set_publisher(recorder);
    CHECK(eq_profile_boot() == 0, "boot");
    CHECK(eq_profile_count() == 7, "5 built-ins + custom1 + custom2");
    CHECK(eq_profile_enabled() == 0, "boot starts disabled");
    CHECK(strcmp(eq_profile_name(), "Flat") == 0, "default profile name");
    CHECK(pub_calls == 1, "boot publishes the curve");

    CHECK(eq_profile_entry_is_custom(5) == 1, "row 5 is custom");
    CHECK(strcmp(eq_profile_entry_name(5), "Rock") == 0,
          "custom slot label comes from name=");

    CHECK(eq_profile_select(1) == 0, "select built-in Rock");
    c = eq_profile_curve();
    CHECK(c->count == 10 && c->band[0].gain_tdb == 40,
          "Rock curve is active");
    CHECK(strcmp(eq_profile_name(), "Rock") == 0, "active name updated");
    CHECK(pub_calls == 2, "selection is published");

    f = fopen(ACTIVE, "rb");
    n = (f != 0) ? fread(text, 1, sizeof text - 1, f) : 0;
    if (f != 0)
        fclose(f);
    text[n] = '\0';
    CHECK(strstr(text, "name=Rock") != 0, "selection persisted to eq.cfg");

    eq_profile_cycle_band(2, +1);
    eq_profile_cycle_band(2, +1);
    eq_profile_cycle_band(2, -1);
    CHECK(eq_profile_curve()->band[2].gain_tdb == 15,
          "band cycles by 0.5 dB");
    CHECK(eq_profile_is_dirty() == 1, "edits mark the curve dirty");
    CHECK(eq_profile_save() == 0, "save after edits");

    eq_profile_set_enabled(1);
    CHECK(eq_profile_enabled() == 1, "enable");
    CHECK(pub_enabled == 1, "enable is published");
    eq_profile_save();

    CHECK(eq_profile_select(5) == 0, "select custom slot 1");
    CHECK(eq_profile_curve()->band[8].gain_tdb == 40,
          "custom file loaded");

    /* A broken slot must not clobber the active curve. */
    write_text(EQDIR "/custom2.cfg", "format=1\nname=Broken\nband0=1,2\n");
    (void)eq_profile_scan();
    {
        eq_curve_t before = *eq_profile_curve();

        CHECK(eq_profile_select(6) < 0, "broken slot is rejected");
        CHECK(eq_profile_curve()->band[0].gain_tdb ==
              before.band[0].gain_tdb, "rejection leaves the curve untouched");
    }

    /* A sixth file appears without recompiling. */
    write_text(EQDIR "/custom6.cfg", flat_5);
    CHECK(eq_profile_scan() == 8, "custom6 appears in the picker");
    CHECK(strcmp(eq_profile_entry_name(7), "Five") == 0,
          "custom6 label parsed");
}

int main(void)
{
    printf("=== eq_profile tests ===\n");
    test_parse();
    test_format_gain();
    test_format_freq();
    test_service();

    printf("\n%s (%d failure%s)\n",
           failures == 0 ? "ALL EQ PROFILE TESTS PASSED" : "FAILURES",
           failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
