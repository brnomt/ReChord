/* Host harness for fs_target.c: mounts a disk image as volume "SD", prints
 * every directory and file (size + FNV-1a of the content, read in odd chunk
 * sizes), then exercises the IN-PLACE rewrite path (fs_target_file_rewrite)
 * and reads the result back through the FS. Driven by
 * tools/test_fs_target.py (which also fsck's the image afterwards). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "services/fs.h"
#include "services/fs_target.h"
#include "services/settings.h"

static FILE *s_img;

static int img_read(uint32_t lba, uint32_t count, void *buf)
{
    return fseek(s_img, (long)lba * 512L, SEEK_SET) ||
           fread(buf, 512, count, s_img) != count;
}

static int img_write(uint32_t lba, uint32_t count, const void *buf)
{
    return fseek(s_img, (long)lba * 512L, SEEK_SET) ||
           fwrite(buf, 512, count, s_img) != count;
}

static void walk(const char *path)
{
    fs_dirent_t e;
    uint32_t n, total, i;
    char sub[1024];

    for (i = 0; fs_list_dir(path, i, &e, 1, &n, &total) == 0 && n; i++) {
        snprintf(sub, sizeof sub, "%s/%s", path, e.name);
        if (e.is_dir) {
            printf("D %s\n", sub);
            walk(sub);
        } else {
            static unsigned char buf[777];
            fs_file_t *f = fs_open_read(sub);
            uint32_t h = 2166136261u;
            long r, got = 0;

            if (!f) {
                printf("E open %s\n", sub);
                continue;
            }
            while ((r = fs_read(f, buf, sizeof buf)) > 0)
                for (long k = 0; k < r; k++, got++)
                    h = (h ^ buf[k]) * 16777619u;
            printf("F %s %ld %08x%s\n", sub, fs_file_size(f), h, r < 0 || got != fs_file_size(f) ? " ERR" : "");
            fs_close(f);
        }
    }
}

/* Read `path` back through the FS and print (size, FNV, bytes read). */
static void report(const char *tag, const char *path)
{
    static unsigned char buf[777];
    fs_file_t *f = fs_open_read(path);
    uint32_t h = 2166136261u;
    long n, got = 0;

    while (f && (n = fs_read(f, buf, sizeof buf)) > 0)
        for (long k = 0; k < n; k++, got++)
            h = (h ^ buf[k]) * 16777619u;
    printf("%s %ld %08x %ld\n", tag, f ? fs_file_size(f) : -1, h, got);
    if (f)
        fs_close(f);
}

/* Creation phase (fs_target_mkdir / fs_target_file_create): run on a FRESH
 * image and print "C <tag> <rc>" lines for tools/test_fs_target.py, which
 * mounts the result with the kernel and fsck's it. Mirrors what the boot
 * initializer does (rechord_files.c) on the device's internal volume. */
static void create_phase(void)
{
    static char text[512];
    static unsigned char buf[4096];
    settings_t d;
    int len, i;

    settings_defaults(&d);
    len = settings_serialize_text(&d, text, sizeof text);

    printf("C mkdir %d\n", fs_target_mkdir("/SD/.rechord"));
    printf("C cfg %d\n",
           fs_target_file_create("/SD/.rechord/rechord.cfg", 64u * 1024u));
    printf("C state %d\n",
           fs_target_file_create("/SD/.rechord/state", 4096u));
    printf("C log %d\n",
           fs_target_file_create("/SD/.rechord/LOG", 32u * 1024u));
    /* The initializer writes the defaults into the fresh (all-zero) cfg. */
    printf("C put %d\n",
           fs_target_file_rewrite("/SD/.rechord/rechord.cfg", text,
                                  (uint32_t)len));
    /* Idempotency: a second pass must create nothing and change nothing. */
    printf("C again %d %d %d %d\n",
           fs_target_mkdir("/SD/.rechord"),
           fs_target_file_create("/SD/.rechord/rechord.cfg", 64u * 1024u),
           fs_target_file_create("/SD/.rechord/state", 4096u),
           fs_target_file_create("/SD/.rechord/LOG", 32u * 1024u));
    /* Refusals: an existing DIRECTORY is not a file, and vice versa; a name
     * with a non-ASCII byte is refused by the LFN writer. */
    printf("C clash %d\n",
           fs_target_file_create("/SD/.rechord", 512u));
    printf("C utf8 %d\n",
           fs_target_file_create("/SD/.rechord/a\xc3\xa9", 0u));
    /* Enough 8.3 names to fill the first directory cluster and force the
     * chained-directory extension path (512-byte clusters here). */
    for (i = 0; i < 20; i++) {
        char p[64];

        snprintf(p, sizeof p, "/SD/.rechord/f%02d", i);
        if (fs_target_file_create(p, 0u) != 0)
            printf("C fill%d ERR\n", i);
    }
    printf("C fill 20\n");
    /* Append (the media index builds incrementally): create an empty file,
     * append in three steps -- a short string, then a known payload twice --
     * and read it back. The chain has to grow across clusters (512-byte
     * clusters here: 1205 bytes = 3 clusters) and the sector the append point
     * falls into must keep the bytes before it. */
    {
        static unsigned char pay[600];
        unsigned k;

        for (k = 0; k < 600u; k++)
            pay[k] = (unsigned char)(k * 17u + 3u);
        printf("C mkapp %d\n",
               fs_target_file_create("/SD/.rechord/app", 0u));
        printf("C ap1 %d\n",
               fs_target_file_append("/SD/.rechord/app", "hello", 5u));
        printf("C ap2 %d\n",
               fs_target_file_append("/SD/.rechord/app", pay, 600u));
        printf("C ap3 %d\n",
               fs_target_file_append("/SD/.rechord/app", pay, 600u));
        printf("C ap0 %d\n",
               fs_target_file_append("/SD/.rechord/app", "", 0u));
        printf("C apmiss %d\n",
               fs_target_file_append("/SD/.rechord/nope", "x", 1u));
        printf("C apdir %d\n",
               fs_target_file_append("/SD/EmptyDir", "x", 1u));
        report("C rapp", "/SD/.rechord/app");
    }
    printf("C read %d\n", fs_open_read("/SD/.rechord/rechord.cfg") ? 1 : 0);
    report("C rcfg", "/SD/.rechord/rechord.cfg");
    report("C rstate", "/SD/.rechord/state");
    (void)buf;
}

int main(int argc, char **argv)
{
    fs_dirent_t e;
    uint32_t n, total, i;

    if (argc < 2 || argc > 3 || !(s_img = fopen(argv[1], "r+b")))
        return 2;
    int r = fs_target_mount(1, "SD", img_read, 0);
    int create = argc == 3 && strcmp(argv[2], "create") == 0;

    printf("mount %d %s\n", r, fs_target_type(1));
    if (create) {
        printf("W set %d\n", fs_target_set_writer(1, img_write));
        create_phase();
        return 0;
    }
    fs_list_dir("/", 0, &e, 1, &n, &total);
    printf("root %u %s\n", total, n ? e.name : "-");
    walk("/SD");

    /* In-place rewrite (the BRNOMT-71 path). lower.txt is 513 B -> 2 clusters
     * on this image (512-byte clusters), i.e. 1024 B of allocation. */
    {
        static unsigned char payload[4096];

        for (i = 0; i < 1000u; i++)
            payload[i] = (unsigned char)(i * 31u + 7u);
        printf("W set %d\n", fs_target_set_writer(1, img_write));
        /* 1 MiB cannot fit any file's single-cluster allocation: it must be
         * refused BEFORE any byte is written (cluster sizes differ per
         * volume, so the length has to be unambiguous). */
        printf("W big %d\n",
               fs_target_file_rewrite("/SD/lower.txt", payload, 1u << 20));
        printf("W put %d\n",
               fs_target_file_rewrite("/SD/lower.txt", payload, 1000u));
        report("W back", "/SD/lower.txt");
        printf("W cut %d\n",
               fs_target_file_rewrite("/SD/lower.txt", payload, 100u));
        report("W back2", "/SD/lower.txt");
        /* Grow back to the full allocation: a preallocated file whose size is
         * smaller than its chain is FAT *slack* (fsck offers to truncate it),
         * so end the test with the size matching the chain and the image
         * fsck-clean. */
        printf("W put2 %d\n",
               fs_target_file_rewrite("/SD/lower.txt", payload, 1000u));
        report("W back3", "/SD/lower.txt");
        /* Missing file and directory targets must be refused. */
        printf("W miss %d\n",
               fs_target_file_rewrite("/SD/nope.txt", payload, 10u));
        printf("W dir %d\n",
               fs_target_file_rewrite("/SD/EmptyDir", payload, 10u));
    }
    return 0;
}
