/**
 * TransCodeFromUTF8ToUnicode @ 0x0302cab0
 * Tags: codec, encoding
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_0302cab0(byte *param_1)



{

  int iVar1;

  int iVar2;

  int iVar3;

  int iVar4;

  bool bVar5;

  bool bVar6;

  int local_10;

  

  local_10 = 0xac44;

  FUN_0302c9aa(&local_10);

  iVar3 = DAT_0302ce50;

  iVar4 = local_10 + -0xac44;

  if (local_10 != 0xac44) {

    if (local_10 < 0xac45) {

      if (local_10 != 16000) {

        if (local_10 < 0x3e81) {

          bVar5 = local_10 == 8000;

          if (!bVar5) {

            iVar4 = local_10 + -0x2b11;

            bVar5 = iVar4 == 0;

          }

          if (!bVar5) {

            bVar5 = iVar4 == 0x3cf;

          }

          if (bVar5) {

            local_10 = 0x2b11;

          }

          goto LAB_0302cb54;

        }

        bVar5 = local_10 == 0x5622;

        iVar4 = 0;

        if (!bVar5) {

          iVar4 = local_10 + -24000;

          bVar5 = iVar4 == 0;

        }

        if (!bVar5) {

          if (iVar4 != 8000) goto LAB_0302cb54;

          goto LAB_0302cb52;

        }

      }

      local_10 = 0x5622;

      goto LAB_0302cb54;

    }

    if (iVar4 != 0xcabc) {

      if (iVar4 < 0xcabd) {

        iVar4 = local_10 + -48000;

        bVar5 = iVar4 != 0;

        bVar6 = iVar4 != 16000;

        if (bVar5 && bVar6) {

          iVar4 = local_10 + -0x15380;

        }

        if ((bVar5 && bVar6) && iVar4 != 0x508) goto LAB_0302cb54;

      }

      else {

        bVar5 = local_10 == 0x1f400;

        if (!bVar5) {

          iVar4 = local_10 + -0x2b110;

          bVar5 = iVar4 == 0;

        }

        if (!bVar5) {

          bVar5 = iVar4 == 0x3cf0;

        }

        if (!bVar5) goto LAB_0302cb54;

      }

    }

  }

LAB_0302cb52:

  local_10 = 0xac44;

LAB_0302cb54:

  switch((uint)*param_1) {

  case 1:

    FUN_0302d9da(local_10,DAT_0302ce4c,1);

    break;

  case 2:

    FUN_0302d9da(local_10,DAT_0302ce54,1);

    break;

  case 3:

  case 4:

  case 5:

  case 8:

    FUN_0302d9da(local_10,DAT_0302ce4c + (uint)*param_1 * 0x3e + -0x3e,1);

    break;

  case 6:

    FUN_0302d9da(local_10,DAT_0302ce4c + 0x3e,1);

    break;

  case 7:

    iVar4 = 0;

    do {

      iVar1 = iVar4 * 2;

      iVar2 = iVar4 * 10;

      iVar4 = iVar4 + 1;

      *(short *)(iVar3 + iVar2 + 0xe) = *(short *)(param_1 + iVar1 + 3) + 0xc;

    } while (iVar4 < 5);

    FUN_0302d9da(local_10,DAT_0302ce50,1);

  }

  return 0;

}




