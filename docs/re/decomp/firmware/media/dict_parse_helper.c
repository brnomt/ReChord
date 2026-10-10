/**
 * dict_parse_helper @ 0x0304db74
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 dict_parse_helper(undefined4 param_1,undefined4 *param_2,undefined4 *param_3)



{

  short *psVar1;

  int *piVar2;

  undefined4 *puVar3;

  char *pcVar4;

  int *piVar5;

  undefined1 uVar6;

  undefined4 uVar7;

  int iVar8;

  int iVar9;

  int *piVar10;

  undefined4 local_30;

  undefined4 uStack_2c;

  undefined4 local_28;

  undefined4 uStack_24;

  

  piVar5 = DAT_0304de0c;

  piVar10 = DAT_0304de04;

  puVar3 = DAT_0304ddec;

  piVar2 = DAT_0304dde0;

  switch(param_1) {

  case 3:

    *param_2 = DAT_0304ddec[2];

    return 1;

  case 4:

    *param_2 = DAT_0304ddec[4];

    return 1;

  case 5:

    *param_2 = DAT_0304ddec[5];

    return 1;

  case 6:

    *param_2 = DAT_0304ddec[1];

    return 1;

  case 7:

    uVar7 = *DAT_0304ddec;

    goto LAB_0304dd52;

  case 8:

    *DAT_0304dde0 = 0;

    FUN_030321cc(1,0);

    FUN_030321a0(0,0,1);

    iVar8 = *piVar2;

    iVar9 = DAT_0304dde4;

    while (iVar8 == 0) {

      func_0x030256e8();

      FUN_0301711c();

      FUN_030295ec(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar2;

    }

    piVar10 = DAT_0304dde8;

    *piVar2 = 0;

    if (*piVar10 == 0) {

      return 1;

    }

    return 0;

  case 9:

  case 0xb:

  case 0xd:

  case 0x10:

    break;

  case 10:

    if (DAT_0304ddec[9] == 1) {

      if (DAT_0304ddec[0xb] != 1) {

        *param_3 = DAT_0304ddec[8];

        puVar3[9] = 0;

        puVar3[10] = 1;

        FUN_030321cc(4,0,1);

        FUN_030321a0(0,0,1);

        pcVar4 = DAT_0304ddf0;

        psVar1 = DAT_0304dddc;

        if (*DAT_0304ddf0 != '\0') {

          local_30 = DAT_0304ddf4[4];

          uStack_2c = DAT_0304ddf4[5];

          local_28 = DAT_0304ddf4[6];

          uStack_24 = DAT_0304ddf4[7];

          *DAT_0304dddc = 0;

          uVar6 = FUN_03024308();

          LineInTask_PlayerOpen

                    (uVar6,puVar3[7],DAT_0304ddfc,(uint)(puVar3[3] * puVar3[8]) >> 4,&local_30,

                     DAT_0304ddf8);

          do {

            FUN_03019ee0();

          } while (*psVar1 != 1);

          *param_2 = DAT_0304ddfc;

          *pcVar4 = '\0';

          return 1;

        }

        local_30 = *DAT_0304ddf4;

        uStack_2c = DAT_0304ddf4[1];

        local_28 = DAT_0304ddf4[2];

        uStack_24 = DAT_0304ddf4[3];

        *DAT_0304dddc = 0;

        uVar6 = FUN_03024308();

        LineInTask_PlayerOpen

                  (uVar6,puVar3[7],DAT_0304de00,(uint)(puVar3[3] * puVar3[8]) >> 4,&local_30,

                   DAT_0304ddf8);

        do {

          FUN_03019ee0();

        } while (*psVar1 != 1);

        *param_2 = DAT_0304de00;

        *pcVar4 = '\x01';

        return 1;

      }

    }

    else {

      if (DAT_0304ddec[10] == 0) {

        DAT_0304ddec[10] = 1;

        FUN_030321cc(4,0,1);

        FUN_030321a0(0,0,1);

        return 2;

      }

      if (DAT_0304ddec[10] == 1) {

        return 2;

      }

    }

    break;

  case 0xc:

    FUN_030321cc(4,0,1);

    FUN_030321a0(0,0,1);

    return 1;

  case 0xe:

    *DAT_0304de04 = 0;

    FUN_030321cc(0xb,0,1);

    FUN_030321a0(param_2,0,1);

    iVar8 = *piVar10;

    iVar9 = DAT_0304de08;

    while (iVar8 == 0) {

      func_0x030256e8();

      FUN_0301711c();

      FUN_030295ec(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar10;

    }

    goto LAB_0304ddce;

  case 0xf:

    *DAT_0304de0c = 0;

    FUN_030321cc(0xd,0,1);

    FUN_030321a0(0,0,1);

    iVar8 = *piVar5;

    iVar9 = DAT_0304de10;

    while (piVar10 = piVar5, iVar8 == 0) {

      func_0x030256e8();

      FUN_0301711c();

      FUN_030295ec(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar5;

    }

LAB_0304ddce:

    *piVar10 = 0;

    return 1;

  case 0x11:

    uVar7 = DAT_0304ddec[3];

LAB_0304dd52:

    *param_2 = uVar7;

    return 1;

  }

  return 0;

}
