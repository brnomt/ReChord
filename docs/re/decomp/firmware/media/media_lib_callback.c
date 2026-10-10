/**
 * media_lib_callback @ 0x030298b8
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4

media_lib_callback(undefined4 param_1,code *param_2,undefined4 param_3,int param_4,

                  undefined4 *param_5,undefined4 param_6,undefined4 param_7)



{

  int iVar1;

  undefined4 uVar2;

  

  freertos_create_task(DAT_03029920,0x40);

  iVar1 = freertos_init_timer(DAT_03029920,param_5,param_4,param_6,param_7);

  if (iVar1 == 0) {

    uVar2 = 0;

  }

  else {

    if (*(int *)(param_4 + 0x16) < 1) {

      (*param_2)(*param_5,*(undefined4 *)(param_4 + 10),0);

    }

    else {

      (*param_2)(*param_5,(*(int *)(DAT_03029920 + 4) + -1) * *(int *)(DAT_03029920 + 0xc) +

                          *(int *)(param_4 + 10),0);

    }

    uVar2 = 1;

  }

  return uVar2;

}
