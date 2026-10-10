/**
 * wifi_ap6212_helper @ 0x03017cca
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void wifi_ap6212_helper(int param_1)



{

  ushort uVar1;

  ushort uVar2;

  byte *pbVar3;

  byte *pbVar4;

  uint uVar5;

  byte bVar6;

  ushort uVar7;

  undefined4 uVar8;

  bool bVar9;

  undefined4 local_30;

  undefined4 uStack_2c;

  undefined4 local_28;

  undefined4 uStack_24;

  undefined4 local_20;

  undefined4 uStack_1c;

  undefined4 local_18;

  undefined4 local_14;

  undefined4 uStack_10;

  

  pbVar4 = DAT_03017f80;

  pbVar3 = DAT_03017f74;

  local_18 = DAT_03017f84;

  local_14 = DAT_03017f88;

  uStack_10 = DAT_03017f8c;

  uVar8 = *(undefined4 *)(DAT_03017f90 + 0x30);

  local_30 = *(undefined4 *)(DAT_03017f90 + 0x28);

  uStack_2c = *(undefined4 *)(DAT_03017f90 + 0x2c);

  local_28 = uVar8;

  uStack_24 = *(undefined4 *)(DAT_03017f90 + 0x34);

  local_20 = *(undefined4 *)(DAT_03017f90 + 0x38);

  uStack_1c = *(undefined4 *)(DAT_03017f90 + 0x3c);

  uVar5 = 0;

  do {

    if (uVar5 + 1 == (uint)DAT_03017f74[1]) {

      if (uVar5 == 2) {

        uVar1 = *(ushort *)(DAT_03017f80 + 2);

        bVar6 = *(byte *)((int)&uStack_1c + uVar1 + 3);

        *DAT_03017f74 = bVar6;

        if ((*pbVar4 & 3) == 0 && uVar1 == 2) {

          bVar6 = bVar6 + 1;

          *pbVar3 = bVar6;

        }

        local_28 = CONCAT22((short)((uint)uVar8 >> 0x10),(ushort)bVar6);

      }

      uVar2 = *(ushort *)(pbVar4 + uVar5 * 2);

      uVar7 = *(ushort *)(&local_30 + uVar5);

      uVar1 = *(ushort *)((int)&local_30 + uVar5 * 4 + 2);

      if (param_1 == 0) {

        bVar9 = uVar2 < uVar7;

        uVar7 = uVar1;

        if (bVar9) {

          uVar7 = uVar2 + 1;

        }

      }

      else if (uVar1 < uVar2) {

        uVar7 = uVar2 - 1;

      }

      *(ushort *)(pbVar4 + uVar5 * 2) = uVar7;

      return;

    }

    uVar5 = uVar5 + 1 & 0xff;

  } while (uVar5 < 6);

  return;

}
