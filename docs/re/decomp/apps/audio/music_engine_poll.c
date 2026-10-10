/**
 * music_engine_poll @ 0x0302b5a4
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void music_engine_poll(void)



{

  int iVar1;

  int *piVar2;

  undefined4 uVar3;

  int iVar4;

  uint uVar5;

  int *piVar6;

  

  piVar2 = DAT_0302b9a0;

  iVar1 = DAT_0302b99c;

  piVar6 = (int *)(DAT_0302b99c + 0x1ac);

  if ((*piVar6 != 2) ||

     (*(uint *)(*(int *)(DAT_0302b99c + 0xc) + 4) < *(uint *)(DAT_0302b99c + 0x1b4)))

  goto LAB_0302b5e4;

  uVar5 = *DAT_0302b9a0 - *(int *)(DAT_0302b99c + 0x34);

  if (*(byte *)(DAT_0302b9a4 + 0x1a2) == 0) {

    if (*DAT_0302b9a8 != 0) {

      if ((*(byte *)(DAT_0302b9a4 + 0x1a3) + 1) * 100 < uVar5) {

        shared_frame_epilogue_c6b4(0x108);

      }

      goto LAB_0302b5e4;

    }

LAB_0302b63a:

    audio_pause();

    *piVar6 = 2;

    *(int *)(iVar1 + 0x34) = *piVar2;

  }

  else {

    if (*(uint *)(DAT_0302b99c + 0x30) < (uint)*(byte *)(DAT_0302b9a4 + 0x1a2)) {

      if (*DAT_0302b9a8 == 0) goto LAB_0302b63a;

      if (uVar5 < 0x65) goto LAB_0302b5e4;

      uVar3 = 0x108;

    }

    else {

      *piVar6 = 0;

      uVar3 = 0x41;

    }

    shared_frame_epilogue_c6b4(uVar3);

  }

LAB_0302b5e4:

  iVar4 = FUN_0300c6d0(0x108);

  if (iVar4 == 0) {

    return;

  }

  *(int *)(iVar1 + 0x30) = *(int *)(iVar1 + 0x30) + 1;

  shared_frame_epilogue_c6b4(0x107);

  *piVar6 = 1;

  wifi_ap_auto_connect();

  return;

}
