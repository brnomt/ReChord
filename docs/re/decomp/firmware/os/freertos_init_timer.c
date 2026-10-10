/**
 * freertos_init_timer @ 0x0302905e
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4

freertos_init_timer(undefined4 *param_1,int param_2,int param_3,uint param_4,uint param_5)



{

  int iVar1;

  undefined4 uVar2;

  uint uVar3;

  undefined4 uVar4;

  undefined4 uVar5;

  char cVar6;

  undefined8 uVar7;

  undefined8 uVar8;

  undefined4 *puVar9;

  

  iVar1 = *(int *)(param_3 + 0x16);

  if (iVar1 < 0) {

    iVar1 = -iVar1;

  }

  param_1[1] = iVar1;

  puVar9 = param_1;

  uVar2 = FUN_02ffb850((int)*(short *)(param_3 + 0x1c));

  uVar2 = FUN_02ffb558(uVar2,0x41000000);

  param_1[2] = uVar2;

  uVar2 = FUN_02ffb850(*(undefined4 *)(param_3 + 0x12));

  FUN_02ffb4ec(uVar2,param_1[2]);

  uVar7 = FUN_02ffb936();

  FUN_02ffb5f0((int)uVar7,(int)((ulonglong)uVar7 >> 0x20),DAT_03029484,DAT_03029488,puVar9,uVar2,

               uVar7);

  uVar3 = FUN_02ffb8f8();

  param_1[3] = uVar3 & 0xfffffffc;

  *param_1 = DAT_0302948c;

  freertos_create_task(*param_1,param_1[3]);

  cVar6 = param_4 <= *(uint *)(param_3 + 0x12);

  if (((int)*(uint *)(param_3 + 0x12) < (int)param_4) &&

     (cVar6 = param_5 <= (uint)param_1[1], (int)param_1[1] < (int)param_5)) {

    param_1[6] = *(undefined4 *)(param_3 + 0x12);

    param_1[7] = param_1[1];

  }

  else {

    uVar2 = FUN_02ffb850(param_1[1]);

    uVar4 = FUN_02ffb850(param_5);

    uVar2 = FUN_02ffb558(uVar4,uVar2);

    uVar4 = FUN_02ffb850(*(undefined4 *)(param_3 + 0x12));

    uVar5 = FUN_02ffb850(param_4);

    uVar4 = FUN_02ffb558(uVar5,uVar4);

    FUN_02ffb9a8(uVar4,uVar2);

    if (cVar6 == '\0') {

      uVar2 = FUN_02ffb850(param_1[1]);

      uVar4 = FUN_02ffb850(param_5);

      FUN_02ffb558(uVar4,uVar2);

    }

    else {

      uVar2 = FUN_02ffb850(*(undefined4 *)(param_3 + 0x12));

      uVar4 = FUN_02ffb850(param_4);

      FUN_02ffb558(uVar4,uVar2);

    }

    uVar7 = FUN_02ffb936();

    *(undefined8 *)(param_1 + 10) = uVar7;

    *(undefined8 *)(param_1 + 0xc) = uVar7;

    uVar7 = FUN_02ffb862(*(undefined4 *)(param_3 + 0x12));

    uVar7 = FUN_02ffb73e((int)uVar7,(int)((ulonglong)uVar7 >> 0x20),param_1[10],param_1[0xb],uVar7);

    uVar2 = FUN_02ffb8f8();

    param_1[6] = uVar2;

    uVar8 = FUN_02ffb862(param_1[1]);

    FUN_02ffb73e((int)uVar8,(int)((ulonglong)uVar8 >> 0x20),param_1[0xc],param_1[0xd],uVar8,uVar7);

    uVar2 = FUN_02ffb8f8();

    param_1[7] = uVar2;

  }

  if (param_1[6] == 0) {

    param_1[6] = 1;

  }

  if (param_1[7] == 0) {

    param_1[7] = 1;

  }

  param_1[5] = param_4 << 2;

  *(undefined4 *)(param_2 + 4) = param_1[6];

  *(undefined4 *)(param_2 + 8) = param_1[7];

  param_1[4] = *(undefined4 *)(param_2 + 0xc);

  param_1[8] = (int)((uint)*(ushort *)(param_3 + 0x12) << 0x10) / (int)param_1[6] + 1;

  param_1[9] = (int)((uint)*(ushort *)(param_1 + 1) << 0x10) / (int)param_1[7] + 1;

  param_1[0xf] = 0;

  return 1;

}
