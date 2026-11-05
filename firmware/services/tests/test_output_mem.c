/* Host tests for output_mem: keys, round trip, replace, eviction, bad lines. */
#include <stdio.h>
#include <string.h>

#include "output_mem.h"

static int fails;
#define CHECK(c, m) do { if (!(c)) { printf("  FAIL: %s (line %d)\n", m, __LINE__); fails++; } } while (0)

static eq_curve_t curve10(int g0)
{
    static const uint16_t f[10] = {80,160,320,640,1250,2500,4000,6300,10000,14000};
    eq_curve_t c;
    int i;

    memset(&c, 0, sizeof c);
    c.count = 10;
    c.enabled = 1;
    for (i = 0; i < 10; i++) {
        c.band[i].freq_hz = f[i];
        c.band[i].q_x100 = (i == 0 || i == 9) ? 70 : 140;
        c.band[i].gain_tdb = (int16_t)(i == 0 ? g0 : 0);
    }
    return c;
}

static om_state_t mk(int vol, int eq_on, const char *name, int g0)
{
    om_state_t s;

    memset(&s, 0, sizeof s);
    s.has_vol = 1;
    s.vol_db = vol;
    s.has_eq = 1;
    s.eq_on = eq_on;
    strcpy(s.name, name);
    s.curve = curve10(g0);
    return s;
}

static void test_typed_bands(void)
{
    char text[OM_FILE_MAX];
    om_state_t a = mk(-30, 1, "Sennheiser HD 650", 64), b;

    a.curve.band[0].freq_hz = 25;
    a.curve.band[0].type = EQ_TYPE_LSHELF;
    a.curve.band[5].type = EQ_TYPE_PEAK;
    a.curve.band[5].q_x100 = 1000;
    a.curve.band[9].type = EQ_TYPE_HSHELF;
    text[0] = '\0';
    CHECK(om_upsert(text, sizeof text, "wired", &a) > 0, "typed curve stored");
    CHECK(om_find(text, "wired", &b) == 1 && b.has_eq &&
          b.curve.band[0].type == EQ_TYPE_LSHELF &&
          b.curve.band[5].type == EQ_TYPE_PEAK &&
          b.curve.band[5].q_x100 == 1000 &&
          b.curve.band[9].type == EQ_TYPE_HSHELF &&
          b.curve.band[1].type == EQ_TYPE_AUTO, "types and Q 10.0 survive");
}

int main(void)
{
    static char text[OM_FILE_MAX];
    char k_w[OM_KEY_MAX], k_a[OM_KEY_MAX], k_b[OM_KEY_MAX];
    static const uint8_t a[6] = {0xAA,0xBB,0xCC,0x01,0x02,0x03};
    static const uint8_t b[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    om_state_t s, got;
    int i, n;

    om_key_wired(k_w);
    om_key_bt(a, k_a);
    om_key_bt(b, k_b);
    CHECK(strcmp(k_w, "wired") == 0, "wired key");
    CHECK(strcmp(k_a, "bt:AABBCC010203") == 0, "bt key is upper-case hex");

    strcpy(text, "# header\n");
    s = mk(-18, 1, "Rock", 60);
    CHECK(om_upsert(text, sizeof text, k_w, &s) > 0, "insert wired");
    s = mk(-30, 0, "Bass Boost", 90);
    CHECK(om_upsert(text, sizeof text, k_a, &s) > 0, "insert BT1");
    s = mk(-9, 1, "Flat", 0);
    CHECK(om_upsert(text, sizeof text, k_b, &s) > 0, "insert BT2");

    CHECK(om_find(text, k_w, &got) == 1 && got.vol_db == -18 && got.eq_on == 1 &&
          strcmp(got.name, "Rock") == 0 && got.curve.band[0].gain_tdb == 60 &&
          got.curve.count == 10 && got.curve.band[1].q_x100 == 140, "wired round trip");
    CHECK(om_find(text, k_a, &got) == 1 && got.vol_db == -30 && got.eq_on == 0 &&
          strcmp(got.name, "Bass Boost") == 0 && got.curve.band[0].gain_tdb == 90,
          "BT1 keeps its own volume and EQ");
    CHECK(om_find(text, k_b, &got) == 1 && got.vol_db == -9, "BT2 separate");
    CHECK(om_find(text, "bt:000000000000", &got) == 0, "unknown output not found");

    /* Replacing an entry keeps one line per output and does not touch others. */
    s = mk(-12, 1, "Pop", 30);
    om_upsert(text, sizeof text, k_a, &s);
    CHECK(om_find(text, k_a, &got) == 1 && got.vol_db == -12 &&
          strcmp(got.name, "Pop") == 0, "BT1 replaced");
    CHECK(om_find(text, k_w, &got) == 1 && got.vol_db == -18, "wired untouched");
    n = 0;
    for (i = 0; text[i]; i++)
        if (text[i] == 'o' && text[i + 1] == '=' && (i == 0 || text[i - 1] == '\n'))
            n++;
    CHECK(n == 3, "one line per output");

    /* Volume-only entry (no EQ) and a name containing the separator. */
    memset(&s, 0, sizeof s);
    s.has_vol = 1;
    s.vol_db = -40;
    om_upsert(text, sizeof text, "bt:0000000000AA", &s);
    CHECK(om_find(text, "bt:0000000000AA", &got) == 1 && got.has_vol &&
          !got.has_eq && got.vol_db == -40, "volume-only entry");
    s = mk(-5, 1, "a;b", 0);
    om_upsert(text, sizeof text, "bt:0000000000BB", &s);
    CHECK(om_find(text, "bt:0000000000BB", &got) == 1 &&
          strcmp(got.name, "a b") == 0 && got.vol_db == -5,
          "';' in a name cannot break the line");

    /* Garbage is ignored, a corrupt curve keeps the volume. */
    strcpy(text, "junk\no=wired;v=-7;e=1;n=X;b=80,70,1 160,140\nmore junk\n");
    CHECK(om_find(text, "wired", &got) == 0 || (got.has_vol && !got.has_eq),
          "bad curve does not poison the volume");

    /* Eviction: only OM_MAX_OUTPUTS lines are kept, newest survive. */
    text[0] = '\0';
    for (i = 0; i < 12; i++) {
        char key[OM_KEY_MAX];
        uint8_t ad[6] = {0, 0, 0, 0, 0, (uint8_t)i};

        om_key_bt(ad, key);
        s = mk(-i, 1, "Rock", i);
        CHECK(om_upsert(text, sizeof text, key, &s) > 0, "insert many");
    }
    {
        char key[OM_KEY_MAX];
        uint8_t first[6] = {0, 0, 0, 0, 0, 0}, last[6] = {0, 0, 0, 0, 0, 11};

        om_key_bt(first, key);
        CHECK(om_find(text, key, &got) == 0, "oldest evicted");
        om_key_bt(last, key);
        CHECK(om_find(text, key, &got) == 1 && got.vol_db == -11, "newest kept");
    }
    CHECK(strlen(text) < OM_FILE_MAX, "stays inside the buffer");

    test_typed_bands();
    if (fails == 0)
        printf("test_output_mem: OK\n");
    return fails != 0;
}
