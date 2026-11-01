/*
 * jack.c — headphone jack detection over the two sense pins.
 *
 * The pins are muxed to GPIO and read through EXT_PORTA (input data), the
 * same register the panel driver uses for its GPIO0 reads. No interrupts: the
 * app polls this at a human-scale rate (~10 Hz), which also keeps the battery
 * cost negligible.
 */
#include <stdint.h>

#include "jack.h"
#include "rknano_i2s.h"     /* grf_iomux() */

#define PIN_SINGLE_ENDED_PORT 0
#define PIN_SINGLE_ENDED      13
#define PIN_BALANCED_PORT     2
#define PIN_BALANCED          8

/* GPIO port register block (same layout as panel_io_rknanoc.c). */
typedef struct {
    volatile uint32_t dr;          /* +0x00 SWPORTA_DR  (output data) */
    volatile uint32_t ddr;         /* +0x04 SWPORTA_DDR (direction)   */
    volatile uint32_t unused[18];  /* +0x08 .. +0x4C                  */
    volatile uint32_t ext_porta;   /* +0x50 EXT_PORTA   (input data)  */
} gpio_regs_t;

#define GPIO0 ((gpio_regs_t *)0x40160000u)
#define GPIO2 ((gpio_regs_t *)0x50030000u)

static jack_sense_t s_sense;
static jack_kind_t s_stable;

static void pin_input(int port, int pin)
{
    gpio_regs_t *g = (port == 0) ? GPIO0 : GPIO2;

    grf_iomux(port, pin, 0);       /* func 0 = GPIO */
    g->ddr &= ~(1u << (unsigned)pin);
}

static int pin_read(int port, int pin)
{
    gpio_regs_t *g = (port == 0) ? GPIO0 : GPIO2;

    return (int)((g->ext_porta >> (unsigned)pin) & 1u);
}

void jack_init(void)
{
    pin_input(PIN_SINGLE_ENDED_PORT, PIN_SINGLE_ENDED);
    pin_input(PIN_BALANCED_PORT, PIN_BALANCED);
}

int jack_poll(jack_kind_t *out)
{
    jack_kind_t reading = jack_kind_of(
        pin_read(PIN_SINGLE_ENDED_PORT, PIN_SINGLE_ENDED),
        pin_read(PIN_BALANCED_PORT, PIN_BALANCED));

    if (!jack_sense_sample(&s_sense, reading))
        return 0;
    s_stable = s_sense.stable;
    if (out != 0)
        *out = s_stable;
    return 1;
}

jack_kind_t jack_current(void)
{
    return s_stable;
}
