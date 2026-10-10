/**
 * MscPreventAllowMediumRemoval @ 0x0306b386
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Removing unreachable block (ram,0x0306b408) */



undefined4 MscPreventAllowMediumRemoval(undefined4 param_1,int param_2)



{

  uint uVar1;

  int iVar2;

  uint unaff_r6;

  undefined1 auStack_2c [2];

  byte local_2a;

  

  iVar2 = 0;

  FUN_03039b00(*(undefined1 *)(DAT_0306b594 + 0x216));

LAB_0306b3a2:

  do {

    ReadReservedData(param_2);

    FUN_0303da28(0x28);

    FUN_0303da28(10);

    MscSenseData(0,0x20,300);

    UsbOtgDev_DisConnect(0,0,auStack_2c,4,3);

    MscCSWHandler(0);

    uVar1 = (uint)local_2a;

    if (uVar1 < 0xf) {

      if (iVar2 == 0) {

        return 0;

      }

      if (iVar2 != 1) {

        if (iVar2 == 2) {

joined_r0x0306b3f4:

          if ((unaff_r6 & 1) == 0) {

            return 0;

          }

        }

        return 1;

      }

    }

    else {

      if (iVar2 == 0) {

        param_2 = param_2 + 10;

        iVar2 = 1;

        unaff_r6 = uVar1;

        goto LAB_0306b3a2;

      }

      if (iVar2 != 1) {

        if (iVar2 != 2) {

          return 1;

        }

        if (unaff_r6 <= local_2a) {

          return 0;

        }

        goto joined_r0x0306b3f4;

      }

      if (unaff_r6 <= uVar1) {

        return 0;

      }

    }

    param_2 = param_2 + -0x14;

    iVar2 = 2;

  } while( true );

}
