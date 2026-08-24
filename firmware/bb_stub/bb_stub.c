/*
 * bb_stub.c — minimal dummy BB core for the Echo Mini (RKnanoC).
 *
 * Purpose: a drop-in section_3 replacement that does NOTHING except keep
 * the stock AP (fw1) alive. It:
 *   1. runs the stock ROM HW-init sequence (Option B boot, V0.20 path),
 *   2. builds a full Cortex-M3 vector table in SRAM and points VTOR at it,
 *   3. enables the mailbox (clock gate + A2B interrupts 0..3 + NVIC IRQ 9..12),
 *   4. sends MSGBOX_CMD_SYSTEM_START_OK on B2A channel 0 (boot handshake),
 *   5. answers every AP->BB mailbox command with a canned reply so the
 *      stock AP never blocks in a `while (!flag)` wait:
 *        ch0 system : BB_HOLD -> BB_HOLD_ACK, waits for BB_HOLD_EXIT
 *        ch1 decode : DEC_OPEN -> DEC_OPEN_ERR (valid: breaks the AP's
 *                     gOpenDone wait), DECODE -> DECODE_ERR,
 *                     GETBUFFER/GETTIME/SEEK/CLOSE -> *_CMPL(0)
 *        ch2 file   : SEEK/READ/WRITE/TELL/GET_LENGTH/CLOSE -> *_CMPL
 *        ch3 debug  : ignored (only PRINT_LOG_OK arrives here)
 *
 * Everything below is verified against the stock binary + SDK source:
 *   - MAILBOX register map: driver/mailbox/hw_mailbox.h, confirmed in the
 *     stock section_3 disasm @0x03073b7e (0x50-byte struct, base 0x40110000).
 *   - Command IDs: the REAL SDK enums (driver/BB/BBSystem.h,
 *     audio/Include/audio_main.h, filesys/file.h) — NOT the old ReChord
 *     shim values. The stock AP only understands these.
 *   - IRQ numbering: system/os/interrupt.h — MAILBOX0..3 = INT_ID 25..28
 *     = NVIC IRQ 9..12 = vector slots 25..28.
 *   - Reply semantics: bbsystem/Main2.c (DEC_OPEN_CMPL carries a pointer,
 *     DEC_OPEN_ERR + 0 is a valid answer that unblocks the AP).
 *
 * Build: `make bb-stub`        -> build/bb/section3_stub.bin
 * Pack : `make pack-bb-stub-img` -> build/ReChord_BB_Stub.IMG
 *
 * Telemetry (readable with a debugger): boot_log @ 0x03000118
 *   [0]='BOTS' reached rechord_main   [1]=mailbox enabled
 *   [2]=START_OK sent                 [3..6]=per-channel reply counters
 *   [7]=last A2B command seen         [8]=last A2B channel
 */
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Hardware map                                                        */
/* ------------------------------------------------------------------ */
#define MAILBOX_BASE       0x40110000u     /* MAIL_BOX (hw_memap.h)      */
#define CRU_BASE           0x40180000u     /* clock/reset unit           */

/* MAIL_BOX register offsets (verified against stock disasm @0x03073b7e):
 * 0x50-byte struct; per channel n: CMD at +0x08+8n (A2B) / +0x30+8n (B2A),
 * DATA at CMD+4. Status registers are write-1-to-clear. */
#define MB_A2B_INTEN       (*(volatile uint32_t *)(MAILBOX_BASE + 0x00u))
#define MB_A2B_STATUS      (*(volatile uint32_t *)(MAILBOX_BASE + 0x04u))   /* W1C */
#define MB_A2B_CMD(n)      (*(volatile uint32_t *)(MAILBOX_BASE + 0x08u + 8u * (n)))
#define MB_A2B_DATA(n)     (*(volatile uint32_t *)(MAILBOX_BASE + 0x0Cu + 8u * (n)))
#define MB_B2A_INTEN       (*(volatile uint32_t *)(MAILBOX_BASE + 0x28u))
#define MB_B2A_STATUS      (*(volatile uint32_t *)(MAILBOX_BASE + 0x2Cu))   /* W1C */
#define MB_B2A_CMD(n)      (*(volatile uint32_t *)(MAILBOX_BASE + 0x30u + 8u * (n)))
#define MB_B2A_DATA(n)     (*(volatile uint32_t *)(MAILBOX_BASE + 0x34u + 8u * (n)))

#define MB_CH_SYSTEM       0u   /* MAILBOX0 / IRQ 9  / vector 25 */
#define MB_CH_DECODE       1u   /* MAILBOX1 / IRQ 10 / vector 26 */
#define MB_CH_FILE         2u   /* MAILBOX2 / IRQ 11 / vector 27 */
#define MB_CH_DEBUG        3u   /* MAILBOX3 / IRQ 12 / vector 28 */

#define MB_INT(ch)         (1u << (ch))

/* NVIC / SCB (standard Cortex-M3; SDK hw_nvic.h uses the same addresses) */
#define NVIC_ISER0         (*(volatile uint32_t *)0xE000E100u)
#define NVIC_ICPR0         (*(volatile uint32_t *)0xE000E280u)
#define NVIC_IPR           ((volatile uint8_t  *)0xE000E400u)
#define SCB_VTOR           (*(volatile uint32_t *)0xE000ED08u)

#define IRQ_MAILBOX0       9u
#define IRQ_MAILBOX3       12u
#define VEC_SLOT(irq)      ((irq) + 16u)

/* ROM HW services (Option B boot, same as rechord_app.c; Thumb bit |1). */
#define ROM_ALLOC       ((void *(*)(uint32_t))(0x02feeedc | 1))
#define ROM_HW_INIT     ((void (*)(uint32_t))(0x02feeebe | 1))
#define ROM_HW_INIT2    ((void (*)(uint32_t))(0x02feee7c | 1))
#define ROM_EARLY_INIT  ((void (*)(void))(0x02fe860e | 1))

/* Telemetry block inside .text.boot padding (same block rechord_app.c uses) */
#define boot_log ((volatile uint32_t *)0x03000118u)

/* ------------------------------------------------------------------ */
/* Mailbox command IDs — REAL SDK enum values                          */
/* ------------------------------------------------------------------ */
/* driver/BB/BBSystem.h  MSGBOX_SYSTEM_CMD */
#define MSGBOX_CMD_SYSTEM_START_OK      1u
#define MSGBOX_CMD_BB_HOLD              2u
#define MSGBOX_CMD_BB_HOLD_ACK          3u
#define MSGBOX_CMD_BB_HOLD_EXIT         4u
#define MSGBOX_CMD_SYSTEM_PRINT_LOG_OK  6u

/* audio/Include/audio_main.h  MEDIA_MSGBOX_DECODE_CMD */
#define MEDIA_MSGBOX_CMD_DEC_OPEN               1u
#define MEDIA_MSGBOX_CMD_DEC_OPEN_ERR           2u
#define MEDIA_MSGBOX_CMD_DECODE                 4u
#define MEDIA_MSGBOX_CMD_DECODE_ERR             6u
#define MEDIA_MSGBOX_CMD_DECODE_GETBUFFER       7u
#define MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL  8u
#define MEDIA_MSGBOX_CMD_DECODE_GETTIME         9u
#define MEDIA_MSGBOX_CMD_DECODE_GETTIME_CMPL    10u
#define MEDIA_MSGBOX_CMD_DECODE_SEEK            11u
#define MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL       12u
#define MEDIA_MSGBOX_CMD_DECODE_CLOSE           13u
#define MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL      14u
#define MEDIA_MSGBOX_CMD_ENCODE_INIT            205u
#define MEDIA_MSGBOX_CMD_ENCODE_INIT_CMPL       206u
#define MEDIA_MSGBOX_CMD_ENCODE                 203u
#define MEDIA_MSGBOX_CMD_ENCODE_ERR             208u

/* filesys/file.h  MEDIA_MSGBOX_FILE_CMD (FILE_NULL = 100) */
#define MEDIA_MSGBOX_CMD_FILE_SEEK              105u
#define MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL         106u
#define MEDIA_MSGBOX_CMD_FILE_READ              107u
#define MEDIA_MSGBOX_CMD_FILE_READ_CMPL         108u
#define MEDIA_MSGBOX_CMD_FILE_WRITE             109u
#define MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL        110u
#define MEDIA_MSGBOX_CMD_FILE_TELL              111u
#define MEDIA_MSGBOX_CMD_FILE_TELL_CMPL         112u
#define MEDIA_MSGBOX_CMD_FILE_GET_LENGTH        113u
#define MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL   114u
#define MEDIA_MSGBOX_CMD_FILE_CLOSE             115u
#define MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL        116u

/* ------------------------------------------------------------------ */
/* Vector table (NUM_INTERRUPTS = 16 core + 41 RKNano IRQs = 57)       */
/* ------------------------------------------------------------------ */
#define NUM_VECTORS 57u

extern uint32_t __stack_top;              /* from firmware.ld: 0x03022794 */
extern void firmware_entry(void);         /* entry_stubs.S @ 0x03000010  */

__attribute__((section("bb_vect"), aligned(256), used))
static void (*bb_vectors[NUM_VECTORS])(void);

static volatile uint32_t reply_count[4];

/* Reply to the AP on B2A channel ch. Stock order: Cmd first, then Data. */
static void reply(uint32_t ch, uint32_t cmd, uint32_t data)
{
    MB_B2A_CMD(ch)  = cmd;
    MB_B2A_DATA(ch) = data;
    reply_count[ch]++;
    boot_log[3 + ch] = reply_count[ch];
}

/* Default handler: park forever (a blank screen is better than a jump
 * into garbage — WFI also keeps power draw down if IRQs get disabled). */
static void bb_default_handler(void)
{
    for (;;)
        __asm volatile ("wfi");
}

/* ---------------- channel 0: system control ---------------- */
static void bb_mb0_isr(void)
{
    uint32_t cmd, n;

    MB_A2B_STATUS = MB_INT(MB_CH_SYSTEM);           /* W1C            */
    cmd = MB_A2B_CMD(MB_CH_SYSTEM);
    boot_log[7] = cmd;
    boot_log[8] = MB_CH_SYSTEM;

    switch (cmd) {
    case MSGBOX_CMD_BB_HOLD:
        /* Stock BBSystemBIsr: ACK, then poll for HOLD_EXIT (~5 ms). */
        reply(MB_CH_SYSTEM, MSGBOX_CMD_BB_HOLD_ACK, 0);
        n = 300000;                                  /* ~5 ms busy-wait */
        while (n--) {
            if (MB_A2B_CMD(MB_CH_SYSTEM) == MSGBOX_CMD_BB_HOLD_EXIT) {
                MB_A2B_STATUS = MB_INT(MB_CH_SYSTEM);
                break;
            }
        }
        break;
    default:
        break;                                       /* START_OK is B2A only */
    }
}

/* ---------------- channel 1: decode ---------------- */
static void bb_mb1_isr(void)
{
    uint32_t cmd;

    MB_A2B_STATUS = MB_INT(MB_CH_DECODE);
    cmd = MB_A2B_CMD(MB_CH_DECODE);
    boot_log[7] = cmd;
    boot_log[8] = MB_CH_DECODE;

    switch (cmd) {
    case MEDIA_MSGBOX_CMD_DEC_OPEN:
        /* ERR + 0 is a valid answer: the AP's decode ISR sets gOpenDone
         * on BOTH DEC_OPEN_ERR and DEC_OPEN_CMPL — the point is to reply. */
        reply(MB_CH_DECODE, MEDIA_MSGBOX_CMD_DEC_OPEN_ERR, 0);
        break;
    case MEDIA_MSGBOX_CMD_DECODE:
    case MEDIA_MSGBOX_CMD_ENCODE:
    case MEDIA_MSGBOX_CMD_ENCODE_INIT:
        reply(MB_CH_DECODE, MEDIA_MSGBOX_CMD_DECODE_ERR, 0);
        break;
    case MEDIA_MSGBOX_CMD_DECODE_GETBUFFER:
        reply(MB_CH_DECODE, MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL, 0);
        break;
    case MEDIA_MSGBOX_CMD_DECODE_GETTIME:
        reply(MB_CH_DECODE, MEDIA_MSGBOX_CMD_DECODE_GETTIME_CMPL, 0);
        break;
    case MEDIA_MSGBOX_CMD_DECODE_SEEK:
        reply(MB_CH_DECODE, MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL, 0);
        break;
    case MEDIA_MSGBOX_CMD_DECODE_CLOSE:
        reply(MB_CH_DECODE, MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL, 0);
        break;
    default:
        break;
    }
}

/* ---------------- channel 2: file ---------------- */
static void bb_mb2_isr(void)
{
    uint32_t cmd;

    MB_A2B_STATUS = MB_INT(MB_CH_FILE);
    cmd = MB_A2B_CMD(MB_CH_FILE);
    boot_log[7] = cmd;
    boot_log[8] = MB_CH_FILE;

    switch (cmd) {
    case MEDIA_MSGBOX_CMD_FILE_SEEK:
        reply(MB_CH_FILE, MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL, 0);
        break;
    case MEDIA_MSGBOX_CMD_FILE_READ:
        /* 0 bytes read = EOF: the AP stops feeding the decoder, no hang. */
        reply(MB_CH_FILE, MEDIA_MSGBOX_CMD_FILE_READ_CMPL, 0);
        break;
    case MEDIA_MSGBOX_CMD_FILE_WRITE:
        reply(MB_CH_FILE, MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL, 0);
        break;
    case MEDIA_MSGBOX_CMD_FILE_TELL:
        reply(MB_CH_FILE, MEDIA_MSGBOX_CMD_FILE_TELL_CMPL, 0);
        break;
    case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH:
        reply(MB_CH_FILE, MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL, 0);
        break;
    case MEDIA_MSGBOX_CMD_FILE_CLOSE:
        reply(MB_CH_FILE, MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL, 1);
        break;
    default:
        break;      /* *_CMPL / *_HANDSHK arriving from the AP: ignore */
    }
}

/* ---------------- channel 3: debug ---------------- */
static void bb_mb3_isr(void)
{
    MB_A2B_STATUS = MB_INT(MB_CH_DEBUG);
    /* MSGBOX_CMD_SYSTEM_PRINT_LOG_OK arrives here; nothing to do. */
}

/* ------------------------------------------------------------------ */
/* ROM HW init — exact stock firmware_entry sequence (V0.20 Option B)  */
/* ------------------------------------------------------------------ */
static void stub_rom_init(void *param)
{
    uint16_t *bp = (uint16_t *)param;
    volatile uint8_t  *lay   = (volatile uint8_t  *)0x03000164u;
    volatile uint16_t *bmode = (volatile uint16_t *)0x03000168u;
    uint16_t mode;
    uint32_t m, cols, pad;
    void *ctx;

    /* boot_param_layout(param) — mirror of stock @0x030000da */
    *lay = 8;
    mode = 0;
    switch (*bp) { case 0: case 1: case 2: case 3: case 4: case 5:
                   case 8: case 10: mode = *bp; }
    *bmode = mode;
    mode = 0;
    switch (*bp) { case 0: case 1: case 2: case 3: case 4: case 5:
                   case 8: case 10: mode = *bp; }
    lay[5] = (uint8_t)mode;
    if (mode > 0x0c)        lay[5]   = 0x0d;
    if (*bmode > 0x0c)      *bmode   = 0x0d;
    m    = lay[5];
    cols = ((m / 6) * 3 & 0x7f) * 2;
    lay[1] = (uint8_t)cols;
    if (m <= cols && cols - m != 0) { cols -= 6; lay[1] = (uint8_t)cols; }
    lay[2] = (uint8_t)(lay[5] - (uint8_t)cols);
    pad = 8 - (cols & 0xff);
    if (pad > 6) pad = 6;
    lay[4] = (uint8_t)pad;
    lay[3] = 0;

    /* rom_alloc + rom_hw_init sequence (exact stock) */
    ctx = ROM_ALLOC(0x1dc);
    if (ctx == 0) {
        ROM_HW_INIT2(0x16f);
    } else {
        ROM_HW_INIT(0x1dc);
        ROM_HW_INIT(0x16f);
        ROM_HW_INIT(0x16f);
        ROM_HW_INIT2(0x171);
        ROM_HW_INIT(0x170);
    }
    ROM_EARLY_INIT();
}

/* entry_stubs.S: firmware_entry -> bl rechord_hw_init -> b rechord_main */
void rechord_hw_init(void *param)
{
    stub_rom_init(param);
}

/* entry_stubs.S trampolines reference these; no UI in stub mode. */
__attribute__((weak)) void rechord_ui_event(void)   {}
__attribute__((weak)) void rechord_key_handler(void) {}

/* entry_stubs.S's rechord_vectors references the SDK symbol — provide it. */
void IntDefaultHandler2(void)
{
    bb_default_handler();
}

/* ------------------------------------------------------------------ */
/* Main: mailbox bring-up + handshake + WFI loop                       */
/* ------------------------------------------------------------------ */
void rechord_main(void)
{
    uint32_t i;

    boot_log[0] = 0x53544F42u;                      /* 'BOTS' */

    /* .bss lives past the loaded binary (objcopy emits .fw_header+.text
     * only), so manually zero the state we care about. */
    reply_count[0] = reply_count[1] = 0;
    reply_count[2] = reply_count[3] = 0;
    boot_log[1] = boot_log[2] = 0;
    boot_log[3] = boot_log[4] = boot_log[5] = boot_log[6] = 0;
    boot_log[7] = boot_log[8] = 0;

    /* 1. Build the RAM vector table (bb_vect is beyond the loaded image). */
    for (i = 0; i < NUM_VECTORS; i++)
        bb_vectors[i] = (void (*)(void))((uint32_t)bb_default_handler | 1u);
    bb_vectors[0] = (void (*)(void))__stack_top;
    bb_vectors[1] = (void (*)(void))((uint32_t)firmware_entry | 1u);
    bb_vectors[VEC_SLOT(IRQ_MAILBOX0)] = (void (*)(void))((uint32_t)bb_mb0_isr | 1u);
    bb_vectors[VEC_SLOT(IRQ_MAILBOX0) + 1u] = (void (*)(void))((uint32_t)bb_mb1_isr | 1u);
    bb_vectors[VEC_SLOT(IRQ_MAILBOX0) + 2u] = (void (*)(void))((uint32_t)bb_mb2_isr | 1u);
    bb_vectors[VEC_SLOT(IRQ_MAILBOX0) + 3u] = (void (*)(void))((uint32_t)bb_mb3_isr | 1u);
    SCB_VTOR = (uint32_t)bb_vectors;

    /* 2. Mailbox clock gate (defensive, same as BSP_Init2):
     *    PCLK_MAILBOX_GATE=146 -> CRU_CLKGATE_CON[9] bit 2,
     *    write-enable + enable = 0x00040004. */
    *(volatile uint32_t *)(CRU_BASE + 0xA4u) |= 0x00040004u;

    /* 3. Clear stale A2B status, enable local interrupts 0..3. */
    MB_A2B_STATUS = 0xFu;
    MB_B2A_STATUS = 0xFu;
    *(volatile uint32_t *)MB_A2B_INTEN |= 0xFu;

    /* 4. NVIC: clear pending, set priority, enable IRQ 9..12. */
    NVIC_ICPR0 = (0xFu << IRQ_MAILBOX0);
    for (i = IRQ_MAILBOX0; i <= IRQ_MAILBOX3; i++)
        NVIC_IPR[i] = 0x90u;                        /* same band as stock */
    NVIC_ISER0 = (0xFu << IRQ_MAILBOX0);

    __asm volatile ("cpsie i");
    boot_log[1] = 0xfeed0001u;                      /* mailbox enabled */

    /* 5. Boot handshake: SYSTEM_START_OK on B2A channel 0. The stock AP
     *    (StartBBSystem) waits ~200 ms for exactly this reply. */
    reply(MB_CH_SYSTEM, MSGBOX_CMD_SYSTEM_START_OK, 0);
    boot_log[2] = 0xfeed0002u;                      /* START_OK sent */

    /* 6. Done. Everything else is interrupt-driven. */
    for (;;)
        __asm volatile ("wfi");
}
