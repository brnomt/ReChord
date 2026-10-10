/**
 * ape_scan_cuesheet @ 0x0301dcb8
 * Tags: codec, ape
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 ape_scan_cuesheet(undefined4 *param_1)



{

  int iVar1;

  int iVar2;

  uint uVar3;

  int iVar4;

  uint uVar5;

  undefined4 *puVar6;

  uint uVar7;

  uint uVar8;

  

  iVar2 = DAT_0301df68;

  param_1[3] = 0;

  iVar1 = DAT_0301df60;

  uVar7 = 0x400;

  if ((uint)param_1[5] < *(uint *)(iVar2 + 0xc)) {

    do {

      uVar3 = param_1[8];

      if (uVar3 < uVar7) {

        puVar6 = (undefined4 *)((int)param_1 + uVar3 + 0x24);

        param_1[8] = uVar3 + 0x10;

      }

      else {

        uVar7 = ape_rom_read_thunk(param_1 + 9,0x400,param_1[1]);

        if (uVar7 < 0x10) {

          return 2;

        }

        *(uint *)(iVar2 + 0x1c) = uVar7;

        puVar6 = param_1 + 9;

        param_1[8] = 0x10;

      }

      *(int *)(iVar2 + 0x10) = *(int *)(iVar2 + 0x10) + 1;

      if (*(short *)((int)puVar6 + 2) == 0x6364) {

        iVar4 = param_1[5];

        param_1[5] = iVar4 + 1U;

        if (*(uint *)(iVar2 + 0xc) < iVar4 + 1U) {

          return 2;

        }

        uVar8 = *(uint *)(iVar1 + 0xc);

        uVar3 = FUN_0301d680(*param_1);

        if (uVar3 < uVar8) {

          uVar3 = 0;

        }

        else {

          uVar3 = uVar3 - uVar8;

        }

        uVar8 = (uint)*(byte *)(puVar6 + 2) | *(int *)((int)puVar6 + 9) << 8;

        param_1[6] = uVar8;

        uVar5 = (uint)*(byte *)(puVar6 + 3) | *(int *)((int)puVar6 + 0xd) << 8;

        param_1[7] = uVar5;

        if (uVar3 <= uVar8) {

          param_1[3] = (uVar8 - uVar3) + 8;

          param_1[4] = uVar5;

          if (uVar5 == 0) {

            return 3;

          }

          if (*(char *)(puVar6 + 1) != '\0' || (*(uint *)((int)puVar6 + 5) & 0xffffff) != 0) {

            return 1;

          }

        }

      }

      else if (*(short *)((int)puVar6 + 2) != 0x6277) {

        return 0xffffffff;

      }

    } while (param_1[5] != *(int *)(iVar2 + 0xc));

  }

  return 2;

}
