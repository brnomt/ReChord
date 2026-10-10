/**
 * BroMemSelKeyMenu_Handler @ 0x03013280
 * Tags: ui, menu
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void BroMemSelKeyMenu_Handler(void)



{

  int iVar1;

  undefined4 *puVar2;

  int iVar3;

  char cVar4;

  int iVar5;

  uint local_38;

  undefined2 local_34;

  undefined2 local_32;

  undefined2 local_30;

  

  func_0x03007118(s_BroMemSelKeyMenu_03013670);

  iVar3 = DAT_0301368c;

  puVar2 = DAT_03013688;

  iVar1 = DAT_03013684;

  if ((ushort)*(byte *)(DAT_03013684 + 0xe) !=

      *(ushort *)(DAT_0301368c + (uint)*(ushort *)((int)DAT_03013688 + 0x1ee) * 2)) {

    iVar5 = FUN_02ff0020(*DAT_03013694,DAT_03013690);

    if (iVar5 == 1) {

      FUN_02feff78(DAT_03013694,DAT_03013690);

    }

    *(undefined1 *)(iVar1 + 0x6c) = 0;

    cVar4 = *(char *)(iVar3 + (uint)*(ushort *)((int)puVar2 + 0x1ee) * 2);

    *(char *)(iVar1 + 0xe) = cVar4;

    if (cVar4 == '\x04') {

      FUN_02fff4f4();

      FUN_03001776(1);

    }

    cVar4 = FUN_02ff0852(*(undefined1 *)(iVar1 + 0xe));

    if (cVar4 != '\0') {

      local_38 = 0x370038;

      local_34 = 3;

      local_30 = 1;

      local_32 = 0;

      FUN_02fefe90(DAT_0301369c,DAT_03013698,&local_38);

      FUN_02fff4f6();

      *(undefined1 *)(iVar1 + 0xe) = 2;

      FUN_02ff0852();

    }

  }

  iVar3 = DAT_030136a0;

  *(ushort *)(DAT_030136a0 + 10) = (ushort)*(byte *)(iVar1 + 0xe);

  *(undefined2 *)(iVar3 + 0xc) = 0;

  USBMSCDevShellTestRead(1);

  BroadMessage_Parse(*(undefined4 *)(puVar2[0x7d] + 4),

                     puVar2 + (uint)*(ushort *)(puVar2 + 0x73) * 0xe + 3,*(undefined1 *)(iVar3 + 8))

  ;

  local_38 = (uint)*(byte *)(iVar3 + 8);

  FUN_030122a2(*(undefined4 *)(puVar2[0x7d] + 4),*puVar2,

               puVar2 + (uint)*(ushort *)(puVar2 + 0x73) * 0xe + 3,

               *(undefined2 *)((int)puVar2 + (uint)*(ushort *)(puVar2 + 0x73) * 2 + 0x1ce));

  MediaLib_GetTotalFiles(0x14d);

  MediaLib_GetTotalFiles(0x15c);

  return;

}
