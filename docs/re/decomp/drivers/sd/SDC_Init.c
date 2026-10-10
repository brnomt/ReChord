/**
 * SDC_Init @ 0x03004612
 * SDK-matched (structural+combined, documented in MULTI-VERSION-PLAN §10).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 SDC_Init(int *param_1)



{

  ushort uVar1;

  short sVar2;

  int iVar3;

  int iVar4;

  uint uVar5;

  

  iVar3 = DAT_03004704;

  do {

    iVar4 = FUN_03004382();

    if (iVar4 == 0) {

      uVar1 = *(ushort *)(iVar3 + -0x128);

      uVar5 = (uint)uVar1;

      iVar4 = iVar3 + uVar5;

      if (*(char *)(iVar4 + 6) != ':') {

        *param_1 = DAT_03004708 + (uint)*(byte *)(iVar4 + 4) * 10 +

                   (uint)*(byte *)(iVar4 + 5) +

                   ((*(byte *)(iVar4 + 1) - 0x30) * 10 + -0x30 + (uint)*(byte *)(uVar5 + iVar3 + 2))

                   * 0x3c;

        sVar2 = 5;

LAB_030046ce:

        *(ushort *)(iVar3 + -0x128) = uVar1 + sVar2;

        return 0;

      }

      if ((*(byte *)(iVar4 + 7) - 0x30 < 10) && (*(byte *)(iVar4 + 8) - 0x30 < 10)) {

        *param_1 = ((uint)*(byte *)(iVar4 + 5) + (*(byte *)(iVar4 + 4) - 0x30) * 10) * 0x3c +

                   (uint)*(byte *)(iVar4 + 7) * 10 +

                   (uint)*(byte *)(iVar4 + 8) +

                   ((*(byte *)(iVar4 + 1) - 0x30) * 10 + -0x30 + (uint)*(byte *)(uVar5 + iVar3 + 2))

                   * 0xe10 + -0xd50;

        sVar2 = 8;

        goto LAB_030046ce;

      }

      iVar4 = MsgDevSuspend();

    }

    if (iVar4 == -1) {

      return 0xffffffff;

    }

  } while( true );

}
