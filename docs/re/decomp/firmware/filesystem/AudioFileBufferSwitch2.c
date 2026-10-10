/**
 * AudioFileBufferSwitch2 @ 0x0306da52
 * Tags: fs, audio
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void AudioFileBufferSwitch2(undefined4 param_1)



{

  int *piVar1;

  int iVar2;

  undefined4 uVar3;

  int iVar4;

  

  piVar1 = DAT_0306de04;

  if (DAT_0306de04[4] == 1) {

    hifi_busy_delay(1);

    iVar4 = DAT_0306de0c;

    do {

      if (piVar1[4] != 1) goto LAB_0306da7a;

      hifi_busy_delay(1);

      iVar4 = iVar4 + -1;

    } while (iVar4 != 0);

    hifi_debug_printf(DAT_0306de14);

  }

LAB_0306da7a:

  piVar1[5] = 0;

  iVar4 = DAT_0306de10;

  iVar2 = piVar1[6];

  *(undefined4 *)(DAT_0306de10 + iVar2 * 4) = 0;

  piVar1[7] = iVar2;

  iVar2 = 1 - iVar2;

  piVar1[6] = iVar2;

  if (*(int *)(iVar4 + iVar2 * 4) == 0) {

    *(undefined1 *)(DAT_0306de08 + 1) = 1;

    uVar3 = HifiFileRead(iVar2 * piVar1[3] + *piVar1,piVar1[3],param_1);

    *(undefined4 *)(iVar4 + piVar1[6] * 4) = uVar3;

  }

  event_set(0x115);

  return;

}
