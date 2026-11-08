/*
 * bt_hw.c — Bluetooth module hardware (UART1 + power + audio route).
 *
 * The register sequence follows EchoPod's hardware bring-up on the same
 * player (community/echopod, GPL-3.0; credited in the README): pins parked low
 * first, UART1 opened on GPIO2.16/2.17 group A, then the module powered from
 * GPIO2.7. Routing to the module is GPIO2.0 high + GPIO2.14 low after the
 * stock housekeeping writes; the DAC keeps playing on the normal I2S1 path.
 *
 * This file is AP-only and has no dependency on the player or the UI.
 */
#include "bt_hw.h"

#include "ao_regs.h"
#include "log.h"
#include "rknano_i2s.h"     /* cru_gate(), cru_rst(), grf_iomux(), delay */

#define BT_UART_BASE 0x400B0000u

#define UART_DATA             REG32(BT_UART_BASE + 0x00u)
#define UART_DIVISOR_HIGH     REG32(BT_UART_BASE + 0x04u)
#define UART_INTERRUPT_ENABLE REG32(BT_UART_BASE + 0x04u)  /* unused */
#define UART_LINE_CONTROL     REG32(BT_UART_BASE + 0x0Cu)
#define UART_MODEM_CONTROL    REG32(BT_UART_BASE + 0x10u)
#define UART_LINE_STATUS      REG32(BT_UART_BASE + 0x14u)
#define UART_STATUS           REG32(BT_UART_BASE + 0x7Cu)
#define UART_SOFTWARE_RESET   REG32(BT_UART_BASE + 0x88u)
#define UART_FIFO_ENABLE      REG32(BT_UART_BASE + 0x98u)
#define UART_RX_TRIGGER       REG32(BT_UART_BASE + 0x9Cu)
#define UART_TX_EMPTY_TRIGGER REG32(BT_UART_BASE + 0xA0u)
#define UART_DMA_ACK          REG32(BT_UART_BASE + 0xA8u)

#define BT_RST_UART1      0x08u
#define BT_GATE_UART1     0x1Du
#define BT_GATE_UART1_BUS 0x89u

#define UART_RESET_ALL    0x08u
#define UART_RESET_FIFOS  0x06u
#define UART_LINE_8N1     0x03u
#define UART_DIVISOR_LATCH 0x80u
#define UART_DIVISOR_115200 13u   /* 24 MHz / (16 * 115200) ~= 13.02 */
#define UART_LINE_DATA_READY 0x01u
#define UART_LINE_OVERRUN    0x02u
#define UART_STATUS_TX_NOT_FULL 0x02u

#define GRF_IOMUX_UART1_GROUP_A_SEL (1u << 9)   /* 0 = group A */
#define GRF_IOMUX_WE(bit)           (1u << ((bit) + 16))

/* CLKSEL_CON for UART (EchoPod hardware sequence: select crystal, 1:1). */
#define CRU_UART_CLKSEL 0x38u
#define CRU_UART1_CRYSTAL 0x00C00040u
#define CRU_UART1_DIVIDER_ONE 0x003F0000u

#define BT_ROUTE_SETTLE_MS 500u

/* Audio HAL hook (audio/audio_output.c): the Bluetooth level rides on the
 * DAC volume table, so the route change must re-apply it. Declared weak so
 * driver-only builds and host tests link without the audio HAL. */
void audio_output_bluetooth_route(int routed) __attribute__((weak));

/* GPIO2 bit numbers. */
#define BT_PIN_POWER   7
#define BT_PIN_UART_TX 16
#define BT_PIN_UART_RX 17
#define BT_PIN_WIRED_MUTE 0
#define BT_PIN_WIRED_OUT  14
#define BT_PIN_PANEL_STRAP 9

static int s_powered;
static int s_routed;
static int s_housekeeping;
static unsigned long s_routed_ms;

static unsigned long now_ms(void)
{
    return log_now_ms();
}

static void pins_low(void)
{
    dac_gpio_out(2, BT_PIN_POWER, 0);
    dac_gpio_out(2, BT_PIN_UART_TX, 0);
    dac_gpio_out(2, BT_PIN_UART_RX, 0);
}

static void uart_clock_on(void)
{
    cru_rst(BT_RST_UART1, 1);
    dac_delay_ms(1);
    cru_rst(BT_RST_UART1, 0);
    cru_gate(BT_GATE_UART1, 1);
    cru_gate(BT_GATE_UART1_BUS, 1);
    REG32(CRU_BASE + CRU_UART_CLKSEL) = CRU_UART1_CRYSTAL;
    REG32(CRU_BASE + CRU_UART_CLKSEL) = CRU_UART1_DIVIDER_ONE;
}

static void uart_configure(void)
{
    UART_SOFTWARE_RESET = UART_RESET_ALL;
    REG32(BT_UART_BASE + 0x04u) = 0;          /* interrupt enable: polled  */
    UART_DMA_ACK = 1;
    UART_MODEM_CONTROL = 0;
    UART_FIFO_ENABLE = 1;
    UART_RX_TRIGGER = 0;
    UART_TX_EMPTY_TRIGGER = 0;
    UART_LINE_CONTROL = UART_LINE_8N1;
    UART_LINE_CONTROL = UART_LINE_8N1 | UART_DIVISOR_LATCH;
    UART_DATA = UART_DIVISOR_115200;
    UART_DIVISOR_HIGH = 0;
    UART_LINE_CONTROL = UART_LINE_8N1;
    UART_SOFTWARE_RESET = UART_RESET_FIFOS;
}

static void uart_open(void)
{
    /* UART1 group A select, written with only that field's write-enable so the
     * I2C and I2S1 selects in the same register are preserved. */
    GRF_IOMUX = (GRF_IOMUX & ~GRF_IOMUX_UART1_GROUP_A_SEL) |
                GRF_IOMUX_WE(9);
    uart_clock_on();
    uart_configure();
    grf_iomux(2, BT_PIN_UART_TX, 1);          /* func 1 = UART1 */
    grf_iomux(2, BT_PIN_UART_RX, 1);
}

void bt_hw_power_on(void)
{
    if (s_powered)
        return;
    pins_low();
    uart_open();
    dac_gpio_out(2, BT_PIN_POWER, 1);
    s_powered = 1;
    log_printf("bt: module powered");
}

void bt_hw_power_off(void)
{
    if (s_routed)
        bt_hw_route(0);
    pins_low();
    /* Park the UART block too: no bytes can arrive from a module whose power
     * pin is low, and uart_clock_on() re-gates it on the next power-on. */
    cru_rst(BT_RST_UART1, 1);
    cru_gate(BT_GATE_UART1, 0);
    cru_gate(BT_GATE_UART1_BUS, 0);
    s_powered = 0;
}

int bt_hw_powered(void)
{
    return s_powered;
}

int bt_hw_write(const uint8_t *bytes, unsigned len)
{
    unsigned i;

    if (!s_powered || bytes == 0)
        return -1;
    for (i = 0; i < len; i++) {
        unsigned guard = 0;

        while ((UART_STATUS & UART_STATUS_TX_NOT_FULL) == 0 &&
               guard++ < 100000u)
            ;
        UART_DATA = bytes[i];
    }
    return 0;
}

int bt_hw_read(void)
{
    uint32_t line;

    if (!s_powered)
        return -1;
    line = UART_LINE_STATUS;
    /* Clear an overrun by reading the status; the byte is lost either way. */
    (void)line;
    if ((line & UART_LINE_DATA_READY) == 0)
        return -1;
    return (int)(UART_DATA & 0xFFu);
}

/* Stock's streaming housekeeping. Applied once, on the first route; the
 * register dumps in EchoPod's hardware notes matched stock while streaming. */
static void stock_housekeeping(void)
{
    REG32(GRF_BASE + 0xC4u) = 0x00010000u;
    REG32(CRU_BASE + 0x84u) = 0x00920092u;
    REG32(CRU_BASE + 0xA0u) = 0x00020002u;
    REG32(CRU_BASE + 0xC0u) = 0x00680068u;
    REG32(GRF_BASE + 0x14u) = 0x00200020u;
    REG32(GRF_BASE + 0x54u) = 0x00010001u;
    dac_gpio_out(2, BT_PIN_PANEL_STRAP, 0);
    s_housekeeping = 1;
}

void bt_hw_route(int to_module)
{
    to_module = to_module ? 1 : 0;
    if (to_module == s_routed)
        return;                        /* called every service pass */
    if (to_module && !s_housekeeping)
        stock_housekeeping();
    /* These two writes are also funnelled through the override below, but set
     * them explicitly so the route takes effect even if a caller holds a
     * cached level. */
    s_routed = to_module;
    dac_gpio_out(2, BT_PIN_WIRED_MUTE, to_module ? 1 : 0);
    dac_gpio_out(2, BT_PIN_WIRED_OUT, to_module ? 0 : 1);
    if (audio_output_bluetooth_route != 0)
        audio_output_bluetooth_route(to_module);
    s_routed_ms = now_ms();
    log_printf("bt: audio route %s", to_module ? "module" : "wired");
}

int bt_hw_routed(void)
{
    return s_routed;
}

int bt_hw_settling(void)
{
    return s_routed_ms != 0 && now_ms() - s_routed_ms < BT_ROUTE_SETTLE_MS;
}

int bt_audio_pin_override(int port, int pin, int level)
{
    if (!s_routed || port != 2)
        return level;
    if (pin == BT_PIN_WIRED_MUTE)
        return 1;                 /* keep the wired output muted */
    if (pin == BT_PIN_WIRED_OUT)
        return 0;                 /* keep the module selected   */
    return level;
}
