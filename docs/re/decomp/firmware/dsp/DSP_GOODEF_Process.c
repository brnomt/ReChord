/**
 * DSP_GOODEF_Process @ 0x0300fb0e
 * Tags: dsp
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint DSP_GOODEF_Process(void)



{

  int iVar1;

  byte *pbVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  uint uVar6;

  

  pbVar2 = DAT_0300fe0c;

  DAT_0300fe0c[1] = 0;

  if (*pbVar2 == 0) {

    return 1;

  }

  if (*DAT_0300fe04 == 1) {

    iVar4 = DSP_GOODEF_Init(s_WOOOOONXBIN_0300fe1c,0);

    iVar3 = DAT_0300fe10;

    iVar1 = DAT_0300fe08;

    if (iVar4 != 0) {

      if (*(char *)(DAT_0300fe10 + 0x1bc) == '\0') {

        *(undefined1 *)(*(int *)(DAT_0300fe08 + 8) + *pbVar2 + 1) = 0;

        *(undefined1 *)(*(int *)(iVar1 + 8) + (uint)*pbVar2) = 0;

      }

      iVar5 = ed25519_test(iVar4,*(undefined4 *)(iVar1 + 8),0);

      if (iVar5 != 0) {

        pbVar2[1] = 1;

      }

    }

    if (*(char *)(iVar3 + 0x1bc) == '\0') {

      *(undefined1 *)(*(int *)(iVar1 + 8) + (uint)*pbVar2) = 0x7c;

    }

    uVar6 = FUN_0300f8a2(iVar4);

    return uVar6;

  }

  return (uint)*DAT_0300fe04;

}
