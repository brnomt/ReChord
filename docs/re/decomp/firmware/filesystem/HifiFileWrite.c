/**
 * HifiFileWrite @ 0x0306ba1e
 * Tags: fs
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined1 HifiFileWrite(undefined4 param_1)



{

  char *pcVar1;

  undefined1 *puVar2;

  int iVar3;

  

  pcVar1 = DAT_0306bb8c;

  *DAT_0306bb8c = '\0';

  ipc_post_cmd(0x73,0,2);

  ipc_post_arg(param_1,0,2);

  if (*pcVar1 == '\0') {

    hifi_busy_delay(1);

    iVar3 = DAT_0306bb90;

    do {

      if (*pcVar1 != '\0') goto LAB_0306ba5c;

      hifi_busy_delay(1);

      iVar3 = iVar3 + -1;

    } while (iVar3 != 0);

    hifi_debug_printf(s_HifiFileClose__timeout___0306bc0c);

  }

LAB_0306ba5c:

  puVar2 = DAT_0306bbb4;

  *pcVar1 = '\0';

  return *puVar2;

}
