/*
 * test_registry.c — module registry add/iterate + lifecycle order
 * (firmware/app/module_registry.c). The registry is real code here; the
 * modules are test fakes whose hooks record into the mock call log, so the
 * init/start/stop ORDER (the registry's core contract) is asserted
 * directly.
 */
#include <stdio.h>
#include <string.h>

#include "app/module_registry.h"

#include "mocks.h"
#include "test_util.h"

/* Fake module hooks — record into the shared ordered call log. */
static int  a_init(void)  { mock_rec("A.init");  return 0; }
static int  a_start(void) { mock_rec("A.start"); return 0; }
static void a_stop(void)  { mock_rec("A.stop");  }

static int  b_init(void)  { mock_rec("B.init");  return 0; }
static int  b_start(void) { mock_rec("B.start"); return 0; }
static void b_stop(void)  { mock_rec("B.stop");  }

static int  fail_init(void)  { mock_rec("F.init"); return -1; }
static int  c_init(void)     { mock_rec("C.init"); return 0; }

/* Assert the complete recorded sequence equals `want` (count-checked). */
static void check_seq(const char *label, const char *const *want, int nwant)
{
    int i;
    int ok = (mock_rec_count() == nwant);

    for (i = 0; ok && i < nwant; i++) {
        const char *got = mock_rec_at(i);
        if (got == 0 || strcmp(got, want[i]) != 0)
            ok = 0;
    }

    if (!ok) {
        printf("  sequence '%s' mismatch (got %d records, want %d):\n",
               label, mock_rec_count(), nwant);
        for (i = 0; i < nwant || i < mock_rec_count(); i++) {
            const char *g = mock_rec_at(i);
            const char *w = (i < nwant) ? want[i] : "-";
            printf("    [%2d] want %-24s got %s\n", i, w, g ? g : "(none)");
        }
    }
    CHECK(ok, label);
}

void test_registry(void)
{
    static const module_descriptor_t a = { "alpha", a_init, a_start, a_stop };
    static const module_descriptor_t b = { "beta",  b_init, b_start, b_stop };
    static const module_descriptor_t f = { "fail",  fail_init, 0, 0 };
    static const module_descriptor_t c = { "gamma", c_init, 0, 0 };
    static const module_descriptor_t noname = { 0, 0, 0, 0 };
    static const module_descriptor_t nohooks = { "bare", 0, 0, 0 };

    module_registry_reset();
    CHECK(module_registry_count() == 0, "empty after reset");
    CHECK(module_registry_get(0) == 0, "get(0) on empty -> NULL");

    /* ---- add ---- */
    CHECK(module_registry_add(&a) == MODULE_REG_OK, "add alpha");
    CHECK(module_registry_add(&b) == MODULE_REG_OK, "add beta");
    CHECK(module_registry_add(&a) == MODULE_REG_ERR_DUP, "duplicate name rejected");
    CHECK(module_registry_add(0) == MODULE_REG_ERR_ARG, "NULL descriptor rejected");
    CHECK(module_registry_add(&noname) == MODULE_REG_ERR_ARG, "NULL name rejected");
    CHECK(module_registry_add(&nohooks) == MODULE_REG_OK, "hook-less module allowed");
    CHECK(module_registry_count() == 3, "count = 3 after 3 adds");

    /* ---- iterate / query ---- */
    CHECK(module_registry_get(0) == &a, "get(0) = alpha (add order)");
    CHECK(module_registry_get(1) == &b, "get(1) = beta");
    CHECK(module_registry_get(2) == &nohooks, "get(2) = bare");
    CHECK(module_registry_get(3) == 0, "get(3) out of range -> NULL");
    CHECK(module_registry_get(-1) == 0, "get(-1) -> NULL");
    CHECK(module_registry_find("beta") == &b, "find(\"beta\")");
    CHECK(module_registry_find("bare") == &nohooks, "find(\"bare\")");
    CHECK(module_registry_find("nope") == 0, "find unknown -> NULL");
    CHECK(module_registry_find(0) == 0, "find(NULL) -> NULL");

    /* ---- lifecycle: init in add order, start in add order, stop
     *      REVERSE (dependents before dependencies) ---- */
    mock_rec_reset();
    CHECK(module_registry_init_all() == MODULE_REG_OK, "init_all ok");
    CHECK(module_registry_start_all() == MODULE_REG_OK, "start_all ok");
    module_registry_stop_all();
    {
        static const char *const want[] = {
            "A.init", "B.init",
            "A.start", "B.start",
            "B.stop", "A.stop",
        };
        check_seq("lifecycle order (init/start fwd, stop rev)",
                  want, (int)(sizeof want / sizeof want[0]));
    }

    /* ---- init failure halts iteration ---- */
    module_registry_reset();
    CHECK(module_registry_add(&f) == MODULE_REG_OK, "add fail");
    CHECK(module_registry_add(&c) == MODULE_REG_OK, "add gamma");
    mock_rec_reset();
    CHECK(module_registry_init_all() == MODULE_REG_ERR_RUN,
          "init_all reports failing hook");
    CHECK(mock_rec_count() == 1, "init_all halted at the failing module");
    CHECK(mock_rec_at(0) != 0 && mock_rec_at(0)[0] == 'F',
          "only the failing module's init ran");

    /* ---- capacity: fixed static table, no allocation ---- */
    {
        module_descriptor_t many[MODULE_REGISTRY_MAX];
        char names[MODULE_REGISTRY_MAX][8];
        int i;
        int filled = 1;

        module_registry_reset();
        for (i = 0; i < MODULE_REGISTRY_MAX; i++) {
            snprintf(names[i], sizeof names[i], "m%d", i);
            many[i].name = names[i];
            many[i].init = 0;
            many[i].start = 0;
            many[i].stop = 0;
            if (module_registry_add(&many[i]) != MODULE_REG_OK)
                filled = 0;
        }
        CHECK(filled, "registry fills to capacity");
        CHECK(module_registry_add(&a) == MODULE_REG_ERR_FULL,
              "add beyond capacity -> ERR_FULL");
        CHECK(module_registry_count() == MODULE_REGISTRY_MAX,
              "count == MODULE_REGISTRY_MAX");
        CHECK(module_registry_find("m15") == &many[15], "find in full registry");
    }

    module_registry_reset();
}
