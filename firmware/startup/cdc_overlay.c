/*
 * cdc_overlay.c -- load /.rechord/cdc.bin into 0x0304AE64 and call it.
 *
 * The CDC driver + console do not fit in the AP code window. They are built
 * as a separate overlay at 0x0304AE64 and copied to the device as a FAT
 * file. The AP only reads the file, zeroes the window (for the overlay .bss),
 * copies the bytes and calls the Thumb entry.
 */
#include "startup/cdc_overlay.h"

#if AP_CDC_OVERLAY

#include <stdint.h>

#define CDC_BASE 0x0304AE64u
#define CDC_END  0x03050000u

extern void *fopen(const char *path, const char *mode);
extern int fgetc(void *stream);
extern int feof(void *stream);
extern int fclose(void *stream);

int ap_cdc_overlay_run(void)
{
    void *f = fopen("/Internal/.rechord/cdc.bin", "rb");
    volatile uint8_t *dst = (volatile uint8_t *)CDC_BASE;
    unsigned n = 0;
    unsigned i;
    int c;

    if (!f)
        return -1;

    for (i = 0; i < CDC_END - CDC_BASE; ++i)
        dst[i] = 0;

    while (!feof(f) && n < CDC_END - CDC_BASE) {
        c = fgetc(f);
        if (c < 0)
            break;
        dst[n++] = (uint8_t)c;
    }
    fclose(f);

    if (!n)
        return -2;

    __asm__ volatile("dsb; isb" ::: "memory");
    return ((int (*)(void))(CDC_BASE | 1u))();
}

#endif
