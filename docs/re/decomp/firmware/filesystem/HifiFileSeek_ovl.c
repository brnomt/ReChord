/**
 * HifiFileSeek_ovl @ 0x03080e96
 * Tags: fs, overlay
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined1 HifiFileSeek_ovl(undefined4 param_1,undefined4 param_2,undefined1 param_3)



{

  undefined4 *puVar1;

  undefined1 *puVar2;

  char *pcVar3;

  int iVar4;

  

  puVar1 = DAT_030810d0;

  pcVar3 = (char *)(DAT_030810d0 + -10);

  *DAT_030810d0 = param_1;

  puVar1[1] = param_2;

  *(undefined1 *)(puVar1 + 2) = param_3;

  *pcVar3 = '\0';

  FUN_0309101c(0x69,0,2);

  FUN_03091048(puVar1,0,2);

  if (*pcVar3 == '\0') {

    hifi_busy_delay_ovl_0817(1);

    iVar4 = DAT_030810b0;

    do {

      if (*pcVar3 != '\0') goto LAB_03080ee0;

      hifi_busy_delay_ovl_0817(1);

      iVar4 = iVar4 + -1;

    } while (iVar4 != 0);

    FUN_03081592(s_HifiFileSeek__timeout___030810d8);

  }

LAB_03080ee0:

  *(undefined4 *)(DAT_030810cc + (uint)*(byte *)(puVar1 + 2) * 4) = *puVar1;

  puVar2 = DAT_030810d4;

  *pcVar3 = '\0';

  return *puVar2;

}
