/**
 * bt_status_check @ 0x0300e9c2
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void bt_status_check(void)



{

  undefined4 *puVar1;

  undefined1 *puVar2;

  int iVar3;

  undefined1 *puVar4;

  int iVar5;

  uint uVar6;

  

  FUN_02ffc8a4(0x1c6);

  FUN_02ffc8e6(0xdf);

  FUN_02ffc8e6(0xe0);

  FUN_02ffc8e6(0x12f);

  puVar1 = DAT_0300eb6c;

  puVar2 = DAT_0300eb64;

  DAT_0300eb64[3] = 0;

  iVar5 = FUN_02feb930(*puVar1,DAT_0300eb68);

  if ((iVar5 == 1) && (*DAT_0300eb70 == 0)) {

    puVar2[3] = 1;

    FUN_03019cec(0xed,0);

  }

  FUN_02ff6036();

  FUN_02ff9b52();

  iVar5 = DAT_0300eb74;

  *(undefined2 *)(puVar2 + 4) = 0;

  uVar6 = 0;

  do {

    *(undefined1 *)(iVar5 + uVar6) = 0;

    iVar3 = DAT_0300eb78;

    uVar6 = uVar6 + 1 & 0xffff;

  } while (uVar6 < 0x80);

  *(undefined4 *)(puVar2 + 0xc) = 0;

  puVar1 = DAT_0300eb58;

  if (*(char *)(iVar3 + 0x34b) == '\0') {

    BrowserUITask_MediaFindFirstFile();

    *(undefined4 *)(puVar2 + 8) = *puVar1;

    FUN_02ffc8a4(0x34);

    FUN_02ffc8e6(0x1c7);

  }

  else {

    iVar5 = FUN_02ffc904(0x1c7);

    puVar4 = DAT_0300eb7c;

    if (iVar5 == 0) {

      *puVar2 = 1;

      puVar2[1] = 1;

      puVar2[2] = 1;

      *puVar4 = 0;

    }

    else {

      *(undefined2 *)(puVar2 + 4) = 4;

      *puVar4 = 0;

      FUN_02ffc8a4(0x1c6);

      *puVar2 = 0;

    }

  }

  FUN_02ffc8a4(0x1c6);

  puVar4 = DAT_0300eb80;

  *(undefined4 *)(puVar2 + 8) = *puVar1;

  *puVar4 = *(undefined1 *)(iVar3 + 0x34a);

  return;

}
