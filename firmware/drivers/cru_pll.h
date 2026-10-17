/*
 * cru_pll.h — AP core clock: 24 MHz oscillator -> APLL 200 MHz.
 *
 * The reference firmware does exactly this at boot ("pll: switching to
 * 200 MHz", ECHOLOG: con0 0x1064 con1 0x9446 con2 0x08000000 mode 1
 * sel0 0x1001 sel1 0x200, pclk 50 MHz). The clean-room AP never did, so it
 * has been running at 24 MHz — which is why FLAC decode cannot keep up at
 * 96 kHz.
 *
 * Call ONCE, early (Main(), before any timing-sensitive bring-up). Raising
 * the clock changes every busy-wait delay; the delay constants in
 * rknano_i2s.c / panel_io_rknanoc.c and AP_CYCLES_PER_MS in ap_ui.c are
 * calibrated for the post-switch 200 MHz.
 */
#ifndef RECHORD_CRU_PLL_H
#define RECHORD_CRU_PLL_H

/* Switch the AP core complex to the APLL at 200 MHz (pclk 50 MHz). Also sets
 * VDD_CORE to 1.20 V (GPIO2A3) and enables the RAM high-response mode. */
void ap_clock_set_200mhz(void);

#endif /* RECHORD_CRU_PLL_H */
