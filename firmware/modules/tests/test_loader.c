/*
 * test_loader.c — host tests for the RMF1 loader (firmware/modules/).
 *
 * Covers the mandatory verification matrix:
 *   - pack -> load round trip (in-process packer mirroring the format),
 *   - CRC corruption rejection (flip 1 byte),
 *   - bad magic / bad version rejection,
 *   - bss zeroing proof (prefilled window, exact ranges checked),
 *   - window overflow rejection,
 *   - length sanity and NULL-argument rejection.
 * Compile: cc -Wall -Werror (see tests/run_tests.sh).
 */
#include <stdlib.h>
#include <string.h>

#include "../module_loader.h"
#include "test_util.h"

/* Device-like module scale: 2980 B code + 20220 B bss (docs/re log facts). */
#define CODE_LEN 2980u
#define BSS_LEN 20220u

typedef struct {
    uint8_t *data;
    size_t len;
} blob_t;

/* Deterministic code pattern shared with the python builder test:
 * byte i = (i*7 + 3) & 0xFF. CRC-32 of 2980 such bytes is a known vector. */
static void fill_pattern(uint8_t *p, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        p[i] = (uint8_t)((i * 7u + 3u) & 0xFFu);
    }
}

static rmf1_header_t *blob_hdr(blob_t *b)
{
    return (rmf1_header_t *)(void *)b->data;
}

/* Build a structurally valid RMF1 blob with a correct CRC over the pattern. */
static void blob_build(blob_t *b, uint16_t id, uint32_t flags,
                       uint32_t code_len, uint32_t bss_start,
                       uint32_t bss_len, uint32_t entry_offset)
{
    rmf1_header_t h;

    b->len = RMF1_HEADER_SIZE + code_len;
    b->data = (uint8_t *)malloc(b->len);
    fill_pattern(b->data + RMF1_HEADER_SIZE, code_len);

    h.magic = RMF1_MAGIC;
    h.version = RMF1_VERSION;
    h.id = id;
    h.flags = flags;
    h.code_len = code_len;
    h.bss_start = bss_start;
    h.bss_len = bss_len;
    h.entry_offset = entry_offset;
    h.crc32 = rmf_crc32(b->data + RMF1_HEADER_SIZE, code_len);
    memcpy(b->data, &h, sizeof h);
}

static void blob_free(blob_t *b)
{
    free(b->data);
    b->data = NULL;
    b->len = 0;
}

static uint8_t *window_alloc(size_t size)
{
    uint8_t *w = (uint8_t *)malloc(size);
    memset(w, 0xAA, size); /* poison: proves exactly what the loader writes */
    return w;
}

static int region_all_is(const uint8_t *p, size_t n, uint8_t val)
{
    size_t i;
    for (i = 0; i < n; i++) {
        if (p[i] != val) {
            return 0;
        }
    }
    return 1;
}

/* --- tests ------------------------------------------------------------- */

static void test_crc32_kat(void)
{
    uint8_t full[256];
    uint8_t *pat;
    size_t i;

    for (i = 0; i < 256; i++) {
        full[i] = (uint8_t)i;
    }

    printf("test_crc32_kat\n");
    CHECK_EQ_U32(rmf_crc32("123456789", 9), 0x889a9615u, "CRC KAT 123456789");
    CHECK_EQ_U32(rmf_crc32("", 0), 0u, "CRC KAT empty input");
    CHECK_EQ_U32(rmf_crc32(full, 256), 0x141e59fcu, "CRC KAT bytes 0..255");

    pat = (uint8_t *)malloc(CODE_LEN);
    fill_pattern(pat, CODE_LEN);
    CHECK_EQ_U32(rmf_crc32(pat, CODE_LEN), 0x33b4999cu,
                 "CRC KAT 2980-byte pattern (device-scale module)");
    free(pat);
}

static void test_roundtrip(void)
{
    blob_t b;
    uint8_t *win;
    rmf_loaded_t m;
    rmf_result_t r;
    const uint32_t win_size = CODE_LEN + BSS_LEN;

    printf("test_roundtrip\n");
    blob_build(&b, 44, 0, CODE_LEN, CODE_LEN, BSS_LEN, 4);
    win = window_alloc(win_size);

    r = rmf_load_module(b.data, b.len, win, win_size, &m);
    CHECK(r == RMF_OK, "load returns RMF_OK");
    CHECK_EQ_U32(m.id, 44u, "loaded id matches");
    CHECK_EQ_U32(m.flags, 0u, "loaded flags match");
    CHECK(m.base == win, "base == window");
    CHECK_EQ_U32(m.code_len, CODE_LEN, "code_len matches");
    CHECK_EQ_U32(m.bss_start, CODE_LEN, "bss_start matches");
    CHECK_EQ_U32(m.bss_len, BSS_LEN, "bss_len matches");
    CHECK(m.entry == (uintptr_t)win + 4u + 1u,
          "entry == base + entry_offset with Thumb bit set");
    CHECK(memcmp(win, b.data + RMF1_HEADER_SIZE, CODE_LEN) == 0,
          "code bytes deployed byte-exact");

    free(win);
    blob_free(&b);
}

static void test_bss_zeroing(void)
{
    /*
     * Layout probe: code 64 B at [0,64), 8-byte gap [64,72) the loader must
     * NOT touch, bss [72,104), and [104,200) must stay poisoned.
     */
    blob_t b;
    uint8_t *win;
    rmf_result_t r;

    printf("test_bss_zeroing\n");
    blob_build(&b, 7, 0, 64u, 72u, 32u, 0u);
    win = window_alloc(200u);

    r = rmf_load_module(b.data, b.len, win, 200u, NULL);
    CHECK(r == RMF_OK, "load with gap+bss returns RMF_OK");
    CHECK(memcmp(win, b.data + RMF1_HEADER_SIZE, 64) == 0,
          "code range holds the code");
    CHECK(region_all_is(win + 64, 8, 0xAA),
          "gap [code_len, bss_start) untouched");
    CHECK(region_all_is(win + 72, 32, 0x00), "bss range fully zeroed");
    CHECK(region_all_is(win + 104, 96, 0xAA),
          "bytes past bss end untouched");

    free(win);
    blob_free(&b);
}

static void test_crc_corruption(void)
{
    blob_t b;
    uint8_t *win;
    rmf_result_t r;
    const uint32_t win_size = 64u;

    printf("test_crc_corruption\n");
    blob_build(&b, 1, 0, 32u, 32u, 32u, 0u);
    win = window_alloc(win_size);

    /* Flip exactly 1 byte of code. */
    b.data[RMF1_HEADER_SIZE + 5] ^= 0x01u;
    r = rmf_load_module(b.data, b.len, win, win_size, NULL);
    CHECK(r == RMF_ERR_CRC, "1-byte code flip rejected with RMF_ERR_CRC");
    CHECK(region_all_is(win, win_size, 0xAA),
          "window untouched after failed verify");

    /* Restore, then corrupt the stored CRC field instead. */
    b.data[RMF1_HEADER_SIZE + 5] ^= 0x01u;
    blob_hdr(&b)->crc32 ^= 0x00000001u;
    r = rmf_load_module(b.data, b.len, win, win_size, NULL);
    CHECK(r == RMF_ERR_CRC, "corrupt crc32 field rejected with RMF_ERR_CRC");
    blob_hdr(&b)->crc32 ^= 0x00000001u;

    /* One flip in the bss metadata cannot hide: code CRC still ok, but the
     * deployed bss range would differ — here we only guard the flag path. */
    blob_free(&b);
    free(win);
}

static void test_bad_magic_version_flags(void)
{
    blob_t b;
    rmf_result_t r;

    printf("test_bad_magic_version_flags\n");
    blob_build(&b, 1, 0, 32u, 32u, 0u, 0u);

    blob_hdr(&b)->magic = 0x31464d53u; /* 'RMF2' */
    {
        uint8_t *win = window_alloc(64u);
        r = rmf_load_module(b.data, b.len, win, 64u, NULL);
        CHECK(r == RMF_ERR_MAGIC, "bad magic rejected with RMF_ERR_MAGIC");
        free(win);
    }

    blob_hdr(&b)->magic = RMF1_MAGIC;
    blob_hdr(&b)->version = (uint16_t)(RMF1_VERSION + 1u);
    {
        uint8_t *win = window_alloc(64u);
        r = rmf_load_module(b.data, b.len, win, 64u, NULL);
        CHECK(r == RMF_ERR_VERSION, "bad version rejected with RMF_ERR_VERSION");
        free(win);
    }

    blob_hdr(&b)->version = RMF1_VERSION;
    blob_hdr(&b)->flags = 0x00000001u;
    {
        uint8_t *win = window_alloc(64u);
        r = rmf_load_module(b.data, b.len, win, 64u, NULL);
        CHECK(r == RMF_ERR_FLAGS, "unknown flag bit rejected with RMF_ERR_FLAGS");
        free(win);
    }

    blob_free(&b);
}

static void test_bad_length(void)
{
    blob_t b;
    uint8_t *win;
    rmf_result_t r;

    printf("test_bad_length\n");
    win = window_alloc(256u);

    /* Header itself truncated. */
    blob_build(&b, 1, 0, 32u, 32u, 0u, 0u);
    r = rmf_load_module(b.data, RMF1_HEADER_SIZE - 1u, win, 256u, NULL);
    CHECK(r == RMF_ERR_LENGTH, "truncated header -> RMF_ERR_LENGTH");

    /* Payload shorter than code_len claims. */
    r = rmf_load_module(b.data, b.len - 1u, win, 256u, NULL);
    CHECK(r == RMF_ERR_LENGTH, "truncated payload -> RMF_ERR_LENGTH");

    /* code_len == 0. */
    blob_hdr(&b)->code_len = 0;
    blob_hdr(&b)->entry_offset = 0;
    r = rmf_load_module(b.data, b.len, win, 256u, NULL);
    CHECK(r == RMF_ERR_LENGTH, "zero code_len -> RMF_ERR_LENGTH");

    /* entry_offset beyond the code. */
    blob_hdr(&b)->code_len = 32u;
    blob_hdr(&b)->entry_offset = 32u;
    r = rmf_load_module(b.data, b.len, win, 256u, NULL);
    CHECK(r == RMF_ERR_LENGTH, "entry past code -> RMF_ERR_LENGTH");

    /* entry_offset odd (Thumb bit must not be stored in the file). */
    blob_hdr(&b)->entry_offset = 3u;
    r = rmf_load_module(b.data, b.len, win, 256u, NULL);
    CHECK(r == RMF_ERR_LENGTH, "odd entry offset -> RMF_ERR_LENGTH");

    /* bss overlapping the code. */
    blob_hdr(&b)->entry_offset = 0u;
    blob_hdr(&b)->bss_start = 28u;
    r = rmf_load_module(b.data, b.len, win, 256u, NULL);
    CHECK(r == RMF_ERR_LENGTH, "bss overlapping code -> RMF_ERR_LENGTH");

    /* bss not 4-aligned. */
    blob_hdr(&b)->bss_start = 34u;
    r = rmf_load_module(b.data, b.len, win, 256u, NULL);
    CHECK(r == RMF_ERR_LENGTH, "unaligned bss_start -> RMF_ERR_LENGTH");

    /* Hostile u32 lengths must not wrap past the checks. */
    blob_hdr(&b)->bss_start = 32u;
    blob_hdr(&b)->code_len = 0xfffffff0u;
    r = rmf_load_module(b.data, b.len, win, 256u, NULL);
    CHECK(r == RMF_ERR_LENGTH, "huge code_len -> RMF_ERR_LENGTH");

    blob_free(&b);
    free(win);
}

static void test_window_overflow(void)
{
    blob_t b;
    uint8_t *win;
    rmf_result_t r;

    printf("test_window_overflow\n");
    blob_build(&b, 1, 0, 64u, 64u, 32u, 0u); /* needs a 96-byte window */

    win = window_alloc(63u);
    r = rmf_load_module(b.data, b.len, win, 63u, NULL);
    CHECK(r == RMF_ERR_WINDOW, "code does not fit -> RMF_ERR_WINDOW");
    free(win);

    win = window_alloc(95u);
    r = rmf_load_module(b.data, b.len, win, 95u, NULL);
    CHECK(r == RMF_ERR_WINDOW, "bss past window end -> RMF_ERR_WINDOW");
    free(win);

    /* Hostile bss_len near UINT32_MAX must fail as window overflow, not
     * wrap around (64-bit intermediate sums). */
    blob_hdr(&b)->bss_len = 0xffffffffu;
    win = window_alloc(96u);
    r = rmf_load_module(b.data, b.len, win, 96u, NULL);
    CHECK(r == RMF_ERR_WINDOW, "huge bss_len -> RMF_ERR_WINDOW (no wrap)");
    free(win);

    blob_free(&b);
}

static void test_null_args(void)
{
    blob_t b;
    uint8_t *win;
    rmf_result_t r;

    printf("test_null_args\n");
    blob_build(&b, 1, 0, 32u, 32u, 0u, 0u);
    win = window_alloc(64u);

    r = rmf_load_module(NULL, b.len, win, 64u, NULL);
    CHECK(r == RMF_ERR_ARG, "NULL blob -> RMF_ERR_ARG");
    r = rmf_load_module(b.data, b.len, NULL, 64u, NULL);
    CHECK(r == RMF_ERR_ARG, "NULL window -> RMF_ERR_ARG");

    /* out == NULL is legal: load for side effect only. */
    r = rmf_load_module(b.data, b.len, win, 64u, NULL);
    CHECK(r == RMF_OK, "NULL out is accepted");

    blob_free(&b);
    free(win);
}

int main(void)
{
    test_crc32_kat();
    test_roundtrip();
    test_bss_zeroing();
    test_crc_corruption();
    test_bad_magic_version_flags();
    test_bad_length();
    test_window_overflow();
    test_null_args();

    printf("test_loader: %d failure(s)\n", failures);
    return failures;
}
