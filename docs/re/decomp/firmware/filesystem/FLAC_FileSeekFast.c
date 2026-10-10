/**
 * FLAC_FileSeekFast @ 0x030ffcac
 * Tags: fs, flac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 FLAC_FileSeekFast(undefined4 param_1,undefined4 param_2,undefined4 param_3)



{

  undefined4 *puVar1;

  char *pcVar2;

  int iVar3;

  

  pcVar2 = DAT_030ffd68;

  puVar1 = DAT_030ffd64;

  DAT_030ffd64[2] = param_3;

  *puVar1 = param_1;

  puVar1[1] = param_2;

  *pcVar2 = '\0';

  FUN_031001f0(0xf,0,1);

  FUN_0310021c(DAT_030ffd64,0,1);

  if (*pcVar2 == '\0') {

    hifi_busy_delay_ovl_0fd1(1);

    iVar3 = DAT_030ffd6c;

    do {

      if (*pcVar2 != '\0') goto LAB_030ffcf0;

      hifi_busy_delay_ovl_0fd1(1);

      iVar3 = iVar3 + -1;

    } while (iVar3 != 0);

    FUN_030fce44(s_FLAC_FileSeekFast__timeout___030ffd70);

  }

LAB_030ffcf0:

  *pcVar2 = '\0';

  return 0;

}
