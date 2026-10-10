/**
 * buffered_fread_ovl_0e65 @ 0x030e6560
 * Tags: fs, buffer, overlay
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int buffered_fread_ovl_0e65(int param_1,int param_2,int param_3,int param_4)



{

  undefined4 *puVar1;

  short sVar2;

  int iVar3;

  int iVar4;

  

  puVar1 = DAT_030e680c;

  iVar3 = param_2 * param_3;

  while( true ) {

    iVar4 = (uint)*(ushort *)(param_4 + 0x202) - (uint)*(ushort *)(param_4 + 0x200);

    if (iVar3 <= iVar4) {

      iVar4 = iVar3;

    }

    FUN_030e41ce(param_1,(uint)*(ushort *)(param_4 + 0x200) + param_4,iVar4);

    param_1 = param_1 + iVar4;

    *(short *)(param_4 + 0x200) = *(short *)(param_4 + 0x200) + (short)iVar4;

    iVar3 = iVar3 - iVar4;

    if (iVar3 == 0) break;

    sVar2 = (*(code *)*puVar1)(param_4,0x200,*(undefined4 *)(param_4 + 0x204));

    if (sVar2 == 0) break;

    *(undefined2 *)(param_4 + 0x200) = 0;

    *(short *)(param_4 + 0x202) = sVar2;

  }

  return param_3 * param_2 - iVar3;

}
