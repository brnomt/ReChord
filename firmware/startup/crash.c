/*
 * crash.c — crash breadcrumbs for the AP.
 *
 * Before this, every fault ended in Default_Handler's infinite loop: a frozen
 * screen, no log, nothing to debug from. Now a HardFault (MemManage/BusFault/
 * UsageFault escalate to it) captures the stacked PC/LR/xPSR/SP and the SCB
 * fault registers, writes them plus the last log lines RAW into the data
 * sectors of the preallocated /.rechord/CRASH (no FAT or directory write, so
 * the volume cannot be corrupted from here), shows a crash screen, and powers
 * off on battery. The next boot copies the summary into the LOG and marks
 * the record seen; the file itself stays readable from a PC.
 *
 * crash_arm() must run whenever the Internal volume is (re)mounted: it resolves
 * the file's sectors once, so the fault path never touches the FS.
 */
#include <stdint.h>
#include <string.h>

#include "drivers/display.h"
#include "drivers/emmc.h"
#include "drivers/panel_io.h"
#include "drivers/rknano_i2s.h"
#include "drivers/usb_msc.h"
#include "services/crash_record.h"
#include "services/fs.h"
#include "services/fs_target.h"
#include "services/log.h"
#include "services/rechord_files.h"
#include "crash.h"

#define CRASH_SECTORS (RECHORD_CRASH_BYTES / 512u)

static uint32_t    s_lba[CRASH_SECTORS];
static uint8_t     s_nsec;
static const char *s_build;

#define SCB_CFSR  (*(volatile uint32_t *)0xE000ED28u)
#define SCB_HFSR  (*(volatile uint32_t *)0xE000ED2Cu)
#define SCB_MMFAR (*(volatile uint32_t *)0xE000ED34u)
#define SCB_BFAR  (*(volatile uint32_t *)0xE000ED38u)

void crash_arm(void)
{
    s_nsec = (uint8_t)fs_target_file_sectors(RECHORD_CRASH, s_lba,
                                             CRASH_SECTORS);
}

void crash_boot(const char *build)
{
    static const char k_blank[] = "# ReChord crash record: none yet\n";
    uint32_t buf[RECHORD_CRASH_BYTES / 4u + 1u];
    char *text = (char *)buf;
    char summary[96];
    unsigned long size = 0;
    int n, off;

    s_build = build;
    n = fs_load_file(RECHORD_CRASH, text, RECHORD_CRASH_BYTES + 1u);
    off = (n > 0) ? crash_find_new(text, n, summary, sizeof summary) : -1;
    if (off >= 0) {
        log_printf("crash: the last session faulted (/.rechord/CRASH)");
        if (summary[0] != '\0')
            log_printf("crash: %s", summary);
        if (fs_target_file_patch(RECHORD_CRASH, (uint32_t)off, "old", 3u) != 0)
            log_printf("crash: could not mark the record seen");
    }
    /* The handler writes the whole 2 KiB; the directory size must cover it
     * or the next boot would only read the template's first line. */
    if (fs_size_file(RECHORD_CRASH, &size) == FS_OK &&
        size < RECHORD_CRASH_BYTES && off < 0) {
        memset(text, '\n', RECHORD_CRASH_BYTES);
        memcpy(text, k_blank, sizeof k_blank - 1u);
        (void)fs_store_file(RECHORD_CRASH, text, RECHORD_CRASH_BYTES);
    }
    crash_arm();
    if (s_nsec == 0)
        log_printf("crash: no record file, faults will not be saved");
}

static void crash_line(int row, const char *s)
{
    display_draw_text(8, 12 + row * 18, s, 0xFFFFu, 0x6000u);
}

static void hex(char *out, const char *label, uint32_t v)
{
    static const char k_hex[] = "0123456789abcdef";
    int i, n = (int)strlen(label);

    memcpy(out, label, (size_t)n);
    out[n++] = '0';
    out[n++] = 'x';
    for (i = 7; i >= 0; i--)
        out[n++] = k_hex[(v >> (i * 4)) & 0xFu];
    out[n] = '\0';
}

/* Runs on a fresh stack (the handler reset MSP), never returns. */
void crash_fault(uint32_t sp, uint32_t pc, uint32_t lr, uint32_t psr)
    __attribute__((noreturn, used));
void crash_fault(uint32_t sp, uint32_t pc, uint32_t lr, uint32_t psr)
{
    uint32_t buf[RECHORD_CRASH_BYTES / 4u];   /* stack: MSP was reset */
    char tail[RECHORD_CRASH_BYTES];
    char line[40];
    crash_regs_t r;
    unsigned i;

    __asm__ volatile("cpsid i" ::: "memory");
    r.pc = pc;
    r.lr = lr;
    r.psr = psr;
    r.sp = sp;
    r.cfsr = SCB_CFSR;
    r.hfsr = SCB_HFSR;
    r.mmfar = SCB_MMFAR;
    r.bfar = SCB_BFAR;

    /* 1. Save first: if anything below faults again the record is on disk. */
    (void)log_tail(tail, (int)sizeof tail - 400);
    crash_format((char *)buf, sizeof buf, &r, s_build, log_now_ms(), tail);
    for (i = 0; i < s_nsec; i++)
        (void)emmc_panic_write(s_lba[i], 1u, (const uint8_t *)buf + i * 512u);

    /* 2. Tell the user instead of freezing. */
    backlight_on();
    display_fill_rect(0, 0, DISPLAY_W - 1, DISPLAY_H - 1, 0x6000u);
    crash_line(0, "ReChord crashed");
    hex(line, "pc ", pc);
    crash_line(2, line);
    hex(line, "lr ", lr);
    crash_line(3, line);
    hex(line, "cfsr ", r.cfsr);
    crash_line(4, line);
    crash_line(6, s_nsec ? "saved: /.rechord/CRASH" : "not saved (no file)");
    crash_line(7, "powering off...");
    display_flush_rows(0, DISPLAY_H - 1);

    /* 3. Off on battery (a few seconds to read it); on USB, stay here. */
    for (i = 0; i < 1500u; i++)
        delay_loops(200000);
    if (!usb_cable())
        board_power_off();
    for (;;)
        __asm__ volatile("");
}

/* HardFault entry: pick the stack the exception used, read the stacked PC/LR/
 * xPSR only if that stack lies inside the AP stack zone (an overflowed stack
 * would fault again here and lock the core), then move MSP to the top of the
 * zone and continue in C. Nothing returns to the faulting code. */
void crash_handler(void) __attribute__((naked, used));
void crash_handler(void)
{
    __asm__ volatile(
        "tst   lr, #4\n"
        "ite   eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "movs  r1, #0\n"
        "movs  r2, #0\n"
        "movs  r3, #0\n"
        "movw  r12, #0x0000\n"
        "movt  r12, #0x0300\n"            /* 0x03000000: stack floor */
        "cmp   r0, r12\n"
        "blo   1f\n"
        "movw  r12, #0x3FE0\n"
        "movt  r12, #0x0300\n"            /* top - one exception frame */
        "cmp   r0, r12\n"
        "bhi   1f\n"
        "ldr   r1, [r0, #24]\n"           /* stacked PC  */
        "ldr   r2, [r0, #20]\n"           /* stacked LR  */
        "ldr   r3, [r0, #28]\n"           /* stacked xPSR */
        "1:\n"
        "movw  r12, #0x4000\n"
        "movt  r12, #0x0300\n"            /* 0x03004000: stack top */
        "mov   sp, r12\n"
        "b     crash_fault\n");
}

#ifdef AP_CRASH_TEST
/* make AP_CRASH_TEST=1: fault on purpose to check the whole path on hardware
 * (undefined instruction -> UsageFault -> HardFault). */
void crash_test_fire(void)
{
    __asm__ volatile("udf #0");
}
#endif
