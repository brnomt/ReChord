/**
 * HifiFileRead_ovl @ 0x03080efc
 * Tags: fs, overlay
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 HifiFileRead_ovl(undefined4 param_1,undefined4 param_2,undefined1 param_3)



{

  undefined4 *puVar1;

  char *pcVar2;

  int iVar3;

  

  puVar1 = DAT_030810f0;

  pcVar2 = (char *)(DAT_030810f0 + -0xd);

  *DAT_030810f0 = param_1;

  puVar1[1] = param_2;

  *(undefined1 *)(puVar1 + 2) = param_3;

  *pcVar2 = '\0';

  FUN_0309101c(0x6b,0,2);

  FUN_03091048(puVar1,0,2);

  if (*(char *)((int)puVar1 + -0x33) == '\0') {

    return 0;

  }

  if (*pcVar2 == '\0') {

    hifi_busy_delay_ovl_0817(1);

    iVar3 = DAT_030810b0;

    do {

      if (*pcVar2 != '\0') goto LAB_03080f50;

      hifi_busy_delay_ovl_0817(1);

      iVar3 = iVar3 + -1;

    } while (iVar3 != 0);

    FUN_03081592(s_HifiFileRead__timeout___030810f4);

  }

LAB_03080f50:

  *(undefined4 *)(DAT_030810cc + (uint)*(byte *)(puVar1 + 2) * 4) = puVar1[1];

  puVar1 = DAT_030810d4;

  *pcVar2 = '\0';

  return *puVar1;

}
