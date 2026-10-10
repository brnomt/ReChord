/**
 * FileDev_FileSeek @ 0x0304fed0
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



undefined4 FileDev_FileSeek(short *param_1,int param_2,int param_3,short *param_4)



{

  short sVar1;

  ushort uVar2;

  int iVar3;

  uint uVar4;

  int iVar5;

  int iVar6;

  int local_58;

  uint local_54;

  undefined1 local_50 [4];

  undefined4 local_4c;

  uint local_48;

  int local_44;

  int *local_40 [3];

  short *psStack_34;

  int iStack_30;

  int local_2c;

  short *psStack_28;

  

  psStack_28 = param_4;

  local_2c = param_3;

  iStack_30 = param_2;

  psStack_34 = param_1;

  if ((param_1 == (short *)0x0 || param_2 == -1) || param_4 == (short *)0x0) {

    return 0;

  }

  if (*param_4 < 1) {

    return 0;

  }

  if (*(int *)(param_4 + 2) == 0) {

    return 0;

  }

  local_58 = param_3 + (uint)(ushort)param_1[8];

  iVar5 = *(int *)(param_1 + 4) * (uint)(ushort)param_1[8] +

          (uint)(ushort)param_1[7] * (uint)(ushort)param_1[6] + (uint)(ushort)param_1[0xb];

  if (iVar5 < local_58) {

    return 0;

  }

  if (local_58 == iVar5) {

    local_58 = *(int *)(param_1 + 0x12);

    local_54 = 1;

  }

  else {

    local_54 = 2;

  }

  local_50[0] = 6;

  local_4c = 1;

  local_48 = (uint)*(byte *)(param_1 + 2);

  local_40[0] = &local_2c;

  iVar5 = 0;

  local_40[1] = &local_58;

  if (local_54 != 0) {

    do {

      iVar6 = *local_40[iVar5];

      iVar3 = FatDev_FileSeek(param_2,0,iVar6);

      if (iVar3 != 0) {

        return 0;

      }

      local_44 = iVar6;

      iVar3 = FUN_0304f550(param_2,local_50);

      if (iVar3 == 0) {

        return 0;

      }

      *local_40[iVar5] = 0;

      uVar4 = dict_stream_read(param_2,param_1[8]);

      if (uVar4 != (ushort)param_1[8]) {

        return 0;

      }

      iVar5 = iVar5 + 1;

    } while (iVar5 < (int)local_54);

  }

  local_50[0] = 3;

  local_4c = 3;

  local_48 = (uint)*(byte *)(param_1 + 2) << 8 | 1;

  local_44 = *local_40[0];

  iVar5 = FUN_0304f550(param_2,local_50);

  if (iVar5 == 0) {

    return 0;

  }

  sVar1 = *param_1;

  if (sVar1 == 0x70) {

    if (param_1[1] == 0x42) {

      iVar5 = FatDev_FileSeek(param_2,0,*local_40[0] + (uint)(ushort)param_1[0x10]);

      if (iVar5 != 0) {

        return 0;

      }

      local_54 = 0;

      iVar5 = dict_stream_read(param_2,4,&local_54);

      if (iVar5 == 4) {

        return 0;

      }

      goto LAB_0305001a;

    }

LAB_0304ffdc:

    if (param_1[1] != 0x62) {

      return 0;

    }

    uVar2 = param_1[9];

  }

  else {

    if (sVar1 != 0xf1) {

      if (sVar1 != 0x70 && sVar1 != 0x71) {

        return 0;

      }

      goto LAB_0304ffdc;

    }

    if (param_1[1] != 0x12) {

      return 0;

    }

    uVar2 = param_1[0x10];

  }

  local_54 = (uint)uVar2;

LAB_0305001a:

  iVar5 = FatDev_FileSeek(param_2,0,*local_40[0] + local_54);

  if (iVar5 == 0) {

    local_54 = (*local_40[1] - *local_40[0]) - local_54;

    if ((int)*param_4 <= (int)local_54) {

      local_54 = (int)*param_4 - 1;

    }

    uVar4 = dict_stream_read(param_2,local_54,*(undefined4 *)(param_4 + 2));

    if (uVar4 == local_54) {

      *(undefined1 *)(*(int *)(param_4 + 2) + local_54) = 0;

      return 1;

    }

  }

  return 0;

}
