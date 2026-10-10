/**
 * phybusif_input @ 0x0300992c
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 phybusif_input(int param_1)



{

  ushort uVar1;

  ushort uVar2;

  int iVar3;

  uint uVar4;

  int unaff_r4;

  int *unaff_r5;

  short *unaff_r6;

  

  if (param_1 == 1) {

    iVar3 = UartDev_Write(0xdf);

    if (iVar3 == 1) {

      iVar3 = *unaff_r5;

      *(undefined2 *)(iVar3 + (uint)*(ushort *)(unaff_r4 + 2) * 2 + 0x14) =

           *(undefined2 *)(iVar3 + 0xc);

      if (*(ushort *)(iVar3 + 0x12) < 0x1d) {

        *(ushort *)(iVar3 + 0x12) = *(ushort *)(iVar3 + 0x12) + 1;

      }

      return 1;

    }

    iVar3 = UartDev_Write(0xe0);

    if (iVar3 == 1) {

      return 1;

    }

  }

  if (*unaff_r6 == 0) {

    iVar3 = UartDev_Write(0xdf);

    if (iVar3 == 1) {

      iVar3 = *unaff_r5;

      uVar1 = *(ushort *)(unaff_r4 + 2);

      uVar2 = *(ushort *)(iVar3 + 0x12);

      if ((uint)uVar2 == (uint)uVar1) {

        uVar4 = 0;

        do {

          if (uVar4 != uVar1) {

            if (*(short *)(iVar3 + uVar4 * 2 + 0x14) != 0) {

              *(short *)(iVar3 + 0x54) = (short)uVar4;

              *(uint *)(iVar3 + 0xc) = (uint)*(ushort *)(iVar3 + uVar4 * 2 + 0x14);

              break;

            }

            *(undefined4 *)(iVar3 + 0xc) = 0x222e;

          }

          uVar4 = uVar4 + 1;

        } while ((int)uVar4 < 0x1e);

        if (uVar2 != 0) {

          *(ushort *)(iVar3 + 0x12) = uVar2 - 1;

        }

        pbuf_free(0xac,1);

      }

      *(undefined2 *)(*unaff_r5 + (uint)*(ushort *)(unaff_r4 + 2) * 2 + 0x14) = 0;

      return 1;

    }

    iVar3 = UartDev_Write(0xe0);

    if (iVar3 == 1) {

      return 1;

    }

  }

  return 0;

}
