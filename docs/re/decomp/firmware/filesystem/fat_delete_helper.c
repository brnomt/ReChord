/**
 * fat_delete_helper @ 0x03084c3c
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 fat_delete_helper(uint *param_1)



{

  undefined1 uVar1;

  char cVar2;

  uint uVar3;

  int iVar4;

  uint uVar5;

  uint uVar6;

  uint uVar7;

  bool bVar8;

  undefined4 local_28;

  

  local_28 = 0;

LAB_03084c5c:

  switch((char)param_1[0x3c]) {

  case '\x03':

    *DAT_0308506c = *DAT_0308506c + 1;

    uVar5 = param_1[2];

    uVar3 = param_1[3];

    if (param_1[0xd] <= uVar3 && (uint)(uVar5 <= param_1[0xc]) <= param_1[0xd] - uVar3) {

      return 6;

    }

    *param_1 = uVar5;

    param_1[1] = uVar3;

    param_1[2] = uVar5 + param_1[5];

    param_1[3] = uVar3 + CARRY4(uVar5,param_1[5]);

    iVar4 = AudioControlTask_Enter(param_1);

    if (iVar4 == 3) {

      param_1[2] = *param_1;

      param_1[3] = param_1[1];

      return 0x12;

    }

    if (iVar4 != 0) {

      return 4;

    }

    uVar3 = 0;

    if (param_1[0x46] != 0) {

      uVar3 = param_1[0x44];

    }

    if (param_1[0x46] == 0 || uVar3 == 0) {

      *(undefined1 *)(param_1 + 0x3c) = 4;

      param_1[0x61] = 0;

    }

    goto LAB_03084c5c;

  case '\x04':

    if (param_1[0x55] <= param_1[0x61]) {

      *(undefined1 *)(param_1 + 0x3c) = 3;

      goto LAB_03084c5c;

    }

    iVar4 = FileShellTest(param_1);

    if (iVar4 == 0) {

      *(short *)(param_1 + 0x62) =

           ((short)param_1[0x56] + *(short *)((int)param_1 + 0x15a)) -

           *(short *)((int)param_1 + 0x16e);

      bVar8 = (char)param_1[0x5b] != '\x01';

      if (bVar8) {

        uVar1 = 5;

      }

      else {

        uVar1 = 8;

      }

      *(undefined1 *)(param_1 + 0x3c) = uVar1;

      if (bVar8) {

        *(undefined1 *)((int)param_1 + 0x175) = 0;

      }

      else {

        *(undefined1 *)((int)param_1 + 0x175) = 1;

      }

      goto LAB_03084c5c;

    }

    break;

  case '\x05':

    if ((ushort)(byte)param_1[0x57] == (ushort)param_1[0x38]) {

      param_1[0x42] = 1;

      uVar7 = *param_1 + (uint)(ushort)param_1[0x62];

      uVar5 = param_1[1] + (uint)CARRY4(*param_1,(uint)(ushort)param_1[0x62]);

      param_1[0x3e] = uVar7;

      param_1[0x3f] = uVar5;

      uVar6 = param_1[0x12];

      param_1[0x43] = uVar6;

      uVar3 = uVar6 - 0xd55;

      bVar8 = uVar3 == 0;

      if (bVar8) {

        uVar3 = param_1[5];

      }

      if ((bVar8 && uVar3 == 0xc80) &&

         (*(short *)((int)param_1 + 0x16e) == 0xc66 || *(short *)((int)param_1 + 0x16e) == 0xef)) {

        param_1[0x40] = 0;

        param_1[0x41] = 0;

      }

      else {

        param_1[0x40] = *(ushort *)((int)param_1 + 0x16e) - uVar6;

        param_1[0x41] = 0;

      }

      if (param_1[99] == 0) goto LAB_03084e54;

      uVar3 = *(ushort *)((int)param_1 + 0x16e) - 0xf;

      iVar4 = wma_input_cache_read

                        (param_1,uVar7,uVar7 + uVar3,

                         ((0xe < *(ushort *)((int)param_1 + 0x16e)) - 1) + uVar5 +

                         (uint)CARRY4(uVar7,uVar3),0xf,&local_28);

      if (iVar4 != 0xf) {

        return 6;

      }

      cVar2 = FUN_0308847c(param_1[0x6d],local_28,*(undefined2 *)((int)param_1 + 0x16e));

joined_r0x03084e46:

      if (cVar2 != '\0') {

        return 0xd;

      }

LAB_03084e54:

      *(undefined1 *)(param_1 + 0x3c) = 6;

      return 0;

    }

    break;

  default:

    return 0x11;

  case '\a':

    goto switchD_03084c68_caseD_7;

  case '\b':

    if ((ushort)(byte)param_1[0x57] == (ushort)param_1[0x38]) {

      param_1[0x42] = 1;

      uVar5 = *param_1 + (uint)(ushort)param_1[0x62];

      uVar3 = param_1[1] + (uint)CARRY4(*param_1,(uint)(ushort)param_1[0x62]);

      param_1[0x3e] = uVar5;

      param_1[0x3f] = uVar3;

      param_1[0x43] = param_1[0x12];

      *(undefined2 *)(param_1 + 0x5c) = 0;

      *(undefined1 *)(param_1 + 0x5d) = 1;

      if (param_1[99] == 0) goto LAB_03084e54;

      uVar7 = *(ushort *)((int)param_1 + 0x16e) - 0xf;

      iVar4 = wma_input_cache_read

                        (param_1,uVar5,uVar5 + uVar7,

                         ((0xe < *(ushort *)((int)param_1 + 0x16e)) - 1) + uVar3 +

                         (uint)CARRY4(uVar5,uVar7),0xf,&local_28);

      if (iVar4 != 0xf) {

        return 6;

      }

      cVar2 = FUN_0308847c(param_1[0x6d],local_28,*(undefined2 *)((int)param_1 + 0x16e));

      goto joined_r0x03084e46;

    }

  }

  *(undefined1 *)(param_1 + 0x3c) = 7;

  goto LAB_03084c5c;

switchD_03084c68_caseD_7:

  param_1[0x61] = param_1[0x61] + 1;

  *(undefined1 *)(param_1 + 0x3c) = 4;

  goto LAB_03084c5c;

}
