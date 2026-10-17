/*
 * test_player_persist.c — host tests for player_state.{c,h} persistence.
 *
 * Core: magic/version/CRC-validated save/load over fs_host, graceful
 * defaults on absent/torn/corrupt files. Policy: tick() debounces
 * structural changes and heartbeats on position drift. Restore: applies a
 * saved state to the player (open + seek + volume + queue).
 */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "player_state.h"
#include "player.h"
#include "mock_codec.h"
#include "fs.h"
#include "fs_host.h"

static int s_checks, s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

#define CHECK_EQI(got, want, name) do {                                    \
    s_checks++;                                                            \
    if ((got) != (want)) {                                                 \
        printf("FAIL %s:%d: %s (got %d want %d)\n", __FILE__, __LINE__,    \
               (name), (int)(got), (int)(want));                           \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

static void write_raw(const void *buf, unsigned long len)
{
    fs_file_t *f = fs_open_write(PLAYER_STATE_PATH);

    if (f) {
        fs_write(f, buf, len);
        fs_close(f);
    }
}

int main(void)
{
    player_state_t st, out;
    mock_codec_cfg_t cfg;
    int n;

    mkdir(".work", 0777);
    mkdir(".work/psroot", 0777);
    mkdir(".work/psroot/SD", 0777);
    fs_host_set_root(".work/psroot");
    remove(".work/psroot/SD/PLAYER.STA");        /* stale runs             */

    /* Long mock track so seeks to 5..40 s are in range. */
    memset(&cfg, 0, sizeof cfg);
    cfg.total_frames = 44100L * 120L;
    mock_codec_configure(&cfg);
    player_init();
    player_register_codec("mock", mock_codec_get());

    /* ---- core: absent file -> defaults ---- */
    CHECK(player_state_load(&out) == PLAYER_STATE_ERR_INVALID);

    /* ---- core: round trip ---- */
    memset(&st, 0, sizeof st);
    st.magic = PLAYER_STATE_MAGIC;
    st.version = (uint16_t)PLAYER_STATE_VERSION;
    st.flags = PS_FLAG_PAUSED;
    st.volume_db = -20;
    st.position_ms = 123456;
    st.duration_ms = 240000;
    st.queue_count = 2;
    memcpy(st.path, "SD/A.MOCK", 10);
    memcpy(st.queue[0], "SD/B.MOCK", 10);
    memcpy(st.queue[1], "SD/C.MOCK", 10);
    CHECK(player_state_save(&st) == 0);
    memset(&out, 0, sizeof out);
    CHECK(player_state_load(&out) == 0);
    CHECK(out.volume_db == -20);
    CHECK(out.position_ms == 123456);
    CHECK(out.duration_ms == 240000);
    CHECK(out.flags == PS_FLAG_PAUSED);
    CHECK(out.queue_count == 2);
    CHECK(strcmp(out.path, "SD/A.MOCK") == 0);
    CHECK(strcmp(out.queue[1], "SD/C.MOCK") == 0);

    /* ---- corruption firewall: torn / garbage / short / bad version ---- */
    memset(&st, 0x5A, sizeof st);                /* garbage: CRC fails     */
    write_raw(&st, sizeof st);
    CHECK(player_state_load(&out) == PLAYER_STATE_ERR_INVALID);
    memset(&st, 0, sizeof st);                   /* short file             */
    write_raw(&st, 100);
    CHECK(player_state_load(&out) == PLAYER_STATE_ERR_INVALID);
    memset(&st, 0, sizeof st);                   /* right size, bad magic  */
    st.magic = 0xDEADBEEFu;
    write_raw(&st, sizeof st);
    CHECK(player_state_load(&out) == PLAYER_STATE_ERR_INVALID);
    st.magic = PLAYER_STATE_MAGIC;               /* unknown future version */
    st.version = (uint16_t)(PLAYER_STATE_VERSION + 1);
    write_raw(&st, sizeof st);
    CHECK(player_state_load(&out) == PLAYER_STATE_ERR_INVALID);

    /* ---- restore applies state to the player ---- */
    memset(&st, 0, sizeof st);
    st.magic = PLAYER_STATE_MAGIC;
    st.version = (uint16_t)PLAYER_STATE_VERSION;
    st.volume_db = -12;
    st.position_ms = 5000;
    st.duration_ms = 120000;                     /* == the mock track     */
    st.queue_count = 1;
    memcpy(st.path, "SD/A.MOCK", 10);
    memcpy(st.queue[0], "SD/B.MOCK", 10);
    CHECK(player_state_save(&st) == 0);
    player_state_restore();
    {
        track_info_t ti;
        char q[PS_PATH_MAX];

        CHECK(player_get_track(&ti) == PLAYER_OK);
        CHECK(strcmp(ti.path, "SD/A.MOCK") == 0);
        CHECK(player_get_position_ms() == 5000);
        CHECK_EQI(player_get_volume(), -12, "restored volume");
        CHECK_EQI(player_queue_count(), 1, "restored queue");
        CHECK(player_queue_get(0, q, sizeof q) == PLAYER_OK);
        CHECK(strcmp(q, "SD/B.MOCK") == 0);
    }

    /* ---- policy: structural change saves after it settles ---- */
    CHECK(player_set_volume(-30) == PLAYER_OK);
    for (n = 0; n < 3; n++)
        player_state_tick();
    memset(&out, 0, sizeof out);
    CHECK(player_state_load(&out) == 0);
    CHECK(out.volume_db == -12);                 /* still debouncing       */
    for (n = 0; n < PS_SETTLE_PASSES + 2; n++)
        player_state_tick();
    CHECK(player_state_load(&out) == 0);
    CHECK(out.volume_db == -30);                 /* settled -> saved       */

    /* ---- policy: position drift heartbeats ---- */
    CHECK(player_seek(40000) == PLAYER_OK);      /* > PS_HEARTBEAT_MS away */
    player_state_tick();
    CHECK(player_state_load(&out) == 0);
    CHECK(out.position_ms >= 40000);             /* heartbeat saved        */

    printf("test_player_persist: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
