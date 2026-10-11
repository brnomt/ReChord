/*
 * test_fw_update.c -- host harness for fw_update.c over a disk image file
 * (built and checked by tools/test_fw_update.py).
 *   test_fw_update <disk.img> <vol_lba> <fw_sectors>
 * Runs find, install, retire, find again; prints results; writes the disk.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "drivers/emmc.h"
#include "services/fw_update.h"

static uint8_t *s_disk;
static uint32_t s_n, s_vol, s_fw;

int emmc_read(uint32_t lba, uint32_t count, void *buf)
{
    if (lba + count > s_n)
        return -30;
    memcpy(buf, s_disk + (size_t)lba * 512, (size_t)count * 512);
    return 0;
}

int emmc_write(uint32_t lba, uint32_t count, const void *buf)
{
    if (lba < s_vol || lba + count > s_n)
        return -30;
    memcpy(s_disk + (size_t)lba * 512, buf, (size_t)count * 512);
    return 0;
}

uint32_t emmc_fw_sectors(void) { return s_fw; }

int emmc_fw_write(uint32_t rel, uint32_t count, const void *buf)
{
    if (rel + count > s_fw)
        return -30;
    memcpy(s_disk + (size_t)(EMMC_FW_BASE + rel) * 512, buf, (size_t)count * 512);
    return 0;
}

int main(int argc, char **argv)
{
    static fw_image_t img;
    FILE *f;
    long len;
    int r;

    if (argc != 4 || !(f = fopen(argv[1], "r+b")))
        return 2;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    s_disk = malloc((size_t)len);
    rewind(f);
    if (fread(s_disk, 1, (size_t)len, f) != (size_t)len)
        return 2;
    s_n = (uint32_t)(len / 512);
    s_vol = (uint32_t)strtoul(argv[2], 0, 0);
    s_fw = (uint32_t)strtoul(argv[3], 0, 0);

    r = fw_update_find(s_vol, &img, NULL);
    printf("find %d %s size %u fw_end %x sha ", r, img.name, img.size, img.fw_end);
    for (int i = 0; i < 32; i++)
        printf("%02x", img.sha256[i]);
    printf("\n");
    if (r == 0) {
        printf("install %d\n", fw_update_install(&img, NULL));
        printf("retire %d\n", fw_update_retire(&img));
        printf("again %d\n", fw_update_find(s_vol, &img, NULL));
    }
    rewind(f);
    fwrite(s_disk, 1, (size_t)len, f);
    fclose(f);
    return 0;
}
