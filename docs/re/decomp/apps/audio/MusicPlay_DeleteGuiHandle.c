/**
 * MusicPlay_DeleteGuiHandle @ 0x03064508
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 MusicPlay_DeleteGuiHandle(void)



{

  ushort uVar1;

  uint uVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  

  uVar2 = DAT_03064880;

  iVar4 = DAT_0306487c;

  while( true ) {

    do {

      iVar3 = FATDelClusChain(1,0);

      GuiTask_DeleteWidget();

      if (*(uint *)(*(int *)(iVar4 + 0x14) + 0xc) == (uint)*(ushort *)(iVar4 + 4)) {

        if (iVar3 == 2) {

          iVar4 = *(int *)(iVar4 + 0x14);

          uVar1 = *(ushort *)(iVar4 + 0x12);

          if (uVar1 < 0x1e) {

            iVar3 = iVar4 + (uint)uVar1 * 2;

            if (*(ushort *)(iVar3 + 0x12) / 10 !=

                (uint)((ulonglong)uVar2 * (ulonglong)*(uint *)(iVar4 + 0xc) >> 0x23)) {

              *(ushort *)(iVar4 + 0x12) = uVar1 + 1;

              *(short *)(iVar3 + 0x14) = (short)*(uint *)(iVar4 + 0xc);

              *(short *)(iVar4 + 0x54) = *(short *)(iVar4 + 0x54) + 1;

              FUN_0304ce2c();

            }

          }

        }

        return 1;

      }

      iVar5 = Http_Close();

      if (iVar5 == 1) {

        return 1;

      }

    } while (iVar3 == 0);

    if (iVar3 == 1) {

      return 0;

    }

    if (iVar3 != 2) break;

    iVar3 = *(int *)(iVar4 + 0x14);

    uVar1 = *(ushort *)(iVar3 + 0x12);

    if (0x1d < uVar1) {

      return 1;

    }

    iVar5 = iVar3 + (uint)uVar1 * 2;

    if (*(ushort *)(iVar5 + 0x12) / 10 !=

        (uint)((ulonglong)uVar2 * (ulonglong)*(uint *)(iVar3 + 0xc) >> 0x23)) {

      *(ushort *)(iVar3 + 0x12) = uVar1 + 1;

      *(short *)(iVar5 + 0x14) = (short)*(uint *)(iVar3 + 0xc);

      *(short *)(iVar3 + 0x54) = *(short *)(iVar3 + 0x54) + 1;

      FUN_0304ce2c();

    }

  }

  return 1;

}
