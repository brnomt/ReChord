/*
 * player_state.c — persistent player state (see player_state.h).
 *
 * Storage is ONE fixed-size, versioned, CRC-protected file rewritten in
 * full (no rename on the target FAT backend). Parser/serializer is a
 * struct memcpy + a bitwise CRC-32: minimal, deterministic, no malloc.
 */
#include "player_state.h"
#include "player.h"
#include "fs.h"

/* Compile-time pin: the on-disk layout is the contract. */
typedef char ps_size_check[(sizeof(player_state_t) == 1180) ? 1 : -1];

/* ---- CRC-32 (IEEE, poly 0xEDB88320), bitwise — no table, no malloc ---- */

uint32_t player_state_crc32(const void *data, unsigned long len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFFu;
    unsigned long i;
    int b;

    for (i = 0; i < len; i++) {
        crc ^= p[i];
        for (b = 0; b < 8; b++)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return crc ^ 0xFFFFFFFFu;
}

/* ---- validation (also the corruption firewall) ------------------------- */

static void ps_terminate(char *s, int cap)
{
    s[cap - 1] = '\0';                     /* force NUL at the bound      */
}

static int ps_valid(const player_state_t *st)
{
    if (st->magic != PLAYER_STATE_MAGIC)
        return 0;
    if (st->version == 0u || st->version > PLAYER_STATE_VERSION)
        return 0;                          /* unknown layout: defaults    */
    if (st->crc32 != player_state_crc32(st, (unsigned long)((char *)&st->crc32 - (char *)st)))
        return 0;                          /* torn write / corruption     */
    return 1;
}

/* Clamp a loaded state into sane bounds (hostile input cannot steer the
 * player into invalid states). */
static void ps_sanitize(player_state_t *st)
{
    int i;

    ps_terminate(st->path, PS_PATH_MAX);
    for (i = 0; i < PS_QUEUE_MAX; i++)
        ps_terminate(st->queue[i], PS_PATH_MAX);
    if (st->queue_count > (uint32_t)PS_QUEUE_MAX)
        st->queue_count = (uint32_t)PS_QUEUE_MAX;
    if (st->volume_db < PLAYER_VOL_MIN_DB)
        st->volume_db = PLAYER_VOL_MIN_DB;
    if (st->volume_db > PLAYER_VOL_MAX_DB)
        st->volume_db = PLAYER_VOL_MAX_DB;
    if (st->position_ms < 0)
        st->position_ms = 0;
    if (st->duration_ms < 0)
        st->duration_ms = 0;
}

/* ---- core storage ------------------------------------------------------ */

int player_state_load(player_state_t *out)
{
    fs_file_t *f;
    player_state_t st;
    long got, size;

    if (out == 0)
        return PLAYER_STATE_ERR_INVALID;

    f = fs_open_read(PLAYER_STATE_PATH);
    if (f == 0)
        return PLAYER_STATE_ERR_INVALID;   /* absent: defaults            */
    size = fs_file_size(f);
    got = fs_read(f, &st, (unsigned long)sizeof st);
    fs_close(f);
    if (size != (long)sizeof st || got != (long)sizeof st)
        return PLAYER_STATE_ERR_INVALID;   /* short/torn: defaults        */
    if (!ps_valid(&st))
        return PLAYER_STATE_ERR_INVALID;
    ps_sanitize(&st);
    *out = st;
    return 0;
}

int player_state_save(const player_state_t *st)
{
    fs_file_t *f;
    player_state_t disk;
    long wrote;

    if (st == 0)
        return PLAYER_STATE_ERR_IO;

    disk = *st;
    ps_terminate(disk.path, PS_PATH_MAX);  /* never persist garbage       */
    disk.magic = PLAYER_STATE_MAGIC;
    disk.version = (uint16_t)PLAYER_STATE_VERSION;
    disk.crc32 = player_state_crc32(&disk, (unsigned long)((char *)&disk.crc32 - (char *)&disk));

    f = fs_open_write(PLAYER_STATE_PATH);
    if (f == 0)
        return PLAYER_STATE_ERR_IO;
    wrote = fs_write(f, &disk, (unsigned long)sizeof disk);
    fs_close(f);
    return (wrote == (long)sizeof disk) ? 0 : PLAYER_STATE_ERR_IO;
}

/* ---- runtime integration ---------------------------------------------- */

static player_state_t s_saved;             /* last save (drift baseline)  */
static player_state_t s_last;              /* last SAMPLED (change detect)*/
static int s_restored;
static int s_settle;                       /* quiet passes since a change */
static int s_dirty;                        /* structural change pending   */

/* Sample the live player into `out`. Returns 0 if there is nothing to
 * persist (no player yet). */
static int ps_sample(player_state_t *out)
{
    track_info_t ti;
    int i, n;

    for (i = 0; i < PS_PATH_MAX; i++)
        out->path[i] = 0;
    out->duration_ms = 0;
    if (player_get_track(&ti) == PLAYER_OK) {
        for (i = 0; i < PS_PATH_MAX - 1 && ti.path[i]; i++)
            out->path[i] = ti.path[i];
        out->path[i] = '\0';
        out->duration_ms = (int32_t)ti.duration_ms;
    }
    out->position_ms = (int32_t)player_get_position_ms();
    out->volume_db = player_get_volume();
    out->flags = (player_get_state() == PLAYER_PAUSED) ? PS_FLAG_PAUSED : 0u;

    n = player_queue_count();
    if (n > PS_QUEUE_MAX)
        n = PS_QUEUE_MAX;
    out->queue_count = (uint32_t)n;
    for (i = 0; i < PS_QUEUE_MAX; i++) {
        int k;
        for (k = 0; k < PS_PATH_MAX; k++)
            out->queue[i][k] = 0;
        if (i < n)
            (void)player_queue_get(i, out->queue[i], PS_PATH_MAX);
    }
    out->magic = PLAYER_STATE_MAGIC;
    out->version = (uint16_t)PLAYER_STATE_VERSION;
    return 0;
}

/* Structural fields (everything except the resume position and duration —
 * duration is re-derived from the codec on open and must not keep the
 * state "dirty" forever). A change in any of these deserves a prompt
 * debounced save; position drift is the heartbeat's business. */
static int ps_structural_changed(const player_state_t *a, const player_state_t *b)
{
    unsigned long i;

    if (a->flags != b->flags || a->volume_db != b->volume_db ||
        a->queue_count != b->queue_count)
        return 1;
    for (i = 0; i < PS_PATH_MAX; i++)
        if (a->path[i] != b->path[i])
            return 1;
    for (i = 0; i < (unsigned long)PS_QUEUE_MAX * PS_PATH_MAX; i++)
        if (((const char *)a->queue)[i] != ((const char *)b->queue)[i])
            return 1;
    return 0;
}

void player_state_restore(void)
{
    player_state_t st;
    int i;

    s_restored = 1;                        /* tick() must not redo this   */
    if (player_state_load(&st) != 0)
        return;                            /* absent/corrupt: defaults    */

    player_set_volume(st.volume_db);       /* cached; applied on the next
                                            * audio_output_configure      */
    for (i = 0; i < (int)st.queue_count; i++)
        (void)player_enqueue(st.queue[i]);
    if (st.path[0] != '\0' && player_open(st.path) == PLAYER_OK) {
        if (st.position_ms > 0)
            (void)player_seek(st.position_ms);
        /* Deliberately NOT auto-playing: the resume point is armed and
         * Play continues where the last session left off. */
    }
    s_saved = st;                          /* no immediate re-save        */
    s_last = st;
}

void player_state_tick(void)
{
    player_state_t cur;
    long drift;

    if (!s_restored) {
        s_restored = 1;
        player_state_restore();            /* first pass: SD is mounted   */
    }

    (void)ps_sample(&cur);

    /* Structural changes (track, volume, pause, queue) debounce against
     * the LAST SAMPLE, so a change stops counting once it stops moving:
     * save after PS_SETTLE_PASSES quiet passes. Position drift vs the
     * last SAVE is the heartbeat: one save per PS_HEARTBEAT_MS of
     * playback, so a running track costs at most one write per 30 s. */
    if (ps_structural_changed(&cur, &s_last)) {
        s_dirty = 1;
        s_settle = 0;
    } else if (s_settle < PS_SETTLE_PASSES) {
        s_settle++;
    }
    s_last = cur;

    drift = (long)cur.position_ms - (long)s_saved.position_ms;
    if (drift < 0)
        drift = -drift;

    if ((s_dirty && s_settle >= PS_SETTLE_PASSES) ||
        drift >= (long)PS_HEARTBEAT_MS) {
        if (player_state_save(&cur) == 0) {
            s_saved = cur;
            s_settle = PS_SETTLE_PASSES;
            s_dirty = 0;
        } else {
            s_settle = 0;                  /* storage gone: retry later   */
        }
    }
}

void player_state_sync(void)
{
    player_state_t cur;

    (void)ps_sample(&cur);
    if (player_state_save(&cur) == 0) {
        s_saved = cur;
        s_last = cur;
    }
}
