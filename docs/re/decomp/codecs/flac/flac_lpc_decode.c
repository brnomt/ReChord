/**
 * flac_lpc_decode @ 0x030e0c54
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined8

flac_lpc_decode(int *param_1,int param_2,int param_3,uint param_4,uint param_5,int param_6)



{

  uint uVar1;

  char extraout_r2;

  int iVar2;

  int iVar3;

  int iVar4;

  undefined4 uVar5;

  uint uVar6;

  undefined4 uVar7;

  bool bVar8;

  undefined1 uVar9;

  longlong lVar10;

  undefined8 uVar11;

  longlong lVar12;

  

  if (param_3 == 0 && param_4 == 0) {

    if (param_6 == 1) {

      param_5 = ~param_5;

    }

    else {

      param_5 = 0;

    }

    *param_1 = (int)&DAT_030e0d9c;

    param_1[1] = param_5;

    param_1[2] = 1;

    param_1[3] = param_6;

  }

  else {

    iVar4 = (int)(((param_4 >> 0x14) - 0x3ff) * 0x4d10) >> 0x10;

    do {

      while( true ) {

        if (param_6 == 1) {

          uVar1 = -param_5;

        }

        else {

          uVar1 = (iVar4 - param_5) + 1;

        }

        lVar10 = (ulonglong)DAT_030e0da0 << 0x20;

        lVar12 = (ulonglong)DAT_030e0da4 << 0x20;

        uVar6 = uVar1;

        if ((int)uVar1 < 0) {

          uVar6 = -uVar1;

          lVar12 = (ulonglong)DAT_030e0da4 << 0x20;

          lVar10 = (ulonglong)DAT_030e0da0 << 0x20;

        }

        while( true ) {

          uVar5 = (undefined4)((ulonglong)lVar10 >> 0x20);

          uVar7 = (undefined4)((ulonglong)lVar12 >> 0x20);

          if (uVar6 == 0) break;

          if ((uVar6 & 1) != 0) {

            lVar12 = FUN_030ddc26((int)lVar12,uVar7,(int)lVar10,uVar5);

          }

          lVar10 = FUN_030ddc26((int)lVar10,uVar5);

          uVar6 = (int)uVar6 >> 1;

        }

        uVar9 = 1;

        if ((int)uVar1 < 0) {

          uVar11 = FUN_030ddc26();

        }

        else {

          uVar11 = flac_lpc_compute(param_3,param_4,(int)lVar12,uVar7);

        }

        bVar8 = true;

        FUN_030dde18();

        if ((bool)uVar9 && !bVar8) {

          FUN_030ddad8((int)uVar11,(int)((ulonglong)uVar11 >> 0x20),0,DAT_030e0dac);

          lVar12 = FUN_030ddde8();

        }

        else {

          lVar12 = -1;

        }

        iVar3 = 0x10;

        while( true ) {

          if ((lVar12 == 0) || (iVar3 < 0)) break;

          lVar12 = FUN_030dda76((int)lVar12,(int)((ulonglong)lVar12 >> 0x20),10,0);

          *(char *)(param_2 + iVar3) = extraout_r2 + '0';

          iVar3 = iVar3 + -1;

        }

        iVar2 = 0x11 - (iVar3 + 1);

        if (param_6 != 1) break;

        if (lVar12 == 0) {

          iVar4 = (iVar2 - param_5) + -1;

          goto LAB_030e0d8c;

        }

        param_5 = 0x11;

        param_6 = 0;

      }

      bVar8 = true;

      if ((lVar12 == 0) && (iVar2 <= (int)param_5)) {

        if (iVar2 < (int)param_5) {

          bVar8 = false;

          iVar4 = iVar4 + -1;

        }

      }

      else {

        bVar8 = false;

        iVar4 = iVar4 + 1;

      }

    } while (!bVar8);

LAB_030e0d8c:

    param_1[2] = iVar2;

    param_1[3] = param_6;

    *param_1 = iVar3 + 1 + param_2;

    param_1[1] = iVar4;

  }

  return CONCAT44(param_2,param_1);

}
