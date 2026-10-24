/*
 * mem_probe.h -- one-shot runtime probe for the 0x03088000 window.
 *
 * Build with AP_MEM_PROBE=1 to include the probe in a test image. The normal
 * image (AP_MEM_PROBE=0) compiles it out.
 */
#ifndef RECHORD_STARTUP_MEM_PROBE_H
#define RECHORD_STARTUP_MEM_PROBE_H

#if AP_MEM_PROBE
int ap_mem_probe_run(void);
#endif

#endif
