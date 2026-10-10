/**
 * SNOR_Erase @ 0x030264dc
 * SDK-matched (structural+combined, documented in MULTI-VERSION-PLAN §10).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 SNOR_Erase(int param_1,undefined4 param_2)



{

  int iVar1;

  int unaff_r4;

  int *unaff_r5;

  

  *(undefined4 *)(unaff_r4 + 0x24) = param_2;

  if (param_1 * 0x10 == -1) {

    return 0;

  }

  iVar1 = mbedtls_dhm_self_test(DAT_03026560);

  if (iVar1 == 0) {

    FUN_03021cc4(unaff_r4 + 0xc,unaff_r4 + 0x10,*(undefined4 *)(unaff_r4 + 0x1c),

                 *(undefined4 *)(unaff_r4 + 0x20));

    if ((*(int *)(unaff_r4 + 0xc) < 0x141) && (iVar1 = FUN_030267fc(), iVar1 == 0)) {

      iVar1 = *(int *)(unaff_r4 + 0x10) - *(int *)(unaff_r4 + 0x18) >> 1;

      *(int *)(unaff_r4 + 0x2c) = iVar1;

      *unaff_r5 = iVar1;

      return 1;

    }

  }

  return 0;

}
