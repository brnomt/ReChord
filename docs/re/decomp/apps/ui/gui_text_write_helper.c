/**
 * gui_text_write_helper @ 0x0301f860
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void gui_text_write_helper(undefined4 param_1,int param_2,int param_3,int param_4)



{

  undefined2 uVar1;

  uint uVar2;

  int iVar3;

  int local_20;

  int aiStack_1c [2];

  

  iVar3 = DAT_0301f8b8 + param_3 * 0x40;

  do {

    uVar1 = SDSwitchSpeedMode(param_1,&local_20,aiStack_1c);

    uVar2 = param_4 + local_20;

    if ((uVar2 & 0xffffffc0) != 0) {

      return;

    }

    param_4 = uVar2 + 1;

    *(undefined2 *)(param_2 + (uint)*(byte *)(iVar3 + uVar2) * 2) = uVar1;

  } while (aiStack_1c[0] == 0);

  return;

}
