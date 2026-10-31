/*
 * bt_hw.h — Bluetooth module hardware: UART1, power, audio route.
 *
 * The module owns its Bluetooth stack and is driven with short ASCII frames
 * (services/bt_proto.c). This file is only the register-level side: pins,
 * UART, and the two GPIOs that route the DAC output to the module.
 *
 * Hardware contract (hardware-verified by EchoPod on the same player; see
 * docs/rewrite/bluetooth.md):
 *   GPIO2.7  module power (high = on)
 *   GPIO2.16 UART1 TX (func 1, group A)
 *   GPIO2.17 UART1 RX (func 1, group A)
 *   GPIO2.0  wired mute (high while routed to Bluetooth)
 *   GPIO2.14 wired output (low while routed to Bluetooth)
 *   GPIO2.9  panel strap (stock drives it low with the routing housekeeping)
 */
#ifndef RECHORD_DRIVERS_BT_HW_H
#define RECHORD_DRIVERS_BT_HW_H

#include <stdint.h>

/* Park the pins, open UART1 and power the module. Idempotent. */
void bt_hw_power_on(void);

/* Stop serving the UART and drive the module pins low. A powered module keeps
 * the player from switching off, so this must run before power_off(). */
void bt_hw_power_off(void);

int  bt_hw_powered(void);

/* Send one frame. Returns 0, or -1 when the UART is not open. */
int  bt_hw_write(const uint8_t *bytes, unsigned len);

/* Next received byte, or -1 when the RX FIFO is empty. */
int  bt_hw_read(void);

/* Route the DAC output to the module (1) or back to the wired output (0).
 * Applies the routing pins and, on the first route, the stock housekeeping. */
void bt_hw_route(int to_module);

int  bt_hw_routed(void);

/* 1 for 500 ms after a route change: the jack sense cannot be trusted then. */
int  bt_hw_settling(void);

/* Hook used by the HAL's dac_gpio_out() so the DAC's own unmute writes cannot
 * clear the routing while it is active. Defined here, declared weak there. */
int  bt_audio_pin_override(int port, int pin, int level);

#endif /* RECHORD_DRIVERS_BT_HW_H */
