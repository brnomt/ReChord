/**
 * audio_stop_helper @ 0x03000eb6
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined2 audio_stop_helper(void)



{

  ushort uVar1;

  undefined2 uVar2;

  uint uVar3;

  int iVar4;

  int iVar5;

  

  uVar3 = (uint)*(ushort *)(DAT_0300161c + 10);

  uVar1 = *DAT_03001620;

  if (uVar3 != 0) {

    iVar5 = DAT_0300161c + (uint)*(ushort *)(DAT_0300161c + 10) * 2;

    iVar4 = DAT_0300161c + -0x664 + (uint)*(ushort *)(DAT_0300161c + 10) * 4;

    if ((*(short *)(DAT_0300161c + uVar3 * 2 + -0x660) == 0x400) && (uVar1 == 3 || uVar1 == 5)) {

      uVar2 = MscHostReadFormatCapacity

                        (*(undefined4 *)(iVar4 + 0xc),

                         (uint)*(ushort *)(iVar5 + 10) + (uint)*(ushort *)(DAT_0300161c + uVar3 * 2)

                         + -1,2);

      return uVar2;

    }

    uVar2 = MscHostReadFormatCapacity

                      (*(undefined4 *)(iVar4 + 0xc),

                       (uint)*(ushort *)(DAT_0300161c + uVar3 * 2) + (uint)*(ushort *)(iVar5 + 10),2

                      );

    return uVar2;

  }

  if (uVar1 == 5) {

    return *(undefined2 *)(DAT_03001624 + 0x36);

  }

  if (uVar1 < 6) {

    if (uVar1 == 1) {

      return *(undefined2 *)(DAT_03001624 + 0x30);

    }

    if (uVar1 == 2) {

      return *(undefined2 *)(DAT_03001624 + 0x38);

    }

    if (uVar1 == 3) {

      return *(undefined2 *)(DAT_03001624 + 0x32);

    }

    if (uVar1 == 4) {

      return *(undefined2 *)(DAT_03001624 + 0x34);

    }

LAB_030012b8:

    uVar2 = 0;

  }

  else {

    uVar2 = *(undefined2 *)(DAT_03001624 + 0x3a);

    if (uVar1 != 8) {

      if (uVar1 == 9) {

        *DAT_03001628 = 0;

        return uVar2;

      }

      if (uVar1 != 0xc) {

        if (uVar1 == 100) {

          return *(undefined2 *)(DAT_03001624 + 0x2e);

        }

        goto LAB_030012b8;

      }

    }

    *DAT_03001628 = 0;

    FUN_02ff6210(s_totalFmfile____d_0300162c,uVar2);

  }

  return uVar2;

}
