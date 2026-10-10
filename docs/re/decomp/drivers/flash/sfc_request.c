/**
 * sfc_request @ 0x0308a50e
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void sfc_request(int *param_1,int param_2,undefined4 param_3,int param_4,int param_5)



{

  uint uVar1;

  uint uVar2;

  int iVar3;

  uint uVar4;

  int iVar5;

  int iVar6;

  undefined8 uVar7;

  

  iVar5 = 0;

  iVar3 = 0;

  if (param_5 == 0) {

    uVar1 = 0x7fffffff;

  }

  else {

    uVar7 = va_start(0,param_3,param_5,param_5 >> 0x1f);

    uVar4 = (uint)((ulonglong)uVar7 >> 0x20);

    uVar2 = (uint)uVar7;

    uVar1 = uVar4;

    if (uVar4 == 0) {

      iVar5 = 0x20;

      uVar1 = uVar2;

    }

    for (; (uVar1 & 0xf0000000) == 0; uVar1 = uVar1 << 4) {

      iVar5 = iVar5 + 4;

    }

    for (; (uVar1 & 0x80000000) == 0; uVar1 = uVar1 << 1) {

      iVar5 = iVar5 + 1;

    }

    iVar6 = iVar5 + 1;

    if (iVar6 < 0x21) {

      uVar1 = DelayUs(uVar2,uVar4,0x20 - iVar6);

    }

    else {

      uVar1 = USB_EP_FIFO(uVar2,uVar4,iVar5 + -0x1f);

    }

    iVar5 = *(int *)(*DAT_0308a684 + (uVar1 >> 0x18) * 4);

    uVar1 = iVar5 + (int)((ulonglong)(uVar1 << 8) *

                          (ulonglong)

                          (uint)(*(int *)(*DAT_0308a684 + ((uVar1 >> 0x18) + 1) * 4) - iVar5) >>

                         0x20);

    iVar5 = param_2 - param_4;

    if (((param_2 + iVar6) - param_4 & 1U) != 0) {

      iVar5 = iVar5 + -1;

      uVar1 = (uint)((ulonglong)uVar1 * (ulonglong)DAT_0308a688 >> 0x20);

    }

    iVar3 = 0;

    for (uVar1 = uVar1 >> 1; uVar1 < 0x1fffffff; uVar1 = uVar1 << 2) {

      iVar3 = iVar3 + 2;

    }

    if (uVar1 < 0x3fffffff) {

      uVar1 = uVar1 << 1;

      iVar3 = iVar3 + 1;

    }

    iVar3 = iVar3 + iVar6 + (iVar5 >> 1) + -3;

  }

  *param_1 = iVar3;

  param_1[1] = uVar1;

  return;

}
