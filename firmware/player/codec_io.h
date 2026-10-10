/*
 * codec_io.h — tiny byte-source abstraction between codecs and storage.
 *
 * Why not stdio directly in the codecs: the WAV/MP3/FLAC decoders must run
 * identically in host tests (temp files, stdio backend) and on the target
 * (storage driver, services/fs or newlib syscalls). Keeping the seam here
 * means a codec body never changes when the file backend does — the target
 * swaps in a backend against its own FS layer (see codec_io_stdio.c).
 */
#ifndef RECHORD_CODEC_IO_H
#define RECHORD_CODEC_IO_H

typedef struct codec_io_s codec_io_t;
struct codec_io_s
{
    void *ctx;
    /* Read up to `len` bytes; returns bytes read (0 at EOF) or < 0. */
    long (*read)(void *ctx, void *buf, long len);
    /* Absolute seek; returns the new offset or < 0 on error. */
    long (*seek)(void *ctx, long offset);
    /* Current absolute offset, or < 0 on error. */
    long (*tell)(void *ctx);
    /* Total size in bytes, or < 0 if unknown. */
    long (*size)(void *ctx);
    void (*close)(void *ctx);
};

/* Open `path` for reading. Returns NULL on failure. */
codec_io_t *cio_open(const char *path);

/* NULL-safe close. */
void cio_close(codec_io_t *io);

/* Convenience wrappers (NULL-safe; return < 0 on failure/EOF where noted). */
long cio_read(codec_io_t *io, void *buf, long len);
long cio_seek(codec_io_t *io, long offset);
long cio_tell(codec_io_t *io);
long cio_size(codec_io_t *io);

#endif /* RECHORD_CODEC_IO_H */
