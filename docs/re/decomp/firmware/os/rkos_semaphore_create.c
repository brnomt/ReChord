/**
 * rkos_semaphore_create @ 0x030233b6
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 rkos_semaphore_create(undefined4 param_1,int param_2)



{

  undefined4 uVar1;

  int unaff_r6;

  int unaff_r7;

  int unaff_r8;

  

  USBMSCHost_Read(param_2 + -6);

  memset(&stack0x0000000c,6);

  if ((0 < unaff_r7) && (unaff_r6 + 1 <= unaff_r8)) {

    uVar1 = rkos_memory_malloc(&stack0x0000000c,s_image__03023674,6);

    return uVar1;

  }

  return 0;

}
