/**
 * gui_text_display_helper @ 0x0301f5f8
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint gui_text_display_helper(undefined4 param_1,uint param_2)



{

  uint uVar1;

  

  uVar1 = FUN_0301e89a();

  if ((int)uVar1 >> (param_2 - 1 & 0xff) != 0) {

    return uVar1;

  }

  return -(uVar1 ^ (1 << (param_2 & 0xff)) - 1U);

}
