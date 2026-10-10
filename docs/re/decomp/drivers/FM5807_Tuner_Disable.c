/**
 * FM5807_Tuner_Disable @ 0x0306b2aa
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void FM5807_Tuner_Disable(int param_1)



{

  int iVar1;

  int iVar2;

  ushort uVar3;

  uint uVar4;

  undefined4 local_10;

  

  local_10 = *DAT_0306b58c;

  FM5807_Tuner_SendCommand(&local_10,2);

  FUN_0303da28(10);

  iVar2 = DAT_0306b590;

  iVar1 = DAT_0306b584;

  uVar4 = 0;

  do {

    *(undefined2 *)(iVar1 + uVar4 * 2) = *(undefined2 *)(iVar2 + uVar4 * 2);

    uVar4 = uVar4 + 1 & 0xff;

  } while (uVar4 < 0x3c);

  if (param_1 == 2) {

    uVar3 = *(ushort *)(iVar1 + 6) | 8;

  }

  else {

    uVar3 = *(ushort *)(iVar1 + 6) & 0xfff7;

  }

  *(ushort *)(iVar1 + 6) = uVar3 & 0xfffd;

  return;

}
