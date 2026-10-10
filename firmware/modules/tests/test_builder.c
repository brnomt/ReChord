/*
 * test_builder.c — round-trip test: an RMF1 file produced by the OFFLINE
 * BUILDER (module_builder.py) must load cleanly through the C loader.
 *
 * This is the cross-implementation check that matters most: the builder's
 * CRC-32 and header serialization must agree byte-for-byte with the C
 * loader's verifier. run_tests.sh packs a module and passes its path:
 *
 *     test_builder <module.rmf1>
 *
 * Compile: cc -Wall -Werror (see tests/run_tests.sh).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../module_loader.h"
#include "test_util.h"

static uint8_t *read_file(const char *path, size_t *out_len)
{
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long size;

    if (f == NULL) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (size = ftell(f)) < 0 ||
        fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    buf = (uint8_t *)malloc((size_t)size);
    if (buf == NULL || fread(buf, 1, (size_t)size, f) != (size_t)size) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_len = (size_t)size;
    return buf;
}

int main(int argc, char **argv)
{
    uint8_t *blob;
    size_t blob_len = 0;
    rmf1_header_t h;
    rmf_loaded_t m;
    uint8_t *win;
    size_t win_size;

    if (argc != 2) {
        fprintf(stderr, "usage: test_builder <module.rmf1>\n");
        return 100;
    }
    blob = read_file(argv[1], &blob_len);
    if (blob == NULL) {
        fprintf(stderr, "cannot read %s\n", argv[1]);
        return 100;
    }
    if (blob_len < RMF1_HEADER_SIZE) {
        fprintf(stderr, "%s shorter than a header\n", argv[1]);
        free(blob);
        return 100;
    }
    memcpy(&h, blob, sizeof h);

    printf("test_builder: %s (id %u, code %u, bss %u..%u, entry %u)\n",
           argv[1], (unsigned)h.id, (unsigned)h.code_len,
           (unsigned)h.bss_start, (unsigned)(h.bss_start + h.bss_len),
           (unsigned)h.entry_offset);

    win_size = (size_t)h.bss_start + (size_t)h.bss_len;
    win = (uint8_t *)malloc(win_size);
    memset(win, 0xAA, win_size);

    printf("test_builder_roundtrip\n");
    CHECK(rmf_load_module(blob, blob_len, win, win_size, &m) == RMF_OK,
          "builder-packed module loads with RMF_OK");
    CHECK(m.id == h.id, "loaded id matches header");
    CHECK(m.entry == (uintptr_t)win + (uintptr_t)h.entry_offset + 1u,
          "entry computed from builder's entry_offset");
    CHECK(memcmp(win, blob + RMF1_HEADER_SIZE, h.code_len) == 0,
          "deployed code matches builder payload byte-exact");
    {
        size_t i, zero_ok = 1;
        for (i = 0; i < h.bss_len; i++) {
            if (win[h.bss_start + i] != 0) {
                zero_ok = 0;
                break;
            }
        }
        CHECK(zero_ok, "bss range zeroed per builder's bss fields");
    }

    printf("test_builder_corruption\n");
    win[0] = 0xAA; /* redeploy target clean-ish; loader only writes ranges */
    blob[RMF1_HEADER_SIZE] ^= 0x01u; /* flip 1 byte of code */
    CHECK(rmf_load_module(blob, blob_len, win, win_size, NULL) == RMF_ERR_CRC,
          "builder CRC caught by loader after 1-byte flip");
    blob[RMF1_HEADER_SIZE] ^= 0x01u;

    memcpy(&h, blob, sizeof h);
    h.magic ^= 0x00000001u;
    memcpy(blob, &h, sizeof h);
    CHECK(rmf_load_module(blob, blob_len, win, win_size, NULL) == RMF_ERR_MAGIC,
          "builder output still validated structurally (bad magic)");
    h.magic ^= 0x00000001u;
    memcpy(blob, &h, sizeof h);

    free(win);
    free(blob);
    printf("test_builder: %d failure(s)\n", failures);
    return failures;
}
