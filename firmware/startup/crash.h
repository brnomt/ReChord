/*
 * crash.h — AP fault record (see crash.c).
 */
#ifndef RECHORD_STARTUP_CRASH_H
#define RECHORD_STARTUP_CRASH_H

/* Boot, after the Internal volume is mounted and writable: report an unseen
 * record to the log, size the file, resolve its sectors. */
void crash_boot(const char *build);

/* Re-resolve the file's sectors after a remount (USB storage session). */
void crash_arm(void);

/* The HardFault/MemManage/BusFault/UsageFault vector. */
void crash_handler(void);

#ifdef AP_CRASH_TEST
void crash_test_fire(void);
#endif

#endif /* RECHORD_STARTUP_CRASH_H */
