/**
 * HifiFileSeek @ 0x0306b8e6
 * Tags: fs
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined1 HifiFileSeek(undefined4 param_1,undefined4 param_2,undefined1 param_3)



{

  undefined4 *puVar1;

  undefined1 *puVar2;

  char *pcVar3;

  int iVar4;

  

  puVar1 = DAT_0306bbb0;

  pcVar3 = (char *)(DAT_0306bbb0 + -10);

  *DAT_0306bbb0 = param_1;

  puVar1[1] = param_2;

  *(undefined1 *)(puVar1 + 2) = param_3;

  *pcVar3 = '\0';

  ipc_post_cmd(0x69,0,2);

  ipc_post_arg(puVar1,0,2);

  if (*pcVar3 == '\0') {

    hifi_busy_delay(1);

    iVar4 = DAT_0306bb90;

    do {

      if (*pcVar3 != '\0') goto LAB_0306b930;

      hifi_busy_delay(1);

      iVar4 = iVar4 + -1;

    } while (iVar4 != 0);

    hifi_debug_printf(s_HifiFileSeek__timeout___0306bbb8);

  }

LAB_0306b930:

  *(undefined4 *)(DAT_0306bbac + (uint)*(byte *)(puVar1 + 2) * 4) = *puVar1;

  puVar2 = DAT_0306bbb4;

  *pcVar3 = '\0';

  return *puVar2;

}
