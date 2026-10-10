/**
 * SNOR_Write @ 0x0304e968
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 SNOR_Write(undefined4 param_1,undefined4 *param_2,undefined4 *param_3)



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

  

  piVar5 = DAT_0304ebf4;

  piVar10 = DAT_0304ebec;

  puVar3 = DAT_0304ebd4;

  piVar2 = DAT_0304ebc8;

  switch(param_1) {

  case 3:

    uVar7 = DAT_0304ebd4[2];

    goto LAB_0304eb80;

  case 4:

    *param_2 = DAT_0304ebd4[4];

    return 1;

  case 5:

    *param_2 = DAT_0304ebd4[5];

    return 1;

  case 6:

    uVar7 = DAT_0304ebd4[1];

LAB_0304eb80:

    *param_2 = uVar7;

    break;

  case 7:

    *param_2 = *DAT_0304ebd4;

    return 1;

  case 8:

    *DAT_0304ebc8 = 0;

    FUN_03032fc0(1,0);

    FUN_03032f94(0,0,1);

    iVar8 = *piVar2;

    iVar9 = DAT_0304ebcc;

    while (iVar8 == 0) {

      SNOR_Erase();

      FUN_03017f10();

      MusicService_Init(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar2;

    }

    piVar10 = DAT_0304ebd0;

    *piVar2 = 0;

    if (*piVar10 != 0) {

      return 0;

    }

    break;

  case 9:

  case 0xb:

  case 0xd:

  case 0x10:

    return 0;

  case 10:

    if (DAT_0304ebd4[9] == 1) {

      if (DAT_0304ebd4[0xb] == 1) {

        return 0;

      }

      *param_3 = DAT_0304ebd4[8];

      puVar3[9] = 0;

      puVar3[10] = 1;

      FUN_03032fc0(4,0,1);

      FUN_03032f94(0,0,1);

      pcVar4 = DAT_0304ebd8;

      psVar1 = DAT_0304ebc4;

      if (*DAT_0304ebd8 != '\0') {

        local_30 = DAT_0304ebdc[4];

        uStack_2c = DAT_0304ebdc[5];

        local_28 = DAT_0304ebdc[6];

        uStack_24 = DAT_0304ebdc[7];

        *DAT_0304ebc4 = 0;

        uVar6 = FUN_030250fc();

        FUN_03025128(uVar6,puVar3[7],DAT_0304ebe4,(uint)(puVar3[3] * puVar3[8]) >> 4,&local_30,

                     DAT_0304ebe0);

        do {

          FUN_0301acd4();

        } while (*psVar1 != 1);

        *param_2 = DAT_0304ebe4;

        *pcVar4 = '\0';

        return 1;

      }

      local_30 = *DAT_0304ebdc;

      uStack_2c = DAT_0304ebdc[1];

      local_28 = DAT_0304ebdc[2];

      uStack_24 = DAT_0304ebdc[3];

      *DAT_0304ebc4 = 0;

      uVar6 = FUN_030250fc();

      FUN_03025128(uVar6,puVar3[7],DAT_0304ebe8,(uint)(puVar3[3] * puVar3[8]) >> 4,&local_30,

                   DAT_0304ebe0);

      do {

        FUN_0301acd4();

      } while (*psVar1 != 1);

      *param_2 = DAT_0304ebe8;

      *pcVar4 = '\x01';

      return 1;

    }

    if (DAT_0304ebd4[10] == 0) {

      DAT_0304ebd4[10] = 1;

      FUN_03032fc0(4,0,1);

      FUN_03032f94(0,0,1);

      return 2;

    }

    if (DAT_0304ebd4[10] != 1) {

      return 0;

    }

    return 2;

  case 0xc:

    FUN_03032fc0(4,0,1);

    FUN_03032f94(0,0,1);

    break;

  case 0xe:

    *DAT_0304ebec = 0;

    FUN_03032fc0(0xb,0,1);

    FUN_03032f94(param_2,0,1);

    iVar8 = *piVar10;

    iVar9 = DAT_0304ebf0;

    while (iVar8 == 0) {

      SNOR_Erase();

      FUN_03017f10();

      MusicService_Init(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar10;

    }

    goto LAB_0304eb36;

  case 0xf:

    *DAT_0304ebf4 = 0;

    FUN_03032fc0(0xd,0,1);

    FUN_03032f94(0,0,1);

    iVar8 = *piVar5;

    iVar9 = DAT_0304ebf8;

    while (piVar10 = piVar5, iVar8 == 0) {

      SNOR_Erase();

      FUN_03017f10();

      MusicService_Init(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar5;

    }

LAB_0304eb36:

    *piVar10 = 0;

    break;

  case 0x11:

    *param_2 = DAT_0304ebd4[3];

    break;

  default:

    return 0;

  }

  return 1;

}
