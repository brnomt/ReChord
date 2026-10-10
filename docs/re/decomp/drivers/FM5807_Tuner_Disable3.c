/**
 * FM5807_Tuner_Disable3 @ 0x0306b43a
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void FM5807_Tuner_Disable3(void)



{

  int iVar1;

  undefined2 uVar2;

  uint uVar3;

  undefined1 local_60 [92];

  

  FM5807_Tuner_Disable();

  uVar3 = 0;

  do {

    uVar2 = *(undefined2 *)(DAT_0306b584 + uVar3 * 2 + 4);

    local_60[uVar3 * 2] = (char)((ushort)uVar2 >> 8);

    iVar1 = uVar3 * 2;

    uVar3 = uVar3 + 1 & 0xff;

    local_60[iVar1 + 1] = (char)uVar2;

  } while (uVar3 < 0x18);

  FM5807_Tuner_SendCommand(local_60,0x30);

  return;

}
