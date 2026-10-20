/*
 * theme_skin.c — bounded declarative skin interpreter (BRNOMT-69).
 *
 * The firmware knows only generic primitives (fill, 1-bit bitmap, text, bar,
 * interactive list) plus a small expression evaluator. The theme package
 * supplies the layout, the animation logic (seq/noise/sin), the assets and
 * the key map. Nothing in this file knows about "Companion" or any other
 * theme.
 */
#include <stdio.h>
#include <string.h>

#include "theme_skin.h"
#include "theme.h"
#include "theme_pkg.h"
#include "theme_assets.h"
#include "../drivers/display.h"

#ifdef RECHORD_AP_BUILD
#include "../services/fs.h"
#endif

#define SKIN_LINE_MAX   256
#define SKIN_SECTIONS   32
#define SKIN_PROPS      12
#define SKIN_KEY_MAX    20
#define SKIN_PROP_MAX   144
#define SKIN_VAL_MAX    64
#define SKIN_EXPR_STEPS 2048
#define SKIN_DEPTH      12
#define SKIN_ITEMS      4
#define SKIN_FONT_W     8
#define SKIN_VARS       20
#define SKIN_VAR_EXPR_MAX 112
#define SKIN_VAR_STR_MAX  24

typedef struct {
    char name[SKIN_KEY_MAX];
    char expr[SKIN_VAR_EXPR_MAX];
} skin_var_t;

static skin_var_t s_vars[SKIN_VARS];
static int        s_var_count;
/* Vars are evaluated once per frame in definition order; expressions read
 * these caches, so user-variable lookup never recurses. */
static int          s_var_i[SKIN_VARS];
static char         s_var_s[SKIN_VARS][SKIN_VAR_STR_MAX];
static unsigned char s_var_is_str[SKIN_VARS];

typedef struct {
    char name[24];
    char key[SKIN_PROPS][SKIN_KEY_MAX];
    char val[SKIN_PROPS][SKIN_PROP_MAX];
    int  count;
} skin_section_t;

typedef struct {
    char label[24];
    int  action;
} skin_item_t;

static char s_text[THEME_SKIN_TEXT_MAX + 1];
static int  s_loaded;

static struct {
    int count;
    int axis;
    int x, y, height;
    int selected;
    int rows;                 /* visible rows; 0 = all                  */
    int dynamic;              /* use the registered list source         */
    int prev_key, next_key;
    skin_item_t item[SKIN_ITEMS];
} s_list;

static const theme_skin_list_source_t *s_list_src;

void theme_skin_set_list(const theme_skin_list_source_t *src)
{
    s_list_src = src;
}

static int s_refresh_ms = 125;
static int s_refresh_h = -1;
static int s_animated;

/* ---------------------------------------------------------------- values */

typedef struct {
    int  is_str;
    int  i;
    char s[SKIN_VAL_MAX];
} skin_val_t;

typedef struct {
    unsigned now;
    unsigned idle;
    int      sel;
    int      top;
    int      aw, ah;
    const theme_skin_state_t *st;
} skin_ctx_t;

static skin_ctx_t *g_ctx;

typedef struct {
    const char *p;
    int steps;
    int err;
} skin_expr_t;

static skin_val_t eval_expr(skin_expr_t *e, int depth);
static skin_val_t eval_raw(const char *text);

static skin_val_t v_int(int i)
{
    skin_val_t v;

    memset(&v, 0, sizeof v);
    v.i = i;
    return v;
}

static skin_val_t v_str(const char *s)
{
    skin_val_t v;

    memset(&v, 0, sizeof v);
    v.is_str = 1;
    snprintf(v.s, sizeof v.s, "%s", s != 0 ? s : "");
    return v;
}

static int as_int(skin_val_t v)
{
    if (!v.is_str)
        return v.i;
    {
        int neg = 0, n = 0, any = 0;
        const char *p = v.s;

        while (*p == ' ')
            p++;
        if (*p == '-') {
            neg = 1;
            p++;
        }
        while (*p >= '0' && *p <= '9') {
            n = n * 10 + (*p - '0');
            p++;
            any = 1;
        }
        while (*p == ' ')
            p++;
        return (any && *p == '\0') ? (neg ? -n : n) : 0;
    }
}

static int truthy(skin_val_t v)
{
    return v.is_str ? v.s[0] != '\0' : v.i != 0;
}

/* ------------------------------------------------------------- tokenizer */

static void skip_ws(skin_expr_t *e)
{
    while (*e->p == ' ' || *e->p == '\t')
        e->p++;
}

static int peek(skin_expr_t *e, const char *tok)
{
    skip_ws(e);
    return strncmp(e->p, tok, strlen(tok)) == 0;
}

static int take(skin_expr_t *e, const char *tok)
{
    skip_ws(e);
    if (strncmp(e->p, tok, strlen(tok)) != 0)
        return 0;
    e->p += strlen(tok);
    return 1;
}

static int ident_start(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int ident_char(char c)
{
    return ident_start(c) || (c >= '0' && c <= '9');
}

/* -------------------------------------------------------------- variables */

static skin_val_t variable(const char *name)
{
    const theme_t *t = theme_get();
    skin_ctx_t *c = g_ctx;
    int i;

    if (strcmp(name, "t") == 0)
        return v_int(c != 0 ? (int)c->now : 0);
    if (strcmp(name, "idle") == 0)
        return v_int(c != 0 ? (int)c->idle : 0);
    if (strcmp(name, "sel") == 0)
        return v_int(c != 0 ? c->sel : 0);
    if (strcmp(name, "top") == 0)
        return v_int(c != 0 ? c->top : 0);
    if (strcmp(name, "n") == 0)
        return v_int(s_list.dynamic && s_list_src != 0
                         ? s_list_src->count : s_list.count);
    if (strcmp(name, "aw") == 0)
        return v_int(c != 0 ? c->aw : 0);
    if (strcmp(name, "ah") == 0)
        return v_int(c != 0 ? c->ah : 0);
    if (strcmp(name, "W") == 0)
        return v_int(THEME_LAYOUT_MAX_W);
    if (strcmp(name, "H") == 0)
        return v_int(THEME_LAYOUT_MAX_H);
    if (strcmp(name, "playing") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->playing : 0);
    if (strcmp(name, "paused") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->paused : 0);
    if (strcmp(name, "stopped") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->stopped : 0);
    if (strcmp(name, "volume") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->volume_db : 0);
    if (strcmp(name, "battery") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->battery_pct : -1);
    if (strcmp(name, "pos") == 0)
        return v_int(c != 0 && c->st != 0
                         ? (int)(c->st->position_ms / 1000) : 0);
    if (strcmp(name, "dur") == 0)
        return v_int(c != 0 && c->st != 0
                         ? (int)(c->st->duration_ms / 1000) : 0);
    if (strcmp(name, "pos_ms") == 0)
        return v_int(c != 0 && c->st != 0 ? (int)c->st->position_ms : 0);
    if (strcmp(name, "dur_ms") == 0)
        return v_int(c != 0 && c->st != 0 ? (int)c->st->duration_ms : 0);
    if (strcmp(name, "pos_fmt") == 0) {
        char buf[32];
        long s = c != 0 && c->st != 0 ? c->st->position_ms / 1000 : 0;

        if (s < 0)
            s = 0;
        snprintf(buf, sizeof buf, "%ld:%02ld", s / 60, s % 60);
        return v_str(buf);
    }
    if (strcmp(name, "dur_fmt") == 0) {
        char buf[32];
        long s = c != 0 && c->st != 0 ? c->st->duration_ms / 1000 : 0;

        if (s < 0)
            s = 0;
        snprintf(buf, sizeof buf, "%ld:%02ld", s / 60, s % 60);
        return v_str(buf);
    }
    if (strcmp(name, "bits") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->bits : 0);
    if (strcmp(name, "rate") == 0)
        return v_int(c != 0 && c->st != 0 ? (int)c->st->sample_rate : 0);
    if (strcmp(name, "pl_i") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->playlist_pos : 0);
    if (strcmp(name, "pl_n") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->playlist_count : 0);
    if (strcmp(name, "shuffle") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->shuffle : 0);
    if (strcmp(name, "repeat") == 0)
        return v_int(c != 0 && c->st != 0 ? c->st->repeat : 0);
    if (strcmp(name, "title") == 0)
        return v_str(c != 0 && c->st != 0 ? c->st->title : "");
    if (strcmp(name, "artist") == 0)
        return v_str(c != 0 && c->st != 0 ? c->st->artist : "");
    if (strcmp(name, "album") == 0)
        return v_str(c != 0 && c->st != 0 ? c->st->album : "");
    if (strcmp(name, "state") == 0)
        return v_str(c != 0 && c->st != 0 ? c->st->state : "");
    if (strcmp(name, "path") == 0)
        return v_str(c != 0 && c->st != 0 ? c->st->path : "");
    if (strcmp(name, "heading") == 0)
        return v_str(c != 0 && c->st != 0 ? c->st->heading : "");
    if (strcmp(name, "true") == 0)
        return v_int(1);
    if (strcmp(name, "false") == 0)
        return v_int(0);

    if (strcmp(name, "background") == 0)
        return v_int(t->background);
    if (strcmp(name, "surface") == 0)
        return v_int(t->surface);
    if (strcmp(name, "text") == 0)
        return v_int(t->text);
    if (strcmp(name, "text_dim") == 0)
        return v_int(t->text_dim);
    if (strcmp(name, "accent") == 0)
        return v_int(t->accent);
    if (strcmp(name, "danger") == 0)
        return v_int(t->danger);
    if (strcmp(name, "success") == 0)
        return v_int(t->success);

    /* Theme-defined variables from [vars], evaluated once per frame. */
    for (i = 0; i < s_var_count; i++) {
        if (strcmp(name, s_vars[i].name) == 0) {
            if (s_var_is_str[i])
                return v_str(s_var_s[i]);
            return v_int(s_var_i[i]);
        }
    }

    /* A bare word is a string (asset names, labels written without quotes). */
    return v_str(name);
}

/* ------------------------------------------------------------- expressions */

static skin_val_t eval_expr(skin_expr_t *e, int depth);

static int noise_int(int x, int amp)
{
    unsigned h;

    if (amp < 0)
        amp = -amp;
    h = (unsigned)x * 1103515245u + 12345u;
    h ^= h >> 13;
    h *= 2654435761u;
    h ^= h >> 16;
    return (int)(h % (unsigned)(2 * amp + 1)) - amp;
}

static int sin_int(int x, int period, int amp)
{
    /* Integer sine, 64 steps. Values are sin(2*pi*i/64) * 1000. */
    static const int table[64] = {
        0, 98, 195, 290, 382, 471, 556, 634,
        707, 773, 831, 881, 924, 956, 981, 995,
        1000, 995, 981, 956, 924, 881, 831, 773,
        707, 634, 556, 471, 382, 290, 195, 98,
        0, -98, -195, -290, -382, -471, -556, -634,
        -707, -773, -831, -881, -924, -956, -981, -995,
        -1000, -995, -981, -956, -924, -881, -831, -773,
        -707, -634, -556, -471, -382, -290, -195, -98
    };
    int phase, idx;

    if (period <= 0)
        return 0;
    phase = x % period;
    if (phase < 0)
        phase += period;
    idx = phase * 64 / period;
    if (idx < 0)
        idx = 0;
    if (idx > 63)
        idx = 63;
    return table[idx] * amp / 1000;
}

/* seq(t, period, dwell, "a,b,c,...") — frame by time; outside `dwell` the
 * first frame (the idle/open one) is returned. */
static skin_val_t seq_value(int t, int period, int dwell, skin_val_t list)
{
    const char *p = list.s;
    char out[SKIN_VAL_MAX];
    int count = 1, i, phase, idx, n = 0;

    if (!list.is_str)
        return v_str("");
    for (i = 0; p[i] != '\0'; i++)
        if (p[i] == ',')
            count++;
    if (period <= 0 || dwell <= 0) {
        idx = 0;
    } else {
        phase = t % period;
        if (phase < 0)
            phase += period;
        idx = (phase >= dwell) ? 0 : phase * count / dwell;
    }
    if (idx >= count)
        idx = count - 1;

    /* Walk to the idx-th token. */
    while (idx > 0 && *p != '\0') {
        if (*p == ',')
            idx--;
        p++;
    }
    while (*p == ' ' || *p == '\t')
        p++;
    while (*p != '\0' && *p != ',' && n < (int)sizeof out - 1)
        out[n++] = *p++;
    while (n > 0 && (out[n - 1] == ' ' || out[n - 1] == '\t'))
        n--;
    out[n] = '\0';
    return v_str(out);
}

static skin_val_t call_function(skin_expr_t *e, const char *name, int depth)
{
    skin_val_t a[4];
    int n = 0;

    skip_ws(e);
    if (!take(e, "("))
        return v_int(0);
    memset(a, 0, sizeof a);
    if (!peek(e, ")")) {
        for (;;) {
            if (n >= 4)
                break;
            a[n++] = eval_expr(e, depth + 1);
            if (take(e, ","))
                continue;
            break;
        }
    }
    (void)take(e, ")");

    if (strcmp(name, "min") == 0 && n >= 2) {
        int x = as_int(a[0]), y = as_int(a[1]);
        return v_int(x < y ? x : y);
    }
    if (strcmp(name, "max") == 0 && n >= 2) {
        int x = as_int(a[0]), y = as_int(a[1]);
        return v_int(x > y ? x : y);
    }
    if (strcmp(name, "abs") == 0 && n >= 1) {
        int x = as_int(a[0]);
        return v_int(x < 0 ? -x : x);
    }
    if (strcmp(name, "clamp") == 0 && n >= 3) {
        int x = as_int(a[0]), lo = as_int(a[1]), hi = as_int(a[2]);
        if (x < lo)
            x = lo;
        if (x > hi)
            x = hi;
        return v_int(x);
    }
    if (strcmp(name, "mod") == 0 && n >= 2) {
        int x = as_int(a[0]), y = as_int(a[1]);
        return v_int(y != 0 ? x % y : 0);
    }
    if (strcmp(name, "noise") == 0 && n >= 2)
        return v_int(noise_int(as_int(a[0]), as_int(a[1])));
    if (strcmp(name, "sin") == 0 && n >= 3)
        return v_int(sin_int(as_int(a[0]), as_int(a[1]), as_int(a[2])));
    if (strcmp(name, "seq") == 0 && n >= 4)
        return seq_value(as_int(a[0]), as_int(a[1]), as_int(a[2]), a[3]);
    if (strcmp(name, "if") == 0 && n >= 3)
        return truthy(a[0]) ? a[1] : a[2];
    return v_int(0);
}

static skin_val_t eval_primary(skin_expr_t *e, int depth)
{
    skip_ws(e);
    if (e->steps++ > SKIN_EXPR_STEPS || depth > SKIN_DEPTH) {
        e->err = 1;
        return v_int(0);
    }
    if (*e->p == '(') {
        skin_val_t v;

        e->p++;
        v = eval_expr(e, depth + 1);
        (void)take(e, ")");
        return v;
    }
    if (*e->p == '"') {
        char buf[SKIN_VAL_MAX];
        int n = 0;

        e->p++;
        while (*e->p != '\0' && *e->p != '"' && n < (int)sizeof buf - 1)
            buf[n++] = *e->p++;
        buf[n] = '\0';
        if (*e->p == '"')
            e->p++;
        return v_str(buf);
    }
    if (*e->p >= '0' && *e->p <= '9') {
        int v = 0;

        while (*e->p >= '0' && *e->p <= '9')
            v = v * 10 + (*e->p++ - '0');
        return v_int(v);
    }
    if (ident_start(*e->p)) {
        char name[24];
        int n = 0;

        while (ident_char(*e->p) && n < (int)sizeof name - 1)
            name[n++] = *e->p++;
        name[n] = '\0';
        skip_ws(e);
        if (*e->p == '(')
            return call_function(e, name, depth);
        return variable(name);
    }
    e->err = 1;
    return v_int(0);
}

static skin_val_t eval_unary(skin_expr_t *e, int depth)
{
    if (take(e, "!")) {
        skin_val_t v = eval_unary(e, depth + 1);
        return v_int(!truthy(v));
    }
    if (take(e, "-")) {
        skin_val_t v = eval_unary(e, depth + 1);
        return v_int(-as_int(v));
    }
    if (take(e, "+"))
        return eval_unary(e, depth + 1);
    return eval_primary(e, depth);
}

static skin_val_t eval_mul(skin_expr_t *e, int depth)
{
    skin_val_t v = eval_unary(e, depth);

    for (;;) {
        if (take(e, "*")) {
            v = v_int(as_int(v) * as_int(eval_unary(e, depth)));
        } else if (take(e, "/")) {
            int d = as_int(eval_unary(e, depth));
            v = v_int(d != 0 ? as_int(v) / d : 0);
        } else if (take(e, "%")) {
            int d = as_int(eval_unary(e, depth));
            v = v_int(d != 0 ? as_int(v) % d : 0);
        } else {
            return v;
        }
    }
}

static void append_val(char *dst, int *n, int cap, skin_val_t v)
{
    char tmp[16];
    const char *p;

    if (v.is_str) {
        p = v.s;
    } else {
        snprintf(tmp, sizeof tmp, "%d", v.i);
        p = tmp;
    }
    while (*p != '\0' && *n < cap - 1)
        dst[(*n)++] = *p++;
}

static int concat(skin_val_t a, skin_val_t b, skin_val_t *out)
{
    int n = 0;

    if (!a.is_str && !b.is_str)
        return 0;
    memset(out, 0, sizeof *out);
    out->is_str = 1;
    append_val(out->s, &n, SKIN_VAL_MAX, a);
    append_val(out->s, &n, SKIN_VAL_MAX, b);
    out->s[n] = '\0';
    return 1;
}

static skin_val_t eval_add(skin_expr_t *e, int depth)
{
    skin_val_t v = eval_mul(e, depth);

    for (;;) {
        if (take(e, "+")) {
            skin_val_t r = eval_mul(e, depth);
            skin_val_t c;

            if (!concat(v, r, &c))
                c = v_int(as_int(v) + as_int(r));
            v = c;
        } else if (take(e, "-")) {
            v = v_int(as_int(v) - as_int(eval_mul(e, depth)));
        } else {
            return v;
        }
    }
}

static int compare(skin_val_t a, skin_val_t b, const char *op)
{
    int r;

    if (a.is_str || b.is_str)
        r = strcmp(a.is_str ? a.s : "", b.is_str ? b.s : "");
    else
        r = (a.i < b.i) ? -1 : (a.i > b.i) ? 1 : 0;

    if (strcmp(op, "==") == 0)
        return r == 0;
    if (strcmp(op, "!=") == 0)
        return r != 0;
    if (strcmp(op, "<") == 0)
        return r < 0;
    if (strcmp(op, "<=") == 0)
        return r <= 0;
    if (strcmp(op, ">") == 0)
        return r > 0;
    return r >= 0;
}

static skin_val_t eval_cmp(skin_expr_t *e, int depth)
{
    skin_val_t v = eval_add(e, depth);

    for (;;) {
        const char *op = 0;

        if (take(e, "<="))
            op = "<=";
        else if (take(e, ">="))
            op = ">=";
        else if (take(e, "=="))
            op = "==";
        else if (take(e, "!="))
            op = "!=";
        else if (take(e, "<"))
            op = "<";
        else if (take(e, ">"))
            op = ">";
        if (op == 0)
            return v;
        v = v_int(compare(v, eval_add(e, depth), op));
    }
}

static skin_val_t eval_and(skin_expr_t *e, int depth)
{
    skin_val_t v = eval_cmp(e, depth);

    while (take(e, "&&"))
        v = v_int(truthy(v) && truthy(eval_cmp(e, depth)));
    return v;
}

static skin_val_t eval_or(skin_expr_t *e, int depth)
{
    skin_val_t v = eval_and(e, depth);

    while (take(e, "||"))
        v = v_int(truthy(v) || truthy(eval_and(e, depth)));
    return v;
}

static skin_val_t eval_expr(skin_expr_t *e, int depth)
{
    skin_val_t c = eval_or(e, depth);

    if (take(e, "?")) {
        skin_val_t a = eval_expr(e, depth + 1);
        skin_val_t b;

        (void)take(e, ":");
        b = eval_expr(e, depth + 1);
        return truthy(c) ? a : b;
    }
    return c;
}

static skin_val_t eval_raw(const char *text)
{
    skin_expr_t e;

    memset(&e, 0, sizeof e);
    e.p = text != 0 ? text : "";
    return eval_expr(&e, 0);
}

static int eval_int(const char *text, skin_ctx_t *ctx, int fallback)
{
    skin_val_t v;

    if (text == 0)
        return fallback;
    g_ctx = ctx;
    v = eval_raw(text);
    g_ctx = 0;
    return as_int(v);
}

static int eval_string(const char *text, skin_ctx_t *ctx, char *out, int cap)
{
    skin_val_t v;

    if (out == 0 || cap <= 0)
        return -1;
    out[0] = '\0';
    if (text == 0)
        return -1;
    g_ctx = ctx;
    v = eval_raw(text);
    g_ctx = 0;
    if (v.is_str)
        snprintf(out, (size_t)cap, "%s", v.s);
    else
        snprintf(out, (size_t)cap, "%d", v.i);
    return 0;
}

static void skin_copy(char *dst, int cap, const char *src)
{
    int n = 0;

    if (dst == 0 || cap <= 0)
        return;
    while (src != 0 && src[n] != '\0' && n < cap - 1) {
        dst[n] = src[n];
        n++;
    }
    dst[n] = '\0';
}

static void eval_vars(skin_ctx_t *ctx)
{
    int i;

    for (i = 0; i < s_var_count; i++) {
        skin_val_t v;

        g_ctx = ctx;
        v = eval_raw(s_vars[i].expr);
        g_ctx = 0;
        if (v.is_str) {
            s_var_is_str[i] = 1;
            skin_copy(s_var_s[i], (int)sizeof s_var_s[i], v.s);
        } else {
            s_var_is_str[i] = 0;
            s_var_i[i] = v.i;
        }
    }
}

/* ---------------------------------------------------------------- parsing */

static char *trim(char *s)
{
    char *end;

    while (*s == ' ' || *s == '\t' || *s == '\r')
        s++;
    end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r'))
        end--;
    *end = '\0';
    return s;
}

static int key_eq(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = *a++, cb = *b++;
        if (ca >= 'a' && ca <= 'z') ca -= 'a' - 'A';
        if (cb >= 'a' && cb <= 'z') cb -= 'a' - 'A';
        if (ca != cb)
            return 0;
    }
    return *a == '\0' && *b == '\0';
}

/* Read one line (without \n), skipping CR. Overlong lines become empty. */
static int read_line(const char **pp, char *line, int cap)
{
    const char *p = *pp;
    int n = 0;
    int over = 0;

    if (*p == '\0')
        return 0;
    while (*p != '\0' && *p != '\n') {
        if (*p != '\r') {
            if (n < cap - 1)
                line[n++] = *p;
            else
                over = 1;
        }
        p++;
    }
    line[n] = '\0';
    if (*p == '\n')
        p++;
    *pp = p;
    if (over)
        line[0] = '\0';
    return 1;
}

/* Read the next [section] and its key=value lines. Returns 1 on a section,
 * 0 at EOF. Leading keys before the first header are ignored. */
static int next_section(const char **pp, skin_section_t *sec)
{
    const char *p = *pp;
    char line[SKIN_LINE_MAX];

    memset(sec, 0, sizeof *sec);

    for (;;) {
        char *s;

        if (!read_line(&p, line, (int)sizeof line)) {
            *pp = p;
            return 0;
        }
        s = trim(line);
        if (s[0] == '\0' || s[0] == '#' || s[0] == ';')
            continue;
        if (s[0] == '[') {
            char *close = strchr(s, ']');

            if (close != 0) {
                *close = '\0';
                snprintf(sec->name, sizeof sec->name, "%s", s + 1);
                break;
            }
        }
        /* key before the first section: ignore */
    }

    for (;;) {
        const char *start = p;
        char *s, *eq;

        if (!read_line(&p, line, (int)sizeof line)) {
            *pp = p;
            return 1;
        }
        s = trim(line);
        if (s[0] == '\0' || s[0] == '#' || s[0] == ';')
            continue;
        if (s[0] == '[') {
            *pp = start;               /* leave the next header for the caller */
            return 1;
        }
        eq = strchr(s, '=');
        if (eq != 0 && sec->count < SKIN_PROPS) {
            *eq = '\0';
            snprintf(sec->key[sec->count], SKIN_KEY_MAX, "%s", trim(s));
            snprintf(sec->val[sec->count], SKIN_PROP_MAX, "%s", trim(eq + 1));
            sec->count++;
        }
    }
}

static const char *prop(const skin_section_t *sec, const char *key)
{
    int i;

    for (i = 0; i < sec->count; i++)
        if (key_eq(sec->key[i], key))
            return sec->val[i];
    return 0;
}

/* --------------------------------------------------------------- loading */

static int read_text(const char *path)
{
#ifdef RECHORD_AP_BUILD
    fs_file_t *f = fs_open_read(path);
    long got;

    if (f == 0)
        return -1;
    got = fs_read(f, s_text, THEME_SKIN_TEXT_MAX + 1u);
    fs_close(f);
    if (got < 0 || got > THEME_SKIN_TEXT_MAX)
        return -1;
    s_text[got] = '\0';
#else
    FILE *f = fopen(path, "rb");
    size_t got;

    if (f == 0)
        return -1;
    got = fread(s_text, 1, THEME_SKIN_TEXT_MAX + 1u, f);
    fclose(f);
    if (got > THEME_SKIN_TEXT_MAX)
        return -1;
    s_text[got] = '\0';
#endif
    return 0;
}

static int key_from_word(const char *s)
{
    /* Only vol+ / vol- are theme-mappable. PLAY is always SELECT and BACK
     * is always BACK, so a theme can never steal those keys. */
    if (key_eq(s, "up"))
        return THEME_SKIN_KEY_UP;
    if (key_eq(s, "down"))
        return THEME_SKIN_KEY_DOWN;
    return -1;
}

static int action_from_word(const char *s)
{
    if (key_eq(s, "library"))
        return THEME_SKIN_LIBRARY;
    if (key_eq(s, "player"))
        return THEME_SKIN_PLAYER;
    if (key_eq(s, "settings"))
        return THEME_SKIN_SETTINGS;
    return THEME_SKIN_NONE;
}

static void parse_list(const skin_section_t *sec)
{
    const char *v;
    int i;

    v = prop(sec, "axis");
    s_list.axis = (v != 0 && key_eq(v, "vertical"))
                      ? THEME_NAV_VERTICAL : THEME_NAV_HORIZONTAL;

    v = prop(sec, "x");
    s_list.x = v != 0 ? as_int(v_str(v)) : -1;
    v = prop(sec, "y");
    s_list.y = v != 0 ? as_int(v_str(v)) : -1;
    v = prop(sec, "height");
    s_list.height = v != 0 ? as_int(v_str(v)) : -1;
    v = prop(sec, "selected");
    s_list.selected = v != 0 ? as_int(v_str(v)) : 0;
    v = prop(sec, "rows");
    s_list.rows = v != 0 ? as_int(v_str(v)) : 0;
    v = prop(sec, "source");
    s_list.dynamic = (v != 0 && v[0] != '\0');

    v = prop(sec, "prev");
    s_list.prev_key = v != 0 ? key_from_word(v) : THEME_SKIN_KEY_UP;
    v = prop(sec, "next");
    s_list.next_key = v != 0 ? key_from_word(v) : THEME_SKIN_KEY_DOWN;
    if (s_list.prev_key < 0)
        s_list.prev_key = THEME_SKIN_KEY_UP;
    if (s_list.next_key < 0)
        s_list.next_key = THEME_SKIN_KEY_DOWN;

    s_list.count = 0;
    for (i = 0; i < SKIN_ITEMS; i++) {
        char key[16];
        char *colon;

        snprintf(key, sizeof key, "item%d", i);
        v = prop(sec, key);
        if (v == 0)
            continue;
        snprintf(s_list.item[i].label, sizeof s_list.item[i].label, "%s", v);
        colon = strchr(s_list.item[i].label, ':');
        if (colon != 0) {
            *colon = '\0';
            s_list.item[i].action = action_from_word(trim(colon + 1));
        } else {
            s_list.item[i].action = THEME_SKIN_NONE;
        }
        s_list.count = i + 1;
    }
}

int theme_skin_load(const char *root, const char *id, const char *path)
{
    char full[256];
    const char *p;
    skin_section_t sec;
    int n = 0;

    theme_skin_reset();
    if (root == 0 || id == 0 || path == 0)
        return -1;
    n = snprintf(full, sizeof full, "%s/%s/%s", root, id, path);
    if (n <= 0 || n >= (int)sizeof full)
        return -1;
    if (read_text(full) != 0)
        return -1;

    s_list.axis = theme_layout()->home_nav;
    s_list.x = -1;
    s_list.y = -1;
    s_list.height = -1;
    s_list.selected = 0;
    s_list.prev_key = THEME_SKIN_KEY_UP;
    s_list.next_key = THEME_SKIN_KEY_DOWN;
    s_list.count = 0;
    s_refresh_ms = 125;
    s_refresh_h = -1;
    s_animated = 0;
    s_var_count = 0;

    p = s_text;
    while ((n = next_section(&p, &sec)) > 0) {
        const char *type = prop(&sec, "type");

        if (key_eq(sec.name, "skin")) {
            const char *v = prop(&sec, "refresh_ms");
            if (v != 0) {
                s_refresh_ms = as_int(v_str(v));
                s_animated = s_refresh_ms > 0;
            }
            v = prop(&sec, "refresh_h");
            if (v != 0)
                s_refresh_h = as_int(v_str(v));
        } else if (key_eq(sec.name, "vars") ||
                   key_eq(sec.name, "vars2") ||
                   key_eq(sec.name, "vars3")) {
            int i;

            for (i = 0; i < sec.count && s_var_count < SKIN_VARS; i++) {
                snprintf(s_vars[s_var_count].name,
                         sizeof s_vars[s_var_count].name, "%s", sec.key[i]);
                snprintf(s_vars[s_var_count].expr,
                         sizeof s_vars[s_var_count].expr, "%s", sec.val[i]);
                s_var_count++;
            }
        } else if (type != 0 && key_eq(type, "list") && s_list.count == 0) {
            parse_list(&sec);
        }
    }
    if (s_animated && s_refresh_ms < 16)
        s_refresh_ms = 16;
    s_loaded = 1;
    return 0;
}

void theme_skin_reset(void)
{
    s_text[0] = '\0';
    s_loaded = 0;
    memset(&s_list, 0, sizeof s_list);
    s_list_src = 0;
    s_refresh_ms = 125;
    s_refresh_h = -1;
    s_animated = 0;
    s_var_count = 0;
}

const char *theme_skin_text(void)
{
    return s_loaded ? s_text : 0;
}

int theme_skin_item_count(void)
{
    if (s_list.dynamic && s_list_src != 0)
        return s_list_src->count;
    return s_list.count;
}

int theme_skin_selected_default(void)
{
    int n = theme_skin_item_count();

    if (n <= 0)
        return 0;
    return (s_list.selected >= 0 && s_list.selected < n)
               ? s_list.selected : 0;
}

int theme_skin_list_rows(void)
{
    const theme_layout_t *l = theme_layout();
    int n = theme_skin_item_count();

    if (n <= 0)
        return 0;
    if (s_list.rows > 0)
        return s_list.rows < n ? s_list.rows : n;
    if (s_list.axis == THEME_NAV_VERTICAL) {
        int y = s_list.y >= 0 ? s_list.y : l->title_h + 4;
        int h = s_list.height > 0 ? s_list.height : l->item_h;
        int fit;

        if (h <= 0)
            h = 1;
        fit = (THEME_LAYOUT_MAX_H - y) / h;
        if (fit < 1)
            fit = 1;
        return fit < n ? fit : n;
    }
    return n;
}

int theme_skin_list_rows_hint(void)
{
    return s_list.rows;
}

int theme_skin_list_bounds(int *y, int *h)
{
    const theme_layout_t *l = theme_layout();
    int row_h;
    int rows;

    if (theme_skin_item_count() <= 0)
        return -1;
    if (y != 0)
        *y = s_list.y >= 0 ? s_list.y : l->home_bar_y;
    row_h = s_list.height > 0 ? s_list.height : l->item_h;
    rows = theme_skin_list_rows();
    if (h != 0)
        *h = s_list.axis == THEME_NAV_VERTICAL ? row_h * rows : row_h;
    return 0;
}

int theme_skin_animated(void)
{
    return s_animated;
}

int theme_skin_refresh_ms(void)
{
    return s_refresh_ms;
}

int theme_skin_refresh_h(void)
{
    return s_refresh_h;
}

/* --------------------------------------------------------------- drawing */

static int isqrt_i(int v)
{
    int r = 0;

    while ((r + 1) * (r + 1) <= v)
        r++;
    return r;
}

/* Filled rounded rectangle with proper corner arcs (not stair-cut). */
static void skin_roundrect(int x, int y, int w, int h, int r, uint16_t color)
{
    int yy;

    if (w <= 0 || h <= 0)
        return;
    if (r <= 0) {
        display_fill_rect(x, y, x + w - 1, y + h - 1, color);
        return;
    }
    if (r > w / 2)
        r = w / 2;
    if (r > h / 2)
        r = h / 2;

    for (yy = y; yy < y + h; yy++) {
        int inset = 0;

        if (yy < y + r) {
            int dy = y + r - yy;
            inset = r - isqrt_i(r * r - dy * dy);
        } else if (yy >= y + h - r) {
            int dy = yy - (y + h - r - 1);
            inset = r - isqrt_i(r * r - dy * dy);
        }
        if (inset < 0)
            inset = 0;
        if (inset * 2 >= w)
            continue;
        display_fill_rect(x + inset, yy, x + w - 1 - inset, yy, color);
    }
}

static int tri_edge(int ax, int ay, int bx, int by, int x, int y)
{
    return (x - ax) * (by - ay) - (y - ay) * (bx - ax);
}

static void skin_triangle(int x1, int y1, int x2, int y2, int x3, int y3,
                          uint16_t color)
{
    int minx, maxx, miny, maxy, y;

    if (tri_edge(x1, y1, x2, y2, x3, y3) == 0)
        return;                       /* degenerate: draw nothing */
    minx = x1 < x2 ? (x1 < x3 ? x1 : x3) : (x2 < x3 ? x2 : x3);
    maxx = x1 > x2 ? (x1 > x3 ? x1 : x3) : (x2 > x3 ? x2 : x3);
    miny = y1 < y2 ? (y1 < y3 ? y1 : y3) : (y2 < y3 ? y2 : y3);
    maxy = y1 > y2 ? (y1 > y3 ? y1 : y3) : (y2 > y3 ? y2 : y3);
    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx > THEME_LAYOUT_MAX_W - 1) maxx = THEME_LAYOUT_MAX_W - 1;
    if (maxy > THEME_LAYOUT_MAX_H - 1) maxy = THEME_LAYOUT_MAX_H - 1;

    for (y = miny; y <= maxy; y++) {
        int first = -1, last = -1, x;

        for (x = minx; x <= maxx; x++) {
            int e0 = tri_edge(x1, y1, x2, y2, x, y);
            int e1 = tri_edge(x2, y2, x3, y3, x, y);
            int e2 = tri_edge(x3, y3, x1, y1, x, y);

            if ((e0 >= 0 && e1 >= 0 && e2 >= 0) ||
                (e0 <= 0 && e1 <= 0 && e2 <= 0)) {
                if (first < 0)
                    first = x;
                last = x;
            }
        }
        if (first >= 0)
            display_fill_rect(first, y, last, y, color);
    }
}

static uint16_t color_of(const char *value, skin_ctx_t *ctx, uint16_t fallback)
{
    skin_val_t v;

    if (value == 0)
        return fallback;
    g_ctx = ctx;
    v = eval_raw(value);
    g_ctx = 0;
    if (!v.is_str)
        return (uint16_t)v.i;
    if (key_eq(v.s, "background")) return theme_get()->background;
    if (key_eq(v.s, "surface"))    return theme_get()->surface;
    if (key_eq(v.s, "text"))       return theme_get()->text;
    if (key_eq(v.s, "text_dim"))   return theme_get()->text_dim;
    if (key_eq(v.s, "accent"))     return theme_get()->accent;
    if (key_eq(v.s, "danger"))     return theme_get()->danger;
    if (key_eq(v.s, "success"))    return theme_get()->success;
    return fallback;
}

static const char *row_label(int idx, char *buf, int cap)
{
    if (s_list.dynamic) {
        if (s_list_src != 0 && s_list_src->label != 0)
            return s_list_src->label(idx, buf, cap);
        return "";
    }
    if (idx >= 0 && idx < s_list.count)
        return s_list.item[idx].label;
    return "";
}

static const char *row_value(int idx, char *buf, int cap)
{
    if (s_list.dynamic && s_list_src != 0 && s_list_src->value != 0)
        return s_list_src->value(idx, buf, cap);
    return 0;
}

static void draw_list(int sel, int top)
{
    const theme_layout_t *l = theme_layout();
    int n = theme_skin_item_count();
    int rows;
    int i;

    if (n <= 0)
        return;
    if (sel < 0 || sel >= n)
        sel = 0;
    rows = theme_skin_list_rows();
    if (rows <= 0 || rows > n)
        rows = n;
    if (top < 0)
        top = 0;
    if (top > n - rows)
        top = n - rows;

    if (s_list.axis == THEME_NAV_VERTICAL) {
        int x = s_list.x >= 0 ? s_list.x : l->pad;
        int y = s_list.y >= 0 ? s_list.y : l->title_h + 4;
        int h = s_list.height > 0 ? s_list.height : l->item_h;

        for (i = 0; i < rows; i++) {
            int idx = top + i;
            int yy = y + i * h;
            char lbuf[64], vbuf[32];
            const char *label = row_label(idx, lbuf, (int)sizeof lbuf);
            const char *value = row_value(idx, vbuf, (int)sizeof vbuf);

            if (idx == sel) {
                display_fill_rect(0, yy, THEME_LAYOUT_MAX_W - 1, yy + h - 1,
                                  theme_get()->text);
                display_draw_text(x, yy + 1, label,
                                  theme_get()->background, theme_get()->text);
            } else {
                display_draw_text(x, yy + 1, label,
                                  theme_get()->text, theme_get()->background);
            }
            if (value != 0 && value[0] != '\0') {
                int vw = (int)strlen(value) * SKIN_FONT_W;
                int vx = THEME_LAYOUT_MAX_W - l->pad - vw;

                if (vx < x + 8)
                    vx = x + 8;
                display_draw_text(vx, yy + 1, value,
                                  idx == sel ? theme_get()->background
                                             : theme_get()->text_dim,
                                  idx == sel ? theme_get()->text
                                             : theme_get()->background);
            }
        }
        return;
    }

    {
        int widths[8];
        int total = 0;
        int x, y, h;

        if (rows > 8)
            rows = 8;
        for (i = 0; i < rows; i++) {
            char lbuf[64];
            const char *label = row_label(top + i, lbuf, (int)sizeof lbuf);

            widths[i] = (int)strlen(label) * SKIN_FONT_W;
            total += widths[i];
        }
        total += 16 * (rows - 1);
        x = s_list.x >= 0 ? s_list.x : (THEME_LAYOUT_MAX_W - total) / 2;
        if (x < l->pad)
            x = l->pad;
        y = s_list.y >= 0 ? s_list.y : l->home_bar_y;
        h = s_list.height > 0 ? s_list.height : l->home_bar_h;

        for (i = 0; i < rows; i++) {
            int idx = top + i;
            char lbuf[64];
            const char *label = row_label(idx, lbuf, (int)sizeof lbuf);

            if (idx == sel) {
                display_fill_rect(x - 4, y - 1, x + widths[i] + 3,
                                  y + h - 2, theme_get()->text);
                display_draw_text(x, y + 1, label,
                                  theme_get()->background, theme_get()->text);
            } else {
                display_draw_text(x, y + 1, label,
                                  theme_get()->text, theme_get()->background);
            }
            x += widths[i] + 16;
        }
    }
}

static void render_section(const skin_section_t *sec, skin_ctx_t *ctx, int sel)
{
    const char *type = prop(sec, "type");
    int x, y, w, h;

    if (type == 0)
        return;
    {
        const char *vis = prop(sec, "visible");

        if (vis != 0 && eval_int(vis, ctx, 1) == 0)
            return;
    }

    if (key_eq(type, "roundrect")) {
        int r = eval_int(prop(sec, "r"), ctx, 0);

        x = eval_int(prop(sec, "x"), ctx, 0);
        y = eval_int(prop(sec, "y"), ctx, 0);
        w = eval_int(prop(sec, "w"), ctx, 0);
        h = eval_int(prop(sec, "h"), ctx, 0);
        skin_roundrect(x, y, w, h, r,
                       color_of(prop(sec, "color"), ctx, theme_get()->text));
        return;
    }

    if (key_eq(type, "triangle")) {
        int x1 = eval_int(prop(sec, "x1"), ctx, 0);
        int y1 = eval_int(prop(sec, "y1"), ctx, 0);
        int x2 = eval_int(prop(sec, "x2"), ctx, 0);
        int y2 = eval_int(prop(sec, "y2"), ctx, 0);
        int x3 = eval_int(prop(sec, "x3"), ctx, 0);
        int y3 = eval_int(prop(sec, "y3"), ctx, 0);

        skin_triangle(x1, y1, x2, y2, x3, y3,
                      color_of(prop(sec, "color"), ctx,
                               theme_get()->background));
        return;
    }

    if (key_eq(type, "fill")) {
        x = eval_int(prop(sec, "x"), ctx, 0);
        y = eval_int(prop(sec, "y"), ctx, 0);
        w = eval_int(prop(sec, "w"), ctx, THEME_LAYOUT_MAX_W);
        h = eval_int(prop(sec, "h"), ctx, THEME_LAYOUT_MAX_H);
        display_fill_rect(x, y, x + w - 1, y + h - 1,
                          color_of(prop(sec, "color"), ctx,
                                   theme_get()->text));
        return;
    }

    if (key_eq(type, "bitmap")) {
        char name[SKIN_VAL_MAX];
        const uint8_t *bits;
        int aw = 0, ah = 0, stride = 0;

        if (eval_string(prop(sec, "asset"), ctx, name, sizeof name) != 0 ||
            name[0] == '\0')
            return;
        bits = theme_asset_get(name, &aw, &ah, &stride);
        if (bits == 0)
            return;
        ctx->aw = aw;
        ctx->ah = ah;
        x = eval_int(prop(sec, "x"), ctx, 0);
        y = eval_int(prop(sec, "y"), ctx, 0);
        display_bitmap1(x, y, aw, ah, stride, bits,
                        color_of(prop(sec, "color"), ctx, theme_get()->text),
                        color_of(prop(sec, "bg"), ctx, theme_get()->background));
        return;
    }

    if (key_eq(type, "text")) {
        char text[SKIN_VAL_MAX];

        if (eval_string(prop(sec, "text"), ctx, text, sizeof text) != 0)
            return;
        x = eval_int(prop(sec, "x"), ctx, 0);
        y = eval_int(prop(sec, "y"), ctx, 0);
        display_draw_text(x, y, text,
                          color_of(prop(sec, "color"), ctx, theme_get()->text),
                          color_of(prop(sec, "bg"), ctx, theme_get()->background));
        return;
    }

    if (key_eq(type, "bar")) {
        int value, max;

        x = eval_int(prop(sec, "x"), ctx, 0);
        y = eval_int(prop(sec, "y"), ctx, 0);
        w = eval_int(prop(sec, "w"), ctx, THEME_LAYOUT_MAX_W);
        h = eval_int(prop(sec, "h"), ctx, 2);
        value = eval_int(prop(sec, "value"), ctx, 0);
        max = eval_int(prop(sec, "max"), ctx, 100);
        if (max <= 0)
            max = 100;
        if (value < 0)
            value = 0;
        if (value > max)
            value = max;
        display_fill_rect(x, y, x + w - 1, y + h - 1,
                          color_of(prop(sec, "bg"), ctx,
                                   theme_get()->surface));
        if (value > 0) {
            int fill = w * value / max;

            display_fill_rect(x, y, x + fill - 1, y + h - 1,
                              color_of(prop(sec, "color"), ctx,
                                       theme_get()->accent));
        }
        return;
    }

    if (key_eq(type, "list")) {
        draw_list(sel, ctx->top);
        return;
    }
}

void theme_skin_render(unsigned now, unsigned idle, int sel, int top,
                       const theme_skin_state_t *st)
{
    skin_ctx_t ctx;
    const char *p;
    skin_section_t sec;
    int sections = 0;

    if (!s_loaded)
        return;
    memset(&ctx, 0, sizeof ctx);
    ctx.now = now;
    ctx.idle = idle;
    ctx.sel = sel;
    ctx.top = top;
    ctx.st = st;
    eval_vars(&ctx);
    p = s_text;
    while (next_section(&p, &sec)) {
        if (++sections > SKIN_SECTIONS)
            break;
        render_section(&sec, &ctx, sel);
    }
}

int theme_skin_nav(int key)
{
    int n = theme_skin_item_count();

    if (key == THEME_SKIN_KEY_BACK)
        return THEME_SKIN_BACK;
    if (n <= 0)
        return THEME_SKIN_NONE;
    if (key == s_list.prev_key)
        return THEME_SKIN_PREV;
    if (key == s_list.next_key)
        return THEME_SKIN_NEXT;
    if (key == THEME_SKIN_KEY_SELECT)
        return THEME_SKIN_SELECT;
    return THEME_SKIN_NONE;
}

int theme_skin_key(int key, int *sel)
{
    int n = theme_skin_item_count();
    int nav;

    if (sel == 0)
        return THEME_SKIN_NONE;
    nav = theme_skin_nav(key);
    if (nav == THEME_SKIN_BACK)
        return THEME_SKIN_BACK;
    if (n <= 0 || nav == THEME_SKIN_NONE)
        return THEME_SKIN_NONE;
    if (*sel < 0 || *sel >= n)
        *sel = 0;
    if (nav == THEME_SKIN_PREV) {
        *sel = (*sel + n - 1) % n;
        return THEME_SKIN_REDRAW;
    }
    if (nav == THEME_SKIN_NEXT) {
        *sel = (*sel + 1) % n;
        return THEME_SKIN_REDRAW;
    }
    if (nav == THEME_SKIN_SELECT) {
        if (s_list.dynamic)
            return THEME_SKIN_SELECT;
        return s_list.item[*sel].action;
    }
    return THEME_SKIN_NONE;
}
