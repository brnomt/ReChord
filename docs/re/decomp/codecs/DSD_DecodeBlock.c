/**
 * DSD_DecodeBlock @ 0x030ffa3c
 * Tags: codec, dsd
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint DSD_DecodeBlock(uint *param_1,int param_2,uint param_3,int param_4,uint param_5)



{

  int iVar1;

  int iVar2;

  int iVar3;

  int iVar4;

  uint uVar5;

  uint uVar6;

  uint uVar7;

  uint uVar8;

  uint uVar9;

  uint uVar10;

  int iVar11;

  uint *puVar12;

  uint *puVar13;

  uint uVar14;

  int iVar15;

  uint uVar16;

  bool bVar17;

  int local_4c;

  

  iVar4 = DAT_030ffca8;

  uVar5 = *param_1;

  uVar9 = param_1[3];

  local_4c = 0;

  if (0 < (int)uVar5) {

    uVar6 = *DAT_030ffca4;

    do {

      uVar8 = param_1[local_4c * 0x11 + 0x15];

      uVar14 = 0;

      iVar3 = local_4c * 0x44 + 0x14;

      if (param_3 / uVar5 != 0) {

        puVar12 = (uint *)(local_4c * (param_5 >> 3) + param_4);

        do {

          uVar8 = uVar8 + 1 & 0x3f;

          *(undefined1 *)((int)param_1 + uVar8 + iVar3) =

               *(undefined1 *)(param_2 + (local_4c * param_3) / uVar5 + uVar14);

          puVar13 = puVar12;

          if (uVar14 == param_1[3] * ((int)uVar14 / (int)param_1[3])) {

            uVar7 = 0;

            if (0 < (int)uVar6) {

              bVar17 = (uVar6 & 1) != 0;

              if (bVar17) {

                uVar7 = *(uint *)(iVar4 + (uint)*(byte *)((int)param_1 + uVar8 + iVar3) * 0xf0);

              }

              uVar10 = (uint)bVar17;

              iVar15 = 0;

              for (iVar11 = (int)uVar6 >> 1; iVar11 != 0; iVar11 = iVar11 + -1) {

                uVar16 = uVar8 + uVar10;

                iVar2 = uVar10 * 4;

                iVar1 = uVar10 * 4;

                uVar10 = uVar10 + 2;

                uVar7 = uVar7 + *(int *)(iVar4 + (uint)*(byte *)((int)param_1 +

                                                                (uVar16 & 0x3f) + iVar3) * 0xf0 +

                                        iVar1);

                iVar15 = iVar15 + *(int *)(iVar4 + (uint)*(byte *)((int)param_1 +

                                                                  (uVar16 + 1 & 0x3f) + iVar3) *

                                                   0xf0 + iVar2 + 4);

              }

              uVar7 = uVar7 + iVar15;

            }

            iVar15 = (int)uVar7 >> 0x1f;

            if ((int)((-1 - iVar15) - (uint)(0x80000000 < uVar7)) < 0 ==

                (SBORROW4(-1,iVar15) != SBORROW4(-1 - iVar15,(uint)(0x80000000 < uVar7)))) {

              uVar7 = 0x80000000;

            }

            puVar13 = puVar12 + 2;

            *puVar12 = uVar7;

          }

          uVar14 = uVar14 + 1;

          param_1[local_4c * 0x11 + 0x15] = uVar8;

          puVar12 = puVar13;

        } while (uVar14 < param_3 / uVar5);

      }

      local_4c = local_4c + 1;

    } while (local_4c < (int)uVar5);

  }

  return param_3 / uVar9;

}
