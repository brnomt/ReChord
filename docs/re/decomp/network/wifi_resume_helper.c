/**
 * wifi_resume_helper @ 0x0300cb04
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 wifi_resume_helper(int param_1,uint param_2)



{

  byte bVar1;

  byte bVar2;

  uint uVar3;

  byte bVar4;

  int iVar5;

  

  bVar2 = 0;

  if (param_2 < 0x14) {

    bVar4 = 1;

  }

  else {

    bVar4 = 5;

    param_2 = param_2 - 4 & 0xffff;

  }

  uVar3 = 0;

  if (param_2 != 0) {

    do {

      iVar5 = 0;

      if (bVar4 <= bVar2) {

        return 1;

      }

      bVar1 = *(byte *)(param_1 + uVar3);

      if ((bVar1 & 0x80) != 0) {

        if ((bVar1 & 0xc0) == 0x80) {

          return 0;

        }

        if ((bVar1 & 0xe0) == 0xc0) {

          if ((*(byte *)(param_1 + uVar3 + 1) & 0xc0) != 0x80) {

            return 0;

          }

          bVar2 = bVar2 + 1;

          iVar5 = 1;

        }

        else if ((bVar1 & 0xf0) == 0xe0) {

          if ((*(byte *)(param_1 + uVar3 + 1) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(param_1 + uVar3 + 2) & 0xc0) != 0x80) {

            return 0;

          }

          bVar2 = bVar2 + 1;

          iVar5 = 2;

        }

        else if ((bVar1 & 0xf8) == 0xf0) {

          iVar5 = param_1 + uVar3;

          if ((*(byte *)(iVar5 + 1) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 2) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 3) & 0xc0) != 0x80) {

            return 0;

          }

          bVar2 = bVar2 + 1;

          iVar5 = 3;

        }

        else if ((bVar1 & 0xfc) == 0xf8) {

          iVar5 = param_1 + uVar3;

          if ((*(byte *)(iVar5 + 1) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 2) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 3) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 4) & 0xc0) != 0x80) {

            return 0;

          }

          bVar2 = bVar2 + 1;

          iVar5 = 4;

        }

        else if ((bVar1 & 0xfe) == 0xfc) {

          iVar5 = param_1 + uVar3;

          if ((*(byte *)(iVar5 + 1) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 2) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 3) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 4) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 5) & 0xc0) != 0x80) {

            return 0;

          }

          bVar2 = bVar2 + 1;

          iVar5 = 5;

        }

        else if (bVar1 == 0xfe) {

          iVar5 = param_1 + uVar3;

          if ((*(byte *)(iVar5 + 1) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 2) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 3) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 4) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 5) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 6) & 0xc0) != 0x80) {

            return 0;

          }

          bVar2 = bVar2 + 1;

          iVar5 = 6;

        }

        else {

          if (bVar1 != 0xff) {

            return 0;

          }

          iVar5 = param_1 + uVar3;

          if ((*(byte *)(iVar5 + 1) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 2) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 3) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 4) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 5) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 6) & 0xc0) != 0x80) {

            return 0;

          }

          if ((*(byte *)(iVar5 + 7) & 0xc0) != 0x80) {

            return 0;

          }

          bVar2 = bVar2 + 1;

          iVar5 = 7;

        }

      }

      uVar3 = uVar3 + iVar5 + 1 & 0xffff;

    } while (uVar3 < param_2);

  }

  return 0;

}
