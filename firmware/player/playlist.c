/*
 * playlist.c — see playlist.h. Pure logic: user order, play order, repeat,
 * and a Rockbox-style shuffle (reorder the list, current track first).
 */
#include "playlist.h"

static uint32_t xs32(playlist_t *pl)
{
    uint32_t x = pl->rng;

    if (x == 0)
        x = 0x9E3779B9u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    pl->rng = x;
    return x;
}

/* In-place Fisher-Yates over [from, n). */
static void shuffle_range(playlist_t *pl, int from)
{
    int i;

    for (i = pl->n - 1; i > from; i--) {
        int j = from + (int)(xs32(pl) % (uint32_t)(i - from + 1));
        uint16_t t = pl->order[i];

        pl->order[i] = pl->order[j];
        pl->order[j] = t;
    }
}

/* Index of `v` in `order`, or -1. */
static int find_val(const playlist_t *pl, uint16_t v)
{
    int i;

    for (i = 0; i < pl->n; i++)
        if (pl->order[i] == v)
            return i;
    return -1;
}

void playlist_reset(playlist_t *pl)
{
    if (pl == 0)
        return;
    pl->n = 0;
    pl->pos = -1;
}

void playlist_seed(playlist_t *pl, uint32_t seed)
{
    if (pl != 0)
        pl->rng = seed ? seed : 0x1234567u;
}

int playlist_load(playlist_t *pl, const uint16_t *idx, int n, int start)
{
    int i;

    if (pl == 0)
        return 0;
    if (idx == 0 || n <= 0) {
        pl->n = 0;
        pl->pos = -1;
        return 0;
    }
    if (n > PLAYLIST_MAX)
        n = PLAYLIST_MAX;
    if (start < 0)
        start = 0;
    if (start >= n)
        start = n - 1;

    for (i = 0; i < n; i++) {
        pl->orig[i] = idx[i];
        pl->order[i] = idx[i];
    }
    pl->n = n;

    if (pl->shuffle) {
        uint16_t first = pl->order[start];

        /* current track first, the rest shuffled */
        pl->order[0] = first;
        pl->order[start] = idx[0];
        shuffle_range(pl, 1);
        pl->pos = 0;
    } else {
        pl->pos = start;
    }
    return n;
}

int playlist_count(const playlist_t *pl)
{
    return (pl == 0) ? 0 : pl->n;
}

int playlist_position(const playlist_t *pl)
{
    if (pl == 0 || pl->pos < 0 || pl->pos >= pl->n)
        return 0;
    return pl->pos + 1;
}

uint16_t playlist_current(const playlist_t *pl)
{
    if (pl == 0 || pl->pos < 0 || pl->pos >= pl->n)
        return PLAYLIST_IDX_NONE;
    return pl->order[pl->pos];
}

int playlist_advance(playlist_t *pl)
{
    if (pl == 0 || pl->n <= 0)
        return -1;

    if (pl->repeat == PLAYLIST_REPEAT_ONE)
        return pl->pos;                 /* replay the current track */

    if (pl->pos + 1 < pl->n) {
        pl->pos++;
        return pl->pos;
    }
    if (pl->repeat == PLAYLIST_REPEAT_ALL) {
        pl->pos = 0;
        return 0;
    }
    return -1;                          /* end of list, repeat off */
}

int playlist_next_manual(playlist_t *pl)
{
    if (pl == 0 || pl->n <= 0)
        return -1;
    if (pl->pos + 1 < pl->n) {
        pl->pos++;
        return pl->pos;
    }
    if (pl->repeat == PLAYLIST_REPEAT_ALL) {
        pl->pos = 0;
        return 0;
    }
    return -1;
}

void playlist_set_shuffle(playlist_t *pl, int on)
{
    uint16_t cur;

    if (pl == 0)
        return;
    on = on ? 1 : 0;
    if (on == pl->shuffle)
        return;

    cur = (pl->pos >= 0 && pl->pos < pl->n) ? pl->order[pl->pos]
                                            : PLAYLIST_IDX_NONE;
    pl->shuffle = on;

    if (on) {
        int i;

        for (i = 0; i < pl->n; i++)     /* restart from the user order */
            pl->order[i] = pl->orig[i];
        if (cur != PLAYLIST_IDX_NONE) {
            int at = find_val(pl, cur);

            if (at > 0) {
                uint16_t t = pl->order[0];

                pl->order[0] = pl->order[at];
                pl->order[at] = t;
            }
        }
        shuffle_range(pl, 1);           /* current stays first */
        pl->pos = (pl->n > 0) ? 0 : -1;
    } else {
        int i;

        for (i = 0; i < pl->n; i++)
            pl->order[i] = pl->orig[i];
        pl->pos = (cur == PLAYLIST_IDX_NONE) ? -1 : find_val(pl, cur);
    }
}

void playlist_set_repeat(playlist_t *pl, int mode)
{
    if (pl == 0)
        return;
    if (mode < PLAYLIST_REPEAT_OFF || mode > PLAYLIST_REPEAT_ONE)
        mode = PLAYLIST_REPEAT_OFF;
    pl->repeat = mode;
}
