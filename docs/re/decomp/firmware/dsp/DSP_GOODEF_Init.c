/**
 * DSP_GOODEF_Init @ 0x0300f7dc
 * Tags: dsp
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int * DSP_GOODEF_Init(undefined4 param_1,uint param_2)



{

  int *piVar1;

  short sVar2;

  int iVar3;

  uint uVar4;

  undefined4 uStack_40;

  undefined4 uStack_3c;

  undefined4 uStack_38;

  undefined4 uStack_34;

  int iStack_30;

  int iStack_2c;

  int iStack_28;

  undefined4 uStack_24;

  undefined4 uStack_20;

  

  piVar1 = DAT_0300f9d0;

  if (param_2 < 3) {

    iVar3 = FUN_02fef2aa(&DAT_0300f9e4,s_GOODEFGHMP3_0300f9d8,&DAT_0300f9d4);

    if (iVar3 != -1) {

      uVar4 = 0;

      if (-1 < iVar3) {

        do {

          FUN_02fef470((int)(char)uVar4);

          uVar4 = uVar4 + 1 & 0xffff;

        } while ((int)uVar4 <= iVar3);

      }

      FUN_02feef6c();

      FUN_02ff2e4a(&DAT_0300f9e4,s_GOODEFGHMP3_0300f9d8);

    }

    if (param_2 == 2) {

      return (int *)0x0;

    }

  }

  FUN_02feb510(DAT_0300f9d0,0x24);

  *piVar1 = -1;

  iVar3 = FUN_02fef2aa(&DAT_0300f9e4,param_1,&DAT_0300f9d4);

  *piVar1 = iVar3;

  if (iVar3 != -1) {

    if (param_2 == 6) {

      FUN_02fef470((int)(char)iVar3);

      return (int *)0x1;

    }

    uStack_40 = 0;

    uStack_3c = 0;

    uStack_38 = 0;

    uStack_34 = 0;

    iStack_30 = 0;

    iStack_2c = 0;

    iStack_28 = 0;

    uStack_24 = 0;

    uStack_20 = 0;

    sVar2 = FUN_02fef58a(&uStack_40,0x24,(int)(char)*piVar1);

    if (sVar2 == 0x24) {

      piVar1[1] = iStack_30;

      piVar1[2] = iStack_2c;

      piVar1[3] = iStack_28;

      return piVar1;

    }

  }

  FUN_030047cc(s_error_________0300f9e8);

  return (int *)0xffffffff;

}
