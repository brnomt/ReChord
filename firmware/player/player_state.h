/*
 * player_state.h — persistent player state on the SD card filesystem.
 *
 * One self-contained, versioned, CRC-protected file on the DEVICE FLASH
 * (eMMC "Internal" user area — NOT the SD card, so state survives card
 * swaps): the resume point (track path + position), playback context
 * (queue, duration, paused flag) and the user volume. Loaded once at
 * boot (lazy, see player_state_tick), synced back on a
 * debounced/heartbeat policy and on shutdown.
 *
 * FILE FORMAT (little-endian, packed, fixed size):
 *   magic 'PSTA' | version | flags | volume_db | position_ms | duration_ms
 *   | queue_count | path[128] | queue[8][128] | crc32
 * The CRC covers every byte before it; a torn write (power cut) fails
 * magic/version/size/CRC validation and the loader falls back to
 * defaults. Extending the schema = add fields before crc32 and bump
 * PLAYER_STATE_VERSION; the loader accepts the versions it knows and
 * ignores unknown newer files (graceful defaults).
 *
 * ATOMICITY: the target FAT backend has no fs_rename (FS_ERR_UNSUPPORTED
 * — the feat/config-logging branch proved tmp+rename unusable on
 * device), so the strategy is full-file rewrite + magic/version/CRC
 * validation on load. Worst case a power cut loses the last save; it can
 * never brick boot (load is bounded and falls back).
 *
 * BARE-METAL RULES: no malloc, no floats, fixed-size struct, minimal
 * deterministic serializer.
 */
#ifndef RECHORD_PLAYER_STATE_H
#define RECHORD_PLAYER_STATE_H

#include <stdint.h>

#define PLAYER_STATE_MAGIC    0x41545350u   /* 'PSTA' little-endian      */
#define PLAYER_STATE_VERSION  1u

/* Device flash: eMMC user area ("Internal" — fs_target_mount(0, ...) in
 * ap_ui). Deliberately NOT the SD card: the state must survive card
 * swaps; the track paths inside still point at SD content. */
#define PLAYER_STATE_PATH     "Internal/PLAYER.CFG"

#define PS_PATH_MAX           128           /* == PLAYER_PATH_MAX         */
#define PS_QUEUE_MAX          8             /* == PLAYER_QUEUE_LEN        */

#define PS_FLAG_PAUSED        0x0001u       /* was paused at last save    */

/* Write policy: structural changes (track/volume/pause/queue) are saved
 * after they settle for PS_SETTLE_PASSES quiet UI-loop passes; position
 * drifts trigger at most one save per PS_HEARTBEAT_MS of playback. */
#define PS_SETTLE_PASSES      60
#define PS_HEARTBEAT_MS       30000

/* On-disk struct. All fields naturally aligned; sizeof == 1180. */
typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t flags;
    int32_t  volume_db;                     /* player volume (dB)          */
    int32_t  position_ms;                   /* resume point                */
    int32_t  duration_ms;
    uint32_t queue_count;                   /* entries used in queue[]     */
    char     path[PS_PATH_MAX];             /* NUL-terminated, may be ""   */
    char     queue[PS_QUEUE_MAX][PS_PATH_MAX];
    uint32_t crc32;                         /* CRC of all preceding bytes  */
} player_state_t;

#define PLAYER_STATE_ERR_IO     (-2)  /* storage missing/unwritable       */
#define PLAYER_STATE_ERR_INVALID (-1) /* absent/torn/unknown: use defaults */

/* Core storage: validate + read. Returns 0, PLAYER_STATE_ERR_*. On any
 * failure `out` is left untouched. */
int player_state_load(player_state_t *out);

/* Core storage: write `st` (magic/version/crc filled in). Returns 0 or
 * PLAYER_STATE_ERR_IO. */
int player_state_save(const player_state_t *st);

/* CRC-32 (IEEE, poly 0xEDB88320) — exported for tests. */
uint32_t player_state_crc32(const void *data, unsigned long len);

/* Runtime integration ---------------------------------------------------- */

/* Load the file and apply it to the player (open + seek resume point,
 * set volume, re-enqueue queue). No-op if the file is absent/invalid.
 * Called lazily on the first player_state_tick(); safe to call directly. */
void player_state_restore(void);

/* One call per UI loop pass: samples player state, saves when the
 * debounced change settles (PS_SETTLE_PASSES quiet passes), when the
 * position drifts PS_HEARTBEAT_MS since the last save, or on the first
 * pass (restore). Cheap and allocation-free. */
void player_state_tick(void);

/* Force a save now (shutdown / power_off). Best-effort. */
void player_state_sync(void);

#endif /* RECHORD_PLAYER_STATE_H */
