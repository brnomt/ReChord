/*
 * mock_eq.c — EQ profile mock for the UI host tests.
 *
 * The real parser/file service is covered by firmware/services/tests; here we
 * need a deterministic, no-filesystem backend so the screen's navigation,
 * edit mode and picker behavior can be driven with keys.
 */
#include <stdio.h>
#include <string.h>

#include "eq_profile.h"

static eq_curve_t s_curve;
static char       s_name[EQ_PROFILE_NAME_MAX];
static int        s_enabled;
static int        s_custom;
static char       s_custom_name[EQ_PROFILE_SLOTS][EQ_PROFILE_NAME_MAX];

static const uint16_t s_freq[10] = {
    31, 62, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};
static const char *const s_builtin[5] = {
    "Flat", "Rock", "Jazz", "EDM", "Vocal"
};

int mock_eq_save_calls;
int mock_eq_select_row;
int mock_eq_store_row;

void mock_eq_reset(void)
{
    int i;

    s_curve.count = 10;
    s_curve.enabled = 0;
    for (i = 0; i < 10; i++) {
        s_curve.band[i].freq_hz = s_freq[i];
        s_curve.band[i].q_x100 = 70;
        s_curve.band[i].gain_tdb = 0;
    }
    snprintf(s_name, sizeof s_name, "%s", "Flat");
    s_enabled = 0;
    s_custom = 2;
    for (i = 0; i < EQ_PROFILE_SLOTS; i++)
        snprintf(s_custom_name[i], sizeof s_custom_name[i], "Custom %d", i + 1);
    mock_eq_save_calls = 0;
    mock_eq_select_row = -1;
    mock_eq_store_row = -1;
}

int eq_profile_boot(void) { return 0; }
int eq_profile_ready(void) { return 1; }
void eq_profile_set_publisher(eq_publish_fn fn) { (void)fn; }
int eq_profile_apply(void) { return 0; }
int eq_profile_save(void) { mock_eq_save_calls++; return 0; }
int eq_profile_scan(void) { return eq_profile_count(); }

int eq_profile_count(void) { return 5 + s_custom; }

const char *eq_profile_entry_name(int index)
{
    if (index < 0 || index >= eq_profile_count())
        return "";
    if (index < 5)
        return s_builtin[index];
    return s_custom_name[index - 5];
}

int eq_profile_entry_is_custom(int index)
{
    return index >= 5 && index < eq_profile_count();
}

int eq_profile_select(int index)
{
    if (index < 0 || index >= eq_profile_count())
        return -1;
    snprintf(s_name, sizeof s_name, "%s", eq_profile_entry_name(index));
    mock_eq_select_row = index;
    return 0;
}

int eq_profile_save_to_row(int index)
{
    if (index < 5 || index >= eq_profile_count())
        return -1;
    mock_eq_store_row = index;
    snprintf(s_name, sizeof s_name, "%s", eq_profile_entry_name(index));
    mock_eq_save_calls++;
    return 0;
}

const eq_curve_t *eq_profile_curve(void) { return &s_curve; }
const char *eq_profile_name(void) { return s_name; }
int eq_profile_enabled(void) { return s_enabled; }
int eq_profile_is_dirty(void) { return 0; }

void eq_profile_set_enabled(int on)
{
    s_enabled = on ? 1 : 0;
    s_curve.enabled = (uint8_t)s_enabled;
    mock_eq_save_calls++;
}

void eq_profile_set_band_gain(int band, int gain_tdb)
{
    if (band < 0 || band >= (int)s_curve.count)
        return;
    if (gain_tdb > EQ_GAIN_MAX_TDB)
        gain_tdb = EQ_GAIN_MAX_TDB;
    if (gain_tdb < -EQ_GAIN_MAX_TDB)
        gain_tdb = -EQ_GAIN_MAX_TDB;
    s_curve.band[band].gain_tdb = (int16_t)gain_tdb;
}

void eq_profile_cycle_band(int band, int dir)
{
    if (band < 0 || band >= (int)s_curve.count)
        return;
    eq_profile_set_band_gain(band,
        (int)s_curve.band[band].gain_tdb + (dir >= 0 ? 5 : -5));
}

int eq_profile_format_gain(int gain_tdb, char *buf, unsigned cap)
{
    unsigned v = (unsigned)(gain_tdb < 0 ? -gain_tdb : gain_tdb);
    int neg = gain_tdb < 0;

    if (cap < 6)
        return -1;
    snprintf(buf, cap, "%s%u.%u", neg ? "-" : (gain_tdb > 0 ? "+" : ""),
             v / 10u, v % 10u);
    return (int)strlen(buf);
}
