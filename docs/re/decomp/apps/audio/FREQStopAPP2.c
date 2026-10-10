/**
 * FREQStopAPP2 @ 0x0308959c
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int FREQStopAPP2(int *param_1,int *param_2,undefined4 param_3,int param_4)



{

  char cVar1;

  short sVar2;

  int *piVar3;

  uint uVar4;

  undefined4 uVar5;

  int iVar6;

  int iVar7;

  int iVar8;

  short *psVar9;

  int iVar10;

  uint local_50 [5];

  int local_3c;

  int local_30;

  int local_2c [2];

  

  local_3c = *param_2;

  iVar8 = *param_1;

  psVar9 = (short *)(iVar8 + 0x6c);

  iVar6 = 0;

  sVar2 = wma_floor_log2(param_4 + -1);

  *(short *)(param_1 + 0x36) = sVar2 + 1;

  if (*psVar9 < param_4) {

    iVar10 = 0;

    do {

      local_30 = 0;

      piVar3 = param_1 + 0x21;

      iVar7 = *param_1;

      cVar1 = (char)param_1[9];

      iVar6 = 0;

      local_2c[0] = iVar10;

      if (cVar1 == '\0') {

LAB_03089614:

        iVar6 = FREQSetFreq(param_2[6],piVar3,local_50,&local_30,local_2c);

        if ((-1 < iVar6) && (iVar6 = wma_bitreader_getbits_copy(piVar3,local_50[0]), -1 < iVar6)) {

          if (local_30 == 0) {

            *(undefined1 *)(param_1 + 9) = 5;

            goto LAB_03089686;

          }

          if (local_30 == 1) {

            *(undefined4 *)(iVar7 + 0x18) = 0;

            *(int *)(iVar7 + 0x14) =

                 (int)(short)(((short)param_2[0xb] - *(short *)(iVar7 + 0x6c)) + -1);

          }

          else {

            iVar6 = wma_bitreader_getbits_copy(piVar3,1);

            if (-1 < iVar6) {

              uVar4 = (*(code *)param_2[9])(param_2[7],local_30 + -2);

              *(uint *)(iVar7 + 0x14) = uVar4 & 0x7fffffff;

              uVar5 = (*(code *)param_2[10])(param_2[8],local_30 + -2);

              *(undefined4 *)(iVar7 + 0x18) = uVar5;

              *(int *)(iVar7 + 0x1c) = -1 - (local_2c[0] >> 0x1f);

            }

          }

        }

LAB_030896e6:

        if (iVar6 < 0) {

          return iVar6;

        }

      }

      else {

        if (cVar1 == '\x04') {

          iVar6 = wma_bitreader_getbits(piVar3,1,local_50);

          if (-1 < iVar6) {

            FUN_03084338(param_1,param_2,(int)(short)local_50[0]);

            *(undefined1 *)(param_1 + 9) = 0;

            goto LAB_03089614;

          }

          goto LAB_030896e6;

        }

        if (cVar1 == '\x05') {

LAB_03089686:

          iVar6 = wma_bitreader_getbits(piVar3,*(undefined2 *)(iVar7 + 0x38),local_50);

          if (-1 < iVar6) {

            *(uint *)(iVar7 + 0x18) = local_50[0];

            *(undefined1 *)(param_1 + 9) = 6;

            goto LAB_030896a0;

          }

          goto LAB_030896e6;

        }

        if (cVar1 == '\x06') {

LAB_030896a0:

          iVar6 = wma_bitreader_getbits(piVar3,*(short *)(iVar7 + 0x94) + 1,local_50);

          if (-1 < iVar6) {

            sVar2 = *(short *)(*param_1 + 0x94);

            *(uint *)(iVar7 + 0x1c) = (local_50[0] & 1) - 1;

            *(uint *)(iVar7 + 0x14) =

                 (int)((0xffffffffU >> (0x20 - (ushort)(sVar2 + 1) & 0xff) & local_50[0]) << 0x10)

                 >> 0x11 & 0x7fffffff;

            *(undefined1 *)(param_1 + 9) = 0;

          }

          goto LAB_030896e6;

        }

      }

      *(uint *)(iVar8 + 0x18) =

           (*(uint *)(iVar8 + 0x18) ^ *(uint *)(iVar8 + 0x1c)) - *(uint *)(iVar8 + 0x1c);

      if (param_4 <= (int)*psVar9 + *(int *)(iVar8 + 0x14)) {

        return DAT_03089afc;

      }

      sVar2 = (short)*(int *)(iVar8 + 0x14) + *psVar9;

      *psVar9 = sVar2;

      *(undefined4 *)(local_3c + sVar2 * 4) = *(undefined4 *)(iVar8 + 0x18);

      *psVar9 = *psVar9 + 1;

      *(undefined1 *)(param_1 + 9) = 0;

    } while (*psVar9 < param_4);

  }

  return iVar6;

}
