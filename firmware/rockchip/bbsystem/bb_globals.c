/*
 * bb_globals.c — definition site for global.h's EXT-pattern globals.
 *
 * global.h declares `EXT _ATTR_SYS_BSS_ UINT32 MediaInfoAddr;` & friends where
 * EXT collapses to nothing only in the TU that defines _IN_MAIN_ (the SDK's
 * "main" TU convention). The stock AP provided that TU; the BB build has no
 * such file, so this one turns the switch on and includes global.h once.
 */
#define _IN_MAIN_

#include "typedef.h"
#include "SysConfig.h"
#include "global.h"


/* RKFLength bridge: ID3.c calls the AP-twin's RKFLength(FILE*); the BB twin
 * (bbsystem/audio_file_access2.c) provides HifiRKFLength over the same
 * handle. Both files defined the RKFIO_* table, so the Common twin is out of
 * the build and this forwards instead. */
extern unsigned long HifiRKFLength();
extern unsigned long RKFLength(void *in);
unsigned long RKFLength(void *in)
{
    return HifiRKFLength(in);
}
