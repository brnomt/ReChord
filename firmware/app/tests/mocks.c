/*
 * mocks.c — host-test mocks implementing the REAL service/driver/ui headers
 * (never shims: the app compiles against the real contracts, so the mocks
 * must match those prototypes exactly).
 *
 * All mocks feed the ordered call recorder in mocks.h; the bring-up test
 * asserts the recorded sequence against the documented bring-up order.
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "app/boot_params.h"
#include "app/internal/shims.h"
#include "drivers/display.h"
#include "services/input.h"
#include "services/log.h"
#include "services/settings.h"
#include "theme/theme.h"
#include "ui/ui.h"

#include "mocks.h"

/* ---- ordered call recorder ---- */
#define MOCK_REC_MAX   256
#define MOCK_LOG_LINES 128
#define MOCK_LOG_LEN   160

static const char *g_rec[MOCK_REC_MAX];
static int         g_rec_n;

/* Persistent buffers for formatted log lines (the recorder stores
 * pointers, so messages must outlive the call). */
static char g_log_bufs[MOCK_LOG_LINES][MOCK_LOG_LEN];
static int  g_log_next;

static void input_reset(void);   /* scripted input queue, defined below */

void mock_rec_reset(void)
{
    g_rec_n = 0;
    g_log_next = 0;
    input_reset();
}

int mock_rec_count(void)
{
    return g_rec_n;
}

const char *mock_rec_at(int idx)
{
    return (idx >= 0 && idx < g_rec_n) ? g_rec[idx] : NULL;
}

void mock_rec(const char *tag)
{
    if (g_rec_n < MOCK_REC_MAX)
        g_rec[g_rec_n++] = tag;
}

static char *log_buf(void)
{
    char *b = g_log_bufs[g_log_next];

    g_log_next = (g_log_next + 1) % MOCK_LOG_LINES;
    return b;
}

/* ---- services/log ---- */
void log_init(log_clock_fn clock_ms)
{
    (void)clock_ms;
    mock_rec("log_init");
}

void log_printf(const char *fmt, ...)
{
    char *buf = log_buf();
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, MOCK_LOG_LEN, fmt, ap);
    va_end(ap);
    mock_rec(buf);
}

int log_flush(const char *tag)
{
    char *buf = log_buf();

    snprintf(buf, MOCK_LOG_LEN, "log_flush:%s", tag ? tag : "(null)");
    mock_rec(buf);
    return LOG_OK;
}

/* ---- services/settings ---- */
void settings_defaults(settings_t *s)
{
    (void)s;
    mock_rec("settings_defaults");
}

int settings_load(settings_t *s)
{
    (void)s;
    mock_rec("settings_load");
    return SETTINGS_OK;   /* no config file in tests: pretend it loaded    */
}

/* ---- services/input (scripted queue) ---- */
typedef struct {
    int type;
    int adc;
    int remaining;   /* polls left before the event becomes visible       */
    int active;
} mock_input_t;

static mock_input_t g_input[8];

static void input_reset(void)
{
    int i;

    for (i = 0; i < 8; i++)
        g_input[i].active = 0;
}

static void input_add(int delay, int type, int adc)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (!g_input[i].active) {
            g_input[i].active = 1;
            g_input[i].type = type;
            g_input[i].adc = adc;
            g_input[i].remaining = delay;
            return;
        }
    }
}

void mock_input_queue(int type, int adc_value)
{
    input_add(0, type, adc_value);
}

void mock_input_delay(int delay_polls, int type, int adc_value)
{
    input_add(delay_polls, type, adc_value);
}

int input_poll(input_event_t *ev)
{
    int i;

    mock_rec("input_poll");

    if (ev == NULL)
        return INPUT_ERR_ARG;

    /* Deliver due events first ... */
    for (i = 0; i < 8; i++) {
        if (g_input[i].active && g_input[i].remaining == 0) {
            ev->type = g_input[i].type;
            ev->adc_value = g_input[i].adc;
            g_input[i].active = 0;
            return 1;
        }
    }
    /* ... then tick the delayed ones. */
    for (i = 0; i < 8; i++) {
        if (g_input[i].active && g_input[i].remaining > 0)
            g_input[i].remaining--;
    }
    return 0;
}

/* ---- drivers/display ---- */
void display_init(void)
{
    mock_rec("display_init");
}

void display_clear(uint16_t rgb565)
{
    (void)rgb565;
    mock_rec("display_clear");
}

void display_flush_rows(int y0, int y1)
{
    (void)y0;
    (void)y1;
    mock_rec("display_flush");
}

/* ---- theme ---- */
static theme_t g_theme = { "Classic" };   /* rest zero: names are enough  */

const theme_t *theme_get(void)
{
    mock_rec("theme_get");
    return &g_theme;
}

int theme_load_named(const char *name)
{
    (void)name;
    mock_rec("theme_load_named");
    return 0;
}

/* ---- ui ---- */
const ui_screen_t boot_screen = { "boot", NULL, NULL, NULL, NULL };

void ui_init(void)
{
    mock_rec("ui_init");
}

bool ui_push_screen(const ui_screen_t *screen)
{
    (void)screen;
    mock_rec("ui_push_screen");
    return true;
}

void ui_handle_event(const input_event_t *ev)
{
    (void)ev;
    mock_rec("ui_handle_event");
}

void ui_run_frame(void)
{
    mock_rec("ui_run_frame");
}

/* ---- modules / player (shim prototypes, internal/shims.h) ---- */
int modules_init(void)
{
    mock_rec("modules_init");
    return 0;
}

int modules_start_default(void)
{
    mock_rec("modules_start_default");
    return 0;
}

int player_init(void)
{
    mock_rec("player_init");
    return 0;
}

/* ---- board hooks (strong overrides of the weak defaults in main.c) ---- */
void rechord_hw_early_init(void)
{
    mock_rec("hw_early");
}

void rechord_platform_idle(void)
{
    mock_rec("platform_idle");
}

void rechord_platform_sleep(void)
{
    mock_rec("platform_sleep");
}

/* ---- boot param storage (boot_params.h STORAGE CONTRACT) ---- */
rechord_boot_params_t rechord_boot_params;
uint32_t              rechord_boot_params_captured;
