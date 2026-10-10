/**
 * USBResetPhy @ 0x03089bec
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



void USBResetPhy(int *param_1)



{

  longlong lVar1;

  longlong lVar2;

  uint uVar3;

  undefined4 *puVar4;

  int iVar5;

  int iVar6;

  int iVar7;

  uint uVar8;

  uint *puVar9;

  uint *puVar10;

  uint uVar11;

  uint uVar12;

  int iVar13;

  uint uVar14;

  int iVar15;

  undefined1 *puVar16;

  uint auStack_100 [52];

  undefined4 uStack_30;

  undefined4 uStack_2c;

  

  auStack_100[1] = 0;

  if (*(short *)(*param_1 + 0x26) == 0) {

    return;

  }

  do {

    iVar5 = *(int *)(*param_1 + 0xd8);

    if (*(int *)(iVar5 + auStack_100[1] * 0xfc + 0x30) != 0) {

      iVar15 = *(int *)(iVar5 + auStack_100[1] * 0xfc + 0x10);

      auStack_100[0] = *(uint *)(iVar5 + auStack_100[1] * 0xfc + 0x40);

      iVar13 = *(int *)(*param_1 + 0x124);

      puVar16 = *(undefined1 **)(iVar5 + auStack_100[1] * 0xfc + 0xc);

      uVar12 = 0;

      while( true ) {

        iVar6 = *param_1;

        iVar5 = *(int *)(*(int *)(iVar6 + 0xcc) + iVar13 * 4);

        if (iVar5 < *(int *)(iVar6 + 0x128)) {

          iVar5 = *(int *)(iVar6 + 0x128);

        }

        iVar7 = *(int *)(iVar6 + 0xa8);

        if (iVar7 <= iVar5) break;

        iVar6 = *(int *)(*(int *)(iVar6 + 0xcc) + iVar13 * 4 + 4);

        if (iVar6 <= iVar7) {

          iVar7 = iVar6;

        }

        if (puVar16[iVar13] == '\x01') {

          uVar14 = iVar7 - iVar5;

          puVar9 = (uint *)(auStack_100[0] + iVar5 * 4);

          lVar2 = 0;

          lVar1 = 0;

          if (0 < (int)uVar14) {

            puVar10 = puVar9 + -1;

            if ((uVar14 & 1) != 0) {

              lVar1 = (ulonglong)*puVar9 * (ulonglong)*puVar9;

              puVar10 = puVar9;

            }

            lVar2 = 0;

            for (iVar5 = (int)uVar14 >> 1; iVar5 != 0; iVar5 = iVar5 + -1) {

              lVar1 = (ulonglong)puVar10[1] * (ulonglong)puVar10[1] + lVar1;

              puVar10 = puVar10 + 2;

              lVar2 = (ulonglong)*puVar10 * (ulonglong)*puVar10 + lVar2;

            }

            lVar2 = lVar1 + lVar2;

          }

          uVar8 = (uint)((ulonglong)lVar2 >> 0x20);

          uVar3 = (uint)lVar2;

          iVar5 = 0;

          uVar11 = uVar8;

          if ((uVar8 == 0) && (iVar5 = 0x20, uVar11 = uVar3, uVar3 == 0)) {

            iVar5 = 0;

            iVar6 = 0;

          }

          else {

            for (; (uVar11 & 0xf0000000) == 0; uVar11 = uVar11 << 3) {

              iVar5 = iVar5 + 3;

            }

            for (; (uVar11 & 0xc0000000) == 0; uVar11 = uVar11 << 1) {

              iVar5 = iVar5 + 1;

            }

            iVar6 = iVar5 + 10;

            if (iVar5 < 0x21) {

              iVar5 = DelayUs(uVar3,uVar8,0x20 - iVar5);

            }

            else {

              iVar5 = USB_EP_FIFO(uVar3,uVar8,iVar5 + -0x20);

            }

          }

          iVar7 = 0;

          for (uVar14 = iVar5 / (int)uVar14; uVar14 < 0x1fffffff; uVar14 = uVar14 << 2) {

            iVar7 = iVar7 + 2;

          }

          if (uVar14 < 0x3fffffff) {

            uVar14 = uVar14 << 1;

            iVar7 = iVar7 + 1;

          }

          auStack_100[uVar12 * 2 + 2] = iVar7 + iVar6;

          auStack_100[uVar12 * 2 + 3] = uVar14;

          uVar12 = uVar12 + 1 & 0xff;

        }

        iVar13 = iVar13 + 1;

      }

      iVar5 = 0;

      if (0 < (int)(uVar12 - 1)) {

        do {

          auStack_100[0] = auStack_100[uVar12 * 2 + 1];

          sfc_request(&uStack_30,auStack_100[iVar5 * 2 + 2],auStack_100[iVar5 * 2 + 3],

                      auStack_100[uVar12 * 2]);

          puVar4 = (undefined4 *)(iVar15 + iVar5 * 8);

          *puVar4 = uStack_30;

          puVar4[1] = uStack_2c;

          iVar5 = iVar5 + 1;

        } while (iVar5 < (int)(uVar12 - 1));

      }

      if (uVar12 != 0) {

        iVar15 = iVar15 + uVar12 * 8;

        *(undefined4 *)(iVar15 + -4) = 0x40000000;

        *(undefined4 *)(iVar15 + -8) = 0x1e;

      }

      *puVar16 = (char)uVar12;

    }

    auStack_100[1] = auStack_100[1] + 1;

  } while ((int)auStack_100[1] < (int)(uint)*(ushort *)(*param_1 + 0x26));

  return;

}
