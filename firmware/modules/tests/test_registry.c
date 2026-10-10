/*
 * test_registry.c — host tests for the loaded-module registry.
 *
 * Covers the registry lifecycle: init -> add -> find -> iterate -> remove ->
 * duplicate/full/not-found errors -> unload-all, plus an end-to-end pass that
 * loads a real RMF1 blob with the loader and registers the result.
 * Compile: cc -Wall -Werror (see tests/run_tests.sh).
 */
#include <stdlib.h>
#include <string.h>

#include "../module_loader.h"
#include "../module_registry.h"
#include "test_util.h"

/* Fake window stand-ins: the registry only stores pointers + sizes. */
static uint8_t fake_win_a[64];
static uint8_t fake_win_b[64];
static uint8_t fake_win_c[64];

static void test_lifecycle(void)
{
    const rmf_registry_entry_t *e;
    size_t i, seen;

    printf("test_lifecycle\n");
    rmf_registry_init();
    CHECK(rmf_registry_count() == 0, "init leaves registry empty");
    CHECK(rmf_registry_at(0) == NULL, "at(0) NULL when empty");
    CHECK(rmf_registry_find(7) == NULL, "find misses when empty");

    CHECK(rmf_registry_add(44, "player", fake_win_a, 23200u) == RMF_OK,
          "add id 44");
    CHECK(rmf_registry_add(45, "browser", fake_win_b, 2280u) == RMF_OK,
          "add id 45");
    CHECK(rmf_registry_add(78, "bcore", fake_win_c, 41196u) == RMF_OK,
          "add id 78");
    CHECK(rmf_registry_count() == 3, "count == 3 after three adds");

    e = rmf_registry_find(45);
    CHECK(e != NULL && strcmp(e->name, "browser") == 0,
          "find(45) returns the browser entry");
    CHECK(e != NULL && e->base == fake_win_b, "entry base preserved");
    CHECK(e != NULL && e->size == 2280u, "entry size preserved");
    CHECK(e != NULL && e->state == RMF_MOD_STATE_LOADED, "entry state LOADED");

    /* Iteration visits every loaded slot exactly once. */
    seen = 0;
    for (i = 0; i < rmf_registry_capacity(); i++) {
        const rmf_registry_entry_t *it = rmf_registry_at(i);
        if (it != NULL) {
            seen++;
        }
    }
    CHECK(seen == rmf_registry_count(), "iteration visits each loaded slot");
    CHECK(rmf_registry_at(rmf_registry_capacity()) == NULL,
          "at() past capacity returns NULL");

    CHECK(rmf_registry_remove(45) == RMF_OK, "remove(45)");
    CHECK(rmf_registry_find(45) == NULL, "find(45) misses after remove");
    CHECK(rmf_registry_count() == 2, "count == 2 after remove");
    CHECK(rmf_registry_remove(45) == RMF_ERR_NOT_FOUND,
          "double remove -> RMF_ERR_NOT_FOUND");

    rmf_registry_unload_all();
    CHECK(rmf_registry_count() == 0, "unload_all empties the registry");
    CHECK(rmf_registry_find(44) == NULL && rmf_registry_find(78) == NULL,
          "find misses after unload_all");
}

static void test_errors(void)
{
    size_t i;

    printf("test_errors\n");
    rmf_registry_init();

    CHECK(rmf_registry_add(5, "dup", fake_win_a, 1u) == RMF_OK, "add id 5");
    CHECK(rmf_registry_add(5, "dup2", fake_win_b, 1u) == RMF_ERR_DUPLICATE,
          "duplicate id -> RMF_ERR_DUPLICATE");
    CHECK(rmf_registry_add(6, NULL, fake_win_b, 1u) == RMF_ERR_ARG,
          "NULL name -> RMF_ERR_ARG");
    CHECK(rmf_registry_add(6, "x", NULL, 1u) == RMF_ERR_ARG,
          "NULL base -> RMF_ERR_ARG");

    for (i = 0; i < rmf_registry_capacity(); i++) {
        uint16_t id = (uint16_t)(100 + i);
        if (rmf_registry_add(id, "filler", fake_win_c, 1u) != RMF_OK) {
            break;
        }
    }
    CHECK(i == rmf_registry_capacity() - 1u,
          "capacity-1 filler adds succeed (slot 0 taken by id 5)");
    CHECK(rmf_registry_add(999, "overflow", fake_win_a, 1u) == RMF_ERR_FULL,
          "table full -> RMF_ERR_FULL");

    /* Long names truncate safely with a terminator. */
    rmf_registry_unload_all();
    CHECK(rmf_registry_add(1, "way_too_long_module_name", fake_win_a, 1u)
              == RMF_OK, "add with overlong name");
    {
        const rmf_registry_entry_t *e = rmf_registry_find(1);
        CHECK(e != NULL && strlen(e->name) == RMF_REGISTRY_NAME_MAX - 1u,
              "overlong name truncated to name_max-1");
        CHECK(e != NULL && e->name[RMF_REGISTRY_NAME_MAX - 1u] == '\0',
              "truncated name stays NUL-terminated");
    }
}

static void test_load_register_unload(void)
{
    /* End-to-end: pack -> load -> register -> find -> unload-all. */
    static uint8_t blob[RMF1_HEADER_SIZE + 64u];
    static uint8_t win[128];
    rmf1_header_t h;
    rmf_loaded_t m;
    const rmf_registry_entry_t *e;

    printf("test_load_register_unload\n");

    memset(blob + RMF1_HEADER_SIZE, 0x5A, 64);
    h.magic = RMF1_MAGIC;
    h.version = RMF1_VERSION;
    h.id = 47;
    h.flags = 0;
    h.code_len = 64;
    h.bss_start = 64;
    h.bss_len = 32;
    h.entry_offset = 0;
    h.crc32 = rmf_crc32(blob + RMF1_HEADER_SIZE, 64);
    memcpy(blob, &h, sizeof h);

    memset(win, 0xAA, sizeof win);
    rmf_registry_init();
    CHECK(rmf_load_module(blob, sizeof blob, win, sizeof win, &m) == RMF_OK,
          "load fixture module");
    CHECK(rmf_registry_add(m.id, "fixture", m.base,
                           m.bss_start + m.bss_len) == RMF_OK,
          "register the loaded module");

    e = rmf_registry_find(47);
    CHECK(e != NULL && e->base == win, "registry entry points at the window");
    CHECK(e != NULL && e->size == 96u, "resident size = bss_start + bss_len");

    rmf_registry_unload_all();
    CHECK(rmf_registry_count() == 0, "unload_all clears the loaded module");
}

int main(void)
{
    test_lifecycle();
    test_errors();
    test_load_register_unload();

    printf("test_registry: %d failure(s)\n", failures);
    return failures;
}
