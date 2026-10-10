/**
 * ui_set_position @ 0x0302b252
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void ui_set_position(int param_1,uint param_2,uint param_3)



{

  int iVar1;

  

  if (param_2 <= param_3) {

    *(uint *)((param_1 >> 8) + 4) = param_2;

    iVar1 = DAT_0302b554 + *DAT_0302b550 * 0x27e;

    *(undefined4 *)(iVar1 + 0x20) = *(undefined4 *)(iVar1 + 0x10);

  }

  FUN_0302c950(*(undefined4 *)((param_1 >> 8) + 4),0);

  return;

}
