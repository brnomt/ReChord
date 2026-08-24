/*
 * rechord_app.c — ReChord V0.15: stock-mirror boot + UI-event draw hooks.
 *
 * THE V0.15 FIX (root cause of V0.1-V0.14 "nothing custom ever shows"):
 * the stock firmware_entry @ 0x03000010 is an HW-init callback that
 * RETURNS to the ROM. Ghidra disasm of stock:
 *     push {r4,lr}; mov r4,r0; bl boot_param_layout; rom_alloc(0x1dc);
 *     [rom_hw_init(0x1dc/0x16f/0x16f) rom_hw_init2(0x171) rom_hw_init(0x170)]
 *     rom_early_init(); ldrh r0,[r4]; cmp r0,#0xb;
 *     pop {r4,lr}; movw r0,#0x18f; b.w rom_hw_init2      <- early path
 *     pop {r4,lr}; movw r0,#0x191; b.w rom_hw_init2      <- app path
 * Our V0.1-V0.14 entries did `b rechord_app` -> infinite loop -> the ROM
 * boot sequence stalled -> cassette stays, no text, no app, ~23s poweroff.
 *
 * V0.15: entry_stubs.S mirrors stock (pop {r4,lr}; b.w rom_hw_init2), so
 * rechord_firmware_entry() below is the C body: it runs the EXACT stock
 * init sequence and returns the rom_hw_init2 code (0x18f/0x191) that the
 * assembly tail-calls. Control returns to the ROM inside rom_hw_init2,
 * exactly like stock.
 *
 * rechord_ui_event() = the draw test, called from the application_start
 * stub (0x0300710a) whenever the ROM dispatches UI events. It paints the
 * screen through the loader's OWN display sequence (the one sec2 uses to
 * draw the cassette — disassembled from sec2 FUN_03000258):
 *     fef124(0x19b) wait-ready -> fea848(1)/fea824(2) save ctx ->
 *     feb0f6(color) -> fea8f4(x,y,w,h,a5,a6) rect -> feabea(1) refresh ->
 *     fea848(saved)/fea824(saved) restore ctx
 * NOTE: fea8f4 takes SIX args (r0=x,r1=y,r2=w,r3=h,sp+0=a5,sp+4=a6) — the
 * V0.10 packed-2-arg and V0.14 4-arg calls were wrong. The known-good
 * top-bar rect is (0,3,320,16,2,0x58).
 */
#include <stdint.h>
#include <stddef.h>

/* ---- ROM display services (the loader's draw sequence) ---- */
#ifndef RECHORD_QEMU_TEST
typedef uint32_t (*rom_ready_fn)(uint32_t);
typedef uint32_t (*rom_ctx_fn)(uint32_t);
typedef void (*rom_color_fn)(uint32_t);
typedef void (*rom_rect_fn)(uint32_t, uint32_t, uint32_t, uint32_t,
                            uint32_t, uint32_t);
typedef void (*rom_refresh_fn)(uint32_t);

#define ROM_DISP_WAIT   ((rom_ready_fn)(0x02fef124 | 1))  /* wait ready(code) -> 0 if ok */
#define ROM_DISP_CTX_A  ((rom_ctx_fn)(0x02fea848 | 1))    /* save/set A -> prev */
#define ROM_DISP_CTX_B  ((rom_ctx_fn)(0x02fea824 | 1))    /* save/set B -> prev */
#define ROM_DISP_COLOR  ((rom_color_fn)(0x02feb0f6 | 1))  /* set fill color */
#define ROM_DISP_RECT   ((rom_rect_fn)(0x02fea8f4 | 1))   /* fill rect(x,y,w,h,a5,a6) */
#define ROM_DISP_REFRESH ((rom_refresh_fn)(0x02feabea | 1)) /* refresh(1) */

/* ---- ROM HW services used by the stock firmware_entry mirror ----
 * NOTE: all addresses MUST have bit 0 set (|1) for Thumb function pointers
 * on Cortex-M3. Without it, BLX to these addresses enters ARM state and
 * faults immediately. This was the root cause of V0.15 crashing on real HW
 * when it tried to call rom_hw_init through a function pointer. ---- */
#define ROM_ALLOC       ((void *(*)(uint32_t))(0x02feeedc | 1))  /* rom_alloc */
#define ROM_HW_INIT     ((void (*)(uint32_t))(0x02feeebe | 1))   /* rom_hw_init */
#define ROM_HW_INIT2    ((void (*)(uint32_t))(0x02feee7c | 1))   /* rom_hw_init2 */
#define ROM_EARLY_INIT  ((void (*)(void))(0x02fe860e | 1))       /* rom_early_init */

/* ---- telemetry RAM (inside our .text.boot padding; see entry_stubs.S) ---- */
#define crash_log ((volatile uint32_t *)0x03000100u)   /* fault.c crash log */
#define CRASH_MAGIC 0x52454348u
#define boot_log  ((volatile uint32_t *)0x03000118u)   /* boot markers */
#define BOOT_DONE 0xfeed0002u
#else  /* RECHORD_QEMU_TEST: redirect ROM calls + telemetry to QEMU RAM stubs
       * (implemented in firmware/qemu/qemu_echo_main.c) */
extern uint32_t rch_qemu_rom_wait(uint32_t);
extern uint32_t rch_qemu_rom_ctx(uint32_t);
extern void rch_qemu_rom_color(uint32_t);
extern void rch_qemu_rom_rect(uint32_t, uint32_t, uint32_t, uint32_t,
                              uint32_t, uint32_t);
extern void rch_qemu_rom_refresh(uint32_t);
extern void *rch_qemu_rom_alloc(uint32_t);
extern void rch_qemu_rom_hw_init(uint32_t);
extern void rch_qemu_rom_hw_init2(uint32_t);
extern void rch_qemu_rom_early_init(void);
#define ROM_DISP_WAIT   rch_qemu_rom_wait
#define ROM_DISP_CTX_A  rch_qemu_rom_ctx
#define ROM_DISP_CTX_B  rch_qemu_rom_ctx
#define ROM_DISP_COLOR  rch_qemu_rom_color
#define ROM_DISP_RECT   rch_qemu_rom_rect
#define ROM_DISP_REFRESH rch_qemu_rom_refresh
#define ROM_ALLOC       rch_qemu_rom_alloc
#define ROM_HW_INIT     rch_qemu_rom_hw_init
#define ROM_HW_INIT2    rch_qemu_rom_hw_init2
#define ROM_EARLY_INIT  rch_qemu_rom_early_init
#define crash_log ((volatile uint32_t *)0x20008000u)
#define CRASH_MAGIC 0x52454348u
#define boot_log  ((volatile uint32_t *)0x20008018u)
#define BOOT_DONE 0xfeed0002u
#endif

/*
 * rechord_firmware_entry — C body of the stock firmware_entry mirror.
 * Called from entry_stubs.S with the boot params in r0; returns the
 * rom_hw_init2 code (0x18f or 0x191) that the assembly tail-calls so
 * control returns to the ROM exactly like stock.
 */
uint32_t rechord_firmware_entry(void *param)
{
    uint16_t *bp = (uint16_t *)param;
#ifdef RECHORD_QEMU_TEST
    volatile uint8_t  *lay = (volatile uint8_t *)0x20008080u;   /* boot layout */
    volatile uint16_t *bmode = (volatile uint16_t *)0x20008084u;
#else
    volatile uint8_t  *lay = (volatile uint8_t *)0x03000164u;  /* boot layout */
    volatile uint16_t *bmode = (volatile uint16_t *)0x03000168u;
#endif
    uint16_t mode;
    uint32_t m, cols, pad;
    void *ctx;

    /* telemetry: firmware_entry was reached, with the boot params ptr */
    boot_log[0] = 0x424F4F54u;              /* 'BOOT' */
    boot_log[1] = (uint32_t)(uintptr_t)param;

    /* ---- boot_param_layout(param) — exact stock behavior ---- */
    *lay = 8;                               /* +0 base */
    mode = 0;
    switch (*bp) { case 0: case 1: case 2: case 3: case 4: case 5:
                   case 8: case 10: mode = *bp; }
    *bmode = mode;                          /* ushort @0x03000168 */
    mode = 0;
    switch (*bp) { case 0: case 1: case 2: case 3: case 4: case 5:
                   case 8: case 10: mode = *bp; }
    lay[5] = (uint8_t)mode;                 /* +5 mode_clamped */
    if (mode > 0x0c) lay[5] = 0x0d;
    if (*bmode > 0x0c) *bmode = 0x0d;
    m = lay[5];
    cols = ((m / 6) * 3 & 0x7f) * 2;
    lay[1] = (uint8_t)cols;                 /* +1 cols */
    if (m <= cols && cols - m != 0) { cols -= 6; lay[1] = (uint8_t)cols; }
    lay[2] = (uint8_t)(lay[5] - (uint8_t)cols);  /* +2 rem */
    pad = 8 - (cols & 0xff);
    if (pad > 6) pad = 6;
    lay[4] = (uint8_t)pad;                  /* +4 pad */
    lay[3] = 0;                             /* +3 zero */

    /* ---- rom_alloc(0x1dc) + rom_hw_init sequence (exact stock) ---- */
    ctx = ROM_ALLOC(0x1dc);
    if (ctx == NULL) {
        ROM_HW_INIT2(0x16f);
    } else {
        ROM_HW_INIT(0x1dc);
        ROM_HW_INIT(0x16f);
        ROM_HW_INIT(0x16f);
        ROM_HW_INIT2(0x171);
        ROM_HW_INIT(0x170);
    }
    ROM_EARLY_INIT();

    boot_log[2] = BOOT_DONE;                /* boot init complete */

    /* mode check: return the code entry_stubs.S must tail-call */
    return (*bp != 0x0b) ? 0x18f : 0x191;
}

/*
 * rechord_hw_init — ROM hardware initialization (Option B boot path).
 * Called from entry_stubs.S with boot params in r0. Runs the stock init
 * sequence (PLLs, clocks, LCD controller, rom_early_init) so the hardware
 * is ready, then returns. After this, entry_stubs.S branches to rechord_main.
 */
void rechord_hw_init(void *param)
{
    rechord_firmware_entry(param);
}

/*
 * rechord_main — our app main (camino B, from-source).
 *
 * Called from firmware_entry AFTER rechord_hw_init sets up the hardware.
 * Zeroes BSS (ScatterLoader2), inits the board (BSP_Init2), registers
 * mailbox servers, then runs our own UI loop.
 */
extern void ScatterLoader2(void);
extern void BSP_Init2(void);
extern int  Main2(void);   /* SDK BB audio service loop (never returns) */

/* ---- BB mailbox handshake: register the decode/file servers and send the
 *      SYSTEM_START_OK heartbeat so the AP knows the BB is alive ---- */
extern void RegHifiDecodeServer(void);
extern void RegHifiFileServer(void);
extern int  MailBoxWriteB2ACmd(uint32_t cmd, uint32_t id, uint32_t channel);
extern int  MailBoxWriteB2AData(uint32_t data, uint32_t id, uint32_t channel);
#define MSGBOX_CMD_SYSTEM_START_OK 0x0001u
#define MAILBOX_ID_0    0u
#define MAILBOX_CHANNEL_0 0u

static volatile uint32_t g_redraw = 0;
static volatile uint32_t g_menu_sel = 0;

/* ROM input-event API (the key handler reads the current key/event) */
#define ROM_GET_INPUT_EVENT ((uint32_t (*)(void))(0x02ff813a | 1))

/* ---- UI framebuffer (the buffer the LCD hardware scans) ---- */
#ifdef RECHORD_QEMU_TEST
#define UI_FB   ((volatile uint16_t *)0x20010000u)
#else
#define UI_FB   ((volatile uint16_t *)0x03024868u)   /* 320x100 RGB565 */
#endif
#define LCD_W   320
#define LCD_H   100

static void fb_fill_rect(int x0, int y0, int x1, int y1, uint16_t c)
{
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
            UI_FB[y * LCD_W + x] = c;
}

/* ---- 5x7 bitmap font (5 columnas, bit0 = fila superior) ---- */
static const uint8_t font5x7[][5] = {
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' (36) */
    {0x00,0x00,0x60,0x60,0x00}, /* '.' (37) */
    {0x00,0x36,0x36,0x00,0x00}, /* ':' (38) */
    {0x20,0x10,0x08,0x04,0x02}, /* '/' (39) */
    {0x08,0x08,0x08,0x08,0x08}, /* '-' (40) */
};

static const uint8_t *fb_glyph(char c)
{
    if (c >= 'A' && c <= 'Z') return font5x7[c - 'A'];
    if (c >= '0' && c <= '9') return font5x7[26 + (c - '0')];
    if (c == ' ')  return font5x7[36];
    if (c == '.')  return font5x7[37];
    if (c == ':')  return font5x7[38];
    if (c == '/')  return font5x7[39];
    if (c == '-')  return font5x7[40];
    return font5x7[36];   /* desconocido -> espacio */
}

static void fb_char(int x, int y, char c, uint16_t color)
{
    const uint8_t *g = fb_glyph(c);
    for (int col = 0; col < 5; col++)
        for (int row = 0; row < 7; row++)
            if (g[col] & (1 << row)) {
                int px = x + col, py = y + row;
                if (px >= 0 && px < LCD_W && py >= 0 && py < LCD_H)
                    UI_FB[py * LCD_W + px] = color;
            }
}

static void fb_text(int x, int y, const char *s, uint16_t color)
{
    while (*s) {
        fb_char(x, y, *s, color);
        x += 6;
        s++;
    }
}

/* draw our menu (text labels + selection) straight into the LCD framebuffer */
static void rechord_draw_menu(void)
{
    static const char *items[4] = {"MUSIC", "EQ", "SETTINGS", "ABOUT"};
    uint32_t ctx_a, ctx_b;

    fb_fill_rect(0, 0, LCD_W, LCD_H, 0x0000);       /* black background */
    fb_fill_rect(0, 0, LCD_W, 20, 0x001F);          /* blue title bar  */
    fb_text(4, 4, "RECHORD", 0xFFFF);               /* title           */

    for (uint32_t i = 0; i < 4; i++) {
        int y = 26 + i * 16;
        uint16_t col = (i == g_menu_sel) ? 0x07E0u /* green sel */
                                         : 0x8410u /* gray */;
        fb_fill_rect(4, y, LCD_W - 4, y + 14, col);
        fb_text(8, y + 3, items[i], (i == g_menu_sel) ? 0x0000u : 0xFFFFu);
    }

    /* ---- V0.17 known-good display refresh ----
     * The LCD reads the framebuffer only after the ROM's refresh is
     * triggered (fef124 wait -> fea848/fea824 ctx -> feabea refresh ->
     * restore). V0.1/V0.17 did exactly this and showed colors; M1 dropped
     * it and the loader cassette stayed on screen. */
    if (ROM_DISP_WAIT(0x19b) == 0) {
        ctx_a = ROM_DISP_CTX_A(1);
        ctx_b = ROM_DISP_CTX_B(2);
        ROM_DISP_REFRESH(1);
        ROM_DISP_CTX_A(ctx_a);
        ROM_DISP_CTX_B(ctx_b);
    }

    boot_log[3] = g_menu_sel;                        /* telemetry: selection */
}

/*
 * rechord_key_handler — called by the ROM on key events (fixed offset
 * 0x0301020c). Runs in interrupt context: only update state + set the
 * redraw flag; the main loop does the actual drawing.
 */
void rechord_key_handler(void)
{
    uint32_t ev = ROM_GET_INPUT_EVENT();
    if (ev != 0) {
        g_menu_sel = (g_menu_sel + 1) & 3;           /* cycle selection */
        g_redraw = 1;
    }
}

void rechord_main(void)
{
    boot_log[0] = 0x53544F42u;   /* 'BOTS' — telemetry for a debugger */

    /* Run the SDK BB service loop. Main2() is what consumes the decode/file
     * mailbox flags set by MailBoxDecService / MailBoxFileService and replies
     * *_ERR / *_CMPL back to the AP. The previous body here re-ran the setup
     * (ScatterLoader2/BSP_Init2/Reg*Server/START_OK) but replaced Main2's
     * service loop with a UI draw loop. The UI loop never displays (the AP
     * owns the screen), so those flags were never consumed, no reply was ever
     * sent, the AP waited ~20 s on DEC_OPEN, and the watchdog powered the
     * device off on every menu entry. */
    Main2();

    for (;;)
        ;
}

/*
 * rechord_ui_event — draw test, called from the application_start stub
 * (0x0300710a) whenever the ROM dispatches UI events. Paints the screen
 * through the loader's OWN display sequence; the full screen alternates
 * red/blue per call (liveness) with a white top status bar over it. If a
 * crash was logged by fault.c, holds a stable PC-derived color instead.
 */
void rechord_ui_event(void)
{
    static uint32_t calls = 0;
    uint32_t n = calls++;

    /* ROM display API DISABLED — the hardcoded 0x02feXXXX addresses fault the
     * Thumb ROM (even ARM-state pointers), crashing the BB on the first UI
     * event. Keep only telemetry; no display until the real ROM API is mapped. */
    boot_log[3] = n;
}
