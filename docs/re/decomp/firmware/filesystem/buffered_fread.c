/**
 * buffered_fread @ 0x030ae6b0
 * Tags: fs, buffer
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int buffered_fread(int param_1,int param_2,int param_3,int param_4)



{

  undefined4 *puVar1;

  short sVar2;

  int iVar3;

  int iVar4;

  

  puVar1 = DAT_030ae954;

  iVar4 = param_2 * param_3;

  if (param_1 == 0) {

    return -1;

  }

  while( true ) {

    iVar3 = (uint)*(ushort *)(param_4 + 0x202) - (uint)*(ushort *)(param_4 + 0x200);

    if (iVar4 <= iVar3) {

      iVar3 = iVar4;

    }

    FUN_030ac8a8(param_1,(uint)*(ushort *)(param_4 + 0x200) + param_4,iVar3);

    param_1 = param_1 + iVar3;

    *(short *)(param_4 + 0x200) = *(short *)(param_4 + 0x200) + (short)iVar3;

    iVar4 = iVar4 - iVar3;

    if (iVar4 == 0) break;

    sVar2 = (*(code *)*puVar1)(param_4,0x200,*(undefined4 *)(param_4 + 0x204));

    if (sVar2 == 0) break;

    *(undefined2 *)(param_4 + 0x200) = 0;

    *(short *)(param_4 + 0x202) = sVar2;

  }

  return param_3 * param_2 - iVar4;

}
