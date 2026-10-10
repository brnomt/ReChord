/**
 * GetFreeMemory @ 0x0302336a
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 GetFreeMemory(int param_1)



{

  int iVar1;

  undefined4 uVar2;

  int iVar3;

  uint in_stack_00000004;

  

  iVar3 = param_1 + in_stack_00000004 * 0x1000000 +

          (in_stack_00000004 >> 8 & 0xff) * 0x10000 + (in_stack_00000004 >> 0x10 & 0xff) * 0x100;

  if (0x200 < iVar3) {

    iVar3 = 0x200;

  }

  iVar1 = memset(&stack0x0000000c,6);

  if ((0 < iVar1) && (-1 < iVar3)) {

    uVar2 = rkos_memory_malloc(&stack0x0000000c,s_image__03023674,6);

    return uVar2;

  }

  return 0;

}
