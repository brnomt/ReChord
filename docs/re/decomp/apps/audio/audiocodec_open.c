/**
 * audiocodec_open @ 0x03029d0e
 * Tags: audio, hardware
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 audiocodec_open(void)



{

  int iVar1;

  int iVar2;

  uint *puVar3;

  int *piVar4;

  uint uVar5;

  int extraout_r2;

  int iVar6;

  bool bVar7;

  

  iVar2 = FUN_0302c914(0,2);

  iVar6 = DAT_03029f04;

  if (iVar2 == 1) {

    FUN_0302c9e6(*(undefined4 *)(DAT_03029f04 + 0xc));

    FUN_0302c9aa(*(int *)(iVar6 + 0xc) + 8);

    FUN_0302c98c(*(int *)(iVar6 + 0xc) + 0xc);

    FUN_0302c9c8(*(int *)(iVar6 + 0xc) + 0x10);

    FUN_0302ca7c(*(int *)(iVar6 + 0xc) + 0x9c);

    puVar3 = *(uint **)(iVar6 + 0xc);

    uVar5 = *puVar3;

    MediaLib_thunk_GetFiles

              (s_bps____d_fs____d_bitrate____d_ti_03029f2c,puVar3[0x27],puVar3[2],puVar3[3],

               uVar5 / 60000,

               uVar5 / 1000 +

               (uint)((ulonglong)DAT_03029f28 * (ulonglong)(uVar5 / 1000) >> 0x25) * -0x3c,

               uVar5 % 1000);

    piVar4 = *(int **)(iVar6 + 0xc);

    bVar7 = piVar4[3] != 0;

    iVar6 = 0;

    iVar2 = extraout_r2;

    if (bVar7) {

      iVar6 = piVar4[2];

      iVar2 = iVar6;

    }

    iVar1 = piVar4[3];

    if (bVar7 && iVar2 != 0) {

      iVar6 = piVar4[0x27];

      iVar1 = iVar6;

    }

    if ((((((bVar7 && iVar2 != 0) && iVar1 != 0) && -1 < iVar6) && (*piVar4 != 0)) &&

        (((*DAT_03029f00 != 6 && (*DAT_03029f00 != 5)) || (0xf < iVar1)))) &&

       (((uint)piVar4[4] < 3 && (iVar2 != 8000 && iVar2 != 0x5dc00)))) {

      return 0;

    }

  }

  return 0xffffffff;

}
