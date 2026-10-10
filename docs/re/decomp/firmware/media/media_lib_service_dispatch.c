/**
 * media_lib_service_dispatch @ 0x03029730
 * Introduced in v3.5.0 (changelog: album sorting / Favorites)
 * Named via cross-version lineage; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4

media_lib_service_dispatch

          (undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,int param_5,

          undefined4 param_6,undefined4 param_7)



{

  short sVar1;

  undefined4 uVar2;

  undefined4 unaff_r5;

  

  if (*(int *)(param_5 + 0x1e) == 0) {

    sVar1 = *(short *)(param_5 + 0x1c);

    if (sVar1 == 0x10) {

      unaff_r5 = media_lib_op_cmd10(param_1,param_2,param_3,param_4,param_5,param_6,param_7);

    }

    else {

      if (sVar1 < 0x11) {

        if (sVar1 == 1) {

          uVar2 = media_lib_op_cmd01(param_1,param_2,param_3,param_4,param_5,param_6,param_7);

          return uVar2;

        }

        if (sVar1 == 4) {

          uVar2 = media_lib_op_cmd04(param_1,param_2,param_3,param_4,param_5,param_6,param_7);

          return uVar2;

        }

        if (sVar1 == 8) {

          uVar2 = media_lib_op_cmd08(param_1,param_2,param_3,param_4,param_5,param_6,param_7);

          return uVar2;

        }

      }

      else {

        if (sVar1 == 0x18) {

          uVar2 = media_lib_op_cmd18(param_1,param_2,param_3,param_4,param_5,param_6,param_7);

          return uVar2;

        }

        if (sVar1 == 0x20) {

          uVar2 = media_lib_op_cmd20(param_1,param_2,param_3,param_4,param_5,param_6,param_7);

          return uVar2;

        }

      }

      unaff_r5 = 0;

    }

  }

  else if (*(int *)(param_5 + 0x1e) == 3) {

    if (*(short *)(param_5 + 0x1c) == 0x10) {

      unaff_r5 = media_lib_op_cmd40(param_1,param_2,param_3,param_4,param_5,param_6,param_7);

    }

    else if (*(short *)(param_5 + 0x1c) == 0x20) {

      unaff_r5 = media_lib_op_cmd80(param_1,param_2,param_3,param_4,param_5,param_6,param_7);

    }

  }

  else {

    unaff_r5 = 0;

  }

  return unaff_r5;

}
