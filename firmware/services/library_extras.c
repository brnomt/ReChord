/*
 * library_extras.c — see library_extras.h.
 *
 * Everything is stack-buffered except the hash tables; a save assembles the
 * whole file (under 1 KB) and rewrites it in place. Called at most once per
 * track start or favorite toggle, never per tick.
 */
#include <stdio.h>
#include <string.h>

#include "library_extras.h"
#include "rechord_files.h"
#include "fs.h"

typedef struct {
    uint32_t hash;
    uint16_t plays;
} recent_t;

static uint32_t   s_fav[EXTRAS_FAV_MAX];
static unsigned   s_fav_count;
static recent_t   s_recent[EXTRAS_RECENT_MAX];
static unsigned   s_recent_count;
static uint8_t    s_loaded;

uint32_t extras_path_hash(const char *path)
{
    uint32_t h = 2166136261u;

    while (path != 0 && *path != '\0') {
        h ^= (uint8_t)*path++;
        h *= 16777619u;
    }
    return h;
}

/* ---- file text ---------------------------------------------------------- */

#define EXTRAS_TEXT_MAX 1024

static void put_hex8(char *out, uint32_t v)
{
    static const char k[] = "0123456789abcdef";
    int i;

    for (i = 0; i < 8; i++)
        out[i] = k[(v >> (28 - 4 * i)) & 0xFu];
}

static int parse_hex8(const char *p, const char *end, uint32_t *out)
{
    uint32_t v = 0;
    int i;

    if (end - p < 8)
        return -1;
    for (i = 0; i < 8; i++) {
        char c = p[i];
        uint32_t d;

        if (c >= '0' && c <= '9')
            d = (uint32_t)(c - '0');
        else if (c >= 'a' && c <= 'f')
            d = (uint32_t)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            d = (uint32_t)(c - 'A' + 10);
        else
            return -1;
        v = (v << 4) | d;
    }
    *out = v;
    return 0;
}

static unsigned parse_dec(const char *p, const char *end)
{
    unsigned v = 0;

    while (p < end && *p >= '0' && *p <= '9') {
        if (v < 65535u)
            v = v * 10u + (unsigned)(*p - '0');
        p++;
    }
    return v;
}

static void parse_favorites(const char *text)
{
    const char *p = text;

    s_fav_count = 0;
    while (p != 0 && *p != '\0' && s_fav_count < EXTRAS_FAV_MAX) {
        const char *eol = p;

        while (*eol != '\0' && *eol != '\n' && *eol != '\r')
            eol++;
        if (*p != '#') {
            uint32_t h;

            if (parse_hex8(p, eol, &h) == 0) {
                unsigned i;

                for (i = 0; i < s_fav_count; i++)
                    if (s_fav[i] == h)
                        break;
                if (i == s_fav_count)
                    s_fav[s_fav_count++] = h;
            }
        }
        p = (*eol != '\0') ? eol + 1 : eol;
    }
}

static void parse_recent(const char *text)
{
    const char *p = text;

    s_recent_count = 0;
    while (p != 0 && *p != '\0' && s_recent_count < EXTRAS_RECENT_MAX) {
        const char *eol = p;

        while (*eol != '\0' && *eol != '\n' && *eol != '\r')
            eol++;
        if (*p != '#') {
            uint32_t h;

            if (parse_hex8(p, eol, &h) == 0) {
                unsigned i;

                for (i = 0; i < s_recent_count; i++)
                    if (s_recent[i].hash == h)
                        break;
                if (i == s_recent_count) {
                    s_recent[i].hash = h;
                    s_recent[i].plays = 1;
                    {
                        const char *q = p + 8;
                        unsigned plays;

                        while (q < eol && *q != ' ')
                            q++;
                        plays = parse_dec(q < eol ? q + 1 : eol, eol);
                        s_recent[i].plays = (uint16_t)(plays ? plays : 1);
                    }
                    s_recent_count++;
                }
            }
        }
        p = (*eol != '\0') ? eol + 1 : eol;
    }
}

void extras_init(void)
{
    char buf[EXTRAS_TEXT_MAX];

    if (s_loaded)
        return;
    s_loaded = 1;
    if (fs_load_file(RECHORD_FAVORITES, buf, sizeof buf) >= 0)
        parse_favorites(buf);
    if (fs_load_file(RECHORD_RECENT, buf, sizeof buf) >= 0)
        parse_recent(buf);
}

int extras_loaded(void)
{
    return s_loaded;
}

/* ---- favorites ---------------------------------------------------------- */

int extras_is_favorite_hash(uint32_t hash)
{
    return extras_favorite_slot(hash) >= 0;
}

int extras_favorite_slot(uint32_t hash)
{
    unsigned i;

    for (i = 0; i < s_fav_count; i++)
        if (s_fav[i] == hash)
            return (int)i;
    return -1;
}

int extras_is_favorite(const char *path)
{
    return extras_is_favorite_hash(extras_path_hash(path));
}

static void save_favorites(void)
{
    char text[EXTRAS_TEXT_MAX];
    unsigned i, n = 0;

    n += (unsigned)snprintf(text + n, sizeof text - n,
                            "# ReChord favorites (path hash)\n");
    for (i = 0; i < s_fav_count && n + 10u < sizeof text; i++) {
        put_hex8(text + n, s_fav[i]);
        text[n + 8] = '\n';
        n += 9;
    }
    text[n] = '\0';
    (void)fs_store_file(RECHORD_FAVORITES, text, n);
}

int extras_toggle_favorite(const char *path)
{
    uint32_t hash;
    unsigned i;

    extras_init();
    hash = extras_path_hash(path);
    for (i = 0; i < s_fav_count; i++) {
        if (s_fav[i] == hash) {
            unsigned j;

            for (j = i; j + 1 < s_fav_count; j++)
                s_fav[j] = s_fav[j + 1];
            s_fav_count--;
            save_favorites();
            return 0;
        }
    }
    if (s_fav_count >= EXTRAS_FAV_MAX) {
        /* Drop the oldest entry rather than refusing the new favorite. */
        memmove(&s_fav[0], &s_fav[1], (s_fav_count - 1) * sizeof s_fav[0]);
        s_fav_count--;
    }
    s_fav[s_fav_count++] = hash;
    save_favorites();
    return 1;
}

unsigned extras_favorite_count(void)
{
    return s_fav_count;
}

uint32_t extras_favorite_hash(unsigned slot)
{
    return slot < s_fav_count ? s_fav[slot] : 0;
}

/* ---- recent / play counts ---------------------------------------------- */

static void save_recent(void)
{
    char text[EXTRAS_TEXT_MAX];
    unsigned i, n = 0;

    n += (unsigned)snprintf(text + n, sizeof text - n,
                            "# ReChord recent (hash plays)\n");
    for (i = 0; i < s_recent_count && n + 16u < sizeof text; i++) {
        unsigned p = s_recent[i].plays;
        char digits[5];
        int d = 0;

        put_hex8(text + n, s_recent[i].hash);
        n += 8;
        text[n++] = ' ';
        do {
            digits[d++] = (char)('0' + p % 10u);
            p /= 10u;
        } while (p > 0u && d < (int)sizeof digits);
        while (d > 0)
            text[n++] = digits[--d];
        text[n++] = '\n';
    }
    text[n] = '\0';
    (void)fs_store_file(RECHORD_RECENT, text, n);
}

int extras_recent_slot(uint32_t hash)
{
    unsigned i;

    for (i = 0; i < s_recent_count; i++)
        if (s_recent[i].hash == hash)
            return (int)i;
    return -1;
}

void extras_note_play(const char *path)
{
    uint32_t hash;
    int slot;
    unsigned i;
    recent_t entry;

    extras_init();
    hash = extras_path_hash(path);
    slot = extras_recent_slot(hash);
    if (slot >= 0) {
        entry = s_recent[slot];
        if (entry.plays < 65535u)
            entry.plays++;
        for (i = (unsigned)slot; i > 0; i--)
            s_recent[i] = s_recent[i - 1];
        s_recent[0] = entry;
    } else {
        if (s_recent_count >= EXTRAS_RECENT_MAX)
            s_recent_count = EXTRAS_RECENT_MAX - 1;
        for (i = s_recent_count; i > 0; i--)
            s_recent[i] = s_recent[i - 1];
        s_recent[0].hash = hash;
        s_recent[0].plays = 1;
        s_recent_count++;
    }
    save_recent();
}

unsigned extras_recent_count(void)
{
    return s_recent_count;
}

uint32_t extras_recent_hash(unsigned slot)
{
    return slot < s_recent_count ? s_recent[slot].hash : 0;
}

unsigned extras_recent_plays(unsigned slot)
{
    return slot < s_recent_count ? s_recent[slot].plays : 0;
}

unsigned extras_plays_for_hash(uint32_t hash)
{
    int slot = extras_recent_slot(hash);

    return slot >= 0 ? s_recent[slot].plays : 0;
}

unsigned extras_most_played_order(unsigned char *order, unsigned cap)
{
    unsigned i, n = 0;

    if (order == 0 || cap == 0)
        return 0;
    for (i = 0; i < s_recent_count && n < cap; i++) {
        unsigned j = n;

        /* Plays descending; a tie keeps the more recent entry first. */
        while (j > 0) {
            unsigned prev = order[j - 1];

            if (s_recent[prev].plays > s_recent[i].plays ||
                (s_recent[prev].plays == s_recent[i].plays && prev < i))
                break;
            j--;
        }
        memmove(&order[j + 1], &order[j], n - j);
        order[j] = (unsigned char)i;
        n++;
    }
    return n;
}
