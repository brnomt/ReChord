/*
 * firmware/firmware/filesystem/hifi_file.c
 * HifiFile IPC wrappers — post cmd to FS task, wait with timeout.
 *
 * SDK reference: firmware/rockchip/bbsystem/audio_file_access2.c (RKNanoD MP3 v1.3)
 * Fiio v3.7.0 Ghidra recovery (FIIO-3.7.0-Decomp):
 *
 *   HifiFileSeek  @ 0x0306b8e6  ipc 0x69
 *   HifiFileRead  @ 0x0306b94c  ipc 0x6b
 *   HifiFileWrite @ 0x0306b9bc  ipc 0x6d
 *   HifiFileClose @ 0x0306ba1e  ipc 0x73
 *
 * HifiFileOpen: SDK spins on gCmdDone (no ipc_post). The "HifiFileOpen: timeout!!"
 * string exists in the binary @ 0x0306bb94 but has no code xrefs — open may be
 * handshake-only on Fiio; implemented per SDK below.
 *
 * Overlay copies of Seek/Read @ 0x03080e96 / 0x03080efc.
 */

#include <stdint.h>
#include <stddef.h>

#include "fs_layer.h"
#include "decomp_support.h"
#include "decomp_globals.h"

extern int ipc_post_cmd(uint32_t opcode, int slot, int bank);
extern int ipc_post_arg(void *arg, int slot, int bank);

extern void hifi_busy_delay(int ticks);
extern void hifi_debug_printf(const char *fmt, ...);

/* --- IPC opcodes (Fiio v3.7.0) --- */
#define HIFI_OPC_SEEK   0x69
#define HIFI_OPC_READ   0x6b
#define HIFI_OPC_WRITE  0x6d
#define HIFI_OPC_CLOSE  0x73

/* Seek / read mailbox block @ DAT_0306bbb0 / DAT_0306bbd0 */
extern volatile uint32_t *g_hifi_seek_mbox;
extern volatile uint8_t  *g_hifi_seek_status;
extern uint32_t          *g_hifi_pos_table;
extern int                g_hifi_timeout_iters;

extern volatile uint32_t *g_hifi_read_mbox;

/* Write mailbox @ DAT_0306bbec; shared done gate @ DAT_0306bb8c */
extern volatile uint32_t *g_hifi_write_mbox;
extern volatile char     *g_hifi_cmd_done;      /* byte[0]=done, byte[1]=gate */
extern volatile uint8_t  *g_hifi_op_status;     /* DAT_0306bbb4 */

static void hifi_ipc_spin(volatile char *done, const char *timeout_msg)
{
    if (*done != 0)
        return;

    hifi_busy_delay(1);
    int left = g_hifi_timeout_iters;
    do {
        if (*done != 0)
            return;
        hifi_busy_delay(1);
    } while (--left != 0);

    hifi_debug_printf(timeout_msg);
}

/*
 * HifiFileOpen — SDK pattern (no ipc_post on Fiio either).
 * Waits for FS task handshake completion flag.
 * Fiio v3.7.0: timeout string present but unreferenced in code.
 */
uint32_t HifiFileOpen(void)
{
    volatile char *done = g_hifi_cmd_done;

    *done = 0;
    hifi_ipc_spin(done, "HifiFileOpen: timeout!!");
    *done = 0;
    return 0;
}

/*
 * HifiFileSeek @ 0x0306b8e6
 */
uint8_t HifiFileSeek(uint32_t offset, uint32_t whence, uint8_t fd)
{
    volatile uint32_t *mbox = g_hifi_seek_mbox;
    volatile char *done = (volatile char *)((uintptr_t)mbox - 0x28);

    mbox[0] = offset;
    mbox[1] = whence;
    *(volatile uint8_t *)&mbox[2] = fd;
    *done = 0;

    ipc_post_cmd(HIFI_OPC_SEEK, 0, 2);
    ipc_post_arg((void *)mbox, 0, 2);

    hifi_ipc_spin(done, "HifiFileSeek: timeout!!");

    g_hifi_pos_table[fd] = mbox[0];
    *done = 0;
    return *g_hifi_seek_status;
}

/*
 * HifiFileRead @ 0x0306b94c
 */
uint32_t HifiFileRead(uint32_t buf, uint32_t len, uint8_t fd)
{
    volatile uint32_t *mbox = g_hifi_read_mbox;
    volatile char *done = (volatile char *)((uintptr_t)mbox - 0x34);

    mbox[0] = buf;
    mbox[1] = len;
    *(volatile uint8_t *)&mbox[2] = fd;
    *done = 0;

    ipc_post_cmd(HIFI_OPC_READ, 0, 2);
    ipc_post_arg((void *)mbox, 0, 2);

    /* Pre-check gate (byte at done+1) — early out if FS not ready */
    if (((volatile char *)done)[1] == 0)
        return 0;

    hifi_ipc_spin(done, "HifiFileRead: timeout!!");

    g_hifi_pos_table[fd] = mbox[1];
    *done = 0;
    return *(volatile uint32_t *)g_hifi_op_status;
}

/*
 * HifiFileWrite @ 0x0306b9bc
 * Args: buf, file_offset, size, fd (packed into g_hifi_write_mbox).
 */
uint32_t HifiFileWrite(uint32_t buf, uint32_t file_offset, uint32_t size, uint8_t fd)
{
    volatile uint32_t *mbox = g_hifi_write_mbox;
    volatile char *done = g_hifi_cmd_done;

    mbox[0] = buf;
    mbox[1] = file_offset;
    mbox[2] = size;
    *(volatile uint8_t *)&mbox[3] = fd;

    ipc_post_cmd(HIFI_OPC_WRITE, 0, 2);
    ipc_post_arg((void *)mbox, 0, 2);

    if (done[1] == 0)
        return 0;

    hifi_ipc_spin(done, "HifiFileWrite: timeout!!");

    *done = 0;
    return *(volatile uint32_t *)g_hifi_op_status;
}

/*
 * HifiFileClose @ 0x0306ba1e
 */
uint8_t HifiFileClose(uint8_t fd)
{
    volatile char *done = g_hifi_cmd_done;

    *done = 0;

    ipc_post_cmd(HIFI_OPC_CLOSE, 0, 2);
    ipc_post_arg((void *)(uintptr_t)fd, 0, 2);

    hifi_ipc_spin(done, "HifiFileClose: timeout!!");

    *done = 0;
    return (uint8_t)*(volatile uint32_t *)g_hifi_op_status;
}

/* Overlay copies (same logic, different globals) @ 0x03080e96 / 0x03080efc */
uint8_t  HifiFileSeek_ovl(uint32_t offset, uint32_t whence, uint8_t fd);
uint32_t HifiFileRead_ovl(uint32_t buf, uint32_t len, uint8_t fd);
