/**
 * wma_frame_helper_a @ 0x0309d8c2
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined1 wma_frame_helper_a(undefined4 param_1,undefined4 param_2,undefined1 param_3)



{

  char *pcVar1;

  undefined4 *puVar2;

  undefined1 *puVar3;

  int iVar4;

  

  puVar2 = DAT_0309db70;

  pcVar1 = DAT_0309db48;

  *DAT_0309db70 = param_1;

  puVar2[1] = param_2;

  *(undefined1 *)(puVar2 + 2) = param_3;

  *pcVar1 = '\0';

  FUN_030a5604(0x69,0,2);

  FUN_030a5630(puVar2,0,2);

  if (*pcVar1 == '\0') {

    hifi_busy_delay_ovl_09e3(1);

    iVar4 = DAT_0309db4c;

    do {

      if (*pcVar1 != '\0') goto LAB_0309d908;

      hifi_busy_delay_ovl_09e3(1);

      iVar4 = iVar4 + -1;

    } while (iVar4 != 0);

    hifi_debug_printf_ovl_09e0(s_HifiFileSeek__timeout___0309db78);

  }

LAB_0309d908:

  *(undefined4 *)(DAT_0309db68 + (uint)*(byte *)(puVar2 + 2) * 4) = *puVar2;

  puVar3 = DAT_0309db74;

  *pcVar1 = '\0';

  return *puVar3;

}
