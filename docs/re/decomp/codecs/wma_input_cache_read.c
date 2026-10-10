/**
 * wma_input_cache_read @ 0x03083d48
 * Tags: codec, wma
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint wma_input_cache_read

               (undefined4 param_1,undefined4 param_2,uint param_3,int param_4,uint param_5,

               int *param_6)



{

  int iVar1;

  int iVar2;

  uint uVar3;

  int iVar4;

  uint uVar5;

  

  iVar2 = DAT_03083e00;

  iVar1 = DAT_03083dfc;

  uVar5 = param_3 - *(uint *)(DAT_03083dfc + 0x20);

  iVar4 = (param_4 - *(int *)(DAT_03083dfc + 0x24)) -

          (uint)(param_3 < *(uint *)(DAT_03083dfc + 0x20));

  *param_6 = 0;

  if ((iVar4 < 0) || (uVar3 = *(uint *)(iVar1 + 8), (int)(uint)(uVar5 < uVar3) <= iVar4)) {

    if ((uint)(param_3 + param_5 < 0x80000000) <= -(param_4 + (uint)CARRY4(param_3,param_5))) {

      do {

                    /* WARNING: Do nothing block with infinite loop */

      } while( true );

    }

    iVar4 = (*(code *)*DAT_03083e0c)(param_3,0,*DAT_03083e08);

    if (iVar4 != 0) {

      return 0;

    }

    *(undefined4 *)(iVar1 + 8) = 0;

  }

  else {

    if ((int)(iVar4 + (uint)CARRY4(uVar5,param_5)) < (int)(uint)(uVar5 + param_5 < uVar3)) {

      *param_6 = iVar2 + uVar5;

      return param_5;

    }

    wma_memmove(DAT_03083e00,iVar2 + uVar5,uVar3 - uVar5,(uVar5 + param_5) - uVar3);

    *(uint *)(iVar1 + 8) = *(int *)(iVar1 + 8) - uVar5;

  }

  *(uint *)(iVar1 + 0x20) = param_3;

  *(int *)(iVar1 + 0x24) = param_4;

  iVar4 = (*(code *)*DAT_03083e04)

                    (*(int *)(iVar1 + 8) + iVar2,0x200 - *(int *)(iVar1 + 8),

                     *(undefined4 *)(iVar1 + 4));

  *(int *)(iVar1 + 8) = iVar4 + *(int *)(iVar1 + 8);

  *param_6 = iVar2;

  return param_5;

}
