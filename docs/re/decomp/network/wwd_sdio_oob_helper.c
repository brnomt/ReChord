/**
 * wwd_sdio_oob_helper @ 0x0300c50a
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


ushort wwd_sdio_oob_helper(short *param_1,short *param_2)



{

  short sVar1;

  ushort uVar2;

  int iVar3;

  ushort uVar4;

  uint uVar5;

  

  uVar2 = 0;

  uVar5 = 0;

  sVar1 = *param_2;

  while (sVar1 != 0) {

    iVar3 = FUN_02ff4dd0(sVar1);

    uVar5 = iVar3 + uVar5 & 0xffff;

    if (((int)param_1[2] - (int)*param_1 <= (int)uVar5) || (*param_2 == 0xd || *param_2 == 10))

    break;

    uVar2 = uVar2 + 1;

    param_2 = param_2 + 1;

    sVar1 = *param_2;

  }

  if ((((int)uVar5 < (int)param_1[2] - (int)*param_1) ||

      (uVar5 = parse_iTunes_frame(*param_2), (uVar5 & 3) == 0)) ||

     (uVar5 = parse_iTunes_frame(param_2[-1]), (uVar5 & 3) == 0)) {

    if (*param_2 == 0xd) {

      if (param_2[1] == 10) {

        return uVar2 + 2;

      }

    }

    else if (*param_2 != 10) {

      return uVar2;

    }

    uVar2 = uVar2 + 1;

  }

  else {

    uVar4 = 0;

    if (uVar2 != 0) {

      do {

        param_2 = param_2 + -1;

        iVar3 = parse_iTunes_frame(*param_2);

        if (iVar3 == 0x8000) break;

        uVar4 = uVar4 + 1;

      } while (uVar4 < uVar2);

    }

    if (uVar4 != uVar2) {

      uVar2 = uVar2 - uVar4;

    }

  }

  return uVar2;

}
