/*
 * test_theme_options.c — per-theme options (theme.cfg `opt_<id>=<Label>,a|b`).
 *
 * Covers the three seams the feature touches:
 *   1. parsing + limits (declaration, clamping of bad lines, defaults to 0),
 *   2. expression access (`opt_<id>` in any property, in `visible=`, and in
 *      the [skin] refresh_ms / dyn declarations),
 *   3. persistence encoding (`theme_opts=<id>:<digits>;` round-trip), plus the
 *      provider hook that theme_pkg_apply() uses to restore selections.
 *
 * Host test: real temporary directory, real manifests and skins.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../theme_pkg.h"
#include "../theme_runtime.h"
#include "../theme_skin.h"
#include "../drivers/display.h"

/* ---- display capture (the skin engine draws through these) ------------- */

static int s_rects;
static int s_last_y;

void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t color)
{
    (void)x0; (void)x1; (void)color;
    (void)y1;
    s_rects++;
    s_last_y = y0;
}

void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
    (void)x; (void)y; (void)s; (void)fg; (void)bg;
    s_rects++;
}

void display_bitmap1(int x, int y, int w, int h, int stride,
                     const uint8_t *bits, uint16_t fg, uint16_t bg)
{
    (void)x; (void)y; (void)w; (void)h; (void)stride;
    (void)bits; (void)fg; (void)bg;
    s_rects++;
}

void display_blit_span(int x, int y, int w, const uint16_t *src)
{
    (void)x; (void)y; (void)w; (void)src;
}

static int checks;
static int fails;

#define CHECK(cond) do {                                                   \
    checks++;                                                              \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        fails++;                                                           \
    }                                                                      \
} while (0)

#define CHECK_EQ_INT(got, want) do {                                       \
    long g_ = (long)(got), w_ = (long)(want);                               \
    checks++;                                                              \
    if (g_ != w_) {                                                        \
        printf("FAIL %s:%d: %s == %s (got %ld, want %ld)\n",              \
               __FILE__, __LINE__, #got, #want, g_, w_);                   \
        fails++;                                                           \
    }                                                                      \
} while (0)

#define CHECK_STR_EQ(got, want) do {                                       \
    const char *g_ = (got), *w_ = (want);                                  \
    checks++;                                                              \
    if (g_ == 0 || w_ == 0 || strcmp(g_, w_) != 0) {                        \
        printf("FAIL %s:%d: %s == \"%s\" (got \"%s\")\n",                 \
               __FILE__, __LINE__, #got, w_ ? w_ : "(null)",                \
               g_ ? g_ : "(null)");                                        \
        fails++;                                                           \
    }                                                                      \
} while (0)

static int write_file(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");

    if (f == 0)
        return -1;
    if (fputs(text, f) < 0) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

/* ---- 1. declaration parsing -------------------------------------------- */

static void test_parse_declaration(void)
{
    theme_layout_t l;

    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse(
        "opt_face=Face,Clock|Artwork\n"
        "opt_fps=Refresh,20 fps|60 fps\n", &l), 0);
    CHECK_EQ_INT(l.opt_count, 2);
    CHECK_STR_EQ(l.opts[0].id, "face");
    CHECK_STR_EQ(l.opts[0].label, "Face");
    CHECK_EQ_INT(l.opts[0].count, 2);
    CHECK_STR_EQ(l.opts[0].choice[0], "Clock");
    CHECK_STR_EQ(l.opts[0].choice[1], "Artwork");
    CHECK_EQ_INT(l.opts[0].value, 0);         /* defaults to choice 0 */
    CHECK_STR_EQ(l.opts[1].id, "fps");
    CHECK_EQ_INT(l.opts[1].value, 0);
    CHECK_EQ_INT(theme_opt_rejected(), 0);

    /* Values are trimmed, and an option line is not a layout key. */
    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse("opt_a= A , b | c \n", &l), 0);
    CHECK_STR_EQ(l.opts[0].label, "A");
    CHECK_STR_EQ(l.opts[0].choice[1], "c");
}

static void test_parse_limits(void)
{
    theme_layout_t l;

    /* The four-option cap: the fifth line is dropped, the package survives. */
    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse(
        "opt_a=A,1|2\nopt_b=B,1|2\nopt_c=C,1|2\nopt_d=D,1|2\nopt_e=E,1|2\n",
        &l), 0);
    CHECK_EQ_INT(l.opt_count, THEME_OPT_MAX);
    CHECK_EQ_INT(theme_opt_rejected(), 1);

    /* Choice cap (4) and the 2-choice minimum. */
    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse("opt_a=A,1|2|3|4|5\n", &l), 0);
    CHECK_EQ_INT(l.opt_count, 0);
    CHECK_EQ_INT(theme_opt_rejected(), 1);

    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse("opt_a=A,only\n", &l), 0);
    CHECK_EQ_INT(l.opt_count, 0);
    CHECK_EQ_INT(theme_opt_rejected(), 1);

    /* Malformed ids, missing label separator, over-long label/choice, dup. */
    {
        static const char *const k_bad[] = {
            "opt_A=A,1|2\n",                 /* uppercase id             */
            "opt_1x=A,1|2\n",                /* starts with a digit      */
            "opt_thisidisfartoolong=A,1|2\n", /* id >= 16 chars          */
            "opt_a=no separator\n",          /* no comma                 */
            "opt_a=,1|2\n",                  /* empty label              */
            "opt_a=labelwithtwentyfour!\n",  /* over-long label          */
            "opt_a=A,thischoiceiswaytoolong|b\n",
            "opt__=A,1|2\n",                 /* "_" alone: no letter start */
        };
        int i;

        for (i = 0; i < (int)(sizeof k_bad / sizeof k_bad[0]); i++) {
            theme_layout_defaults(&l);
            CHECK_EQ_INT(theme_layout_parse(k_bad[i], &l), 0);
            CHECK_EQ_INT(l.opt_count, 0);
        }
    }

    /* A duplicate id keeps the first declaration only. */
    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse("opt_a=A,1|2\nopt_a=B,3|4\n", &l), 0);
    CHECK_EQ_INT(l.opt_count, 1);
    CHECK_STR_EQ(l.opts[0].label, "A");

    /* A malformed option NEVER rejects the package: the layout keys still
     * apply (this is the documented exception to the atomic policy). */
    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse("pad=7\nopt_bad\nopt_a=A,1|2\n", &l), 0);
    CHECK_EQ_INT(l.pad, 7);
    CHECK_EQ_INT(l.opt_count, 1);
}

static void test_parse_replaces_previous(void)
{
    theme_layout_t l;

    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse("opt_a=A,1|2\n", &l), 0);
    /* Re-parsing the same struct must not keep the old declaration. */
    CHECK_EQ_INT(theme_layout_parse("pad=4\n", &l), 0);
    CHECK_EQ_INT(l.opt_count, 0);
}

/* ---- 2. accessors + clamping ------------------------------------------- */

static void test_accessors(void)
{
    theme_layout_t l;

    theme_layout_defaults(&l);
    (void)theme_layout_parse("opt_face=Face,Clock|Artwork\n", &l);
    theme_layout_set(&l);

    CHECK_EQ_INT(theme_opt_value("face"), 0);
    CHECK_EQ_INT(theme_opt_value("nope"), -1);
    CHECK_EQ_INT(theme_opt_value(0), -1);

    CHECK_EQ_INT(theme_opt_set("face", 1), 1);
    CHECK_EQ_INT(theme_opt_value("face"), 1);
    /* Out-of-range values clamp to the last declared choice. */
    CHECK_EQ_INT(theme_opt_set("face", 9), 1);
    CHECK_EQ_INT(theme_opt_set("face", -3), 0);
    CHECK_EQ_INT(theme_opt_set("unknown", 1), -1);

    /* theme_opts_apply() is what the runtime uses after parsing. */
    {
        int vals[THEME_OPT_MAX] = { 1, 0, 0, 0 };

        theme_opts_apply(vals, THEME_OPT_MAX);
        CHECK_EQ_INT(theme_opt_value("face"), 1);
        /* A shrunk option list clamps instead of rendering an invalid face. */
        vals[0] = 3;
        theme_opts_apply(vals, THEME_OPT_MAX);
        CHECK_EQ_INT(theme_opt_value("face"), 1);

        theme_opts_values(vals, THEME_OPT_MAX);
        CHECK_EQ_INT(vals[0], 1);
        CHECK_EQ_INT(vals[1], 0);
    }
    theme_layout_reset();
    CHECK_EQ_INT(theme_opt_value("face"), -1);
}

/* ---- 3. persistence encoding ------------------------------------------- */

static void test_persistence_text(void)
{
    char buf[128];
    int vals[THEME_OPT_MAX];

    buf[0] = '\0';
    /* No entry yet: every digit defaults to 0. */
    theme_opts_decode(buf, "glifo", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 0);
    CHECK_EQ_INT(vals[1], 0);

    /* Round-trip one theme. */
    {
        int v[4] = { 1, 0, 0, 0 };

        CHECK(theme_opts_encode_entry(buf, (int)sizeof buf, "glifo", v, 2) > 0);
        CHECK_STR_EQ(buf, "glifo:10;");
    }
    theme_opts_decode(buf, "glifo", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 1);
    CHECK_EQ_INT(vals[1], 0);

    /* A second theme appends without touching the first. */
    {
        int v[4] = { 1, 0, 0, 0 };

        CHECK(theme_opts_encode_entry(buf, (int)sizeof buf, "oscilloscope",
                                      v, 1) > 0);
        CHECK_STR_EQ(buf, "glifo:10;oscilloscope:1;");
        theme_opts_decode(buf, "glifo", vals, THEME_OPT_MAX);
        CHECK_EQ_INT(vals[0], 1);
        theme_opts_decode(buf, "oscilloscope", vals, THEME_OPT_MAX);
        CHECK_EQ_INT(vals[0], 1);
    }

    /* Re-encoding an existing theme REPLACES its entry (no duplicates). */
    {
        int v[4] = { 0, 0, 0, 0 };

        CHECK(theme_opts_encode_entry(buf, (int)sizeof buf, "glifo", v, 2) > 0);
        CHECK_STR_EQ(buf, "oscilloscope:1;glifo:00;");
    }

    /* Switching themes and back keeps the choice (the whole point). */
    theme_opts_decode(buf, "glifo", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 0);
    theme_opts_decode(buf, "oscilloscope", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 1);

    /* Unknown themes and malformed text read as zeros, never a crash. */
    theme_opts_decode(buf, "missing", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 0);
    theme_opts_decode("garbage", "glifo", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 0);
    theme_opts_decode("glifo:", "glifo", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 0);
    theme_opts_decode("glifo:xx;", "glifo", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 0);
    theme_opts_decode(0, "glifo", vals, THEME_OPT_MAX);
    CHECK_EQ_INT(vals[0], 0);

    /* The return value is the digit count actually found, not the buffer
     * size: 0 means "no entry", which is how a fresh theme is detected. */
    CHECK_EQ_INT(theme_opts_decode(buf, "nope", vals, THEME_OPT_MAX), 0);
    CHECK_EQ_INT(theme_opts_decode("g:1;", "g", vals, THEME_OPT_MAX), 1);

    /* An id that cannot round-trip (separator inside it) is refused rather
     * than corrupting the list. */
    {
        int v[2] = { 1, 1 };

        CHECK_EQ_INT(theme_opts_decode("a:b:11;", "a:b", vals, THEME_OPT_MAX),
                     -1);
        CHECK(theme_opts_encode_entry(buf, (int)sizeof buf, "a:b", v, 2) < 0);
        /* Untouched on failure: the previous contents survive verbatim. */
        CHECK_STR_EQ(buf, "oscilloscope:1;glifo:00;");
    }

    /* A too-small buffer reports failure instead of truncating. */
    {
        int v[4] = { 1, 1, 1, 1 };

        CHECK(theme_opts_encode_entry(buf, 4, "glifo", v, 4) < 0);
    }
}

/* ---- 4. expression access through the skin engine ---------------------- */

#define ROOT "opts_root"

static void make_package(const char *name, const char *cfg,
                         const char *skin)
{
    char path[256];
    FILE *f;

    snprintf(path, sizeof path, ROOT "/%s", name);
    mkdir(path, 0777);
    snprintf(path, sizeof path, ROOT "/%s/theme.cfg", name);
    (void)write_file(path, cfg);
    if (skin != 0) {
        snprintf(path, sizeof path, ROOT "/%s/home.skin", name);
        (void)write_file(path, skin);
    }
    if ((f = fopen(path, "r")) == 0) { /* silence unused warnings */ }
    else fclose(f);
}

static void test_expression_access(void)
{
    theme_layout_t l;
    theme_skin_state_t st;

    theme_layout_defaults(&l);
    (void)theme_layout_parse("opt_face=Face,Clock|Artwork\n", &l);
    theme_layout_set(&l);
    CHECK_EQ_INT(theme_opt_value("face"), 0);

    /* `opt_face` reads as 0/1 in a plain property and in visible=. */
    {
        const char *skin =
            "[bg]\n"
            "type=fill\n"
            "x=0\ny=0\nw=W\nh=H\ncolor=background\n"
            "[clock]\n"
            "type=fill\n"
            "x=0\ny=(opt_face?50:0)\nw=10\nh=10\ncolor=text\n"
            "visible=!opt_face\n"
            "[art]\n"
            "type=fill\n"
            "x=0\ny=0\nw=10\nh=10\ncolor=accent\n"
            "visible=opt_face\n";

        make_package("expr", "format=1\nhome=skin\nskin=home.skin\n"
                             "opt_face=Face,Clock|Artwork\n", skin);
        CHECK_EQ_INT(theme_skin_load(ROOT, "expr", "home.skin"), 0);

        memset(&st, 0, sizeof st);
        s_rects = 0;
        theme_skin_render(0, 0, 0, 0, &st);
        /* bg + clock = 2 shapes; the art section is skipped. */
        CHECK_EQ_INT(s_rects, 2);

        theme_opt_set("face", 1);
        s_rects = 0;
        theme_skin_render(0, 0, 0, 0, &st);
        /* bg + art = 2 shapes; the clock section is skipped. */
        CHECK_EQ_INT(s_rects, 2);
    }

    /* Undeclared options read as an empty string: `!opt_none` is true and a
     * numeric use is 0, so a package can probe for an optional variant. */
    {
        const char *skin =
            "[a]\n"
            "type=fill\n"
            "x=0\ny=0\nw=(opt_none+1)\nh=2\ncolor=text\n";

        make_package("probe", "format=1\nhome=skin\nskin=home.skin\n", skin);
        CHECK_EQ_INT(theme_skin_load(ROOT, "probe", "home.skin"), 0);
        memset(&st, 0, sizeof st);
        s_rects = 0;
        theme_skin_render(0, 0, 0, 0, &st);
        CHECK_EQ_INT(s_rects, 1);
        CHECK_EQ_INT(s_last_y, 0);
    }
}

static void test_refresh_from_option(void)
{
    theme_layout_t l;

    theme_layout_defaults(&l);
    (void)theme_layout_parse("opt_fps=Refresh,20 fps|60 fps\n", &l);
    theme_layout_set(&l);

    make_package("fps",
                 "format=1\nname=Fps\nhome=skin\nskin=home.skin\n"
                 "opt_fps=Refresh,20 fps|60 fps\n",
                 "[skin]\n"
                 "refresh_ms=(opt_fps?16:50)\n"
                 "dyn=(opt_fps?18:13),(opt_fps?50:13),(opt_fps?209:213),(opt_fps?97:121)\n"
                 "dyn_every=(opt_fps?59:20)\n"
                 "[a]\ntype=fill\nx=0\ny=0\nw=2\nh=2\ncolor=text\n");
    CHECK_EQ_INT(theme_skin_load(ROOT, "fps", "home.skin"), 0);
    /* Same package, two refresh rates: this is what replaced the second
     * oscilloscope package. */
    theme_opt_set("fps", 0);
    CHECK_EQ_INT(theme_skin_refresh_ms(), 50);
    theme_opt_set("fps", 1);
    CHECK_EQ_INT(theme_skin_refresh_ms(), 16);
    CHECK_EQ_INT(theme_skin_animated(), 1);

    /* dyn rects/cadence follow the same expression. */
    {
        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;

        CHECK_EQ_INT(theme_skin_dyn_rect(0, &x0, &y0, &x1, &y1), 0);
        CHECK_EQ_INT(x0, 18);
        CHECK_EQ_INT(y0, 50);
        CHECK_EQ_INT(x1, 209);
        CHECK_EQ_INT(y1, 97);
        CHECK_EQ_INT(theme_skin_dyn_every(0), 59);

        theme_opt_set("fps", 0);
        CHECK_EQ_INT(theme_skin_dyn_rect(0, &x0, &y0, &x1, &y1), 0);
        CHECK_EQ_INT(x0, 13);
        CHECK_EQ_INT(y1, 121);
        CHECK_EQ_INT(theme_skin_dyn_every(0), 20);
    }

    /* A literal refresh_ms keeps behaving exactly as before. */
    {
        const char *skin =
            "[skin]\n"
            "refresh_ms=80\n"
            "[a]\n"
            "type=fill\n"
            "x=0\ny=0\nw=2\nh=2\ncolor=text\n";

        make_package("static", "format=1\nhome=skin\nskin=home.skin\n", skin);
        CHECK_EQ_INT(theme_skin_load(ROOT, "static", "home.skin"), 0);
        CHECK_EQ_INT(theme_skin_refresh_ms(), 80);
    }
}

/* ---- 5. the provider hook used by theme_pkg_apply() -------------------- */

static int s_provider_calls;
static char s_provider_id[32];

static void provider(const char *id, int *out, int max)
{
    int i;

    s_provider_calls++;
    snprintf(s_provider_id, sizeof s_provider_id, "%s", id);
    for (i = 0; i < max; i++)
        out[i] = 0;
    out[0] = 1;                            /* "60 fps" / "Artwork" */
}

static void test_apply_restores_selection(void)
{
    make_package("restore",
                 "format=1\nname=Restore\nhome=skin\nskin=home.skin\n"
                 "opt_fps=Refresh,20 fps|60 fps\n",
                 "[skin]\nrefresh_ms=(opt_fps?16:50)\n"
                 "[a]\ntype=fill\nx=0\ny=0\nw=2\nh=2\ncolor=text\n");

    /* Without a provider every option stays at its default. */
    theme_pkg_set_option_provider(0);
    CHECK_EQ_INT(theme_pkg_apply(ROOT, "restore"), 0);
    CHECK_EQ_INT(theme_opt_value("fps"), 0);
    CHECK_EQ_INT(theme_skin_refresh_ms(), 50);

    /* The provider is asked for THIS theme's digits, and the value lands on
     * the package before the first draw. */
    s_provider_calls = 0;
    s_provider_id[0] = '\0';
    theme_pkg_set_option_provider(provider);
    CHECK_EQ_INT(theme_pkg_apply(ROOT, "restore"), 0);
    CHECK_EQ_INT(s_provider_calls, 1);
    CHECK_STR_EQ(s_provider_id, "restore");
    CHECK_EQ_INT(theme_opt_value("fps"), 1);
    CHECK_EQ_INT(theme_skin_refresh_ms(), 16);

    /* A theme that declares no options never calls the provider. */
    make_package("plain", "format=1\nhome=skin\nskin=home.skin\n",
                 "[a]\ntype=fill\nx=0\ny=0\nw=2\nh=2\ncolor=text\n");
    s_provider_calls = 0;
    CHECK_EQ_INT(theme_pkg_apply(ROOT, "plain"), 0);
    CHECK_EQ_INT(s_provider_calls, 0);

    theme_pkg_set_option_provider(0);
}

int main(void)
{
    if (system("rm -rf " ROOT) != 0) { /* best effort */ }
    CHECK_EQ_INT(mkdir(ROOT, 0777), 0);

    printf("== theme options: declaration ==\n");
    test_parse_declaration();
    test_parse_limits();
    test_parse_replaces_previous();

    printf("== theme options: accessors ==\n");
    test_accessors();

    printf("== theme options: persistence ==\n");
    test_persistence_text();

    printf("== theme options: expressions ==\n");
    test_expression_access();
    test_refresh_from_option();

    printf("== theme options: package apply ==\n");
    test_apply_restores_selection();

    if (system("rm -rf " ROOT) != 0) { /* best effort */ }
    printf("%d checks, %d failures\n", checks, fails);
    return fails == 0 ? 0 : 1;
}
