/**
 * bb_debug_ok_helper @ 0x0306e97c
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 bb_debug_ok_helper(undefined4 *param_1,undefined4 *param_2)



{

  undefined4 uVar1;

  

  uVar1 = *DAT_0306ed10;

  *(undefined4 *)(DAT_0306ed14 + 0x38) = uVar1;

  *param_1 = *DAT_0306ed18;

  *param_2 = uVar1;

  return 1;

}
