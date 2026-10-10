/* ReChord GUI dev — lm3s6965evb OLED via PL022 SSI (0x40008000).
 * PL022 regs: CR0@0x00, CR1@0x04, DR@0x08, SR@0x0C, CPSR@0x10.
 * Wiring: SSI0=data, PC7=DC, PD0=CS.
 */
#define SSI0 0x40008000u
static volatile unsigned int *ssi = (volatile unsigned int *)SSI0;

#define GPIO_C_AHB_DATA 0x400073FCu

static void dc(int data_mode) {
    volatile unsigned int *gpioc = (volatile unsigned int *)GPIO_C_AHB_DATA;
    if (data_mode) gpioc[0] |=  (1 << 7);
    else           gpioc[0] &= ~(1 << 7);
}
static void ssi_write9(unsigned int data9) {
    while (!(ssi[3] & 0x02)) ;   /* SR.TNF */
    ssi[2] = data9;              /* DR: bit8=DC, bits7:0=data */
}
static void oled_cmd(unsigned char c) { ssi_write9(0x000 | c); }
static void oled_data(unsigned char d) { ssi_write9(0x100 | d); }

void _start(void)
{
    /* Clock gating: GPIO A(0)+C(2) RCGC2; SSI0 RCGC1 bit4 */
    *(volatile unsigned int *)0x400FE108u |= (1<<0)|(1<<2);
    *(volatile unsigned int *)0x400FE104u |= (1<<4);

    /* A2=Clk, A5=Tx -> SSI peripheral */
    *(volatile unsigned int *)0x40004420u |= (1<<2)|(1<<5);
    *(volatile unsigned int *)0x4000451Cu |= (1<<2)|(1<<5);

    /* PL022: CR0=8bit/mode0, CPSR=16, CR1=SSE|MS */
    ssi[0] = 0x0008;      /* CR0: DSS=9bit (DC+data), SPO=0, SPH=0 */
    ssi[4] = 16;          /* CPSR */
    ssi[1] = 0x02 | 0x04; /* CR1: SSE + MS (0x06) */

    /* OLED init (QEMU ssd0323 model) */
    oled_cmd(0xAE);          /* display off */
    oled_cmd(0xA4);          /* normal mode */
    oled_cmd(0xA8); oled_cmd(0x3F);   /* mux ratio 64 */
    oled_cmd(0xAF);          /* display on */

    /* draw: row 0..7 = white, rest black (128 cols x 64 rows) */
    for (int r = 0; r < 64; r++) {
        oled_cmd(0x15); oled_cmd(0x00); oled_cmd(0x7F);  /* col 0..127 */
        oled_cmd(0x75); oled_cmd(r); oled_cmd(r);        /* row r */
        for (int c = 0; c < 128; c++)
            oled_data((r < 8) ? 0xFF : 0x00);
    }

    for (;;) ;
}
void _default_handler(void) { for (;;) ; }
__attribute__((section(".vectors")))
const unsigned int vector_table[8] = {
    0x20010000, (unsigned int)_start,
    (unsigned int)_default_handler, (unsigned int)_default_handler,
    (unsigned int)_default_handler, (unsigned int)_default_handler,
    (unsigned int)_default_handler, 0,
};
