/*
 * panel_io_rknanoc.c -- TARGET panel_io backend for the RKNanoC (Cortex-M3).
 *
 * Clean-room implementation of the panel byte transport as FACTS
 * extracted from the reference firmware image (full trace in
 * docs/rewrite/drivers.md, "Transport"): the Echo Mini panel sits on a
 * GPIO-attached 8080-style 8-bit parallel bus that the reference
 * bit-bangs through the GPIO0 port registers -- it does NOT use the
 * SDK's VOP MCU interface at 0x60070000 (an earlier version of this
 * file assumed VOP from the SDK docs alone; the proven display path in
 * the reference writes GPIO0, so this file follows the reference's
 * hardware facts).
 *
 * Bus wiring (extracted from the reference's byte-write routine at
 * 0x03051c6c and its bring-up code):
 *   GPIO0.16..23  D0..D7   (byte value lives in bits 16..23 of GPIO0 DR)
 *   GPIO0.24      WR       (write strobe; the panel latches on the
 *                           rising edge of this bit)
 *   GPIO0.25      D/C      (0 = command byte, 1 = data/parameter byte)
 *   GPIO0.12      CS/RD-style control, held high outside ID probing
 *   GPIO2.15      panel RESET (active low pulse)
 *
 * One bus byte = three GPIO0 DR writes (exactly the extracted pattern:
 * two writes with WR low, one with WR high):
 *   DR = ctl | byte<<16 | D/C  with WR=0, again with WR=0, then WR=1.
 *
 * Register map (vendor SDK driver/hw_memap.h + reference usage):
 *   GPIO0_BASE 0x40160000, GPIO2_BASE 0x50030000,
 *   SWPORTA_DR +0x00, SWPORTA_DDR +0x04, EXT_PORTA +0x50.
 *
 * Link this file ONLY in target builds, in place of panel_io_sim.c.
 *
 * TODO (bring-up, see docs/rewrite/drivers.md "open items"):
 *   - panel_io_delay_ms(): the loop below is an uncalibrated placeholder
 *     (the reference uses a timer-based delay; that code is not decoded
 *     yet). Only affects init timing margins, not correctness.
 *   - CRU clock gating is assumed to be done by earlier boot stages
 *     (the reference's 0x030518d8 does a broad CRU/GRF setup not ported
 *     yet). Pinmux IS done here (panel_io_init).
 *   - The panel ID probe (GPIO0 bus read via EXT_PORTA) is not
 *     implemented; panel variant selection is compile-time (see
 *     panel_init_rknanoc.c).
 */

#include <stdint.h>

#include "panel_io.h"

/* GPIO port register block (see file header). */
typedef struct {
    volatile uint32_t dr;          /* +0x00 SWPORTA_DR  (output data)   */
    volatile uint32_t ddr;         /* +0x04 SWPORTA_DDR (direction)     */
    volatile uint32_t unused[18];  /* +0x08 .. +0x4C                   */
    volatile uint32_t ext_porta;   /* +0x50 EXT_PORTA   (input data)    */
} gpio_regs_t;

#define GPIO0 ((gpio_regs_t *)0x40160000u)
#define GPIO2 ((gpio_regs_t *)0x50030000u)

/* Bus bit positions in GPIO0 DR (extracted facts). */
#define PIN_DATA_SHIFT 16
#define PIN_DATA_MASK  (0xFFu << PIN_DATA_SHIFT)  /* GPIO0.16..23 = D0..D7 */
#define PIN_WR         (1u << 24)                 /* GPIO0.24 = WR strobe */
#define PIN_DC         (1u << 25)                 /* GPIO0.25 = D/C       */
#define PIN_CS         (1u << 12)                 /* GPIO0.12 = CS/RD ctl */

/* GPIO0 pins the reference drives for the panel: 12, 15, 16..25
 * (0x03FF9000 = bits 12 + 15 + 16..25; exact mask extracted from the
 * reference's direction-register setup). */
#define PANEL_DDR_MASK 0x03FF9000u

/* GPIO2.15 = panel RESET (active low). */
#define PIN_RESET      (1u << 15)

/*
 * Static shadow of the GPIO0 DR control bits (WR high, CS high, data
 * pins low). The reference keeps the same shadow in RAM (at 0x03006030)
 * and ORs each outgoing byte into it, so unrelated GPIO0 pins keep
 * their state across panel writes.
 */
static uint32_t bus_ctl;

static void bus_write(uint8_t byte, uint32_t dc_bit)
{
    uint32_t word = bus_ctl | ((uint32_t)byte << PIN_DATA_SHIFT) | dc_bit;

    GPIO0->dr = word & ~PIN_WR;   /* WR low  */
    GPIO0->dr = word & ~PIN_WR;   /* WR low  (pattern as extracted) */
    GPIO0->dr = word;             /* WR high: panel latches the byte */
}

/* GRF IOMUX (reference 0x03051458): 2-bit function per pin, 8 pins per
 * word, 0x20 per port, write-masked in the upper half. Function 0 = GPIO. */
static void iomux_gpio(unsigned port, unsigned pin)
{
    *(volatile uint32_t *)(0x50010000u + port * 0x20u + (pin >> 3) * 4u) =
        0x30000u << ((pin & 7u) * 2u);
}

/* Backlight full on: GPIO2.4 as GPIO output high (reference 0x030514c0;
 * its PWM dimming at 0x400F0000 is 0x03051dd8, not ported). */
void backlight_on(void)
{
    iomux_gpio(2, 4);
    GPIO2->dr |= 1u << 4;
    GPIO2->ddr |= 1u << 4;
}

/* Backlight off: drive the same pin low. power_off() only clears the
 * framebuffer, so without this the panel keeps GLOWING over a black screen
 * (field report: "el backlight sigue encendido" after turning off while
 * plugged into USB). */
void backlight_off(void)
{
    iomux_gpio(2, 4);
    GPIO2->ddr |= 1u << 4;
    GPIO2->dr &= ~(1u << 4);
}

/* Keys (reference key scan 0x03057eb0 + TRM SARADC chapter): SAR-ADC at
 * 0x400D0000 [0] SARADC_DATA[9:0], [1] SARADC_STAS bit0 = "conversion in
 * progress" (RO), [2] SARADC_CTRL bit4 = SOC, bit5 = int_en, bit6 =
 * int_status/EOC ("set to 1 when end of conversion, set 0 to clear").
 * The reference pipelines "read previous, start next" but ONLY because
 * its scan is rate-gated to 10 ms. The TRM's own procedure is: start,
 * poll for end of conversion, read SARADC_DATA.
 *
 * FIX 2026-10-16 (phantom keys under playback): our UI calls keys_adc()
 * up to 8x per tick while a track plays (player_process returns instantly
 * once the ring is full), so the old pipeline read ADC[0] MID-conversion
 * and restarted the converter on every call — stale/partial values mapped
 * into wrong key bands (volume presses firing Menu/Select/Back). Each
 * call now runs THREE fresh conversions, each waited to TRM end-of-
 * conversion, and returns the median. */
#define ADC ((volatile uint32_t *)0x400D0000u)

/* Fallback/conversion floor (~200 us at the 200 MHz AP clock). The SARADC
 * runs at <= 1 MHz (TRM), so a conversion is tens of us at most. */
static void adc_settle(void)
{
    volatile unsigned n = 40000;

    while (n--)
        ;
}

static int adc_read_fresh(int ch)
{
    unsigned t = 4000;

    /* ch1 + power + SOC + int_en: per the TRM, int_status (CTRL bit6) is
     * set at end of conversion ONLY with int_en (bit5) enabled — the
     * 367066f regression polled that flag with int_en clear, so every
     * read burned the full poll timeout and the UI crawled (DFIU
     * ae88365a). */
    ADC[2] &= ~0x50u;                        /* SOC=0, clear EOC           */
    ADC[2] = (ADC[2] & ~0x47u) | ((unsigned)ch & 7u) | 0x38u;
    /* EOC is only a FAST PATH: bounded so unknown flag semantics can
     * never stall the UI again; adc_settle() is the always-there floor
     * that guarantees a completed conversion before the read. */
    while ((ADC[2] & 0x40u) == 0u) {
        if (--t == 0)
            break;
    }
    adc_settle();
    return (int)(ADC[0] & 0x3FFu);
}

/* One fresh conversion on an arbitrary channel (BRNOMT-83: the battery gauge
 * uses channel 0; the keys use 1, exactly as the stock SDK's Adc.h defines). */
int adc_read_channel(int ch)
{
    if (ch < 0 || ch > 7)
        return -1;
    return adc_read_fresh(ch);
}

int keys_adc(void)
{
    int s0 = adc_read_fresh(1);
    int s1 = adc_read_fresh(1);
    int s2 = adc_read_fresh(1);

    /* median of three */
    if (s0 > s1) { int t = s0; s0 = s1; s1 = t; }
    if (s1 > s2) { int t = s1; s1 = s2; s2 = t; }
    if (s0 > s1) { int t = s0; s0 = s1; s1 = t; }
    return s1;
}

/* Power key: GPIO2.5 input, 1 = pressed (reference 0x03051b2c). */
int keys_power(void)
{
    iomux_gpio(2, 5);
    GPIO2->ddr &= ~(1u << 5);
    return (int)((GPIO2->ext_porta >> 5) & 1u);
}

/* Power hold: GPIO2.6 low cuts the supply (reference 0x03051c1c). With
 * USB plugged in the player stays powered by VBUS. */
void board_power_off(void)
{
    __asm__ volatile ("cpsid i");
    iomux_gpio(2, 6);
    GPIO2->dr &= ~(1u << 6);
    GPIO2->ddr |= 1u << 6;
    for (;;) { }
}

void panel_io_init(void)
{
    /* Mux every panel pin to GPIO first, as the reference does (0x03051ce8:
     * GPIO0 16..25, 15, 12; 0x03051cb0: GPIO2.15). Nothing guarantees the
     * ROM left them in GPIO mode. */
    static const uint8_t gpio0_pins[] = {16, 17, 18, 19, 20, 21, 22, 23,
                                         24, 25, 15, 12};
    for (unsigned i = 0; i < sizeof gpio0_pins; i++)
        iomux_gpio(0, gpio0_pins[i]);
    iomux_gpio(2, 15);

    /* GPIO2.15 = reset: output, released (high). */
    GPIO2->ddr |= PIN_RESET;
    GPIO2->dr |= PIN_RESET;

    /* GPIO0 panel pins: outputs (data bus + WR + D/C + CS/RD ctl). */
    GPIO0->ddr |= PANEL_DDR_MASK;

    /* Control idle state: WR high, CS high, data pins low; unrelated
     * GPIO0 pins preserved. */
    bus_ctl = (GPIO0->dr & ~PANEL_DDR_MASK) | PIN_WR | PIN_CS;
    GPIO0->dr = bus_ctl;

    /*
     * Reset pulse, timing extracted from the reference bring-up
     * (active low: high 50 ms, low 150 ms, high). The reference pulses
     * reset twice around its panel ID probe: first with a 30 ms settle,
     * later with a 120 ms settle right before the DCS stream. We have
     * no ID probe, so both pulses run back-to-back with their extracted
     * settle times (harmless, and keeps the pre-stream timing exact).
     */
    GPIO2->dr |= PIN_RESET;
    panel_io_delay_ms(50);
    GPIO2->dr &= ~PIN_RESET;
    panel_io_delay_ms(150);
    GPIO2->dr |= PIN_RESET;
    panel_io_delay_ms(30);

    GPIO2->dr |= PIN_RESET;
    panel_io_delay_ms(50);
    GPIO2->dr &= ~PIN_RESET;
    panel_io_delay_ms(150);
    GPIO2->dr |= PIN_RESET;
    panel_io_delay_ms(120);
}

void panel_io_send_cmd(uint8_t cmd)
{
    bus_write(cmd, 0);            /* D/C low = command byte */
}

void panel_io_send_data(uint8_t data)
{
    bus_write(data, PIN_DC);      /* D/C high = parameter/data byte */
}

void panel_io_send_pixel(uint16_t rgb565)
{
    /* Two data bytes on the wire, HIGH byte first (panel_io contract;
     * the reference's flush loop sends them exactly this way). */
    bus_write((uint8_t)(rgb565 >> 8), PIN_DC);
    bus_write((uint8_t)(rgb565 & 0xFFu), PIN_DC);
}

void panel_io_delay_ms(uint16_t ms)
{
    /* TODO: replace with a calibrated timer/SysTick delay at bring-up.
     * Busy loop scaled crudely for the 200 MHz boot clock. */
    volatile uint32_t count = (uint32_t)ms * 20000u;
    while (count-- > 0u) {
        /* spin */
    }
}
