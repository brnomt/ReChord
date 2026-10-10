/**
 * buffered_fseek @ 0x030ae61a
 * Tags: fs, buffer
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 buffered_fseek(int param_1,int param_2,uint param_3)



{

  ushort uVar1;

  

  if (param_3 != 1) {

    if (param_3 == 0 || param_3 == 2) {

      (*(code *)*DAT_030ae950)(param_2,param_3 & 0xff,*(undefined4 *)(param_1 + 0x204));

      *(undefined2 *)(param_1 + 0x200) = 0;

      *(undefined2 *)(param_1 + 0x202) = 0;

      return 0;

    }

    return 0xffffffff;

  }

  if (param_2 == 0) {

    return 0;

  }

  if (param_2 < 1) {

    uVar1 = *(ushort *)(param_1 + 0x200);

    if (-(uint)uVar1 != param_2 && (int)(uint)uVar1 <= -param_2) goto LAB_030ae66e;

  }

  else {

    uVar1 = *(ushort *)(param_1 + 0x200);

    if ((int)((uint)*(ushort *)(param_1 + 0x202) - (uint)uVar1) < param_2) {

LAB_030ae66e:

      (*(code *)*DAT_030ae950)

                (param_2 - ((uint)*(ushort *)(param_1 + 0x202) - (uint)uVar1),1,

                 *(undefined4 *)(param_1 + 0x204));

      *(undefined2 *)(param_1 + 0x200) = 0;

      *(undefined2 *)(param_1 + 0x202) = 0;

      return 0;

    }

  }

  *(ushort *)(param_1 + 0x200) = uVar1 + (short)param_2;

  return 0;

}
