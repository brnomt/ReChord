/**
 * AudioFileMhSeek2 @ 0x0306dec4
 * Tags: fs, audio
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void AudioFileMhSeek2(undefined4 param_1,undefined4 param_2,int param_3)



{

  int iVar1;

  int iVar2;

  

  iVar1 = DAT_0306e014;

  if (param_3 == 0) {

    AudioFileSeek2(param_1,param_2,0);

    return;

  }

  if (*(int *)(DAT_0306e014 + 0x10) == 1) {

    hifi_busy_delay(1);

    iVar2 = DAT_0306e018;

    do {

      if (*(int *)(iVar1 + 0x10) != 1) goto LAB_0306defc;

      hifi_busy_delay(1);

      iVar2 = iVar2 + -1;

    } while (iVar2 != 0);

    hifi_debug_printf(s_AudioFileMhSeek2__timeout___0306e01c);

  }

LAB_0306defc:

  HifiFileSeek(param_1,param_2,(int)(char)param_3);

  return;

}
