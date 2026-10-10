/**
 * record_write_helper @ 0x03071042
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void record_write_helper(int param_1,undefined4 *param_2,int param_3,int param_4)



{

  int *piVar1;

  int *piVar2;

  int iVar3;

  int *piVar4;

  

  piVar4 = (int *)(param_1 + -4);

  piVar1 = param_2 + -1;

  piVar2 = (int *)(param_3 + param_4 * 4 + -0x80);

  iVar3 = 9;

  do {

    iVar3 = iVar3 + -1;

    piVar2[0x20] = piVar4[1] + piVar1[1];

    piVar4 = piVar4 + 2;

    piVar1 = piVar1 + 2;

    piVar2 = piVar2 + 0x40;

    *piVar2 = *piVar4 + *piVar1;

  } while (iVar3 != 0);

  iVar3 = (int)param_2 - param_1 >> 2;

  if ((0x12 < iVar3) && (iVar3 < 0x24)) {

    *param_2 = *(undefined4 *)(param_1 + 0x48);

    param_2[1] = *(undefined4 *)(param_1 + 0x4c);

    param_2[2] = *(undefined4 *)(param_1 + 0x50);

    param_2[3] = *(undefined4 *)(param_1 + 0x54);

    param_2[4] = *(undefined4 *)(param_1 + 0x58);

    param_2[5] = *(undefined4 *)(param_1 + 0x5c);

    param_2[6] = *(undefined4 *)(param_1 + 0x60);

    param_2[7] = *(undefined4 *)(param_1 + 100);

    param_2[8] = *(undefined4 *)(param_1 + 0x68);

    param_2[9] = *(undefined4 *)(param_1 + 0x6c);

    param_2[10] = *(undefined4 *)(param_1 + 0x70);

    param_2[0xb] = *(undefined4 *)(param_1 + 0x74);

    param_2[0xc] = *(undefined4 *)(param_1 + 0x78);

    param_2[0xd] = *(undefined4 *)(param_1 + 0x7c);

    param_2[0xe] = *(undefined4 *)(param_1 + 0x80);

    param_2[0xf] = *(undefined4 *)(param_1 + 0x84);

    param_2[0x10] = *(undefined4 *)(param_1 + 0x88);

    param_2[0x11] = *(undefined4 *)(param_1 + 0x8c);

    return;

  }

  hifi_memmove(param_2,param_1 + 0x48,0x48);

  return;

}
