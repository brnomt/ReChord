/*
 * playlist.h — ordered track list for the player (BRNOMT-90).
 *
 * WHY indices, not paths: a path is up to 128 bytes, so a 706-track "All Songs"
 * list of paths would be ~90 KB — far more than the AP has free. The media
 * index already assigns every scanned file a small record index, so the
 * playlist stores uint16_t record indices (PLAYLIST_MAX * 2 B = 2 KB) and the
 * player resolves an index to a path through a callback the app installs
 * (player_set_resolver() -> media_db_get()).
 *
 * TWO arrays: `orig` is the user's order (as handed in), `order` is the play
 * order. Shuffle reorders `order` (Rockbox-style: it reorders the list rather
 * than picking a random next); turning shuffle off restores `orig`.
 *
 * Pure C, no I/O — host-testable on its own.
 */
#ifndef RECHORD_PLAYLIST_H
#define RECHORD_PLAYLIST_H

#include <stdint.h>

#define PLAYLIST_MAX 1024
#define PLAYLIST_IDX_NONE 0xFFFFu

/* Repeat modes (player_set_repeat). */
enum {
    PLAYLIST_REPEAT_OFF = 0,   /* stop at the end of the list   */
    PLAYLIST_REPEAT_ALL = 1,   /* wrap to the first track        */
    PLAYLIST_REPEAT_ONE = 2    /* replay the current track      */
};

typedef struct {
    uint16_t orig[PLAYLIST_MAX];    /* user order                    */
    uint16_t order[PLAYLIST_MAX];   /* play order (shuffled or ==orig) */
    int      n;                     /* track count                    */
    int      pos;                   /* current position [0,n) or -1   */
    int      repeat;                /* PLAYLIST_REPEAT_*              */
    int      shuffle;               /* 0/1                            */
    uint32_t rng;                   /* xorshift32 state               */
} playlist_t;

/* Reset to empty (repeat/shuffle keep their current values only if set after). */
void playlist_reset(playlist_t *pl);

/* Seed the shuffle PRNG (e.g. from a tick counter). Optional. */
void playlist_seed(playlist_t *pl, uint32_t seed);

/* Install `n` indices and pick the starting track.
 *  - shuffle off: play order == user order, position = `start`.
 *  - shuffle on : the starting track goes first, the rest are shuffled.
 * Returns the count actually stored (0 when n <= 0). Clamps to PLAYLIST_MAX. */
int playlist_load(playlist_t *pl, const uint16_t *idx, int n, int start);

int      playlist_count(const playlist_t *pl);
int      playlist_position(const playlist_t *pl);   /* 1-based, 0 when empty */
uint16_t playlist_current(const playlist_t *pl);    /* PLAYLIST_IDX_NONE if none */

/* Advance to the next track at end-of-stream, honouring repeat:
 *   ONE -> stays on the current track;
 *   ALL -> wraps to the first after the last;
 *   OFF -> returns -1 (stop) at the end.
 * On a real advance pl->pos is updated; returns the new position, or -1. */
int playlist_advance(playlist_t *pl);

/* 1 when playlist_advance() would advance rather than end the list: a real
 * next track, a repeat-ALL wrap, or a repeat-ONE replay. Pure — does not
 * mutate `pl`. The player uses it to decide, at end-of-stream, whether to
 * hand the next track off (gapless) or drain to END_OF_TRACK. */
int playlist_has_next(const playlist_t *pl);

/* The record index the next auto-advance WOULD play, without moving the list.
 * Returns 1 and writes *out when there is a next track (a real next, a
 * repeat-ALL wrap, or a repeat-ONE replay), 0 otherwise. Used to pre-open the
 * next file while the current one still plays (BRNOMT-73). */
int playlist_peek_next(const playlist_t *pl, uint16_t *out);

/* Manual "next" (a key press): always moves forward, ignoring repeat-ONE
 * (which only applies to end-of-stream). Wraps only when repeat == ALL.
 * Returns the new position, or -1 to stop. */
int playlist_next_manual(playlist_t *pl);

/* Turn shuffle on/off. On enables: the CURRENT track is moved to the front of
 * a freshly shuffled order (so it keeps playing). Off restores the user
 * order and re-points the position at the current track. */
void playlist_set_shuffle(playlist_t *pl, int on);

/* Set the repeat mode (PLAYLIST_REPEAT_*). */
void playlist_set_repeat(playlist_t *pl, int mode);

#endif /* RECHORD_PLAYLIST_H */
