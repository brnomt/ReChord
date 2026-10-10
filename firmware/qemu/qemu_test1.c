/* ReChord QEMU harness test 1 — CMSDK CLCD on mps2-an385, proper init. */
#define CLCD_BASE    0x41100000u
#define LCD_FB       0x60000000u   /* try SDRAM */
#define LCD_W        320
#define LCD_H        240

#define REG(o)       (*(volatile unsigned int *)(CLCD_BASE + (o)))

void _start(void)
{
    volatile unsigned short *fb = (volatile unsigned short *)LCD_FB;
    int x, y;

    /* configure PL110 CLCD: 320x240x16bpp */
    REG(0x00) = ((320/16)-1) | (2 << 2);   /* Timing0: PPL + HSW */
    REG(0x04) = (240-1);                    /* Timing1: LPP */
    REG(0x08) = 0;                          /* Timing2 */
    REG(0x0C) = 0;                          /* Timing3 */
    REG(0x10) = LCD_FB;                     /* UPBASE */
    REG(0x14) = LCD_FB;                     /* LPBASE */
    REG(0x18) = 0x05;                       /* Control: EN + 16bpp */
    REG(0x20) = 0;                          /* IMSC off */
    REG(0x2C) = 0xFFFFFFFF;                 /* clear IRQ */

    /* clear + draw quadrants */
    for (y = 0; y < LCD_H; y++)
        for (x = 0; x < LCD_W; x++)
            fb[y*LCD_W+x] = 0x0000;
    for (y = 0; y < LCD_H/2; y++)
        for (x = 0; x < LCD_W/2; x++)
            fb[y*LCD_W+x] = 0xF800;
    for (y = 0; y < LCD_H/2; y++)
        for (x = LCD_W/2; x < LCD_W; x++)
            fb[y*LCD_W+x] = 0x07E0;
    for (y = LCD_H/2; y < LCD_H; y++)
        for (x = 0; x < LCD_W/2; x++)
            fb[y*LCD_W+x] = 0x001F;
    for (y = LCD_H/2; y < LCD_H; y++)
        for (x = LCD_W/2; x < LCD_W; x++)
            fb[y*LCD_W+x] = 0xFFFF;

    for (;;)
        ;
}

void _default_handler(void) { for (;;) ; }

__attribute__((section(".vectors")))
const unsigned int vector_table[8] = {
    0x28040000,
    (unsigned int)_start,
    (unsigned int)_default_handler,
    (unsigned int)_default_handler,
    (unsigned int)_default_handler,
    (unsigned int)_default_handler,
    (unsigned int)_default_handler,
    0,
};
