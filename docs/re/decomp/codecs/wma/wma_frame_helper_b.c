/**
 * wma_frame_helper_b @ 0x0309d924
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 wma_frame_helper_b(undefined4 param_1,undefined4 param_2,undefined1 param_3)



{

  char *pcVar1;

  undefined4 *puVar2;

  int iVar3;

  

  puVar2 = DAT_0309db90;

  pcVar1 = DAT_0309db48;

  *DAT_0309db90 = param_1;

  puVar2[1] = param_2;

  *(undefined1 *)(puVar2 + 2) = param_3;

  *pcVar1 = '\0';

  FUN_030a5604(0x6b,0,2);

  FUN_030a5630(puVar2,0,2);

  if (pcVar1[1] == '\0') {

    return 0;

  }

  if (*pcVar1 == '\0') {

    hifi_busy_delay_ovl_09e3(1);

    iVar3 = DAT_0309db4c;

    do {

      if (*pcVar1 != '\0') goto LAB_0309d974;

      hifi_busy_delay_ovl_09e3(1);

      iVar3 = iVar3 + -1;

    } while (iVar3 != 0);

    hifi_debug_printf_ovl_09e0(s_HifiFileRead__timeout___0309db94);

  }

LAB_0309d974:

  *(undefined4 *)(DAT_0309db68 + (uint)*(byte *)(puVar2 + 2) * 4) = puVar2[1];

  puVar2 = DAT_0309db74;

  *pcVar1 = '\0';

  return *puVar2;

}
