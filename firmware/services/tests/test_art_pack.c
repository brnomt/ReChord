/*
 * test_art_pack.c — host test for services/art_pack.c (precomputed album art).
 *
 * Builds an ARTPACK.BIN by hand (same layout the Python tool writes), then
 * exercises: key vectors shared with tools/artpack, header/CRC validation,
 * binary search, palette load, pixel streaming + clipping through the real
 * display driver into panel_io_sim, and every failure mode (missing file, bad
 * magic, bad header/key CRC, key not found, pack replaced under us).
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "art_pack.h"
#include "fs.h"
#include "fs_host.h"
#include "../../drivers/display.h"
#include "../../drivers/panel_io_sim.h"

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

#define PACK_PATH "/Internal/.rechord/ARTPACK.BIN"
#define TRACK     "/Internal/Music/x.wav"
#define TRACK_LEN 100

#define PACK_LARGE 8
#define PACK_SMALL 4
#define IMG_SLOTS  3

static uint8_t s_pack[4096];
static uint32_t s_pack_len;

/* ---- little-endian writers + CRC (zlib convention) ---------------------- */

static void wr16(uint32_t off, uint32_t v)
{
    s_pack[off] = (uint8_t)v;
    s_pack[off + 1] = (uint8_t)(v >> 8);
}

static void wr32(uint32_t off, uint32_t v)
{
    s_pack[off] = (uint8_t)v;
    s_pack[off + 1] = (uint8_t)(v >> 8);
    s_pack[off + 2] = (uint8_t)(v >> 16);
    s_pack[off + 3] = (uint8_t)(v >> 24);
}

static uint32_t crc_of(const uint8_t *p, uint32_t n)
{
    uint32_t crc = 0xFFFFFFFFu;

    while (n--) {
        int i;

        crc ^= *p++;
        for (i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)-(int32_t)(crc & 1u));
    }
    return crc ^ 0xFFFFFFFFu;
}

static uint32_t align512(uint32_t n)
{
    return (n + 511u) & ~511u;
}

static uint16_t pixel_at(int x, int y)
{
    return (uint16_t)(0x1000 + (y * PACK_LARGE + x));
}

/* Build the pack for three keys; `mid` is the real track's key. */
static void build_pack(uint64_t mid, uint32_t c1, uint32_t c2, uint32_t mean,
                       uint32_t accent, int corrupt_key_crc)
{
    uint32_t key_off = 512;
    uint32_t key_bytes = align512(3 * 16);
    uint32_t img_off = key_off + key_bytes;
    uint32_t img_bytes = align512(IMG_SLOTS * 16);
    uint32_t small_off = img_off + img_bytes;
    uint32_t small_bytes = align512(PACK_SMALL * PACK_SMALL * 2);
    uint32_t large_off = small_off + small_bytes;
    uint32_t large_bytes = align512(PACK_LARGE * PACK_LARGE * 2);
    int i;

    memset(s_pack, 0, sizeof s_pack);
    s_pack_len = large_off + large_bytes;

    /* keys, ascending */
    {
        uint64_t keys[3];
        int n;

        keys[0] = mid - 2u;
        keys[1] = mid;
        keys[2] = mid + 2u;
        for (n = 0; n < 3; n++) {
            for (i = 0; i < 8; i++)
                s_pack[key_off + n * 16 + i] =
                    (uint8_t)(keys[n] >> (8 * i));
            wr32(key_off + n * 16 + 8, 0);      /* image 0 */
        }
    }

    /* image 0: offsets + palette */
    wr32(img_off + 0, 0);
    wr32(img_off + 4, 0);
    wr16(img_off + 8, c1);
    wr16(img_off + 10, c2);
    wr16(img_off + 12, mean);
    wr16(img_off + 14, accent);

    /* pixels: small then large, LE RGB565 */
    for (i = 0; i < PACK_SMALL * PACK_SMALL; i++)
        wr16(small_off + (uint32_t)i * 2, 0x0100 + i);
    for (i = 0; i < PACK_LARGE * PACK_LARGE; i++)
        wr16(large_off + (uint32_t)i * 2, 0x1000 + i);

    /* header */
    memcpy(s_pack, "ARTP", 4);
    wr32(4, 1);
    wr32(8, 512);
    wr32(12, 1);                       /* dither flag */
    wr16(16, PACK_LARGE);
    wr16(18, PACK_SMALL);
    wr32(20, 3);                       /* tracks */
    wr32(24, 1);                       /* images */
    wr32(28, IMG_SLOTS);
    wr32(32, IMG_SLOTS - 1);
    wr32(36, key_off);
    wr32(40, key_bytes);
    wr32(44, img_off);
    wr32(48, img_bytes);
    wr32(52, small_off);
    wr32(56, small_bytes);
    wr32(60, large_off);
    wr32(64, large_bytes);
    wr32(68, crc_of(s_pack + key_off, key_bytes) ^
             (corrupt_key_crc ? 1u : 0u));
    wr32(72, crc_of(s_pack + img_off, img_bytes));
    wr32(76, crc_of(s_pack + small_off, small_bytes));
    wr32(80, crc_of(s_pack + large_off, large_bytes));
    wr32(84, 0);
    wr32(84, crc_of(s_pack, 512));
}

static void write_file(const char *path, const void *data, uint32_t len)
{
    fs_file_t *f = fs_open_write(path);

    if (f == 0) {
        printf("FAIL setup: cannot create %s\n", path);
        return;
    }
    if (len > 0)
        (void)fs_write(f, data, len);
    fs_close(f);
}

static void make_fixture(uint64_t mid)
{
    uint8_t zeros[TRACK_LEN];

    (void)mkdir(".work", 0777);
    (void)mkdir(".work/Internal", 0777);
    (void)mkdir(".work/Internal/Music", 0777);
    (void)mkdir(".work/Internal/.rechord", 0777);
    fs_host_set_root(".work");

    memset(zeros, 0, sizeof zeros);
    write_file(TRACK, zeros, sizeof zeros);
    build_pack(mid, 0x1234, 0x5678, 0x9ABC, 0xDEF0, 0);
    write_file(PACK_PATH, s_pack, s_pack_len);
}

static void clear_screen(void)
{
    display_clear(0);
    display_flush_rows(0, DISPLAY_H - 1);
}

int main(void)
{
    uint64_t key;

    printf("== art_pack: keys, lookup, streaming, failures ==\n");

    /* ---- key vectors shared with tools/artpack/test_artpack.py ---------- */
    CHECK(art_pack_key("Internal/Music/Album/01 Track.flac", 1234567) ==
          0xECA81DB7A41F0270ull);
    CHECK(art_pack_key("Internal/Music/Album/02 Track.mp3", 42) ==
          0x12C3628A14C79B82ull);
    CHECK(art_pack_key("SD/Other/song.wav", 0) ==
          0x7CD2E6CFF42D87CDull);
    CHECK(art_pack_key("Internal/x.flac", 0xFFFFFFFFu) ==
          0x976542C384FDA54Dull);
    /* the device path carries a leading '/', the key does not */
    CHECK(art_pack_key("/Internal/x.flac", 0xFFFFFFFFu) ==
          art_pack_key("Internal/x.flac", 0xFFFFFFFFu));

    key = art_pack_key(TRACK, TRACK_LEN);
    make_fixture(key);

    /* ---- lookup + palette ---------------------------------------------- */
    art_pack_clear();
    CHECK(art_pack_ok() == 0);
    CHECK(art_pack_draw(0, 0, ART_SIZE_LARGE) < 0);
    CHECK(art_pack_set_track(TRACK) == 0);
    CHECK(art_pack_ok() == 1);
    CHECK(art_pack_color(ART_C_DOMINANT) == 0x1234);
    CHECK(art_pack_color(ART_C_SECONDARY) == 0x5678);
    CHECK(art_pack_color(ART_C_MEAN) == 0x9ABC);
    CHECK(art_pack_color(ART_C_ACCENT) == 0xDEF0);
    CHECK(art_pack_dim(ART_SIZE_LARGE) == PACK_LARGE);
    CHECK(art_pack_dim(ART_SIZE_SMALL) == PACK_SMALL);

    /* ---- streaming + clipping ------------------------------------------ */
    clear_screen();
    CHECK(art_pack_draw(10, 10, ART_SIZE_LARGE) == 0);
    display_flush_rows(0, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(10, 10) == pixel_at(0, 0));
    CHECK(panel_io_sim_pixel(17, 10) == pixel_at(7, 0));
    CHECK(panel_io_sim_pixel(10, 17) == pixel_at(0, 7));
    CHECK(panel_io_sim_pixel(17, 17) == pixel_at(7, 7));
    CHECK(panel_io_sim_pixel(9, 10) == 0);       /* untouched left  */
    CHECK(panel_io_sim_pixel(18, 17) == 0);      /* untouched right */

    clear_screen();
    CHECK(art_pack_draw(-3, -2, ART_SIZE_LARGE) == 0);
    display_flush_rows(0, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(0, 0) == pixel_at(3, 2));
    CHECK(panel_io_sim_pixel(4, 0) == pixel_at(7, 2));

    clear_screen();
    CHECK(art_pack_draw(DISPLAY_W - 3, DISPLAY_H - 2, ART_SIZE_LARGE) == 0);
    display_flush_rows(0, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(DISPLAY_W - 1, DISPLAY_H - 1) ==
          pixel_at(2, 1));

    clear_screen();
    CHECK(art_pack_draw(400, 400, ART_SIZE_LARGE) == 0);   /* off-screen */
    display_flush_rows(0, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(0, 0) == 0);

    clear_screen();
    CHECK(art_pack_draw(10, 10, ART_SIZE_SMALL) == 0);
    display_flush_rows(0, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(10, 10) == 0x0100);

    /* ---- key not found -------------------------------------------------- */
    CHECK(art_pack_set_track("/Internal/Music/other.wav") < 0);
    CHECK(art_pack_ok() == 0);
    CHECK(art_pack_draw(0, 0, ART_SIZE_LARGE) < 0);

    /* ---- missing pack --------------------------------------------------- */
    (void)remove(".work/Internal/.rechord/ARTPACK.BIN");
    CHECK(art_pack_set_track(TRACK) < 0);
    CHECK(art_pack_ok() == 0);

    /* ---- bad magic ------------------------------------------------------ */
    build_pack(key, 0x1234, 0x5678, 0x9ABC, 0xDEF0, 0);
    s_pack[0] = 'X';
    write_file(PACK_PATH, s_pack, s_pack_len);
    CHECK(art_pack_set_track(TRACK) < 0);

    /* ---- corrupted header (CRC catches it) ------------------------------ */
    build_pack(key, 0x1234, 0x5678, 0x9ABC, 0xDEF0, 0);
    s_pack[12] ^= 0x02;                 /* flags byte, CRC not refreshed */
    write_file(PACK_PATH, s_pack, s_pack_len);
    CHECK(art_pack_set_track(TRACK) < 0);

    /* ---- corrupted key table ------------------------------------------- */
    build_pack(key, 0x1234, 0x5678, 0x9ABC, 0xDEF0, 1);
    write_file(PACK_PATH, s_pack, s_pack_len);
    CHECK(art_pack_set_track(TRACK) < 0);

    /* ---- pack replaced between lookup and draw -------------------------- */
    build_pack(key, 0x1234, 0x5678, 0x9ABC, 0xDEF0, 0);
    write_file(PACK_PATH, s_pack, s_pack_len);
    CHECK(art_pack_set_track(TRACK) == 0);
    build_pack(key, 0x4321, 0x8765, 0xCBA9, 0x0FED, 0);
    write_file(PACK_PATH, s_pack, s_pack_len);
    CHECK(art_pack_draw(0, 0, ART_SIZE_LARGE) < 0);
    CHECK(art_pack_ok() == 0);          /* cleared, not stale */

    /* ---- track file missing (no size -> no key) ------------------------ */
    build_pack(key, 0x1234, 0x5678, 0x9ABC, 0xDEF0, 0);
    write_file(PACK_PATH, s_pack, s_pack_len);
    (void)remove(".work/Internal/Music/x.wav");
    CHECK(art_pack_set_track(TRACK) < 0);

    if (s_failures == 0)
        printf("test_art_pack: %d checks, OK\n", s_checks);
    else
        printf("test_art_pack: %d checks, %d FAILURES\n", s_checks,
               s_failures);
    return s_failures ? 1 : 0;
}
