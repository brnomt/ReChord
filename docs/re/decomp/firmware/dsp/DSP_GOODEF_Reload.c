/**
 * DSP_GOODEF_Reload @ 0x0301022c
 * Tags: dsp, ui
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* Reload + apply GOODEF DSP firmware (WOOOOONXBIN / GOODEFGHMP3). Rewrites DSP marker bytes (0x7c),

   re-inits via DSP_GOODEF_Init, re-runs DSP_GOODEF_Process, prints "V333!!!". Called from ~15

   UI/DSP toggle handlers (FUN_03007896 family). Formerly misnamed DICTDECODER_DispatchFile — not

   a codec dispatcher; real DICTDECODER path is DICTDECODER_InitStream @ 0x0304fb44. */



void DSP_GOODEF_Reload(void)



{

  byte bVar1;

  byte bVar2;

  int iVar3;

  byte *pbVar4;

  char cVar5;

  int iVar6;

  undefined4 uVar7;

  int iVar8;

  uint uVar9;

  undefined2 unaff_r4;

  int unaff_r5;

  byte *unaff_r6;

  

  iVar3 = DAT_03010254;

  bVar2 = (byte)unaff_r4;

  *(byte *)(*(int *)(unaff_r5 + 8) + 1) = bVar2;

  **(undefined1 **)(unaff_r5 + 8) = 0x7c;

  pbVar4 = DAT_03010284;

  *(undefined2 *)(unaff_r6 + 8) = unaff_r4;

  *(byte *)(iVar3 + 0x1bb) = bVar2;

  *pbVar4 = bVar2;

  unaff_r6[2] = bVar2;

  unaff_r6[3] = bVar2;

  iVar6 = rom_event_pending(0x159);

  pbVar4 = DAT_030104d0;

  if (iVar6 == 0) {

    cVar5 = DSP_GOODEF_Init(s_WOOOOONXBIN_0300fe1c,6);

    if (cVar5 == '\x01') {

      *pbVar4 = 1;

    }

    else {

      *pbVar4 = bVar2;

    }

  }

  else {

    DSP_GOODEF_Init(s_WOOOOONXBIN_0300fe1c,2);

  }

  FUN_02ffec94(0x3b,7);

  uVar7 = FUN_03020810(*(undefined4 *)(unaff_r5 + 4));

  *(undefined4 *)(pbVar4 + 4) = uVar7;

  iVar8 = FUN_02ffe664(0x159);

  iVar6 = DAT_030104d8;

  if (iVar8 == 0) {

    rom_post_event(0x32);

  }

  else {

    bVar1 = *DAT_030104d4;

    *unaff_r6 = bVar1;

    unaff_r6[4] = bVar1;

    uVar9 = 0;

    do {

      *(undefined1 *)(*(int *)(unaff_r5 + 8) + uVar9) = *(undefined1 *)(iVar6 + uVar9 * 2);

      uVar9 = uVar9 + 1 & 0xff;

    } while (uVar9 < 2);

    if (*(char *)(iVar3 + 0x1bc) == '\0') {

      *(byte *)(*(int *)(unaff_r5 + 8) + *unaff_r6 + 1) = bVar2;

      *(byte *)(*(int *)(unaff_r5 + 8) + (uint)*unaff_r6) = bVar2;

    }

    if (*(char *)(iVar3 + 0x356) != '\0') {

      MediaGotoNextDir(DAT_030104dc);

    }

    if (*(char *)(iVar3 + 0x1bc) == '\0') {

      *(undefined1 *)(*(int *)(unaff_r5 + 8) + (uint)*unaff_r6) = 0x7c;

    }

    DSP_GOODEF_Process();

  }

  FUN_030047cc(s_V333____030104e0);

  return;

}
