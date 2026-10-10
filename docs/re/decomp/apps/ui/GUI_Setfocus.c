/**
 * GUI_Setfocus @ 0x0308cb4c
 * SDK-matched (structural+combined, documented in MULTI-VERSION-PLAN §10).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



undefined4 GUI_Setfocus(uint *param_1)



{

  byte bVar1;

  byte bVar2;

  byte bVar3;

  byte bVar4;

  byte bVar5;

  byte bVar6;

  int iVar7;

  uint uVar8;

  int iStack_28;

  short sStack_24;

  short sStack_22;

  byte bStack_20;

  byte bStack_1f;

  byte bStack_1e;

  byte bStack_1d;

  byte bStack_1c;

  byte bStack_1b;

  byte bStack_1a;

  byte bStack_19;

  byte *local_18;

  

  local_18 = (byte *)0x0;

  if (param_1 == (uint *)0x0) {

    return 2;

  }

  iVar7 = wma_input_cache_read(param_1,&local_18,*param_1,param_1[1]);

  if (iVar7 == 0x1e) {

    uVar8 = *param_1;

    *param_1 = uVar8 + 0x1e;

    param_1[1] = param_1[1] + (uint)(0xffffffe1 < uVar8);

    iStack_28 = (uint)*local_18 + (uint)local_18[3] * 0x1000000 +

                (uint)local_18[2] * 0x10000 + (uint)local_18[1] * 0x100;

    sStack_24 = (ushort)local_18[4] + (ushort)local_18[5] * 0x100;

    sStack_22 = (ushort)local_18[6] + (ushort)local_18[7] * 0x100;

    bStack_20 = local_18[8];

    bStack_1f = local_18[9];

    bStack_1e = local_18[10];

    bStack_1d = local_18[0xb];

    bStack_1c = local_18[0xc];

    bStack_1b = local_18[0xd];

    bStack_1a = local_18[0xe];

    bStack_19 = local_18[0xf];

    bVar4 = local_18[0x10];

    bVar1 = local_18[0x13];

    bVar2 = local_18[0x11];

    bVar3 = local_18[0x12];

    bVar5 = local_18[0x1c];

    bVar6 = local_18[0x1d];

    local_18 = local_18 + 0x1e;

    iVar7 = wma_memcmp(*DAT_0308cd2c,&iStack_28,0x10);

    if ((iVar7 == 0 && bVar5 == 1) && (bVar6 == 2)) {

      param_1[4] = (uint)bVar4 + (uint)bVar1 * 0x1000000 +

                   (uint)bVar3 * 0x10000 + (uint)bVar2 * 0x100;

      return 0;

    }

    return 4;

  }

  return 3;

}
