/**
 * FLAC_FileGetSeekInfo @ 0x030ffcfe
 * Tags: fs, flac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 FLAC_FileGetSeekInfo(undefined4 *param_1,undefined4 *param_2,undefined4 param_3)



{

  char *pcVar1;

  int *piVar2;

  undefined4 *puVar3;

  int iVar4;

  

  pcVar1 = DAT_030ffd68;

  *DAT_030ffd68 = '\0';

  FUN_031001f0(0x11,0,1);

  FUN_0310021c(param_3,0,1);

  if (*pcVar1 == '\0') {

    hifi_busy_delay_ovl_0fd1(1);

    iVar4 = DAT_030ffd6c;

    do {

      if (*pcVar1 != '\0') goto LAB_030ffd46;

      hifi_busy_delay_ovl_0fd1(1);

      iVar4 = iVar4 + -1;

    } while (iVar4 != 0);

    FUN_030fce44(s_FLAC_FileGetSeekInfo__timeout___030ffd94);

  }

LAB_030ffd46:

  piVar2 = DAT_030ffd90;

  *pcVar1 = '\0';

  puVar3 = (undefined4 *)*piVar2;

  *param_1 = *puVar3;

  *param_2 = puVar3[1];

  return 0;

}
