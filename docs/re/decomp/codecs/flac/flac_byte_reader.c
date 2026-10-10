/**
 * flac_byte_reader @ 0x030df344
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int flac_byte_reader(byte *param_1,int param_2)



{

  byte bVar1;

  undefined4 *puVar2;

  undefined4 *puVar3;

  undefined4 *puVar4;

  byte *pbVar5;

  byte *pbVar6;

  uint uVar7;

  int iVar8;

  

  puVar4 = DAT_030df5ac;

  puVar3 = DAT_030df5a8;

  puVar2 = DAT_030df5a4;

  pbVar5 = param_1 + 4;

  iVar8 = 4;

  do {

    bVar1 = *pbVar5;

    uVar7 = (uint)pbVar5[3] | (uint)pbVar5[1] << 0x10 | (uint)pbVar5[2] << 8;

    pbVar6 = pbVar5 + 4;

    iVar8 = iVar8 + 4 + uVar7;

    pbVar5 = pbVar6 + uVar7;

    if (param_1 + param_2 < pbVar5) {

      (*(code *)*puVar3)(uVar7 - ((int)(param_1 + param_2) - (int)pbVar6),1,*puVar2);

      (*(code *)*puVar4)(param_1,param_2,*puVar2);

      pbVar5 = param_1;

    }

  } while ((bVar1 & 0x80) == 0);

  return iVar8;

}
