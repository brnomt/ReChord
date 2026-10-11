/* Host harness for fs_target.c: mounts a disk image as volume "SD" and
 * prints every directory and file (size + FNV-1a of the content, read in odd
 * chunk sizes). Driven by tools/test_fs_target.py. */
#include <stdio.h>
#include <stdlib.h>

#include "services/fs.h"
#include "services/fs_target.h"

static FILE *s_img;

static int img_read(uint32_t lba, uint32_t count, void *buf)
{
    return fseek(s_img, (long)lba * 512L, SEEK_SET) ||
           fread(buf, 512, count, s_img) != count;
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

int main(int argc, char **argv)
{
    fs_dirent_t e;
    uint32_t n, total;

    if (argc != 2 || !(s_img = fopen(argv[1], "rb")))
        return 2;
    int r = fs_target_mount(1, "SD", img_read, 0);

    printf("mount %d %s\n", r, fs_target_type(1));
    fs_list_dir("/", 0, &e, 1, &n, &total);
    printf("root %u %s\n", total, n ? e.name : "-");
    walk("/SD");
    return 0;
}
