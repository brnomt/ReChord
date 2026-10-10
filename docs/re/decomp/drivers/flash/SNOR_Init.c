/**
 * SNOR_Init @ 0x0308eaf2
 * SDK-matched (structural+combined, documented in MULTI-VERSION-PLAN §10).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int SNOR_Init(undefined1 *param_1,int param_2,int param_3,int param_4,int param_5,int param_6,

             int *param_7)



{

  undefined1 *puVar1;

  int iVar2;

  undefined1 uStack_38;

  undefined1 auStack_37 [19];

  

  if ((param_2 != 0 && param_3 != 0) && (param_4 != 0 && param_6 != 0)) {

    puVar1 = param_1;

    if (param_7 != (int *)0x0) {

      puVar1 = (undefined1 *)(uint)*(byte *)(param_2 + 8);

    }

    if (((param_7 != (int *)0x0 && puVar1 != (undefined1 *)0x0) &&

        (iVar2 = FUN_0308282e(param_2 + 8), iVar2 + 1U < 0x1a)) &&

       ((param_5 != 0 || (*param_7 == 0)))) {

      iVar2 = PmuSuspend(param_1 + 0x314,param_1,param_3,param_4,param_2 + 8,param_2,param_5,param_6

                         ,param_7,&uStack_38);

      if (iVar2 == 0) {

        iVar2 = aes_setkey_enc(param_1 + 4,uStack_38,auStack_37);

        if (iVar2 == 0) {

          *param_1 = 1;

        }

        return iVar2;

      }

      return iVar2;

    }

  }

  return 1;

}
