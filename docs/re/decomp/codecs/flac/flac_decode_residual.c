/**
 * flac_decode_residual @ 0x030def58
 * Tags: codec, flac, decoder
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 flac_decode_residual(int param_1)



{

  bool bVar1;

  byte bVar2;

  undefined4 *puVar3;

  int *piVar4;

  undefined4 *puVar5;

  int iVar6;

  undefined4 *puVar7;

  int *piVar8;

  uint uVar9;

  int iVar10;

  char *pcVar11;

  int iVar12;

  uint uVar13;

  int iVar14;

  int iVar15;

  uint uVar16;

  int local_78 [16];

  undefined4 local_38;

  undefined4 uStack_34;

  

  puVar5 = DAT_030df0f4;

  uVar9 = (*(code *)*DAT_030df0f8)(*DAT_030df0f4);

  uVar13 = ((uint)((longlong)param_1 * (longlong)DAT_030df0f0[8]) >> 3 |

           (int)((ulonglong)((longlong)param_1 * (longlong)DAT_030df0f0[8]) >> 0x20) << 0x1d) +

           *DAT_030df0fc + *DAT_030df100;

  uVar16 = param_1 * *DAT_030df0f0;

  if ((uVar13 <= uVar9) &&

     (iVar10 = (*(code *)*DAT_030df104)(uVar13,0,*puVar5), puVar7 = DAT_030df10c,

     iVar12 = DAT_030df108, iVar10 == 0)) {

    iVar14 = 0;

    (*(code *)*DAT_030df10c)(DAT_030df108,0x4000,*puVar5);

    piVar4 = DAT_030df0e8;

    puVar3 = DAT_030df0e4;

    iVar10 = 0;

    do {

      do {

        iVar6 = DAT_030df108;

        local_38 = *puVar3;

        uStack_34 = puVar3[1];

        local_78[0] = *piVar4;

        local_78[1] = piVar4[1];

        local_78[2] = piVar4[2];

        local_78[3] = piVar4[3];

        pcVar11 = (char *)(iVar12 + iVar14);

        local_78[4] = piVar4[4];

        local_78[5] = piVar4[5];

        local_78[6] = piVar4[6];

        local_78[7] = piVar4[7];

        local_78[8] = piVar4[8];

        local_78[9] = piVar4[9];

        local_78[10] = piVar4[10];

        local_78[0xb] = piVar4[0xb];

        iVar15 = iVar10;

        if ((*pcVar11 == -1) && (pcVar11[1] == *DAT_030df0ec)) {

          bVar2 = pcVar11[3];

          uVar9 = (byte)pcVar11[2] & 0xf;

          if ((uVar9 != 0xf) && ((10 < uVar9 - 1 || (*DAT_030df0f0 == local_78[uVar9])))) {

            uVar9 = (bVar2 & 0xf) >> 1;

            if ((((bVar2 & 1) == 0) && (bVar2 >> 4 < 0xd)) &&

               ((uVar9 == 0 ||

                ((uVar9 != 3 && uVar9 != 7 &&

                 (DAT_030df0f0[2] == (uint)*(byte *)((int)&local_38 + uVar9)))))))

            goto LAB_030df080;

          }

        }

        iVar14 = iVar14 + 1;

      } while (iVar14 < 0x4000);

      iVar14 = 0;

      iVar12 = (*(code *)*puVar7)(DAT_030df108,0x4000,*puVar5);

    } while ((iVar12 == 0x4000) &&

            (iVar15 = iVar10 + 1, bVar1 = iVar10 < 0x800, iVar10 = iVar15, iVar12 = iVar6, bVar1));

LAB_030df080:

    iVar12 = (*(code *)*DAT_030df104)(uVar13 + iVar14 + iVar15 * 0x4000,0,*puVar5);

    if (iVar12 == 0) {

      (*(code *)*puVar7)(DAT_030df108,0x4000,*puVar5);

      piVar4 = DAT_030df0f0;

      iVar12 = DAT_030df0f0[0xb];

      if ((int)(-(uint)(uVar16 < (uint)DAT_030df0f0[10]) - iVar12) < 0 !=

          (SBORROW4(0,iVar12) != SBORROW4(-iVar12,(uint)(uVar16 < (uint)DAT_030df0f0[10])))) {

        DAT_030df0fc[1] = 1;

      }

      piVar8 = DAT_030df110;

      *DAT_030df110 = DAT_030df108;

      piVar8[1] = 0x4000;

      piVar4[10] = uVar16;

      piVar4[0xb] = 0;

      return 0;

    }

  }

  return 0xffffffff;

}
