/**
 * gui_widget_delete @ 0x030645c8
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 gui_widget_delete(int param_1)



{

  ushort uVar1;

  int iVar2;

  int iVar3;

  uint uVar4;

  

  GuiTask_DeleteWidget();

  if (param_1 != 0) {

    (**(code **)(DAT_03064884 + 0x1c))(1);

  }

  iVar3 = MusicPlay_DeleteGuiHandle(param_1);

  iVar2 = DAT_0306487c;

  if (iVar3 != 0) {

    FUN_0303a488(0x8f);

    iVar3 = *(int *)(iVar2 + 0x14);

    *(undefined2 *)(iVar3 + 10) = 3;

    *(undefined2 *)(iVar3 + 0x54) = 0;

    uVar4 = (uint)*(ushort *)(iVar3 + 0x14);

    *(uint *)(iVar3 + 0xc) = uVar4;

    uVar1 = *(ushort *)(iVar2 + 6);

    if ((uVar4 < uVar1) || (*(ushort *)(iVar2 + 4) < uVar4)) {

      *(uint *)(iVar3 + 0xc) = (uint)uVar1;

      *(ushort *)(iVar3 + 0x14) = uVar1;

    }

    return 0;

  }

  *(undefined1 *)(*(int *)(DAT_0306487c + 0x14) + 0x10) = 0;

  return 1;

}
