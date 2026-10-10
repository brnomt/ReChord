/**
 * media_lib_op_cmd04 @ 0x030295fc
 * Introduced in v3.5.0 (changelog: album sorting / Favorites)
 * Named via cross-version lineage; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4

media_lib_op_cmd04(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,

                  undefined4 param_5,undefined4 param_6,undefined4 param_7)



{

  undefined4 uVar1;

  undefined4 uStack_28;

  undefined4 uStack_24;

  undefined4 uStack_20;

  undefined4 uStack_1c;

  

  uStack_28 = param_1;

  uStack_24 = param_2;

  uStack_20 = param_3;

  uStack_1c = param_4;

  uVar1 = media_lib_op_dispatch_core(param_6,param_7,1,param_5,DAT_03029920,param_4,&uStack_28);

  return uVar1;

}
