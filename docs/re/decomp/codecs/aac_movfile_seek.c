/**
 * aac_movfile_seek @ 0x030ae90c
 * Tags: codec, aac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 aac_movfile_seek(undefined4 param_1,undefined4 *param_2,undefined4 *param_3)



{

  int iVar1;

  int iVar2;

  int iVar3;

  uint uVar4;

  uint uVar5;

  byte local_34 [4];

  char local_30 [4];

  byte local_2c [8];

  byte local_24 [8];

  

  iVar1 = DAT_030ae95c;

  local_30[0] = '\0';

  *(undefined4 *)(DAT_030ae95c + 0x50) = 0;

  buffered_fseek(param_1,4,1);

  *(int *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + 4;

  iVar2 = buffered_fread(local_30,1,1,param_1);

  if (iVar2 == 0) {

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1a2);

    return 0xfffffffb;

  }

  *(int *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + 1;

  if (local_30[0] == '\x03') {

    uVar4 = 0;

    do {

      iVar2 = buffered_fread(local_34,1,1,param_1);

      if (iVar2 == 0) goto LAB_030ae9c6;

      uVar4 = uVar4 + 1 & 0xff;

    } while ((local_34[0] & 0x80) != 0 && uVar4 < 4);

    *(uint *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + uVar4;

LAB_030ae9c6:

    buffered_fseek(param_1,3,1);

    iVar2 = *(int *)(iVar1 + 0x50) + 3;

  }

  else {

    buffered_fseek(param_1,2,1);

    iVar2 = *(int *)(iVar1 + 0x50) + 2;

  }

  *(int *)(iVar1 + 0x50) = iVar2;

  iVar2 = buffered_fread(local_30,1,1,param_1);

  if (iVar2 == 0) {

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1b4);

    return 0xfffffffb;

  }

  *(int *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + 1;

  if (local_30[0] != '\x04') {

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1ba);

    return 0xfffffff8;

  }

  uVar5 = 0;

  uVar4 = uVar5;

  do {

    iVar2 = buffered_fread(local_34,1,1,param_1);

    if (iVar2 == 0) goto LAB_030aea58;

    uVar4 = uVar4 + 1 & 0xff;

    uVar5 = local_34[0] & 0x7f | uVar5 << 7;

  } while ((local_34[0] & 0x80) != 0 && uVar4 < 4);

  *(uint *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + uVar4;

  if ((int)uVar5 < 0xd) {

LAB_030aea58:

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1c0);

    return 0xfffffff8;

  }

  buffered_fseek(param_1,0xd,1);

  *(int *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + 0xd;

  iVar2 = buffered_fread(local_30,1,1,param_1);

  if (iVar2 == 0) {

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1c7);

    return 0xfffffffb;

  }

  *(int *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + 1;

  if (local_30[0] != '\x05') {

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1cd);

    return 0xfffffff8;

  }

  uVar4 = 0;

  do {

    iVar2 = buffered_fread(local_34,1,1,param_1);

    if (iVar2 == 0) goto LAB_030aeaf2;

    uVar4 = uVar4 + 1 & 0xff;

  } while ((local_34[0] & 0x80) != 0 && uVar4 < 4);

  *(uint *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + uVar4;

LAB_030aeaf2:

  iVar2 = buffered_fread(local_24,1,1,param_1);

  if (iVar2 == 0) {

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1d4);

    return 0xfffffffb;

  }

  *(int *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + 1;

  iVar3 = buffered_fread(local_2c,1,1,param_1);

  iVar2 = DAT_030aed94;

  if (iVar3 == 0) {

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__030aed7c,s_______Common_Codec_Audio_AAC_lib_030aed4c,0x1da);

    return 0xfffffffb;

  }

  *(int *)(iVar1 + 0x50) = *(int *)(iVar1 + 0x50) + 1;

  *param_2 = *(undefined4 *)(iVar2 + ((local_24[0] & 7) << 1 | (uint)(local_2c[0] >> 7)) * 4);

  *param_3 = *(undefined4 *)(iVar2 + 0x30 + ((local_2c[0] & 0x7f) >> 3) * 4);

  buffered_fseek(param_1,-*(int *)(iVar1 + 0x50),1);

  return 0;

}
