/*
 * stdio_fs.c — the host-stdio file subset the theme loader needs, over the
 * read-only FAT backend (firmware/services/fs.h).
 *
 * WHY: theme_file_load() (firmware/theme/theme_file.c) is written against
 * stdio so the same code runs on the host with real stdio. The freestanding
 * AP build has no FILE I/O, so those five symbols used to be weak stubs in
 * firmware/startup/ap_main.c — which linked fine and silently made file
 * themes inert. This is the real target path: same subset, pooled streams,
 * read-only (the shim only ever calls fs_open_read).
 *
 * NOT part of firmware/libc/libc.mk: the host tests link real stdio, so this
 * file is target-only (firmware/ap.mk).
 */
#include <stddef.h>

#include "../services/fs.h"

/* More than one stream is allowed (the UI opens one file per load) but the
 * pool is static: no malloc in this build. */
#ifndef STDIO_FS_STREAMS
#define STDIO_FS_STREAMS 2
#endif

typedef struct {
    fs_file_t *f;
    int        used;
    int        eof;
} stdio_fs_stream_t;

static stdio_fs_stream_t s_streams[STDIO_FS_STREAMS];

void *fopen(const char *path, const char *mode)
{
    int i;

    if (path == 0 || mode == 0 || mode[0] != 'r')
        return 0;                     /* read-only shim */
    for (i = 0; i < STDIO_FS_STREAMS; i++) {
        if (!s_streams[i].used) {
            s_streams[i].f = fs_open_read(path);
            if (s_streams[i].f == 0)
                return 0;
            s_streams[i].used = 1;
            s_streams[i].eof = 0;
            return &s_streams[i];
        }
    }
    return 0;                         /* pool exhausted */
}

char *fgets(char *s, int n, void *stream)
{
    stdio_fs_stream_t *st = (stdio_fs_stream_t *)stream;
    int i = 0;

    if (st == 0 || s == 0 || n <= 0)
        return 0;
    while (i < n - 1) {
        char c;

        if (fs_read(st->f, &c, 1) != 1) {
            st->eof = 1;
            break;
        }
        s[i++] = c;
        if (c == '\n')
            break;
    }
    if (i == 0)
        return 0;
    s[i] = '\0';
    return s;
}

int fgetc(void *stream)
{
    stdio_fs_stream_t *st = (stdio_fs_stream_t *)stream;
    unsigned char c;

    if (st == 0)
        return -1;
    if (fs_read(st->f, &c, 1) != 1) {
        st->eof = 1;
        return -1;
    }
    return (int)c;
}

int feof(void *stream)
{
    stdio_fs_stream_t *st = (stdio_fs_stream_t *)stream;

    if (st == 0)
        return 1;
    return st->eof;
}

int fclose(void *stream)
{
    stdio_fs_stream_t *st = (stdio_fs_stream_t *)stream;

    if (st == 0)
        return -1;
    fs_close(st->f);
    st->f = 0;
    st->used = 0;
    st->eof = 0;
    return 0;
}
