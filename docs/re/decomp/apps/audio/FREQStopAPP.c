/**
 * FREQStopAPP @ 0x030898fe
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int FREQStopAPP(int *param_1)



{

  undefined4 *puVar1;

  int iVar2;

  short sVar3;

  int iVar4;

  int iVar5;

  int iVar6;

  short *psVar7;

  int *piVar8;

  int iVar9;

  int *piVar10;

  int iVar11;

  byte *pbVar12;

  char *pcVar13;

  uint uStack_34;

  int iStack_30;

  undefined4 auStack_2c [2];

  

  piVar8 = param_1 + 0xd;

  iVar6 = *param_1;

  psVar7 = (short *)((int)param_1 + 0x36);

  iVar2 = 0;

  if ((char)param_1[8] == '\x06') {

    if ((int)(short)*piVar8 < (int)(uint)*(ushort *)(iVar6 + 0x26)) {

      do {

        iVar5 = *(int *)(iVar6 + 0xd8) + (short)*piVar8 * 0xfc;

        iVar2 = wma_bitreader_getbits_copy_copy

                          (param_1 + 0x21,*(int *)(iVar6 + 200) - *(int *)(iVar6 + 0x124));

        if (iVar2 < 0) {

          return iVar2;

        }

        if (*(int *)(iVar5 + 0x30) == 0) {

          *(undefined4 *)(iVar5 + 0x2c) = 0;

        }

        else {

          pcVar13 = *(char **)(iVar5 + 0xc);

          *pcVar13 = '\0';

          *(int *)(iVar5 + 0x2c) = *(int *)(iVar6 + 0x128) - *(int *)(iVar6 + 0xa4);

          while( true ) {

            iVar4 = (int)*psVar7;

            iVar9 = *(int *)(*(int *)(iVar6 + 0xcc) + iVar4 * 4);

            if (iVar9 < *(int *)(iVar6 + 0x128)) {

              iVar9 = *(int *)(iVar6 + 0x128);

            }

            iVar11 = *(int *)(iVar6 + 0xa8);

            if (iVar11 <= iVar9) break;

            iVar2 = *(int *)(*(int *)(iVar6 + 0xcc) + iVar4 * 4 + 4);

            if (iVar2 <= iVar11) {

              iVar11 = iVar2;

            }

            iVar2 = wma_bitreader_getbits(param_1 + 0x21,1,&uStack_34);

            if (iVar2 < 0) {

              return iVar2;

            }

            pcVar13[*psVar7] = (char)uStack_34;

            if ((uStack_34 & 0xff) == 0) {

              *(int *)(iVar5 + 0x2c) = *(int *)(iVar5 + 0x2c) + (iVar11 - iVar9);

            }

            else {

              *pcVar13 = *pcVar13 + '\x01';

            }

            *psVar7 = *psVar7 + 1;

          }

          if ((int)*psVar7 < *(int *)(iVar6 + 200)) {

            pcVar13[iVar4] = '\0';

          }

        }

        *psVar7 = *(short *)(iVar6 + 0x124);

        sVar3 = (short)*piVar8 + 1;

        *(short *)piVar8 = sVar3;

      } while ((int)sVar3 < (int)(uint)*(ushort *)(iVar6 + 0x26));

    }

    *(undefined1 *)(param_1 + 8) = 7;

    *(short *)piVar8 = 0;

    *psVar7 = 0;

  }

  else if ((char)param_1[8] != '\a') {

    return 0;

  }

  puVar1 = DAT_03089b00;

  if ((int)(uint)*(ushort *)(iVar6 + 0x26) <= (int)(short)*piVar8) {

    return iVar2;

  }

  do {

    iVar5 = *(int *)(iVar6 + 0xd8) + (short)*piVar8 * 0xfc;

    if (*(int *)(iVar5 + 0x30) != 0) {

      pbVar12 = *(byte **)(iVar5 + 0xc);

      piVar10 = *(int **)(iVar5 + 0x14);

      if (*pbVar12 != 0) {

        if (*psVar7 == 0) {

          iVar2 = wma_bitreader_getbits(param_1 + 0x21,7,&uStack_34);

          if (iVar2 < 0) {

            return iVar2;

          }

          *piVar10 = uStack_34 - 0x13;

          *psVar7 = *psVar7 + 1;

        }

        if (*psVar7 < (short)(ushort)*pbVar12) {

          do {

            iVar2 = FREQSetFreq(*puVar1,param_1 + 0x21,auStack_2c,&iStack_30);

            if (iVar2 < 0) {

              return iVar2;

            }

            iVar2 = wma_bitreader_getbits_copy(param_1 + 0x21,auStack_2c[0]);

            if (iVar2 < 0) {

              return iVar2;

            }

            piVar10[*psVar7] = piVar10[*psVar7 + -1] + iStack_30 + -0x12;

            sVar3 = *psVar7;

            *psVar7 = sVar3 + 1;

          } while ((short)(sVar3 + 1) < (short)(ushort)*pbVar12);

        }

      }

    }

    *psVar7 = 0;

    sVar3 = (short)*piVar8 + 1;

    *(short *)piVar8 = sVar3;

    if ((int)(uint)*(ushort *)(iVar6 + 0x26) <= (int)sVar3) {

      return iVar2;

    }

  } while( true );

}
