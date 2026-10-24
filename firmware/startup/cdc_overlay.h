/* cdc_overlay.h -- AP-side loader for /.rechord/cdc.bin (code overlay). */
#ifndef RECHORD_STARTUP_CDC_OVERLAY_H
#define RECHORD_STARTUP_CDC_OVERLAY_H

#if AP_CDC_OVERLAY
int ap_cdc_overlay_run(void);
#endif

#endif
