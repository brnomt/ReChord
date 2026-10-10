/**
 * HifiFileRead @ 0x0306b94c
 * Tags: fs
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 HifiFileRead(undefined4 param_1,undefined4 param_2,undefined1 param_3)



{

  undefined4 *puVar1;

  char *pcVar2;

  int iVar3;

  

  puVar1 = DAT_0306bbd0;

  pcVar2 = (char *)(DAT_0306bbd0 + -0xd);

  *DAT_0306bbd0 = param_1;

  puVar1[1] = param_2;

  *(undefined1 *)(puVar1 + 2) = param_3;

  *pcVar2 = '\0';

  ipc_post_cmd(0x6b,0,2);

  ipc_post_arg(puVar1,0,2);

  if (*(char *)((int)puVar1 + -0x33) == '\0') {

    return 0;

  }

  if (*pcVar2 == '\0') {

    hifi_busy_delay(1);

    iVar3 = DAT_0306bb90;

    do {

      if (*pcVar2 != '\0') goto LAB_0306b9a0;

      hifi_busy_delay(1);

      iVar3 = iVar3 + -1;

    } while (iVar3 != 0);

    hifi_debug_printf(s_HifiFileRead__timeout___0306bbd4);

  }

LAB_0306b9a0:

  *(undefined4 *)(DAT_0306bbac + (uint)*(byte *)(puVar1 + 2) * 4) = puVar1[1];

  puVar1 = DAT_0306bbb4;

  *pcVar2 = '\0';

  return *puVar1;

}
