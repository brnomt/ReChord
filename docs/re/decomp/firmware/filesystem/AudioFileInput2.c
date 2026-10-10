/**
 * AudioFileInput2 @ 0x0306dac4
 * Tags: fs, audio
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 AudioFileInput2(undefined4 param_1)



{

  int *piVar1;

  undefined4 uVar2;

  int iVar3;

  

  piVar1 = DAT_0306de04;

  if (DAT_0306de04[4] == 1) {

    hifi_busy_delay(1);

    iVar3 = DAT_0306de0c;

    do {

      if (piVar1[4] != 1) goto LAB_0306daec;

      hifi_busy_delay(1);

      iVar3 = iVar3 + -1;

    } while (iVar3 != 0);

    hifi_debug_printf(s_AudioFileInput2__timeout___0306de18);

  }

LAB_0306daec:

  *(undefined1 *)(DAT_0306de08 + 1) = 0;

  piVar1[4] = 1;

  uVar2 = HifiFileRead(piVar1[7] * piVar1[3] + *piVar1,piVar1[3],param_1);

  *(undefined4 *)(DAT_0306de10 + piVar1[7] * 4) = uVar2;

  return 0;

}
