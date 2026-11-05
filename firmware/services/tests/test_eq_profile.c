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
    (void)remove(EQDIR "/Sennheiser HD 650 ParametricEQ.txt");
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
    CHECK(eq_profile_count() == EQ_PROFILE_BUILTINS + 2,
          "built-ins + custom1 + custom2");
    CHECK(eq_profile_enabled() == 0, "boot starts disabled");
    CHECK(strcmp(eq_profile_name(), "Flat") == 0, "default profile name");
    CHECK(pub_calls == 1, "boot publishes the curve");

    CHECK(eq_profile_entry_is_custom(EQ_PROFILE_BUILTINS) == 1,
          "first row after the built-ins is custom");
    CHECK(strcmp(eq_profile_entry_name(EQ_PROFILE_BUILTINS), "Rock") == 0,
          "custom slot label comes from name=");

    {
        int r, saved = pub_calls;

        for (r = 0; r < EQ_PROFILE_BUILTINS; r++) {
            CHECK(eq_profile_select(r) == 0, "every built-in selects");
            CHECK(eq_curve_valid(eq_profile_curve()),
                  "every built-in is a valid curve");
        }
        (void)eq_profile_select(0);
        pub_calls = saved;
    }
    CHECK(strcmp(eq_profile_entry_name(2), "Rock") == 0, "built-in row 2 is Rock");
    CHECK(eq_profile_select(2) == 0, "select built-in Rock");
    c = eq_profile_curve();
    CHECK(c->count == 10 && c->band[0].gain_tdb == 60 &&
          c->band[0].freq_hz == 80 && c->band[0].q_x100 == 70 &&
          c->band[1].q_x100 == 140,
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
    CHECK(eq_profile_curve()->band[2].gain_tdb == 5,
          "band cycles by 0.5 dB");
    CHECK(eq_profile_is_dirty() == 1, "edits mark the curve dirty");
    CHECK(eq_profile_save() == 0, "save after edits");

    eq_profile_set_enabled(1);
    CHECK(eq_profile_enabled() == 1, "enable");
    CHECK(pub_enabled == 1, "enable is published");
    eq_profile_save();

    CHECK(eq_profile_select(EQ_PROFILE_BUILTINS) == 0, "select custom slot 1");
    CHECK(eq_profile_curve()->band[8].gain_tdb == 40,
          "custom file loaded");

    /* A broken slot must not clobber the active curve. */
    write_text(EQDIR "/custom2.cfg", "format=1\nname=Broken\nband0=1,2\n");
    (void)eq_profile_scan();
    {
        eq_curve_t before = *eq_profile_curve();

        CHECK(eq_profile_select(EQ_PROFILE_BUILTINS + 1) < 0, "broken slot is rejected");
        CHECK(eq_profile_curve()->band[0].gain_tdb ==
              before.band[0].gain_tdb, "rejection leaves the curve untouched");
    }

    /* A sixth file appears without recompiling. */
    write_text(EQDIR "/custom6.cfg", flat_5);
    CHECK(eq_profile_scan() == EQ_PROFILE_BUILTINS + 3, "custom6 appears in the picker");
    CHECK(strcmp(eq_profile_entry_name(EQ_PROFILE_BUILTINS + 2), "Five") == 0,
          "custom6 label parsed");
}


/* The device bug: eq.cfg is created holding the short default template, and a
 * save longer than that was rejected, so EQ on/off and the profile never
 * survived a restart. The host fs now refuses growth like the target does. */
static void test_persist_across_boot(void)
{
    const eq_curve_t *c;

    TEST("persist");
    fs_host_set_root(ROOT);
    reset_tree();
    write_text(ACTIVE, "format=1\nenabled=0\nname=Flat\n"
                       "band0=80,70,0\nband1=160,140,0\nband2=320,140,0\n"
                       "band3=640,140,0\nband4=1250,140,0\n");   /* short */
    CHECK(eq_profile_boot() == 0, "boot from a short file");
    eq_profile_set_enabled(1);
    CHECK(eq_profile_select(2) == 0, "pick Rock");
    eq_profile_set_band_gain(1, 77);
    CHECK(eq_profile_save() == 0, "a longer profile saves");

    CHECK(eq_profile_boot() == 0, "reboot");            /* reload from disk */
    c = eq_profile_curve();
    CHECK(eq_profile_enabled() == 1, "EQ is still on after a restart");
    CHECK(strcmp(eq_profile_name(), "Rock") == 0, "profile name survived");
    CHECK(c->count == 10 && c->band[0].gain_tdb == 60 &&
          c->band[1].gain_tdb == 77, "gains survived");
}

static int obs_calls;
static void observer(void) { obs_calls++; }

/* Restoring an output's EQ replaces curve + name + on/off, persists it and
 * publishes it, but is not reported as a user edit. */
static void test_restore(void)
{
    eq_curve_t c;
    int before;

    TEST("restore");
    fs_host_set_root(ROOT);
    reset_tree();
    CHECK(eq_profile_boot() == 0, "boot");
    eq_profile_set_observer(observer);
    CHECK(eq_profile_select(2) == 0, "pick Rock");
    CHECK(obs_calls > 0, "a user edit notifies the observer");

    c = *eq_profile_curve();
    c.band[0].gain_tdb = 33;
    obs_calls = 0;
    before = pub_calls;
    CHECK(eq_profile_restore(&c, "Phones", 1) == 0, "restore applies");
    CHECK(eq_profile_enabled() == 1 && strcmp(eq_profile_name(), "Phones") == 0 &&
          eq_profile_curve()->band[0].gain_tdb == 33, "state replaced");
    CHECK(obs_calls == 0, "restore is not a user edit");
    CHECK(pub_calls == before + 1, "restore publishes to the engine");
    CHECK(eq_profile_boot() == 0, "reboot");
    CHECK(eq_profile_enabled() == 1 && eq_profile_curve()->band[0].gain_tdb == 33 &&
          strcmp(eq_profile_name(), "Phones") == 0, "restore persisted");

    c.count = 3;                                 /* invalid curve */
    CHECK(eq_profile_restore(&c, "Bad", 0) < 0, "invalid curve rejected");
    CHECK(strcmp(eq_profile_name(), "Phones") == 0, "rejection changes nothing");
    eq_profile_set_observer(0);
}

/* BRNOMT-77: the real AutoEQ export for the Sennheiser HD 650 (oratory1990),
 * unsorted, PK/LSC/HSC, one Q above the old 4.0 ceiling, CRLF line ends. */
static const char *hd650 =
    "Preamp: -6.1 dB\r\n"
    "Filter 1: ON LSC Fc 105 Hz Gain 6.4 dB Q 0.70\r\n"
    "Filter 2: ON PK Fc 8800 Hz Gain 5.1 dB Q 1.42\r\n"
    "Filter 3: ON PK Fc 118 Hz Gain -3.1 dB Q 0.50\r\n"
    "Filter 4: ON PK Fc 37 Hz Gain 0.7 dB Q 3.96\r\n"
    "Filter 5: ON PK Fc 3169 Hz Gain -1.7 dB Q 3.89\r\n"
    "Filter 6: ON HSC Fc 10000 Hz Gain -2.1 dB Q 0.70\r\n"
    "Filter 7: ON PK Fc 1227 Hz Gain -1.2 dB Q 2.53\r\n"
    "Filter 8: ON PK Fc 2055 Hz Gain 1.2 dB Q 3.23\r\n"
    "Filter 9: ON PK Fc 587 Hz Gain 0.4 dB Q 1.19\r\n"
    "Filter 10: ON PK Fc 5332 Hz Gain -1.1 dB Q 5.75\r\n";

static void test_autoeq(void)
{
    eq_curve_t c;
    char text[600];
    int n, i, sorted = 1;

    TEST("AutoEQ import");
    CHECK(eq_profile_parse_autoeq(hd650, (int)strlen(hd650), &c) == 0,
          "HD 650 ParametricEQ parses");
    CHECK(c.count == 10 && c.enabled == 1, "10 bands, enabled");
    for (i = 1; i < c.count; i++)
        if (c.band[i].freq_hz <= c.band[i - 1].freq_hz)
            sorted = 0;
    CHECK(sorted, "bands sorted by frequency");
    CHECK(c.band[0].freq_hz == 37 && c.band[0].type == EQ_TYPE_PEAK &&
          c.band[0].gain_tdb == 7 && c.band[0].q_x100 == 396,
          "37 Hz peak stays a peak below the shelf");
    CHECK(c.band[1].freq_hz == 105 && c.band[1].type == EQ_TYPE_LSHELF &&
          c.band[1].gain_tdb == 64 && c.band[1].q_x100 == 70,
          "105 Hz low shelf, +6.4 dB, Q 0.70");
    CHECK(c.band[8].freq_hz == 8800 && c.band[8].type == EQ_TYPE_PEAK &&
          c.band[8].gain_tdb == 51, "8800 Hz peak +5.1 dB");
    CHECK(c.band[9].freq_hz == 10000 && c.band[9].type == EQ_TYPE_HSHELF &&
          c.band[9].gain_tdb == -21, "10 kHz high shelf -2.1 dB");
    CHECK(c.band[7].freq_hz == 5332 && c.band[7].q_x100 == 575,
          "narrow Q 5.75 kept");

    /* The typed curve survives eq.cfg (4th field) and old files still parse. */
    n = eq_profile_format(&c, "HD 650", text, sizeof text);
    {
        eq_curve_t back;
        char name[EQ_PROFILE_NAME_MAX];
        int same = n > 0 &&
                   eq_profile_parse(text, n, &back, name, sizeof name) == 0 &&
                   back.count == c.count;

        for (i = 0; same && i < c.count; i++)
            same = back.band[i].freq_hz == c.band[i].freq_hz &&
                   back.band[i].q_x100 == c.band[i].q_x100 &&
                   back.band[i].gain_tdb == c.band[i].gain_tdb &&
                   back.band[i].type == c.band[i].type;
        CHECK(same, "typed curve round-trips through eq.cfg");
    }

    {
        const char *few = "Filter 1: ON PK Fc 1000 Hz Gain -3 dB Q 1.0\n"
                          "Filter 2: OFF PK Fc 2000 Hz Gain 3 dB Q 1.0\n"
                          "Filter 3: ON LSC Fc 100 Hz Gain 20 dB\n";

        CHECK(eq_profile_parse_autoeq(few, (int)strlen(few), &c) == 0 &&
              c.count == 5 && c.band[0].freq_hz == 100 &&
              c.band[0].gain_tdb == 120 && c.band[1].freq_hz == 1000 &&
              c.band[2].gain_tdb == 0,
              "2 filters + OFF: padded to 5, gain clamped to +12 dB");
    }
    {
        const char *lp = "Filter 1: ON LP Fc 1000 Hz\n";
        const char *noq = "Filter 1: ON PK Fc 1000 Hz Gain 3 dB\n";
        const char *junk = "Preamp: -3 dB\n";

        CHECK(eq_profile_parse_autoeq(lp, (int)strlen(lp), &c) < 0,
              "unsupported filter type rejected");
        CHECK(eq_profile_parse_autoeq(noq, (int)strlen(noq), &c) < 0,
              "peak without Q rejected");
        CHECK(eq_profile_parse_autoeq(junk, (int)strlen(junk), &c) < 0,
              "no filters rejected");
    }

    /* Dropped into the EQ folder it shows up as a read-only profile. */
    fs_host_set_root(ROOT);
    reset_tree();
    write_text(EQDIR "/Sennheiser HD 650 ParametricEQ.txt", hd650);
    CHECK(eq_profile_boot() == 0, "boot with an import");
    n = eq_profile_count();
    CHECK(n == EQ_PROFILE_BUILTINS + eq_profile_custom_count() + 1,
          "one import row after the custom slots");
    CHECK(strcmp(eq_profile_entry_name(n - 1), "Sennheiser HD 650") == 0,
          "row named after the file");
    CHECK(!eq_profile_entry_is_custom(n - 1), "import is not a writable slot");
    CHECK(eq_profile_save_to_row(n - 1) < 0, "cannot save over an import");
    CHECK(eq_profile_select(n - 1) == 0, "select the import");
    CHECK(strcmp(eq_profile_name(), "Sennheiser HD 650") == 0 &&
          eq_profile_curve()->band[1].type == EQ_TYPE_LSHELF &&
          pub_curve.band[9].type == EQ_TYPE_HSHELF,
          "typed curve is active and published");
    CHECK(eq_profile_boot() == 0 &&
          strcmp(eq_profile_name(), "Sennheiser HD 650") == 0 &&
          eq_profile_curve()->band[7].q_x100 == 575,
          "import survives a reboot via eq.cfg");
    (void)remove(EQDIR "/Sennheiser HD 650 ParametricEQ.txt");
}

int main(void)
{
    printf("=== eq_profile tests ===\n");
    test_parse();
    test_format_gain();
    test_format_freq();
    test_service();
    test_persist_across_boot();
    test_restore();
    test_autoeq();

    printf("\n%s (%d failure%s)\n",
           failures == 0 ? "ALL EQ PROFILE TESTS PASSED" : "FAILURES",
           failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
