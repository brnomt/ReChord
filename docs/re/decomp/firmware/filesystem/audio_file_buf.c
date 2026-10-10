/*
 * firmware/firmware/filesystem/audio_file_buf.c
 * Double-buffered audio file I/O layered on HifiFileRead/Seek.
 *
 * Addresses (primary copy near mp3 codec):
 *   AudioFileInput2       @ 0x0306dac4
 *   AudioFileMhRead2      @ 0x0306dc72
 *   AudioFileSeek2        @ 0x0306dd54
 *   AudioFileMhSeek2      @ 0x0306dec4
 *   AudioFileBufferSwitch2 @ 0x0306da52
 *   FLAC_FileSeekFast     @ 0x0306e1f4
 *   FLAC_FileGetSeekInfo  @ 0x0306e24a
 */

#include <stdint.h>
#include "decomp_support.h"
#include "decomp_globals.h"

extern uint32_t HifiFileRead(uint32_t buf, uint32_t len, uint8_t fd);
extern uint8_t  HifiFileSeek(uint32_t offset, uint32_t whence, uint8_t fd);
extern void     hifi_busy_delay(int ticks);
extern void     hifi_debug_printf(const char *fmt, ...);
extern void     hifi_memmove(void *dst, const void *src, uint32_t n);
extern int      ipc_post_cmd(uint32_t opcode, int slot, int bank);
extern int      ipc_post_arg(void *arg, int slot, int bank);
extern void     event_set(uint32_t id); /* FUN_03073840-style */

/* Interpretive view of the double-buffer control block (DAT_0306de04) */
typedef struct {
    uint32_t base;          /* [0] buffer base */
    uint32_t _pad1;
    uint32_t _pad2;
    uint32_t chunk;         /* [3] chunk size */
    int32_t  busy;          /* [4] 1 while FS op in flight */
    uint32_t cursor;        /* [5] read cursor within active buf */
    int32_t  active;        /* [6] active buffer index 0/1 */
    int32_t  prev;          /* [7] previous buffer index */
} AudioFileBuf;

extern AudioFileBuf *g_afb;           /* DAT_0306de04 */
extern uint8_t      *g_afb_flags;     /* DAT_0306de08 */
extern int           g_afb_timeout;   /* DAT_0306de0c */
extern uint32_t     *g_afb_filled;    /* DAT_0306de10 — filled[2] */
extern uint32_t     *g_afb_file_size; /* DAT_0306de50 */

static void wait_not_busy(void)
{
    if (g_afb->busy != 1)
        return;
    hifi_busy_delay(1);
    int left = g_afb_timeout;
    do {
        if (g_afb->busy != 1)
            return;
        hifi_busy_delay(1);
    } while (--left != 0);
    hifi_debug_printf("AudioFile*: timeout!!");
}

/* AudioFileInput2 @ 0x0306dac4 — kick a fill of the previous buffer */
uint32_t AudioFileInput2(uint8_t fd)
{
    wait_not_busy();
    g_afb_flags[1] = 0;
    g_afb->busy = 1;
    uint32_t n = HifiFileRead(
        g_afb->prev * g_afb->chunk + g_afb->base,
        g_afb->chunk,
        fd);
    g_afb_filled[g_afb->prev] = n;
    return 0;
}

/* AudioFileBufferSwitch2 @ 0x0306da52 — flip active buffer, prefetch next */
void AudioFileBufferSwitch2(uint8_t fd)
{
    wait_not_busy();
    g_afb->cursor = 0;
    int cur = g_afb->active;
    g_afb_filled[cur] = 0;
    g_afb->prev = cur;
    int next = 1 - cur;
    g_afb->active = next;
    if (g_afb_filled[next] == 0) {
        g_afb_flags[1] = 1;
        g_afb_filled[next] = HifiFileRead(
            next * g_afb->chunk + g_afb->base,
            g_afb->chunk,
            fd);
    }
    event_set(0x115);
}

/*
 * AudioFileMhRead2 @ 0x0306dc72
 * Copy from the double buffer into dst; refill via HifiFileRead as needed.
 * If fd != 0, bypass buffer and call HifiFileRead directly.
 */
int AudioFileMhRead2(uint8_t *dst, uint32_t len, int fd)
{
    if (fd != 0) {
        wait_not_busy();
        return (int)HifiFileRead((uint32_t)dst, len, (uint8_t)fd);
    }
    if (len == 0)
        return 0;

    int got = 0;
    while (1) {
        uint32_t cursor = g_afb->cursor;
        uint32_t avail = g_afb_filled[g_afb->active] - cursor;
        if (len < avail)
            break;
        hifi_memmove(dst,
                     (void *)(g_afb->active * g_afb->chunk + g_afb->base + cursor),
                     avail);
        len -= avail;
        g_afb->cursor = cursor + avail;
        got += (int)avail;
        dst += avail;
        if (g_afb->cursor >= g_afb_filled[g_afb->active]) {
            AudioFileBufferSwitch2(0);
            if (g_afb_filled[g_afb->active] == 0)
                return got;
        }
    }
    hifi_memmove(dst,
                 (void *)(g_afb->active * g_afb->chunk + g_afb->base + g_afb->cursor),
                 len);
    g_afb->cursor += len;
    if (g_afb->cursor >= g_afb_filled[g_afb->active])
        AudioFileBufferSwitch2(0);
    return got + (int)len;
}

/* AudioFileSeek2 @ 0x0306dd54 — SEEK_SET/CUR with 512-byte sector align */
uint32_t AudioFileSeek2(uint32_t offset, int whence, int fd)
{
    wait_not_busy();
    /* Full sector-aware logic retained in Ghidra; simplified surface here. */
    uint32_t r = HifiFileSeek((offset / 0x200) * 0x200, (uint32_t)whence, (uint8_t)fd);
    g_afb->cursor = (offset % 0x200) + g_afb->cursor;
    return r;
}

/* AudioFileMhSeek2 @ 0x0306dec4 */
void AudioFileMhSeek2(uint32_t offset, uint32_t whence, int fd)
{
    if (fd == 0) {
        AudioFileSeek2(offset, (int)whence, 0);
        return;
    }
    wait_not_busy();
    HifiFileSeek(offset, whence, (uint8_t)fd);
}

/* FLAC_FileSeekFast @ 0x0306e1f4 — IPC opcode 0x0f bank 1 */
uint32_t FLAC_FileSeekFast(uint32_t a, uint32_t b, uint32_t c)
{
    volatile uint32_t *mbox = (volatile uint32_t *)0; /* DAT_0306e2b0 */
    /* Reconstructive stub — see Ghidra for mailbox globals */
    (void)a; (void)b; (void)c;
    ipc_post_cmd(0x0f, 0, 1);
    return 0;
}

/* FLAC_FileGetSeekInfo @ 0x0306e24a — IPC opcode 0x11 */
uint32_t FLAC_FileGetSeekInfo(uint32_t *out0, uint32_t *out1, uint32_t arg)
{
    (void)arg;
    ipc_post_cmd(0x11, 0, 1);
    /* results copied from DAT_0306e2dc after completion */
    if (out0) *out0 = 0;
    if (out1) *out1 = 0;
    return 0;
}
